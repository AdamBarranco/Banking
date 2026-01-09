# Banking Kata

## Description

Create a banking console app that can do the following.

- customer login using account number and pin
- customer deposit into account.
- customer debit account (spend).
- customer transfer money to another account.
- customer statement

Additionally, we need to have a login for the bank this will have the account 00000000 and the pin 9999

The bank administrator needs to list accounts and total the banks holdings.

- create a customer account and result should be a number and a pin for customer login.

## Project Structure

```
Banking/
├── src/
│   ├── Bank.cpp         # Bank class implementation
│   ├── Bank.h           # Bank class declaration
│   ├── Constants.h      # Application constants
│   ├── Transaction.h    # Transaction types and structures
│   └── main.cpp         # Console application entry point
├── sample_data/         # Sample accounts with CSV statements
│   ├── accounts/
│   │   ├── 00000000/    # Admin account (PIN: 9999)
│   │   ├── 12345678/    # Sample customer (PIN: 1234)
│   │   └── 87654321/    # Sample customer (PIN: 4321)
│   └── sessions/
├── bank_tests.cpp       # Unit tests
├── CMakeLists.txt       # CMake build configuration
└── README.md
```

## Building

```bash
mkdir build && cd build
cmake ..
make
```

## Running Tests

```bash
cd build
./bank_tests
```

## Implementation
This will be a console app with simple commands like.

### Administrator

login 00000000 9999 # admin login returns session_id
create_account session_id <account> <pin> # returns ok or error.

### Customer Can...
login <account_number> <pin> # return session_id
logout
deposit session_id <ammount>
debit session_id <ammount>
transfer session_id <to_account> <amount>
statement session_id <lines>

### Sample Accounts

The `sample_data/` folder contains pre-configured accounts for testing:

| Account    | PIN  | Description     |
|------------|------|-----------------|
| 00000000   | 9999 | Admin account   |
| 12345678   | 1234 | Customer account with transaction history |
| 87654321   | 4321 | Customer account with transaction history |

To use sample data, copy it to the `data/` directory or pass the path to the Bank constructor.

### Storage

data/accounts/{account_number}/statement.csv
data/sessions/{session_id}.txt # contains account number.

for account number 00000000 bank statements should show the current bank status.


