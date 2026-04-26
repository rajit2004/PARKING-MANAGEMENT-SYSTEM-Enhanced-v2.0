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

// ============================================================
//  CLASS: Vehicle
// ============================================================
class Vehicle {
    int    token;
    string owner;
    string vno;
    Time   inT, outT;
    Date   date;
    int    durationHours;
    bool   isCar;

public:
    // New vehicle (minimal constructor — call inputDetails() after)
    Vehicle(int t, bool car) : token(t), durationHours(0), isCar(car) {}

    // Reconstruct from saved file data
    Vehicle(int t, string o, string v, bool car,
            int h1, int m1, int s1,
            int h2, int m2, int s2,
            int dd, int mm, int yy)
        : token(t), owner(move(o)), vno(move(v)),
          durationHours(0), isCar(car)
    {
        inT  = {h1, m1, s1};
        outT = {h2, m2, s2};
        date = {dd, mm, yy};
        calcDuration();
    }

    // ---- Accessors ----
    int    getToken()  const { return token; }
    string getVNo()    const { return vno;   }
    bool   getIsCar()  const { return isCar; }
    void   setVNo(const string& v) { vno = v; }

    // ---- Rush hour check ----
    static bool isRushHour(int h) {
        return (h >= 9 && h <= 11) || (h >= 17 && h <= 20);
    }

    // ---- Duration calculation ----
    void calcDuration() {
        long diff = outT.toSeconds() - inT.toSeconds();
        if (diff < 0) diff += 24L * 3600;
        durationHours = max(1, static_cast<int>((diff + 3599) / 3600));
    }

    // ---- Fare calculation using rate table ----
    int getFare() const {
        int fare = 0;
        int hour = inT.hh;
        for (int i = 0; i < durationHours; i++) {
            bool rush = isRushHour(hour);
            if (isCar) fare += rush ? CAR_RUSH  : CAR_NORMAL;
            else       fare += rush ? BIKE_RUSH : BIKE_NORMAL;
            if (++hour >= 24) hour = 0;
        }
        return fare;
    }

    // ---- Serialise to file line ----
    string toFileLine() const {
        ostringstream os;
        os << token << "|" << owner << "|" << vno << "|" << isCar << "|"
           << inT.hh  << " " << inT.mm  << " " << inT.ss  << "|"
           << outT.hh << " " << outT.mm << " " << outT.ss << "|"
           << date.dd << " " << date.mm << " " << date.yy;
        return os.str();
    }

    // ---- Interactive input ----
    void inputDetails() {
        cout << "Owner Name : ";
        getline(cin, owner);
        if (owner.empty()) owner = "Unknown";

        inT  = Time::readFromUser("Arrival Time");
        outT = Time::readFromUser("Exit Time   ");
        date = Date::readFromUser();
        calcDuration();
    }

    // ---- Display ----
    void display() const {
        printDivider();
        cout << " Token      : " << token                        << "\n"
             << " Vehicle No : " << vno                          << "\n"
             << " Type       : " << (isCar ? "Car" : "Bike")    << "\n"
             << " Owner      : " << owner                        << "\n"
             << " Date       : " << date.toString()              << "\n"
             << " In / Out   : " << inT.toString()
                         << "  ->  " << outT.toString()          << "\n"
             << " Duration   : " << durationHours << " hour(s)"  << "\n"
             << " Total Fare : Rs " << getFare()                 << "\n";
        printDivider();
    }
};

// ============================================================
//  CLASS: SecuritySystem  (passwords persist to config.txt)
// ============================================================
class SecuritySystem {
    string adminPass = "1234";
    string staffPass = "0000";

    void loadPasswords() {
        ifstream f(CONFIG_FILE);
        if (!f) return;
        string a, s;
        if (getline(f, a) && getline(f, s)) {
            adminPass = a;
            staffPass = s;
        }
    }

    void savePasswords() const {
        ofstream f(CONFIG_FILE);
        if (f) { f << adminPass << "\n" << staffPass << "\n"; }
    }

public:
    SecuritySystem() { loadPasswords(); }

    // Returns 1 = Admin, 2 = Staff; exits on lockout
    int login() const {
        while (true) {
            printHeader("SYSTEM LOGIN");
            cout << "Username (admin / staff): ";
            string user;
            getline(cin, user);
            user = toUpperStr(user);

            if (user != "ADMIN" && user != "STAFF") {
                cout << "\n[!] Invalid username. Enter 'admin' or 'staff'.\n";
                pauseScreen();
                continue;
            }

            for (int attempt = 1; attempt <= MAX_LOGIN_TRIES; attempt++) {
                printHeader("SYSTEM LOGIN");
                cout << "Username : " << user << "\n";
                cout << "Password : ";
                string pass = getMaskedInput();

                bool ok = (user == "ADMIN" && pass == adminPass)
                       || (user == "STAFF" && pass == staffPass);
                if (ok) return (user == "ADMIN") ? 1 : 2;

                int left = MAX_LOGIN_TRIES - attempt;
                if (left == 0) {
                    cout << "\n[!] Security lockout — too many failed attempts. Exiting.\n";
                    Sleep(1500);
                    exit(0);
                }
                cout << "\n[!] Wrong password. " << left << " attempt(s) remaining.\n";
                pauseScreen();
            }
        }
    }

    void changePassword() {
        while (true) {
            printHeader("CHANGE PASSWORD");
            cout << "Username (admin / staff) or '0' to go back: ";
            string user;
            getline(cin, user);
            if (user == "0") return;
            user = toUpperStr(user);

            if (user != "ADMIN" && user != "STAFF") {
                cout << "\n[!] Invalid username.\n";
                pauseScreen();
                continue;
            }

            for (int attempt = 1; attempt <= MAX_LOGIN_TRIES; attempt++) {
                cout << "Current password: ";
                string oldP = getMaskedInput();

                bool ok = (user == "ADMIN" && oldP == adminPass)
                       || (user == "STAFF" && oldP == staffPass);
                if (ok) {
                    string newP;
                    while (newP.empty()) {
                        cout << "New password (cannot be empty): ";
                        newP = getMaskedInput();
                    }
                    string confirm;
                    cout << "Confirm new password: ";
                    confirm = getMaskedInput();
                    if (newP != confirm) {
                        cout << "\n[!] Passwords do not match. Try again.\n";
                        pauseScreen();
                        return;
                    }
                    if (user == "ADMIN") adminPass = newP;
                    else                 staffPass = newP;
                    savePasswords();
                    cout << "\n[+] Password changed and saved successfully!\n";
                    pauseScreen();
                    return;
                }

                int left = MAX_LOGIN_TRIES - attempt;
                if (left == 0) {
                    cout << "\n[!] Too many wrong attempts. Exiting.\n";
                    Sleep(1500);
                    exit(0);
                }
                cout << "[!] Wrong password. " << left << " attempt(s) remaining.\n";
            }
        }
    }
};

