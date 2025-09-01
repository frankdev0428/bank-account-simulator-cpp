// Bank Account Simulator — Single-file starter (C++17)
// Features:
//  - Create accounts with PIN (hashed; not cryptographically secure)
//  - Multiple accounts stored in-memory (std::vector)
//  - Deposit, withdraw, check balance
//  - Simple login by account ID + PIN
//  - Money stored as cents (integer) to avoid floating-point errors
//
// Build (Linux/Mac):
//   g++ -std=gnu++17 -O2 -Wall -Wextra -pedantic -o bank main.cpp
// Run:
//   ./bank
//
// NOTE: This single-file version is great for learning. Later, we can split
// into Account.hpp/Bank.hpp and add file persistence (save/load).


using namespace std;
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <random>
#include <algorithm>
#include <fstream>

// ---------------- Money helpers ----------------
static long long parseAmountCents(const string &s) {
    // Accepts formats like "123", "123.45", "0.99"; ignores leading/trailing spaces
    string t; for (char c : s) if (!isspace((unsigned char)c)) t.push_back(c);
    if (t.empty()) throw invalid_argument("empty amount");
    size_t dot = t.find('.');
    if (dot == string::npos) {
        // dollars only
        long long dollars = stoll(t);
        return dollars * 100LL;
    }
    string dollarsStr = t.substr(0, dot);
    string centsStr = t.substr(dot + 1);
    if (centsStr.size() > 2) centsStr = centsStr.substr(0, 2); // truncate extra digits
    while (centsStr.size() < 2) centsStr.push_back('0');
    long long dollars = dollarsStr.empty() ? 0 : stoll(dollarsStr);
    long long cents = centsStr.empty() ? 0 : stoll(centsStr);
    if (dollars < 0 || (dollars == 0 && t[0] == '-')) cents = -cents; // handle negatives
    return dollars * 100LL + (t[0] == '-' ? -cents : cents);
}

static string formatCents(long long cents) {
    bool neg = cents < 0; if (neg) cents = -cents;
    long long dollars = cents / 100; long long rem = cents % 100;
    ostringstream oss; oss << (neg ? "-" : "") << "$" << dollars << "." << setw(2) << setfill('0') << rem;
    return oss.str();
}

// ---------------- Simple hash (demo only) ----------------
static size_t hashPin(const string &pin, size_t salt) {
    // WARNING: This is NOT cryptographically secure; for learning purposes only.
    // Combines std::hash with a per-account salt.
    return std::hash<string>{}(pin + to_string(salt)) ^ (salt << 1);
}

// ---------------- Account class ----------------
class Account {
    int id_;
    string owner_;
    long long balanceCents_ = 0;  // store as cents
    size_t salt_ = 0;
    size_t pinHash_ = 0;
public:
    friend class bank;

    Account(int id, string owner, const string &pin)
        : id_(id), owner_(std::move(owner)) {
        // random-ish salt
        random_device rd; mt19937_64 gen(rd()); uniform_int_distribution<size_t> dist;
        salt_ = dist(gen);
        setPin(pin);
    }

    int id() const { return id_; }
    const string& owner() const { return owner_; }
    long long balanceCents() const { return balanceCents_; }

    bool verifyPin(const string &pin) const { return hashPin(pin, salt_) == pinHash_; }

    void setPin(const string &pin) {
        if (pin.size() < 4 || pin.size() > 12 || !all_of(pin.begin(), pin.end(), ::isdigit))
            throw invalid_argument("PIN must be 4-12 digits");
        pinHash_ = hashPin(pin, salt_);
    }

    void deposit(long long cents) {
        if (cents <= 0) throw invalid_argument("Deposit must be positive");
        balanceCents_ += cents;
    }

    void withdraw(long long cents) {
        if (cents <= 0) throw invalid_argument("Withdrawal must be positive");
        if (cents > balanceCents_) throw runtime_error("Insufficient funds");
        balanceCents_ -= cents;
    }
};

// ---------------- Bank class ----------------
class Bank {
    vector<Account> accounts_;
    int nextId_ = 1001; // simple incremental IDs
public:
    int createAccount(const string &owner, const string &pin) {
        accounts_.emplace_back(nextId_, owner, pin);
        return nextId_++;
    }

