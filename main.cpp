#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "bank.h"

void printHelp() {
    std::cout << "Banking Console Application\n";
    std::cout << "===========================\n";
    std::cout << "Commands:\n";
    std::cout << "  login <account_number> <pin>         - Login to account (returns session_id)\n";
    std::cout << "  logout <session_id>                  - Logout from session\n";
    std::cout << "  create_account <session_id> <account> <pin> - Create new account (admin only)\n";
    std::cout << "  deposit <session_id> <amount>        - Deposit money\n";
    std::cout << "  debit <session_id> <amount>          - Withdraw money\n";
    std::cout << "  statement <session_id> [lines]       - View account statement\n";
    std::cout << "  list_accounts <session_id>           - List all accounts (admin only)\n";
    std::cout << "  help                                 - Show this help\n";
    std::cout << "  exit                                 - Exit application\n";
    std::cout << "\n";
    std::cout << "Admin login: login 00000000 9999\n";
}

std::vector<std::string> parseCommand(const std::string& input) {
    std::vector<std::string> tokens;
    std::istringstream iss(input);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

int main() {
    Bank bank;
    std::string line;
    
    std::cout << "Banking Console Application\n";
    std::cout << "Type 'help' for available commands.\n\n";

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) {
            break;
        }

        auto tokens = parseCommand(line);
        if (tokens.empty()) {
            continue;
        }

        const std::string& cmd = tokens[0];

        if (cmd == "exit" || cmd == "quit") {
            std::cout << "Goodbye!\n";
            break;
        }
        else if (cmd == "help") {
            printHelp();
        }
        else if (cmd == "login") {
            if (tokens.size() < 3) {
                std::cout << "error: usage: login <account_number> <pin>\n";
                continue;
            }
            std::string sessionId = bank.login(tokens[1], tokens[2]);
            if (sessionId.empty()) {
                std::cout << "error: invalid account or pin\n";
            } else {
                std::cout << sessionId << "\n";
            }
        }
        else if (cmd == "logout") {
            if (tokens.size() < 2) {
                std::cout << "error: usage: logout <session_id>\n";
                continue;
            }
            if (bank.logout(tokens[1])) {
                std::cout << "ok\n";
            } else {
                std::cout << "error: invalid session\n";
            }
        }
        else if (cmd == "create_account") {
            if (tokens.size() < 4) {
                std::cout << "error: usage: create_account <session_id> <account> <pin>\n";
                continue;
            }
            std::cout << bank.createAccount(tokens[1], tokens[2], tokens[3]) << "\n";
        }
        else if (cmd == "deposit") {
            if (tokens.size() < 3) {
                std::cout << "error: usage: deposit <session_id> <amount>\n";
                continue;
            }
            try {
                double amount = std::stod(tokens[2]);
                std::cout << bank.deposit(tokens[1], amount) << "\n";
            } catch (...) {
                std::cout << "error: invalid amount\n";
            }
        }
        else if (cmd == "debit") {
            if (tokens.size() < 3) {
                std::cout << "error: usage: debit <session_id> <amount>\n";
                continue;
            }
            try {
                double amount = std::stod(tokens[2]);
                std::cout << bank.debit(tokens[1], amount) << "\n";
            } catch (...) {
                std::cout << "error: invalid amount\n";
            }
        }
        else if (cmd == "statement") {
            if (tokens.size() < 2) {
                std::cout << "error: usage: statement <session_id> [lines]\n";
                continue;
            }
            int lines = 10;
            if (tokens.size() >= 3) {
                try {
                    lines = std::stoi(tokens[2]);
                } catch (...) {
                    lines = 10;
                }
            }
            std::cout << bank.getStatement(tokens[1], lines);
        }
        else if (cmd == "list_accounts") {
            if (tokens.size() < 2) {
                std::cout << "error: usage: list_accounts <session_id>\n";
                continue;
            }
            std::cout << bank.listAccounts(tokens[1]);
        }
        else {
            std::cout << "error: unknown command '" << cmd << "'. Type 'help' for available commands.\n";
        }
    }

    return 0;
}