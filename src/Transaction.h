#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>

namespace Banking {

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

} // namespace Banking

#endif // TRANSACTION_H