    Account* findById(int id) {
        for (auto &a : accounts_) if (a.id() == id) return &a;
        return nullptr;
    }

    Account* login(int id, const string &pin) {
        Account* acc = findById(id);
        if (!acc) return nullptr;
        if (!acc->verifyPin(pin)) return nullptr;
        return acc;
    }

    void listAccounts() const {
        cout << "\n=== Accounts (for demo) ===\n";
        for (auto &a : accounts_) {
            cout << "ID: " << a.id() << ", Owner: " << a.owner() << ", Balance: " << formatCents(a.balanceCents()) << "\n";
        }
        if (accounts_.empty()) cout << "(none)\n";
    }

    bool saveToFile(const std::string& path) const {
        std::ofstream out(path, std::ios::trunc);
        if (!out) return false;
        for (const auto& a: accounts_){
          string owner = a.owner();
          replace(owner.begin(), owner.end(), '\t', ' ');



        }
    };
    bool loadFromFile(const std::string& path) {
        ifstream in(path);
        if (!in) return false;
        
        accounts_.clear();
        int maxId = 1000;
        string line;
        while (getline(in, line)) {

        }
        nextId_ = maxId + 1;
        return true;
    };
};

// ---------------- CLI helpers ----------------
static string prompt(const string &msg) { cout << msg; cout.flush(); string s; getline(cin, s); return s; }
static int promptInt(const string &msg) { while (true) { cout << msg; cout.flush(); string s; getline(cin, s); try { return stoi(s); } catch (...) { cout << "Invalid number. Try again.\n"; } } }

static long long promptAmountCents(const string &msg) { while (true) { string s = prompt(msg); try { return parseAmountCents(s); } catch (const exception &e) { cout << "Invalid amount: " << e.what() << ". Try again.\n"; } } }

// ---------------- Main menu ----------------
static void accountSession(Account* acc) {
    while (true) {
        cout << "\n[Account " << acc->id() << "] Options:\n"
             << " 1) Check balance\n"
             << " 2) Deposit\n"
             << " 3) Withdraw\n"
             << " 4) Logout\n";
        int ch = promptInt("Choose: ");
        try {
            if (ch == 1) {
                cout << "Balance: " << formatCents(acc->balanceCents()) << "\n";
            } else if (ch == 2) {
                long long cents = promptAmountCents("Amount to deposit (e.g., 100 or 12.34): ");
                acc->deposit(cents);
                cout << "Deposited. New balance: " << formatCents(acc->balanceCents()) << "\n";
            } else if (ch == 3) {
                long long cents = promptAmountCents("Amount to withdraw: ");
                acc->withdraw(cents);
                cout << "Withdrawn. New balance: " << formatCents(acc->balanceCents()) << "\n";
            } else if (ch == 4) {
                cout << "Logging out...\n"; break;
            } else {
                cout << "Invalid option.\n";
            }
        } catch (const exception &e) {
            cout << "Error: " << e.what() << "\n";
        }
    }
}

int main() {
    ios::sync_with_stdio(false); cin.tie(nullptr);
    Bank bank;
    const string DB = "accounts.tsv";
    bank.loadFromFile(DB);


    cout << "=== Bank Account Simulator ===\n";
    while (true) {
        cout << "\nMain Menu:\n"
             << " 1) Create account\n"
             << " 2) Login\n"
             << " 3) List accounts (demo)\n"
             << " 4) Exit\n";
        int choice = promptInt("Choose: ");
        if (choice == 1) {
            string name = prompt("Owner name: ");
            string pin = prompt("Choose PIN (4-12 digits): ");
            try {
                int id = bank.createAccount(name, pin);
                cout << "Account created! Your ID is: " << id << "\n";
            } catch (const exception &e) {
                cout << "Failed to create account: " << e.what() << "\n";
            }
        } else if (choice == 2) {
            int id = promptInt("Account ID: ");
            string pin = prompt("PIN: ");
            Account* acc = bank.login(id, pin);
            if (!acc) { cout << "Login failed. Check ID/PIN.\n"; continue; }
            accountSession(acc);
        } else if (choice == 3) {
            bank.listAccounts();
        } else if (choice == 4) {
            bank.saveToFile(DB);
            cout << "Goodbye!\n"; break;
        } else {
            cout << "Invalid choice.\n";
        }
    }
}
