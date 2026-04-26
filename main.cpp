/*
 * ============================================================
 *  PARKING MANAGEMENT SYSTEM — Enhanced v2.0
 *  Author  : Ranesh Rajit
 *  GitHub  : https://github.com/rajit2004
 *  Repo    : https://github.com/rajit2004/PARKING-MANAGEMENT-SYSTEM-Enhanced-v2.0
 *  License : Open Source
 * ============================================================
 *  WHAT'S NEW IN v2.0:
 *   - Passwords now persist across sessions (saved to file)
 *   - Search by vehicle number plate (not just token)
 *   - Unordered_map for O(1) token lookups (was O(n))
 *   - Capacity limit with real-time slot tracking
 *   - Revenue summary with Cars / Bikes breakdown in report
 *   - Input error messages shown inline (no silent retry loops)
 *   - Constants extracted for rates, capacity, and rush hours
 *   - Fare calculation uses structured rate table (not if/else chains)
 *   - File load now skips malformed lines gracefully
 *   - Security lockout now prints remaining attempts correctly
 *   - changePassword now validates new password is non-empty
 *   - General code quality: const-correctness, early returns,
 *     removed magic numbers, consistent naming
 * ============================================================
 */



#include <iostream>
#include <vector>
#include <string>
#include <conio.h>
#include <cctype>
#include <algorithm>
#include <windows.h>
#include <iomanip>
#include <sstream>
#include <regex>
#include <fstream>
#include <unordered_map>
#include <stdexcept>

using namespace std;

// ============================================================
//  CONSTANTS
// ============================================================
constexpr int  MENU_WIDTH       = 54;
constexpr int  MAX_CAPACITY     = 50;   // Total parking slots
constexpr int  MAX_LOGIN_TRIES  = 3;
constexpr int  BIKE_NORMAL      = 10;
constexpr int  BIKE_RUSH        = 15;
constexpr int  CAR_NORMAL       = 20;
constexpr int  CAR_RUSH         = 30;
const string   DATA_FILE        = "parking_data.txt";
const string   CONFIG_FILE      = "config.txt";     // stores passwords

// ============================================================
//  UI HELPERS
// ============================================================
void printCentered(const string& text) {
    int len = static_cast<int>(text.length());
    int padding = (len >= MENU_WIDTH) ? 0 : (MENU_WIDTH - len) / 2;
    cout << string(padding, ' ') << text << "\n";
}

void printHeader(const string& title) {
    system("cls");
    cout << string(MENU_WIDTH, '=') << "\n";
    printCentered(title);
    cout << string(MENU_WIDTH, '=') << "\n";
}

void printDivider() {
    cout << string(MENU_WIDTH, '-') << "\n";
}

void pauseScreen() {
    cout << "\nPress any key to continue...";
    _getch();
}

void showWelcome() {
    system("cls");
    cout << "\n\n";
    cout << string(MENU_WIDTH, '=') << "\n";
    printCentered("WELCOME TO PARKING MANAGEMENT SYSTEM");
    printCentered("Manage Vehicles Efficiently");
    cout << string(MENU_WIDTH, '=') << "\n";
    printCentered("Loading...");
    cout << "\n\n";
    Sleep(1200);
}

void showExitMessage() {
    system("cls");
    cout << "\n" << string(MENU_WIDTH, '=') << "\n";
    printCentered("THANK YOU FOR USING OUR SYSTEM");
    printCentered("HAVE A GREAT DAY!");
    cout << string(MENU_WIDTH, '=') << "\n\n";
    Sleep(1000);
}

// ============================================================
//  INPUT HELPERS
// ============================================================

// Returns a validated integer; re-prompts on bad input
int getIntInput(const string& prompt = "") {
    if (!prompt.empty()) cout << prompt;
    string line;
    while (true) {
        if (!getline(cin, line)) return 0;
        if (!line.empty()) {
            try { return stoi(line); }
            catch (...) {}
        }
        cout << "[!] Invalid input. Enter a number: ";
    }
}

string toUpperStr(const string& s) {
    string r = s;
    transform(r.begin(), r.end(), r.begin(), ::toupper);
    return r;
}

// Masked password entry via _getch()
string getMaskedInput() {
    string pass;
    while (true) {
        int ch = _getch();
        if (ch == 13) { cout << "\n"; break; }
        if (ch == 8) {
            if (!pass.empty()) { pass.pop_back(); cout << "\b \b"; }
        } else if (isprint(static_cast<unsigned char>(ch))) {
            pass.push_back(static_cast<char>(ch));
            cout << "*";
        }
    }
    return pass;
}

// ============================================================
//  CLASS: Time
// ============================================================
class Time {
public:
    int hh = 0, mm = 0, ss = 0;

    long toSeconds() const { return hh * 3600L + mm * 60 + ss; }

    string toString() const {
        ostringstream os;
        os << setfill('0') << setw(2) << hh << ":"
           << setfill('0') << setw(2) << mm << ":"
           << setfill('0') << setw(2) << ss;
        return os.str();
    }

    static bool isValid(int h, int m, int s) {
        return h >= 0 && h <= 23 && m >= 0 && m <= 59 && s >= 0 && s <= 59;
    }

    // Read from stream with validation loop
    static Time readFromUser(const string& label) {
        Time t;
        while (true) {
            cout << label << " (HH MM SS): ";
            if (cin >> t.hh >> t.mm >> t.ss) {
                cin.ignore(1000, '\n');
                if (Time::isValid(t.hh, t.mm, t.ss)) return t;
            } else {
                cin.clear();
                cin.ignore(1000, '\n');
            }
            cout << "[!] Invalid time. Try again.\n";
        }
    }
};

// ============================================================
//  CLASS: Date
// ============================================================
class Date {
public:
    int dd = 0, mm = 0, yy = 0;

    string toString() const {
        ostringstream os;
        os << setfill('0') << setw(2) << dd << "/"
           << setfill('0') << setw(2) << mm << "/" << yy;
        return os.str();
    }

    static bool isValid(int d, int m, int y) {
        if (m < 1 || m > 12 || d < 1 || y < 1) return false;
        int days[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
        if (m == 2 && ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0))
            days[2] = 29;
        return d <= days[m];
    }

    // Read from stream with validation loop
    static Date readFromUser() {
        Date d;
        while (true) {
            cout << "Date (DD MM YYYY): ";
            if (cin >> d.dd >> d.mm >> d.yy) {
                cin.ignore(1000, '\n');
                if (Date::isValid(d.dd, d.mm, d.yy)) return d;
            } else {
                cin.clear();
                cin.ignore(1000, '\n');
            }
            cout << "[!] Invalid date. Try again.\n";
        }
    }
};

