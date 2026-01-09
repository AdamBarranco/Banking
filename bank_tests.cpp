#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include "Bank.h"

namespace fs = std::filesystem;

using Banking::Bank;

// Helper to clean up test data
class TestFixture {
public:
    std::string testDataDir = "test_data";
    
    TestFixture() {
        cleanup();
    }
    
    ~TestFixture() {
        cleanup();
    }
    
    void cleanup() {
        if (fs::exists(testDataDir)) {
            fs::remove_all(testDataDir);
        }
    }
};

TEST_CASE("Bank") {
    TestFixture fixture;
    Bank bank(fixture.testDataDir);
    
    SECTION("Admin can login with correct credentials") {
        std::string sessionId = bank.login("00000000", "9999");
        CHECK(!sessionId.empty());
        CHECK(bank.isAdmin(sessionId));
    }
    
    SECTION("Admin login fails with wrong pin") {
        std::string sessionId = bank.login("00000000", "1234");
        CHECK(sessionId.empty());
    }
    
    SECTION("Admin can create account") {
        std::string adminSession = bank.login("00000000", "9999");
        REQUIRE(!adminSession.empty());
        
        std::string result = bank.createAccount(adminSession, "12345678", "1234");
        CHECK(result == "ok");
    }
    
    SECTION("Cannot create account without admin session") {
        std::string result = bank.createAccount("invalid_session", "12345678", "1234");
        CHECK(result == "error: unauthorized");
    }
    
    SECTION("Customer can login after account creation") {
        std::string adminSession = bank.login("00000000", "9999");
        REQUIRE(!adminSession.empty());
        
        bank.createAccount(adminSession, "12345678", "1234");
        
        std::string customerSession = bank.login("12345678", "1234");
        CHECK(!customerSession.empty());
        CHECK(!bank.isAdmin(customerSession));
    }
    
    SECTION("Customer can deposit money") {
        std::string adminSession = bank.login("00000000", "9999");
        bank.createAccount(adminSession, "12345678", "1234");
        
        std::string customerSession = bank.login("12345678", "1234");
        
        std::string result = bank.deposit(customerSession, 100.00);
        CHECK(result == "ok");
    }
    
    SECTION("Customer can debit money after deposit") {
        std::string adminSession = bank.login("00000000", "9999");
        bank.createAccount(adminSession, "12345678", "1234");
        
        std::string customerSession = bank.login("12345678", "1234");
        bank.deposit(customerSession, 100.00);
        
        std::string result = bank.debit(customerSession, 50.00);
        CHECK(result == "ok");
    }
    
    SECTION("Customer cannot debit more than balance") {
        std::string adminSession = bank.login("00000000", "9999");
        bank.createAccount(adminSession, "12345678", "1234");
        
        std::string customerSession = bank.login("12345678", "1234");
        bank.deposit(customerSession, 100.00);
        
        std::string result = bank.debit(customerSession, 200.00);
        CHECK(result == "error: insufficient funds");
    }
    
    SECTION("Customer can get statement") {
        std::string adminSession = bank.login("00000000", "9999");
        bank.createAccount(adminSession, "12345678", "1234");
        
        std::string customerSession = bank.login("12345678", "1234");
        bank.deposit(customerSession, 100.00);
        bank.debit(customerSession, 30.00);
        
        std::string statement = bank.getStatement(customerSession, 10);
        CHECK(statement.find("DEPOSIT") != std::string::npos);
        CHECK(statement.find("DEBIT") != std::string::npos);
    }
    
    SECTION("Logout invalidates session") {
        std::string adminSession = bank.login("00000000", "9999");
        CHECK(bank.logout(adminSession));
        
        // Session should now be invalid
        std::string account = bank.getAccountFromSession(adminSession);
        CHECK(account.empty());
    }
    
    SECTION("Admin can list accounts and see total holdings") {
        std::string adminSession = bank.login("00000000", "9999");
        bank.createAccount(adminSession, "12345678", "1234");
        bank.createAccount(adminSession, "87654321", "4321");
        
        std::string customer1Session = bank.login("12345678", "1234");
        bank.deposit(customer1Session, 100.00);
        
        std::string customer2Session = bank.login("87654321", "4321");
        bank.deposit(customer2Session, 200.00);
        
        std::string report = bank.listAccounts(adminSession);
        CHECK(report.find("Total Accounts: 2") != std::string::npos);
        CHECK(report.find("Total Holdings: 300.00") != std::string::npos);
    }
    
    SECTION("Admin statement shows bank status") {
        std::string adminSession = bank.login("00000000", "9999");
        bank.createAccount(adminSession, "12345678", "1234");
        
        std::string customerSession = bank.login("12345678", "1234");
        bank.deposit(customerSession, 500.00);
        
        std::string statement = bank.getStatement(adminSession, 10);
        CHECK(statement.find("Bank Status Report") != std::string::npos);
    }
    
    SECTION("Account number must be 8 digits") {
        std::string adminSession = bank.login("00000000", "9999");
        
        std::string result = bank.createAccount(adminSession, "123", "1234");
        CHECK(result == "error: account number must be 8 digits");
    }
    
    SECTION("Pin must be 4 digits") {
        std::string adminSession = bank.login("00000000", "9999");
        
        std::string result = bank.createAccount(adminSession, "12345678", "12");
        CHECK(result == "error: pin must be 4 digits");
    }
    
    SECTION("Cannot create duplicate account") {
        std::string adminSession = bank.login("00000000", "9999");
        bank.createAccount(adminSession, "12345678", "1234");
        
        std::string result = bank.createAccount(adminSession, "12345678", "5678");
        CHECK(result == "error: account already exists");
    }
    
    SECTION("Account number must contain only digits") {
        std::string adminSession = bank.login("00000000", "9999");
        
        std::string result = bank.createAccount(adminSession, "abcd1234", "1234");
        CHECK(result == "error: account number must contain only digits");
    }
    
    SECTION("Pin must contain only digits") {
        std::string adminSession = bank.login("00000000", "9999");
        
        std::string result = bank.createAccount(adminSession, "12345678", "ab12");
        CHECK(result == "error: pin must contain only digits");
    }
    
    SECTION("Customer can transfer money to another account") {
        std::string adminSession = bank.login("00000000", "9999");
        bank.createAccount(adminSession, "12345678", "1234");
        bank.createAccount(adminSession, "87654321", "4321");
        
        std::string customer1Session = bank.login("12345678", "1234");
        bank.deposit(customer1Session, 500.00);
        
        std::string result = bank.transfer(customer1Session, "87654321", 200.00);
        CHECK(result == "ok");
        
        // Check statements show transfer
        std::string statement1 = bank.getStatement(customer1Session, 10);
        CHECK(statement1.find("TRANSFER_OUT") != std::string::npos);
        
        std::string customer2Session = bank.login("87654321", "4321");
        std::string statement2 = bank.getStatement(customer2Session, 10);
        CHECK(statement2.find("TRANSFER_IN") != std::string::npos);
    }
    
    SECTION("Cannot transfer more than balance") {
        std::string adminSession = bank.login("00000000", "9999");
        bank.createAccount(adminSession, "12345678", "1234");
        bank.createAccount(adminSession, "87654321", "4321");
        
        std::string customerSession = bank.login("12345678", "1234");
        bank.deposit(customerSession, 100.00);
        
        std::string result = bank.transfer(customerSession, "87654321", 200.00);
        CHECK(result == "error: insufficient funds");
    }
    
    SECTION("Cannot transfer to non-existent account") {
        std::string adminSession = bank.login("00000000", "9999");
        bank.createAccount(adminSession, "12345678", "1234");
        
        std::string customerSession = bank.login("12345678", "1234");
        bank.deposit(customerSession, 100.00);
        
        std::string result = bank.transfer(customerSession, "99999999", 50.00);
        CHECK(result == "error: destination account does not exist");
    }
    
    SECTION("Cannot transfer to same account") {
        std::string adminSession = bank.login("00000000", "9999");
        bank.createAccount(adminSession, "12345678", "1234");
        
        std::string customerSession = bank.login("12345678", "1234");
        bank.deposit(customerSession, 100.00);
        
        std::string result = bank.transfer(customerSession, "12345678", 50.00);
        CHECK(result == "error: cannot transfer to same account");
    }
    
    SECTION("Cannot transfer negative amount") {
        std::string adminSession = bank.login("00000000", "9999");
        bank.createAccount(adminSession, "12345678", "1234");
        bank.createAccount(adminSession, "87654321", "4321");
        
        std::string customerSession = bank.login("12345678", "1234");
        bank.deposit(customerSession, 100.00);
        
        std::string result = bank.transfer(customerSession, "87654321", -50.00);
        CHECK(result == "error: amount must be positive");
    }
}