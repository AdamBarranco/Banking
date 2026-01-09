#ifndef BANK_H
#define BANK_H

#include <string>
#include <map>
#include <vector>
#include "Constants.h"
#include "Transaction.h"

namespace Banking {

class Bank {
private:
    std::string dataDir;

    std::string getAccountDir(const std::string& accountNumber) const;
    std::string getStatementPath(const std::string& accountNumber) const;
    std::string getPinPath(const std::string& accountNumber) const;
    std::string getSessionPath(const std::string& sessionId) const;
    std::string generateSessionId();
    std::string getCurrentTimestamp();
    void ensureDirectories();
    bool accountExists(const std::string& accountNumber) const;
    std::string getStoredPin(const std::string& accountNumber) const;
    double getBalance(const std::string& accountNumber) const;
    void appendTransaction(const std::string& accountNumber, TransactionType type, double amount);

public:
    explicit Bank(const std::string& dataDirectory = DATA_DIR);

    // Login: returns session_id or empty string on failure
    std::string login(const std::string& accountNumber, const std::string& pin);

    // Logout: invalidates session
    bool logout(const std::string& sessionId);

    // Get account number from session
    std::string getAccountFromSession(const std::string& sessionId) const;

    // Check if session belongs to admin
    bool isAdmin(const std::string& sessionId) const;

    // Create account (admin only)
    std::string createAccount(const std::string& sessionId, const std::string& accountNumber, const std::string& pin);

    // Deposit (customer)
    std::string deposit(const std::string& sessionId, double amount);

    // Debit (customer)
    std::string debit(const std::string& sessionId, double amount);

    // Statement (customer)
    std::string getStatement(const std::string& sessionId, int lines = 10);

    // Get bank status (admin only) - lists all accounts and total holdings
    std::string getBankStatus();

    // List accounts (admin only)
    std::string listAccounts(const std::string& sessionId);
};

} // namespace Banking

#endif // BANK_H
