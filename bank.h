#ifndef BANK_H
#define BANK_H

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <random>
#include <filesystem>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <cctype>

namespace fs = std::filesystem;

// Constants
constexpr const char* ADMIN_ACCOUNT = "00000000";
constexpr const char* ADMIN_PIN = "9999";
constexpr const char* DATA_DIR = "data";
constexpr const char* ACCOUNTS_DIR = "data/accounts";
constexpr const char* SESSIONS_DIR = "data/sessions";

// Transaction types
enum class TransactionType {
    DEPOSIT,
    DEBIT,
    ACCOUNT_CREATED
};

struct Transaction {
    std::string timestamp;
    TransactionType type;
    double amount;
    double balance;
};

class Bank {
private:
    std::string dataDir;

    std::string getAccountDir(const std::string& accountNumber) const {
        return dataDir + "/accounts/" + accountNumber;
    }

    std::string getStatementPath(const std::string& accountNumber) const {
        return getAccountDir(accountNumber) + "/statement.csv";
    }

    std::string getPinPath(const std::string& accountNumber) const {
        return getAccountDir(accountNumber) + "/pin.txt";
    }

    std::string getSessionPath(const std::string& sessionId) const {
        return dataDir + "/sessions/" + sessionId + ".txt";
    }

    std::string generateSessionId() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);
        static const char* hex = "0123456789abcdef";

        std::string sessionId;
        for (int i = 0; i < 32; ++i) {
            sessionId += hex[dis(gen)];
        }
        return sessionId;
    }

    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    void ensureDirectories() {
        fs::create_directories(dataDir + "/accounts");
        fs::create_directories(dataDir + "/sessions");
    }

    bool accountExists(const std::string& accountNumber) const {
        return fs::exists(getAccountDir(accountNumber));
    }

    std::string getStoredPin(const std::string& accountNumber) const {
        std::ifstream file(getPinPath(accountNumber));
        if (!file.is_open()) return "";
        std::string pin;
        std::getline(file, pin);
        return pin;
    }

    double getBalance(const std::string& accountNumber) const {
        std::ifstream file(getStatementPath(accountNumber));
        if (!file.is_open()) return 0.0;

        std::string line;
        double balance = 0.0;
        while (std::getline(file, line)) {
            // CSV format: timestamp,type,amount,balance
            std::istringstream ss(line);
            std::string timestamp, type, amountStr, balanceStr;
            std::getline(ss, timestamp, ',');
            std::getline(ss, type, ',');
            std::getline(ss, amountStr, ',');
            std::getline(ss, balanceStr, ',');
            if (!balanceStr.empty()) {
                try {
                    balance = std::stod(balanceStr);
                } catch (...) {}
            }
        }
        return balance;
    }

    void appendTransaction(const std::string& accountNumber, TransactionType type, double amount) {
        double currentBalance = getBalance(accountNumber);
        double newBalance = currentBalance;
        
        if (type == TransactionType::DEPOSIT) {
            newBalance += amount;
        } else if (type == TransactionType::DEBIT) {
            newBalance -= amount;
        }

        std::string typeStr;
        switch (type) {
            case TransactionType::DEPOSIT: typeStr = "DEPOSIT"; break;
            case TransactionType::DEBIT: typeStr = "DEBIT"; break;
            case TransactionType::ACCOUNT_CREATED: typeStr = "ACCOUNT_CREATED"; break;
        }

        std::ofstream file(getStatementPath(accountNumber), std::ios::app);
        file << getCurrentTimestamp() << "," << typeStr << "," 
             << std::fixed << std::setprecision(2) << amount << "," << newBalance << "\n";
    }

