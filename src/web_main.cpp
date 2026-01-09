#include <iostream>
#include <fstream>
#include <sstream>
#include <signal.h>
#include "Bank.h"
#include "WebServer.h"

// Global server pointer for signal handling
Banking::WebServer* g_server = nullptr;

void signalHandler(int signum) {
    std::cout << "\nShutting down server...\n";
    if (g_server) {
        g_server->stop();
    }
}

// Simple JSON helper functions
std::string jsonEscape(const std::string& str) {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c;
        }
    }
    return result;
}

std::string makeJsonResponse(bool success, const std::string& message, const std::string& data = "") {
    std::ostringstream json;
    json << "{\"success\":" << (success ? "true" : "false") 
         << ",\"message\":\"" << jsonEscape(message) << "\"";
    if (!data.empty()) {
        json << ",\"data\":\"" << jsonEscape(data) << "\"";
    }
    json << "}";
    return json.str();
}

// HTML content for the banking UI
const char* getHtmlContent() {
    return R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Banking App</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <div class="container">
        <header>
            <h1>🏦 Banking App</h1>
            <div id="user-info" class="hidden">
                <span id="account-display"></span>
                <button onclick="logout()" class="btn btn-secondary">Logout</button>
            </div>
        </header>

        <!-- Login Section -->
        <section id="login-section" class="card">
            <h2>Login</h2>
            <form onsubmit="login(event)">
                <div class="form-group">
                    <label for="account-number">Account Number</label>
                    <input type="text" id="account-number" placeholder="e.g., 12345678" maxlength="8" required>
                </div>
                <div class="form-group">
                    <label for="pin">PIN</label>
                    <input type="password" id="pin" placeholder="e.g., 1234" maxlength="4" required>
                </div>
                <button type="submit" class="btn btn-primary">Login</button>
            </form>
            <p class="hint">Demo accounts: 12345678/1234 or Admin: 00000000/9999</p>
        </section>

        <!-- Dashboard Section -->
        <section id="dashboard-section" class="hidden">
            <!-- Balance Card -->
            <div class="card balance-card">
                <h2>Account Balance</h2>
                <div id="balance-display" class="balance">$0.00</div>
                <button onclick="refreshStatement()" class="btn btn-secondary">Refresh</button>
            </div>

            <!-- Customer Operations -->
            <div id="customer-operations">
                <div class="card-grid">
                    <!-- Deposit -->
                    <div class="card">
                        <h3>💰 Deposit</h3>
                        <form onsubmit="deposit(event)">
                            <div class="form-group">
                                <label for="deposit-amount">Amount</label>
                                <input type="number" id="deposit-amount" step="0.01" min="0.01" placeholder="0.00" required>
                            </div>
                            <button type="submit" class="btn btn-success">Deposit</button>
                        </form>
                    </div>

                    <!-- Debit -->
                    <div class="card">
                        <h3>💳 Withdraw</h3>
                        <form onsubmit="debit(event)">
                            <div class="form-group">
                                <label for="debit-amount">Amount</label>
                                <input type="number" id="debit-amount" step="0.01" min="0.01" placeholder="0.00" required>
                            </div>
                            <button type="submit" class="btn btn-warning">Withdraw</button>
                        </form>
                    </div>

                    <!-- Transfer -->
                    <div class="card">
                        <h3>🔄 Transfer</h3>
                        <form onsubmit="transfer(event)">
                            <div class="form-group">
                                <label for="transfer-account">To Account</label>
                                <input type="text" id="transfer-account" placeholder="e.g., 87654321" maxlength="8" required>
                            </div>
                            <div class="form-group">
                                <label for="transfer-amount">Amount</label>
                                <input type="number" id="transfer-amount" step="0.01" min="0.01" placeholder="0.00" required>
                            </div>
                            <button type="submit" class="btn btn-primary">Transfer</button>
                        </form>
                    </div>
                </div>

                <!-- Statement -->
                <div class="card">
                    <h3>📊 Recent Transactions</h3>
                    <div id="statement-container">
                        <table id="statement-table">
                            <thead>
                                <tr>
                                    <th>Date</th>
                                    <th>Type</th>
                                    <th>Amount</th>
                                    <th>Balance</th>
                                </tr>
                            </thead>
                            <tbody id="statement-body">
                            </tbody>
                        </table>
                    </div>
                </div>
            </div>

            <!-- Admin Operations -->
            <div id="admin-operations" class="hidden">
                <div class="card">
                    <h3>🔧 Admin: Create Account</h3>
                    <form onsubmit="createAccount(event)">
                        <div class="form-group">
                            <label for="new-account">Account Number (8 digits)</label>
                            <input type="text" id="new-account" placeholder="e.g., 11112222" maxlength="8" required>
                        </div>
                        <div class="form-group">
                            <label for="new-pin">PIN (4 digits)</label>
                            <input type="password" id="new-pin" placeholder="e.g., 1234" maxlength="4" required>
                        </div>
                        <button type="submit" class="btn btn-primary">Create Account</button>
                    </form>
                </div>

                <div class="card">
                    <h3>📋 Bank Status</h3>
                    <pre id="bank-status"></pre>
                    <button onclick="listAccounts()" class="btn btn-secondary">Refresh Status</button>
                </div>
            </div>
        </section>

        <!-- Message Display -->
        <div id="message" class="message hidden"></div>
    </div>

    <script src="/app.js"></script>
</body>
</html>
)HTML";
}

