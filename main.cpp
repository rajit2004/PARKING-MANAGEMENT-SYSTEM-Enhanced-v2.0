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

// ============================================================
//  CLASS: ParkingSystem
// ============================================================
class ParkingSystem {
    vector<Vehicle>            vehicles;
    unordered_map<int, size_t> tokenIndex;  // token -> index in vehicles
    int nextToken = 100;

    // ---- Helpers ----
    bool isDuplicate(const string& vno) const {
        return any_of(vehicles.begin(), vehicles.end(),
            [&](const Vehicle& v){ return v.getVNo() == vno; });
    }

    static bool isValidPlate(const string& plate) {
        // Indian number plate: e.g. MH12 AB 1234
        static const regex pattern(R"(^[A-Z]{2}[0-9]{2} [A-Z]{1,3} [0-9]{1,7}$)");
        return regex_match(plate, pattern);
    }

    void rebuildIndex() {
        tokenIndex.clear();
        for (size_t i = 0; i < vehicles.size(); i++)
            tokenIndex[vehicles[i].getToken()] = i;
    }

    // ---- Persistence ----
    void saveToFile() const {
        ofstream f(DATA_FILE);
        if (!f) { cerr << "[!] Could not save data.\n"; return; }
        for (const auto& v : vehicles)
            f << v.toFileLine() << "\n";
    }

    void loadFromFile() {
        ifstream f(DATA_FILE);
        if (!f) return;
        string line;
        int lineNo = 0;
        while (getline(f, line)) {
            lineNo++;
            if (line.empty()) continue;
            istringstream ss(line);
            string seg;
            vector<string> data;
            while (getline(ss, seg, '|')) data.push_back(seg);
            if (data.size() != 7) {
                cerr << "[!] Skipping malformed line " << lineNo << " in data file.\n";
                continue;
            }
            try {
                int    t  = stoi(data[0]);
                string o  = data[1];
                string v  = data[2];
                bool   c  = (data[3] == "1");
                int h1,m1,s1, h2,m2,s2, dd,mm,yy;
                istringstream(data[4]) >> h1 >> m1 >> s1;
                istringstream(data[5]) >> h2 >> m2 >> s2;
                istringstream(data[6]) >> dd >> mm >> yy;
                vehicles.emplace_back(t, o, v, c, h1,m1,s1, h2,m2,s2, dd,mm,yy);
                if (t >= nextToken) nextToken = t + 1;
            } catch (...) {
                cerr << "[!] Parse error on line " << lineNo << " — skipped.\n";
            }
        }
        rebuildIndex();
    }

    // ---- Find vehicle by token (O(1)) ----
    Vehicle* findByToken(int token) {
        auto it = tokenIndex.find(token);
        if (it == tokenIndex.end()) return nullptr;
        return &vehicles[it->second];
    }

    const Vehicle* findByToken(int token) const {
        auto it = tokenIndex.find(token);
        if (it == tokenIndex.end()) return nullptr;
        return &vehicles[it->second];
    }

    // ---- Find vehicle by plate number ----
    const Vehicle* findByPlate(const string& plate) const {
        for (const auto& v : vehicles)
            if (v.getVNo() == plate) return &v;
        return nullptr;
    }

public:
    ParkingSystem() { loadFromFile(); }

    int availableSlots() const {
        return MAX_CAPACITY - static_cast<int>(vehicles.size());
    }

    // ---- Show rates ----
    static void showRates() {
        printHeader("PARKING RATES");
        cout << "  TYPE  |  NORMAL (Per Hr)  |  RUSH HOUR (Per Hr)\n";
        printDivider();
        cout << "  BIKE  |      Rs " << setw(3) << BIKE_NORMAL
             << "        |       Rs " << setw(3) << BIKE_RUSH << "\n";
        cout << "  CAR   |      Rs " << setw(3) << CAR_NORMAL
             << "        |       Rs " << setw(3) << CAR_RUSH  << "\n";
        printDivider();
        cout << "  Rush hours: 09:00-11:00 and 17:00-20:00\n";
        printDivider();
        pauseScreen();
    }