public:
    explicit Bank(const std::string& dataDirectory = DATA_DIR) : dataDir(dataDirectory) {
        ensureDirectories();
        
        // Create admin account if it doesn't exist
        if (!accountExists(ADMIN_ACCOUNT)) {
            fs::create_directories(getAccountDir(ADMIN_ACCOUNT));
            std::ofstream pinFile(getPinPath(ADMIN_ACCOUNT));
            pinFile << ADMIN_PIN;
            std::ofstream statementFile(getStatementPath(ADMIN_ACCOUNT));
            // Admin account statement will show bank status
        }
    }

    // Login: returns session_id or empty string on failure
    std::string login(const std::string& accountNumber, const std::string& pin) {
        if (!accountExists(accountNumber)) {
            return "";
        }
        
        std::string storedPin = getStoredPin(accountNumber);
        if (storedPin != pin) {
            return "";
        }

        std::string sessionId = generateSessionId();
        std::ofstream sessionFile(getSessionPath(sessionId));
        sessionFile << accountNumber;
        
        return sessionId;
    }

    // Logout: invalidates session
    bool logout(const std::string& sessionId) {
        std::string sessionPath = getSessionPath(sessionId);
        if (fs::exists(sessionPath)) {
            fs::remove(sessionPath);
            return true;
        }
        return false;
    }

    // Get account number from session
    std::string getAccountFromSession(const std::string& sessionId) const {
        std::ifstream file(getSessionPath(sessionId));
        if (!file.is_open()) return "";
        std::string accountNumber;
        std::getline(file, accountNumber);
        return accountNumber;
    }

    // Check if session belongs to admin
    bool isAdmin(const std::string& sessionId) const {
        return getAccountFromSession(sessionId) == ADMIN_ACCOUNT;
    }

    // Create account (admin only)
    std::string createAccount(const std::string& sessionId, const std::string& accountNumber, const std::string& pin) {
        if (!isAdmin(sessionId)) {
            return "error: unauthorized";
        }
        
        if (accountNumber.length() != 8) {
            return "error: account number must be 8 digits";
        }
        
        // Verify account number contains only digits
        for (char c : accountNumber) {
            if (!std::isdigit(static_cast<unsigned char>(c))) {
                return "error: account number must contain only digits";
            }
        }
        
        if (pin.length() != 4) {
            return "error: pin must be 4 digits";
        }
        
        // Verify PIN contains only digits
        for (char c : pin) {
            if (!std::isdigit(static_cast<unsigned char>(c))) {
                return "error: pin must contain only digits";
            }
        }
        
        if (accountExists(accountNumber)) {
            return "error: account already exists";
        }

        fs::create_directories(getAccountDir(accountNumber));
        std::ofstream pinFile(getPinPath(accountNumber));
        pinFile << pin;
        
        // Create empty statement file
        std::ofstream statementFile(getStatementPath(accountNumber));
        appendTransaction(accountNumber, TransactionType::ACCOUNT_CREATED, 0);
        
        return "ok";
    }

    // Deposit (customer)
    std::string deposit(const std::string& sessionId, double amount) {
        std::string accountNumber = getAccountFromSession(sessionId);
        if (accountNumber.empty()) {
            return "error: invalid session";
        }
        
        if (amount <= 0) {
            return "error: amount must be positive";
        }

        appendTransaction(accountNumber, TransactionType::DEPOSIT, amount);
        
        return "ok";
    }

    // Debit (customer)
    std::string debit(const std::string& sessionId, double amount) {
        std::string accountNumber = getAccountFromSession(sessionId);
        if (accountNumber.empty()) {
            return "error: invalid session";
        }
        
        if (amount <= 0) {
            return "error: amount must be positive";
        }
        
        double balance = getBalance(accountNumber);
        if (amount > balance) {
            return "error: insufficient funds";
        }

        appendTransaction(accountNumber, TransactionType::DEBIT, amount);
        
        return "ok";
    }

    // Statement (customer)
    std::string getStatement(const std::string& sessionId, int lines = 10) {
        std::string accountNumber = getAccountFromSession(sessionId);
        if (accountNumber.empty()) {
            return "error: invalid session";
        }

        // For admin account, show bank status
        if (accountNumber == ADMIN_ACCOUNT) {
            return getBankStatus();
        }

        std::ifstream file(getStatementPath(accountNumber));
        if (!file.is_open()) {
            return "error: no statement found";
        }

        std::vector<std::string> allLines;
        std::string line;
        while (std::getline(file, line)) {
            allLines.push_back(line);
        }

        std::stringstream result;
        result << "timestamp,type,amount,balance\n";
        
        int startIdx = std::max(0, static_cast<int>(allLines.size()) - lines);
        for (int i = startIdx; i < static_cast<int>(allLines.size()); ++i) {
            result << allLines[i] << "\n";
        }
        
        return result.str();
    }

    // Get bank status (admin only) - lists all accounts and total holdings
    std::string getBankStatus() {
        std::stringstream result;
        result << "Bank Status Report\n";
        result << "==================\n";
        
        double totalHoldings = 0.0;
        int accountCount = 0;
        
        std::string accountsPath = dataDir + "/accounts";
        if (fs::exists(accountsPath)) {
            for (const auto& entry : fs::directory_iterator(accountsPath)) {
                if (entry.is_directory()) {
                    std::string accountNum = entry.path().filename().string();
                    if (accountNum != ADMIN_ACCOUNT) {
                        double balance = getBalance(accountNum);
                        result << "Account " << accountNum << ": " 
                               << std::fixed << std::setprecision(2) << balance << "\n";
                        totalHoldings += balance;
                        accountCount++;
                    }
                }
            }
        }
        
        result << "==================\n";
        result << "Total Accounts: " << accountCount << "\n";
        result << "Total Holdings: " << std::fixed << std::setprecision(2) << totalHoldings << "\n";
        
        return result.str();
    }

    // List accounts (admin only)
    std::string listAccounts(const std::string& sessionId) {
        if (!isAdmin(sessionId)) {
            return "error: unauthorized";
        }
        return getBankStatus();
    }
};

#endif // BANK_H