const char* getCssContent() {
    return R"CSS(* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

body {
    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, sans-serif;
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    min-height: 100vh;
    padding: 20px;
}

.container {
    max-width: 900px;
    margin: 0 auto;
}

header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 20px;
    background: white;
    border-radius: 12px;
    margin-bottom: 20px;
    box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
}

header h1 {
    color: #333;
    font-size: 1.8rem;
}

#user-info {
    display: flex;
    align-items: center;
    gap: 15px;
}

#account-display {
    font-weight: 600;
    color: #667eea;
}

.card {
    background: white;
    border-radius: 12px;
    padding: 25px;
    margin-bottom: 20px;
    box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
}

.card h2, .card h3 {
    color: #333;
    margin-bottom: 20px;
}

.card-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
    gap: 20px;
    margin-bottom: 20px;
}

.balance-card {
    text-align: center;
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    color: white;
}

.balance-card h2 {
    color: white;
    opacity: 0.9;
}

.balance {
    font-size: 3rem;
    font-weight: 700;
    margin: 20px 0;
}

.form-group {
    margin-bottom: 15px;
}

.form-group label {
    display: block;
    margin-bottom: 5px;
    font-weight: 600;
    color: #555;
}

.form-group input {
    width: 100%;
    padding: 12px;
    border: 2px solid #e0e0e0;
    border-radius: 8px;
    font-size: 1rem;
    transition: border-color 0.3s;
}

.form-group input:focus {
    outline: none;
    border-color: #667eea;
}

.btn {
    padding: 12px 24px;
    border: none;
    border-radius: 8px;
    font-size: 1rem;
    font-weight: 600;
    cursor: pointer;
    transition: transform 0.2s, box-shadow 0.2s;
}

.btn:hover {
    transform: translateY(-2px);
    box-shadow: 0 4px 12px rgba(0, 0, 0, 0.15);
}

.btn:active {
    transform: translateY(0);
}

.btn-primary {
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    color: white;
}

.btn-secondary {
    background: #f0f0f0;
    color: #333;
}

.btn-success {
    background: linear-gradient(135deg, #11998e 0%, #38ef7d 100%);
    color: white;
}

.btn-warning {
    background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
    color: white;
}

.hint {
    margin-top: 15px;
    color: #888;
    font-size: 0.9rem;
}

#statement-container {
    overflow-x: auto;
}

#statement-table {
    width: 100%;
    border-collapse: collapse;
}

#statement-table th,
#statement-table td {
    padding: 12px;
    text-align: left;
    border-bottom: 1px solid #e0e0e0;
}

#statement-table th {
    background: #f8f9fa;
    font-weight: 600;
    color: #555;
}

#statement-table tr:hover {
    background: #f8f9fa;
}

.message {
    position: fixed;
    bottom: 20px;
    right: 20px;
    padding: 15px 25px;
    border-radius: 8px;
    color: white;
    font-weight: 600;
    animation: slideIn 0.3s ease;
    z-index: 1000;
}

.message.success {
    background: linear-gradient(135deg, #11998e 0%, #38ef7d 100%);
}

.message.error {
    background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
}

@keyframes slideIn {
    from {
        transform: translateX(100%);
        opacity: 0;
    }
    to {
        transform: translateX(0);
        opacity: 1;
    }
}

.hidden {
    display: none !important;
}

#bank-status {
    background: #f8f9fa;
    padding: 15px;
    border-radius: 8px;
    font-family: monospace;
    white-space: pre-wrap;
    margin-bottom: 15px;
    max-height: 300px;
    overflow-y: auto;
}

