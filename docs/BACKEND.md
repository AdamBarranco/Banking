# Banking App - Backend Documentation

## Version History

| Version | Date       | Author | Changes                                              |
|---------|------------|--------|------------------------------------------------------|
| 1.0.0   | 2026-01-09 | System | Initial console application                          |
| 1.1.0   | 2026-01-09 | System | Added web server with REST API                       |

---

## Overview

The Banking App backend is a C++ application that provides core banking functionality including account management, transactions, and session handling. It supports both console and web interfaces.

## Technology Stack

- **C++20** - Core language
- **CMake 3.30+** - Build system
- **Catch2** - Unit testing framework
- **POSIX Sockets** - HTTP server (web interface)

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                     Interfaces                          │
├─────────────────────┬───────────────────────────────────┤
│   Console (main.cpp)│      Web Server (web_main.cpp)    │
│                     │           ↓                       │
│                     │     WebServer.cpp                 │
│                     │     (HTTP handling)               │
└─────────────────────┴───────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────┐
│                    Core Banking                         │
├─────────────────────────────────────────────────────────┤
│  Bank.cpp / Bank.h                                      │
│  - Account management                                   │
│  - Transaction processing                               │
│  - Session management                                   │
│  - Balance calculations                                 │
├─────────────────────────────────────────────────────────┤
│  Transaction.h        Constants.h                       │
│  - Transaction types  - Admin credentials               │
│  - Data structures    - Directory paths                 │
└─────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────┐
│                    File Storage                         │
├─────────────────────────────────────────────────────────┤
│  data/                                                  │
│  ├── accounts/{account_number}/                         │
│  │   ├── pin.txt           # Account PIN                │
│  │   └── statement.csv     # Transaction history        │
│  └── sessions/{session_id}.txt  # Active sessions       │
└─────────────────────────────────────────────────────────┘
```

## Core Classes

### Bank Class (`Bank.h` / `Bank.cpp`)

The main class providing all banking operations.

#### Public Methods

| Method | Parameters | Returns | Description |
|--------|------------|---------|-------------|
| `login` | `accountNumber`, `pin` | `sessionId` or `""` | Authenticate user |
| `logout` | `sessionId` | `bool` | End session |
| `getAccountFromSession` | `sessionId` | `accountNumber` | Get account for session |
| `isAdmin` | `sessionId` | `bool` | Check if admin session |
| `createAccount` | `sessionId`, `accountNumber`, `pin` | `"ok"` or error | Create new account |
| `deposit` | `sessionId`, `amount` | `"ok"` or error | Add funds |
| `debit` | `sessionId`, `amount` | `"ok"` or error | Withdraw funds |
| `transfer` | `sessionId`, `toAccount`, `amount` | `"ok"` or error | Transfer between accounts |
| `getStatement` | `sessionId`, `lines` | CSV string or error | Get transaction history |
| `listAccounts` | `sessionId` | Status report or error | Admin: list all accounts |
| `getBankStatus` | - | Status report | Get bank holdings summary |

#### Private Methods

| Method | Description |
|--------|-------------|
| `getAccountDir` | Get path to account directory |
| `getStatementPath` | Get path to statement CSV |
| `getPinPath` | Get path to PIN file |
| `getSessionPath` | Get path to session file |
| `generateSessionId` | Create random 32-char hex session ID |
| `getCurrentTimestamp` | Get formatted timestamp |
| `ensureDirectories` | Create required directories |
| `accountExists` | Check if account exists |
| `getStoredPin` | Read PIN from file |
| `getBalance` | Calculate current balance |
| `appendTransaction` | Add transaction to statement |

### WebServer Class (`WebServer.h` / `WebServer.cpp`)

HTTP server for the web interface.

#### Key Features
- Single-threaded request handling
- Route-based request dispatch
- Query string parsing
- URL decoding
- Static file serving

### Transaction Types (`Transaction.h`)

```cpp
enum class TransactionType {
    DEPOSIT,        // Money added to account
    DEBIT,          // Money withdrawn
    TRANSFER_IN,    // Money received from transfer
    TRANSFER_OUT,   // Money sent via transfer
    ACCOUNT_CREATED // Initial account creation
};
```

## API Reference

### REST API Endpoints

All endpoints use GET method with query parameters.

#### Authentication

**Login**
```
GET /api/login?account={account}&pin={pin}
Response: { "success": true, "message": "...", "data": "{session_id}" }
```

**Logout**
```
GET /api/logout?session_id={session_id}
Response: { "success": true, "message": "Logged out" }
```

#### Customer Operations

**Deposit**
```
GET /api/deposit?session_id={session_id}&amount={amount}
Response: { "success": true, "message": "Deposit successful" }
```

**Debit (Withdraw)**
```
GET /api/debit?session_id={session_id}&amount={amount}
Response: { "success": true, "message": "Debit successful" }
Errors: "error: insufficient funds", "error: amount must be positive"
```

**Transfer**
```
GET /api/transfer?session_id={session_id}&to_account={account}&amount={amount}
Response: { "success": true, "message": "Transfer successful" }
Errors: "error: destination account does not exist", "error: insufficient funds"
```

**Statement**
```
GET /api/statement?session_id={session_id}&lines={count}
Response: { "success": true, "data": "timestamp,type,amount,balance\n..." }
```

#### Admin Operations

**Create Account**
```
GET /api/create_account?session_id={session_id}&account={account}&pin={pin}
Response: { "success": true, "message": "Account created" }
Errors: "error: unauthorized", "error: account already exists"
```

**List Accounts**
```
GET /api/list_accounts?session_id={session_id}
Response: { "success": true, "data": "Bank Status Report\n..." }
```

## Data Storage

### Directory Structure
```
data/
├── accounts/
│   ├── 00000000/           # Admin account
│   │   ├── pin.txt         # Contains: 9999
│   │   └── statement.csv   # Bank-wide status
│   ├── 12345678/           # Customer account
│   │   ├── pin.txt         # Contains: 1234
│   │   └── statement.csv   # Transaction history
│   └── ...
└── sessions/
    ├── abc123...def.txt    # Contains: 12345678
    └── ...
