# Banking Kata

## Description

Create a banking console app that can do the following.

- customer login using account number and pin
- customer deposit into account.
- customer debit account (spend).
- customer statement

Additionally, we need to have a login for the bank this will have the account 00000000 and the pin 9999

The bank administrator needs to list accounts and total the banks holdings.

- create a customer account and result should be a number and a pin for customer login.

## Implementation
This will be a console app with simple commands like.

### Administrator

login 00000000 9999 # admin login returns session_id
create_account session_id <account> <pin> # returns ok or error.

### Customer Can...
login <account_number> <pin> # return session_id
logout
deposit session_id <ammount>
debit session_id<ammount>
statement session_id <lines>

### Storage

data/accounts/{account_number}/statement.csv
data/sessions/{session_id}.txt # contains account number.

for account number 00000000 bank statements should show the current bank status.