@media (max-width: 600px) {
    header {
        flex-direction: column;
        gap: 15px;
        text-align: center;
    }
    
    .balance {
        font-size: 2rem;
    }
    
    .card-grid {
        grid-template-columns: 1fr;
    }
}
)CSS";
}

const char* getJsContent() {
    return R"JS(// State
let sessionId = null;
let currentAccount = null;
let isAdmin = false;

// DOM Elements
const loginSection = document.getElementById('login-section');
const dashboardSection = document.getElementById('dashboard-section');
const userInfo = document.getElementById('user-info');
const accountDisplay = document.getElementById('account-display');
const customerOperations = document.getElementById('customer-operations');
const adminOperations = document.getElementById('admin-operations');
const balanceDisplay = document.getElementById('balance-display');
const statementBody = document.getElementById('statement-body');
const bankStatus = document.getElementById('bank-status');
const messageEl = document.getElementById('message');

// API helper
async function api(endpoint, data = {}) {
    const params = new URLSearchParams(data);
    const response = await fetch(`${endpoint}?${params}`);
    return response.json();
}

// Show message
function showMessage(text, isError = false) {
    messageEl.textContent = text;
    messageEl.className = `message ${isError ? 'error' : 'success'}`;
    messageEl.classList.remove('hidden');
    setTimeout(() => messageEl.classList.add('hidden'), 3000);
}

// Login
async function login(event) {
    event.preventDefault();
    const accountNumber = document.getElementById('account-number').value;
    const pin = document.getElementById('pin').value;
    
    const result = await api('/api/login', { account: accountNumber, pin: pin });
    
    if (result.success) {
        sessionId = result.data;
        currentAccount = accountNumber;
        isAdmin = accountNumber === '00000000';
        
        // Update UI
        loginSection.classList.add('hidden');
        dashboardSection.classList.remove('hidden');
        userInfo.classList.remove('hidden');
        accountDisplay.textContent = isAdmin ? 'Admin Account' : `Account: ${accountNumber}`;
        
        if (isAdmin) {
            customerOperations.classList.add('hidden');
            adminOperations.classList.remove('hidden');
            listAccounts();
        } else {
            customerOperations.classList.remove('hidden');
            adminOperations.classList.add('hidden');
            refreshStatement();
        }
        
        showMessage('Login successful!');
    } else {
        showMessage(result.message, true);
    }
}

// Logout
async function logout() {
    if (sessionId) {
        await api('/api/logout', { session_id: sessionId });
    }
    
    sessionId = null;
    currentAccount = null;
    isAdmin = false;
    
    // Reset UI
    loginSection.classList.remove('hidden');
    dashboardSection.classList.add('hidden');
    userInfo.classList.add('hidden');
    
    // Clear forms
    document.getElementById('account-number').value = '';
    document.getElementById('pin').value = '';
    
    showMessage('Logged out successfully');
}

// Deposit
async function deposit(event) {
    event.preventDefault();
    const amount = document.getElementById('deposit-amount').value;
    
    const result = await api('/api/deposit', { session_id: sessionId, amount: amount });
    
    if (result.success) {
        showMessage(`Deposited $${parseFloat(amount).toFixed(2)} successfully!`);
        document.getElementById('deposit-amount').value = '';
        refreshStatement();
    } else {
        showMessage(result.message, true);
    }
}

// Debit
async function debit(event) {
    event.preventDefault();
    const amount = document.getElementById('debit-amount').value;
    
    const result = await api('/api/debit', { session_id: sessionId, amount: amount });
    
    if (result.success) {
        showMessage(`Withdrew $${parseFloat(amount).toFixed(2)} successfully!`);
        document.getElementById('debit-amount').value = '';
        refreshStatement();
    } else {
        showMessage(result.message, true);
    }
}

// Transfer
async function transfer(event) {
    event.preventDefault();
    const toAccount = document.getElementById('transfer-account').value;
    const amount = document.getElementById('transfer-amount').value;
    
    const result = await api('/api/transfer', { 
        session_id: sessionId, 
        to_account: toAccount, 
        amount: amount 
    });
    
    if (result.success) {
        showMessage(`Transferred $${parseFloat(amount).toFixed(2)} to ${toAccount} successfully!`);
        document.getElementById('transfer-account').value = '';
        document.getElementById('transfer-amount').value = '';
        refreshStatement();
    } else {
        showMessage(result.message, true);
    }
}

// Get statement
async function refreshStatement() {
    const result = await api('/api/statement', { session_id: sessionId, lines: 10 });
    
    if (result.success) {
        const lines = result.data.split('\n').filter(line => line.trim());
        statementBody.innerHTML = '';
        
        let lastBalance = 0;
        
        // Skip header line
        for (let i = 1; i < lines.length; i++) {
            const parts = lines[i].split(',');
            if (parts.length >= 4) {
                const row = document.createElement('tr');
                row.innerHTML = `
                    <td>${parts[0]}</td>
                    <td>${parts[1]}</td>
                    <td>$${parseFloat(parts[2]).toFixed(2)}</td>
                    <td>$${parseFloat(parts[3]).toFixed(2)}</td>
                `;
                statementBody.appendChild(row);
                lastBalance = parseFloat(parts[3]);
            }
        }
        
        balanceDisplay.textContent = `$${lastBalance.toFixed(2)}`;
    } else {
        showMessage(result.message, true);
    }
}

// Create account (admin)
async function createAccount(event) {
    event.preventDefault();
    const newAccount = document.getElementById('new-account').value;
    const newPin = document.getElementById('new-pin').value;
    
    const result = await api('/api/create_account', { 
        session_id: sessionId, 
        account: newAccount, 
        pin: newPin 
    });
    
    if (result.success) {
        showMessage(`Account ${newAccount} created successfully!`);
        document.getElementById('new-account').value = '';
        document.getElementById('new-pin').value = '';
        listAccounts();
    } else {
        showMessage(result.message, true);
    }
}

// List accounts (admin)
async function listAccounts() {
    const result = await api('/api/list_accounts', { session_id: sessionId });
    
    if (result.success) {
        bankStatus.textContent = result.data;
    } else {
        showMessage(result.message, true);
    }
}
)JS";
}

