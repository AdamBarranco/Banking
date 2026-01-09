# Banking App - Frontend Documentation

## Version History

| Version | Date       | Author | Changes                                      |
|---------|------------|--------|----------------------------------------------|
| 1.0.0   | 2026-01-09 | System | Initial release with web-based UI            |

---

## Overview

The Banking App frontend is a single-page web application that provides a user-friendly interface for banking operations. It communicates with the backend via REST API endpoints.

## Technology Stack

- **HTML5** - Structure and layout
- **CSS3** - Styling with modern gradients and responsive design
- **JavaScript (ES6+)** - Client-side logic and API communication

## Features

### 1. User Authentication
- Login with account number (8 digits) and PIN (4 digits)
- Session management with automatic logout
- Visual feedback for login status

### 2. Customer Dashboard
- **Balance Display** - Real-time account balance
- **Deposit** - Add funds to account
- **Withdraw (Debit)** - Remove funds from account
- **Transfer** - Send money to another account
- **Transaction History** - View recent transactions

### 3. Admin Dashboard
- **Bank Status** - Overview of all accounts and total holdings
- **Create Account** - Add new customer accounts

## UI Components

### Login Section
```
┌─────────────────────────────────┐
│         🏦 Banking App          │
├─────────────────────────────────┤
│  Account Number: [________]     │
│  PIN:            [____]         │
│                                 │
│         [Login Button]          │
│                                 │
│  Demo: 12345678/1234            │
└─────────────────────────────────┘
```

### Customer Dashboard
```
┌─────────────────────────────────┐
│ 🏦 Banking App    Account: XXX  │
├─────────────────────────────────┤
│      Account Balance            │
│         $X,XXX.XX               │
├─────────────────────────────────┤
│ ┌─────────┐ ┌─────────┐ ┌─────┐│
│ │ Deposit │ │Withdraw │ │Trans││
│ │ [____]  │ │ [____]  │ │[___]││
│ │ [Submit]│ │ [Submit]│ │[Sub]││
│ └─────────┘ └─────────┘ └─────┘│
├─────────────────────────────────┤
│  Recent Transactions            │
│  Date | Type | Amount | Balance │
│  ─────┼──────┼────────┼──────── │
│  ...  │ ...  │  ...   │   ...   │
└─────────────────────────────────┘
```

## API Endpoints Used

| Endpoint              | Method | Parameters                          | Description            |
|-----------------------|--------|-------------------------------------|------------------------|
| `/api/login`          | GET    | `account`, `pin`                    | Authenticate user      |
| `/api/logout`         | GET    | `session_id`                        | End session            |
| `/api/deposit`        | GET    | `session_id`, `amount`              | Deposit funds          |
| `/api/debit`          | GET    | `session_id`, `amount`              | Withdraw funds         |
| `/api/transfer`       | GET    | `session_id`, `to_account`, `amount`| Transfer funds         |
| `/api/statement`      | GET    | `session_id`, `lines`               | Get transactions       |
| `/api/create_account` | GET    | `session_id`, `account`, `pin`      | Create account (admin) |
| `/api/list_accounts`  | GET    | `session_id`                        | List all accounts      |

## Response Format

All API responses follow this JSON structure:

```json
{
  "success": true|false,
  "message": "Human-readable message",
  "data": "Optional data payload"
}
```

## Styling

### Color Palette
- **Primary Gradient**: `#667eea` → `#764ba2` (Purple gradient)
- **Success**: `#11998e` → `#38ef7d` (Green gradient)
- **Warning/Danger**: `#f093fb` → `#f5576c` (Pink/Red gradient)
- **Background**: White cards on gradient background

### Responsive Design
- Mobile-first approach
- Breakpoint at 600px for mobile devices
- Flexible grid layout for operation cards

## Browser Compatibility

- Chrome 80+
- Firefox 75+
- Safari 13+
- Edge 80+

## File Structure

```
src/
└── web_main.cpp      # Contains embedded HTML, CSS, and JavaScript
    ├── getHtmlContent()  # Main HTML structure
    ├── getCssContent()   # Stylesheet
    └── getJsContent()    # Client-side logic
```

## Security Considerations

1. Session IDs are stored in JavaScript memory (not cookies)
2. Sessions are invalidated on logout
3. All sensitive operations require valid session
4. PIN is transmitted but not stored client-side

## Future Enhancements

- [ ] Add password visibility toggle
- [ ] Implement "Remember Me" functionality
- [ ] Add transaction search/filter
- [ ] Export statement to PDF/CSV
- [ ] Dark mode theme
- [ ] Multi-language support