```

### Statement CSV Format
```csv
timestamp,type,amount,balance
2026-01-09 10:30:00,ACCOUNT_CREATED,0.00,0.00
2026-01-09 10:31:00,DEPOSIT,1000.00,1000.00
2026-01-09 10:32:00,DEBIT,50.00,950.00
2026-01-09 10:33:00,TRANSFER_OUT,100.00,850.00
```

## Building

### Prerequisites
- C++20 compatible compiler (GCC 10+, Clang 10+)
- CMake 3.30+

### Build Commands
```bash
mkdir build && cd build
cmake ..
make
```

### Build Targets
| Target | Description |
|--------|-------------|
| `Banking` | Console application |
| `BankingWeb` | Web server application |
| `bank_tests` | Unit tests |

## Running

### Console Application
```bash
./Banking
```

### Web Server
```bash
./BankingWeb [options]

Options:
  --port <port>   Port to listen on (default: 8080)
  --data <dir>    Data directory (default: data)
  --help          Show help
```

### Running Tests
```bash
./bank_tests
```

## Error Handling

All errors are returned as strings prefixed with "error:":
- `error: invalid session`
- `error: unauthorized`
- `error: insufficient funds`
- `error: amount must be positive`
- `error: account already exists`
- `error: account number must be 8 digits`
- `error: pin must be 4 digits`
- `error: destination account does not exist`
- `error: cannot transfer to same account`

## Security Considerations

1. **PIN Storage**: PINs stored in plain text (demo only - production should hash)
2. **Session IDs**: 32-character random hex strings
3. **Input Validation**: Account numbers and PINs validated for format
4. **Authorization**: Admin operations require admin session

## Configuration

### Constants (`Constants.h`)
```cpp
constexpr const char* ADMIN_ACCOUNT = "00000000";
constexpr const char* ADMIN_PIN = "9999";
constexpr const char* DATA_DIR = "data";
```

## Future Enhancements

- [ ] Password hashing (bcrypt/argon2)
- [ ] HTTPS support
- [ ] Database storage (SQLite/PostgreSQL)
- [ ] Rate limiting
- [ ] Audit logging
- [ ] Multi-threaded request handling
- [ ] WebSocket support for real-time updates