int main(int argc, char* argv[]) {
    int port = 8080;
    std::string dataDir = "data";
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (arg == "--data" && i + 1 < argc) {
            dataDir = argv[++i];
        } else if (arg == "--help") {
            std::cout << "Banking Web Server\n";
            std::cout << "Usage: " << argv[0] << " [options]\n";
            std::cout << "Options:\n";
            std::cout << "  --port <port>  Port to listen on (default: 8080)\n";
            std::cout << "  --data <dir>   Data directory (default: data)\n";
            std::cout << "  --help         Show this help\n";
            return 0;
        }
    }
    
    // Set up signal handling
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Create bank instance
    Banking::Bank bank(dataDir);
    
    // Create web server
    Banking::WebServer server(port);
    g_server = &server;
    
    // API Routes
    server.addRoute("GET", "/api/login", [&bank](const Banking::HttpRequest& req, Banking::HttpResponse& res) {
        auto it_account = req.queryParams.find("account");
        auto it_pin = req.queryParams.find("pin");
        
        if (it_account == req.queryParams.end() || it_pin == req.queryParams.end()) {
            res.setJson(makeJsonResponse(false, "Missing account or pin"));
            return;
        }
        
        std::string sessionId = bank.login(it_account->second, it_pin->second);
        if (sessionId.empty()) {
            res.setJson(makeJsonResponse(false, "Invalid account or pin"));
        } else {
            res.setJson(makeJsonResponse(true, "Login successful", sessionId));
        }
    });
    
    server.addRoute("GET", "/api/logout", [&bank](const Banking::HttpRequest& req, Banking::HttpResponse& res) {
        auto it = req.queryParams.find("session_id");
        if (it == req.queryParams.end()) {
            res.setJson(makeJsonResponse(false, "Missing session_id"));
            return;
        }
        
        if (bank.logout(it->second)) {
            res.setJson(makeJsonResponse(true, "Logged out"));
        } else {
            res.setJson(makeJsonResponse(false, "Invalid session"));
        }
    });
    
    server.addRoute("GET", "/api/deposit", [&bank](const Banking::HttpRequest& req, Banking::HttpResponse& res) {
        auto it_session = req.queryParams.find("session_id");
        auto it_amount = req.queryParams.find("amount");
        
        if (it_session == req.queryParams.end() || it_amount == req.queryParams.end()) {
            res.setJson(makeJsonResponse(false, "Missing session_id or amount"));
            return;
        }
        
        try {
            double amount = std::stod(it_amount->second);
            std::string result = bank.deposit(it_session->second, amount);
            if (result == "ok") {
                res.setJson(makeJsonResponse(true, "Deposit successful"));
            } else {
                res.setJson(makeJsonResponse(false, result));
            }
        } catch (...) {
            res.setJson(makeJsonResponse(false, "Invalid amount"));
        }
    });
    
    server.addRoute("GET", "/api/debit", [&bank](const Banking::HttpRequest& req, Banking::HttpResponse& res) {
        auto it_session = req.queryParams.find("session_id");
        auto it_amount = req.queryParams.find("amount");
        
        if (it_session == req.queryParams.end() || it_amount == req.queryParams.end()) {
            res.setJson(makeJsonResponse(false, "Missing session_id or amount"));
            return;
        }
        
        try {
            double amount = std::stod(it_amount->second);
            std::string result = bank.debit(it_session->second, amount);
            if (result == "ok") {
                res.setJson(makeJsonResponse(true, "Debit successful"));
            } else {
                res.setJson(makeJsonResponse(false, result));
            }
        } catch (...) {
            res.setJson(makeJsonResponse(false, "Invalid amount"));
        }
    });
    
    server.addRoute("GET", "/api/transfer", [&bank](const Banking::HttpRequest& req, Banking::HttpResponse& res) {
        auto it_session = req.queryParams.find("session_id");
        auto it_to = req.queryParams.find("to_account");
        auto it_amount = req.queryParams.find("amount");
        
        if (it_session == req.queryParams.end() || it_to == req.queryParams.end() || it_amount == req.queryParams.end()) {
            res.setJson(makeJsonResponse(false, "Missing session_id, to_account, or amount"));
            return;
        }
        
        try {
            double amount = std::stod(it_amount->second);
            std::string result = bank.transfer(it_session->second, it_to->second, amount);
            if (result == "ok") {
                res.setJson(makeJsonResponse(true, "Transfer successful"));
            } else {
                res.setJson(makeJsonResponse(false, result));
            }
        } catch (...) {
            res.setJson(makeJsonResponse(false, "Invalid amount"));
        }
    });
    
    server.addRoute("GET", "/api/statement", [&bank](const Banking::HttpRequest& req, Banking::HttpResponse& res) {
        auto it_session = req.queryParams.find("session_id");
        
        if (it_session == req.queryParams.end()) {
            res.setJson(makeJsonResponse(false, "Missing session_id"));
            return;
        }
        
        int lines = 10;
        auto it_lines = req.queryParams.find("lines");
        if (it_lines != req.queryParams.end()) {
            try {
                lines = std::stoi(it_lines->second);
            } catch (...) {}
        }
        
        std::string result = bank.getStatement(it_session->second, lines);
        if (result.substr(0, 5) == "error") {
            res.setJson(makeJsonResponse(false, result));
        } else {
            res.setJson(makeJsonResponse(true, "Statement retrieved", result));
        }
    });
    
    server.addRoute("GET", "/api/create_account", [&bank](const Banking::HttpRequest& req, Banking::HttpResponse& res) {
        auto it_session = req.queryParams.find("session_id");
        auto it_account = req.queryParams.find("account");
        auto it_pin = req.queryParams.find("pin");
        
        if (it_session == req.queryParams.end() || it_account == req.queryParams.end() || it_pin == req.queryParams.end()) {
            res.setJson(makeJsonResponse(false, "Missing session_id, account, or pin"));
            return;
        }
        
        std::string result = bank.createAccount(it_session->second, it_account->second, it_pin->second);
        if (result == "ok") {
            res.setJson(makeJsonResponse(true, "Account created"));
        } else {
            res.setJson(makeJsonResponse(false, result));
        }
    });
    
    server.addRoute("GET", "/api/list_accounts", [&bank](const Banking::HttpRequest& req, Banking::HttpResponse& res) {
        auto it_session = req.queryParams.find("session_id");
        
        if (it_session == req.queryParams.end()) {
            res.setJson(makeJsonResponse(false, "Missing session_id"));
            return;
        }
        
        std::string result = bank.listAccounts(it_session->second);
        if (result.substr(0, 5) == "error") {
            res.setJson(makeJsonResponse(false, result));
        } else {
            res.setJson(makeJsonResponse(true, "Accounts listed", result));
        }
    });
    
    // Static file handler
    server.setStaticHandler([](const Banking::HttpRequest& req, Banking::HttpResponse& res) {
        if (req.path == "/" || req.path == "/index.html") {
            res.setHtml(getHtmlContent());
        } else if (req.path == "/style.css") {
            res.setCss(getCssContent());
        } else if (req.path == "/app.js") {
            res.setJs(getJsContent());
        } else {
            res.setNotFound();
        }
    });
    
    // Start server
    if (!server.start()) {
        std::cerr << "Failed to start server\n";
        return 1;
    }
    
    std::cout << "Banking Web Server is running.\n";
    std::cout << "Open http://localhost:" << port << " in your browser.\n";
    std::cout << "Press Ctrl+C to stop.\n";
    
    // Keep running until stopped
    while (server.isRunning()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 0;
}