    // ---- Add vehicle ----
    void addVehicle() {
        printHeader("ADD NEW VEHICLE");

        if (availableSlots() <= 0) {
            cout << "\n[!] Parking is FULL. No available slots.\n";
            pauseScreen();
            return;
        }

        cout << "  Available Slots : " << availableSlots()
             << " / " << MAX_CAPACITY << "\n";
        cout << "  Next Token      : " << nextToken << "\n\n";

        cout << "Vehicle type:\n  1. Car\n  2. Bike\n";
        int type = getIntInput("Select: ");
        if (type != 1 && type != 2) {
            cout << "\n[!] Invalid choice.\n";
            pauseScreen();
            return;
        }

        string plate;
        while (true) {
            cout << "Vehicle number (e.g. MH12 AB 1234): ";
            getline(cin, plate);
            plate = toUpperStr(plate);
            if (isValidPlate(plate)) break;
            cout << "[!] Invalid format. Example: MH12 AB 1234\n";
        }

        if (isDuplicate(plate)) {
            cout << "\n[!] This vehicle is already parked.\n";
            pauseScreen();
            return;
        }

        Vehicle v(nextToken, type == 1);
        v.setVNo(plate);
        v.inputDetails();

        tokenIndex[nextToken] = vehicles.size();
        vehicles.push_back(v);
        nextToken++;
        saveToFile();

        cout << "\n[+] Vehicle added. Token: " << v.getToken() << "\n";
        pauseScreen();
    }

    // ---- Search by token ----
    void searchByToken() const {
        printHeader("SEARCH BY TOKEN");
        int t = getIntInput("Enter token: ");
        const Vehicle* v = findByToken(t);
        if (v) { v->display(); }
        else   { cout << "\n[!] No vehicle found with token " << t << ".\n"; }
        pauseScreen();
    }

    // ---- NEW: Search by plate number ----
    void searchByPlate() const {
        printHeader("SEARCH BY VEHICLE NUMBER");
        cout << "Enter vehicle number: ";
        string plate;
        getline(cin, plate);
        plate = toUpperStr(plate);
        const Vehicle* v = findByPlate(plate);
        if (v) { v->display(); }
        else   { cout << "\n[!] No vehicle found with plate " << plate << ".\n"; }
        pauseScreen();
    }

    // ---- Checkout ----
    void checkout() {
        printHeader("VEHICLE CHECKOUT");
        cout << "Enter token(s) separated by spaces: ";
        string line;
        getline(cin, line);

        istringstream ss(line);
        vector<int> tokens;
        int t;
        while (ss >> t) tokens.push_back(t);
        sort(tokens.begin(), tokens.end());
        tokens.erase(unique(tokens.begin(), tokens.end()), tokens.end());

        if (tokens.empty()) {
            cout << "\n[!] No tokens entered.\n";
            pauseScreen();
            return;
        }

        vector<int> valid;
        int grandTotal = 0;
        cout << "\n===== CHECKOUT SUMMARY =====\n";
        for (int tok : tokens) {
            const Vehicle* v = findByToken(tok);
            if (v) {
                v->display();
                grandTotal += v->getFare();
                valid.push_back(tok);
            } else {
                cout << "  Token " << tok << ": not found — skipped.\n";
            }
        }

        if (valid.empty()) {
            cout << "\n[!] No valid tokens found.\n";
            pauseScreen();
            return;
        }

        printDivider();
        cout << " Vehicles   : " << valid.size() << "\n";
        cout << " Grand Total: Rs " << grandTotal << "\n";
        printDivider();
        cout << "Confirm checkout? (y/n): ";
        string confirm;
        getline(cin, confirm);

        if (toUpperStr(confirm) == "Y") {
            vehicles.erase(
                remove_if(vehicles.begin(), vehicles.end(), [&](const Vehicle& v){
                    return find(valid.begin(), valid.end(), v.getToken()) != valid.end();
                }),
                vehicles.end()
            );
            rebuildIndex();
            saveToFile();
            cout << "\n[+] Checkout complete!\n";
        } else {
            cout << "\n[-] Cancelled.\n";
        }
        pauseScreen();
    }

    // ---- Full report ----
    void report() const {
        printHeader("FULL PARKING REPORT");
        if (vehicles.empty()) {
            cout << "\n[i] No vehicles currently parked.\n";
            pauseScreen();
            return;
        }

        int cars = 0, bikes = 0, revenue = 0;
        for (const auto& v : vehicles) {
            v.display();
            if (v.getIsCar()) cars++;
            else              bikes++;
            revenue += v.getFare();
        }

        printDivider();
        cout << " SUMMARY\n";
        printDivider();
        cout << " Cars          : " << cars                            << "\n"
             << " Bikes         : " << bikes                           << "\n"
             << " Total parked  : " << vehicles.size()                 << "\n"
             << " Slots free    : " << availableSlots()
                                    << " / " << MAX_CAPACITY           << "\n"
             << " Total revenue : Rs " << revenue                      << "\n";
        printDivider();
        pauseScreen();
    }
};

// ============================================================
//  DASHBOARD MENUS
// ============================================================
void adminDashboard(ParkingSystem& parking) {
    while (true) {
        printHeader("ADMIN DASHBOARD");
        cout << " 1. Add Vehicle\n"
             << " 2. Search by Token\n"
             << " 3. Search by Vehicle Number\n"
             << " 4. Checkout (Remove Vehicle)\n"
             << " 5. Full Parking Report\n"
             << " 6. View Parking Rates\n"
             << " 7. Logout\n";
        printDivider();
        int c = getIntInput("Choice: ");
        switch (c) {
            case 1: parking.addVehicle();       break;
            case 2: parking.searchByToken();    break;
            case 3: parking.searchByPlate();    break;
            case 4: parking.checkout();         break;
            case 5: parking.report();           break;
            case 6: ParkingSystem::showRates(); break;
            case 7: return;
            default:
                cout << "\n[!] Invalid choice.\n";
                pauseScreen();
        }
    }
}

void staffDashboard(ParkingSystem& parking) {
    while (true) {
        printHeader("STAFF DASHBOARD");
        cout << " 1. Add Vehicle\n"
             << " 2. Search by Token\n"
             << " 3. Search by Vehicle Number\n"
             << " 4. Full Parking Report\n"
             << " 5. View Parking Rates\n"
             << " 6. Logout\n";
        printDivider();
        int c = getIntInput("Choice: ");
        switch (c) {
            case 1: parking.addVehicle();       break;
            case 2: parking.searchByToken();    break;
            case 3: parking.searchByPlate();    break;
            case 4: parking.report();           break;
            case 5: ParkingSystem::showRates(); break;
            case 6: return;
            default:
                cout << "\n[!] Invalid choice.\n";
                pauseScreen();
        }
    }
}


// ============================================================
//  MAIN
// ============================================================
int main() {
    SecuritySystem security;
    ParkingSystem  parking;

    showWelcome();

    while (true) {
        printHeader("MAIN MENU");
        cout << " 1. Login\n"
             << " 2. Change Password\n"
             << " 3. View Parking Rates\n"
             << " 4. Exit\n";
        printDivider();
        int choice = getIntInput("Choice: ");

        switch (choice) {
            case 1: {
                int role = security.login();
                if (role == 1) adminDashboard(parking);
                else           staffDashboard(parking);
                break;
            }
            case 2: security.changePassword();   break;
            case 3: ParkingSystem::showRates();  break;
            case 4: showExitMessage(); return 0;
            default:
                cout << "\n[!] Invalid choice.\n";
                pauseScreen();
        }
    }
}