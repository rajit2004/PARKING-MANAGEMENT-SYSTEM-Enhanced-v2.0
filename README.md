# 🅿️ Parking Management System v2.0

> A complete C++17 rebuild of a parking management system — featuring role-based Admin/Staff login, dynamic rush-hour pricing, regex-based number plate validation, persistent file storage, and fast O(1) token lookups using unordered_map.

[![C++](https://img.shields.io/badge/C++-17-blue?style=flat&logo=cplusplus)](https://isocpp.org/)
[![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey?style=flat&logo=windows)](https://github.com/rajit2004)
[![GitHub Sponsors](https://img.shields.io/github/sponsors/rajit2004?style=flat&logo=githubsponsors&color=EA4AAA)](https://github.com/sponsors/rajit2004)
[![Version](https://img.shields.io/badge/Version-2.0-brightgreen?style=flat)](https://github.com/rajit2004)

---

## 📌 About This Project

This is a **complete ground-up rebuild** of [ModedRepo_ParkingSystem](https://github.com/rajit2004/ModedRepo_ParkingSystem), which was itself a fork of an older project.

Version 2.0 is not just an upgrade — it's a full rewrite with a cleaner architecture, new features, and proper software engineering practices applied throughout.

**What's new in v2.0:**
- ⚡ O(1) token lookups using `unordered_map` (was a slow linear scan)
- 🔑 Passwords now persist across sessions via `config.txt`
- 🔍 Search by vehicle number plate — not just token
- 🅿️ Capacity tracking with real-time available slot count
- 🛡️ Password confirmation on change — prevents accidental mismatches
- 📋 Revenue summary broken down by Cars and Bikes in reports
- 🧱 Cleaner OOP — input logic moved into `Time` and `Date` classes
- 🐛 Multiple bug fixes from v1 (lockout counter, empty password, stream errors)

---
## 📸 Screenshots

| | |
|--|--|
| **Main Menu** | **Login & Admin Dashboard** |
| ![Main Menu](Screenshots/Screenshot%20(34).png) | ![Login & Dashboard](Screenshots/Screenshot%20(35).png) |
| **Search by Token** | **Vehicle Checkout** |
| ![Search by Token](Screenshots/Screenshot%20(36).png) | ![Checkout](Screenshots/Screenshot%20(37).png) |
---

## 🚀 Features

- 🔐 **Role-Based Access** — Admin and Staff login with masked passwords and security lockout
- 🚗 **Vehicle Management** — Add Cars/Bikes with auto-generated tokens, duplicate plate detection
- 🔍 **Dual Search** — Find vehicles by token number or number plate
- 💰 **Smart Fare Calculation** — Hour-by-hour calculation with automatic rush-hour pricing
- 💾 **Persistent Storage** — All data auto-saved and reloaded between sessions
- 🅿️ **Capacity Tracking** — Real-time slot availability shown when adding vehicles
- 📊 **Full Reports** — Complete vehicle list with revenue summary, car/bike breakdown
- ✅ **Input Validation** — Leap-year dates, 24-hour times, numeric guards, plate format check

---

## 🧠 How It Works

```
User logs in (Admin / Staff)
        ↓
Add vehicle → plate validated → token auto-generated
        ↓
System tracks entry time and vehicle type
        ↓
On checkout → fare calculated hour by hour
        ↓
Rush-hour windows applied per hour automatically
        ↓
Data saved to file → survives program restarts
        ↓
Reports & revenue summary available at any time
```

---

## 💰 Pricing Model

| Vehicle | Normal (per hour) | Rush Hour (per hour) |
|---------|:-----------------:|:--------------------:|
| Bike    | Rs 10             | Rs 15                |
| Car     | Rs 20             | Rs 30                |

**Rush Hours:** `09:00 – 11:00` and `17:00 – 20:00`

> Fare is calculated hour-by-hour from entry time — each hour is checked individually against rush windows for accurate pricing.

---

## 🛠️ Tech Stack

| Layer | Technology |
|-------|------------|
| Language | C++17 |
| Lookup | `unordered_map` for O(1) token access |
| Data Structures | STL (`vector`, `algorithm`, `sstream`) |
| Validation | `<regex>` for number plate format |
| Storage | `<fstream>` for persistent file handling |
| UI | Windows Console API (`_getch`, `Sleep`, `system("cls")`) |

---

## 🧱 OOP Architecture

**5 dedicated classes:**

| Class | Responsibility |
|-------|---------------|
| `Time` | Clock time — validation, formatting, user input |
| `Date` | Calendar date — leap year handling, user input |
| `Vehicle` | All vehicle data — fare calc, serialisation, display |
| `SecuritySystem` | Login, lockout, password change, file persistence |
| `ParkingSystem` | Core engine — add, search, checkout, report, file I/O |

---

## 🆚 v2.0 vs v1.0

| Feature | v1.0 | v2.0 |
|---------|------|------|
| Token lookup | O(n) linear scan | O(1) via `unordered_map` |
| Password storage | In-memory only | Persisted to `config.txt` |
| Search options | Token only | Token + number plate |
| Capacity tracking | None | Real-time slot counter |
| Password change | No confirmation | Must confirm new password |
| Revenue report | Total only | Cars + Bikes breakdown |
| Input validation | Partial | Full across all entry points |
| Lockout counter | Off-by-one bug | Fixed |
| Empty password | Allowed | Blocked |

---

## 📂 Project Structure

```
ParkingSystem-v2/
├── main.cpp            # Complete application source (single file)
├── Screenshots/        # App screenshots
├── .gitignore
├── parking_data.txt    # Auto-generated — stores parked vehicles
├── config.txt          # Auto-generated — stores login passwords
└── README.md
```

> `parking_data.txt` and `config.txt` are created automatically on first run.

---

## ⚡ Getting Started

### Prerequisites
- Windows OS
- g++ compiler (MinGW) or MSVC

### Compile & Run

```bash
g++ main.cpp -o parking_system.exe -std=c++17
./parking_system.exe
```

Or run the included `parking_system.exe` directly on Windows — no setup needed.

### Default Login Credentials

| Role  | Username | Password |
|-------|----------|----------|
| Admin | admin    | 1234     |
| Staff | staff    | 0000     |

> ⚠️ Change both passwords after first login using the built-in **Change Password** option on the main menu.

---

## 💖 Support This Project

This project is completely free and open source. If you found it useful or want to support more projects like this:

[![Sponsor](https://img.shields.io/badge/Sponsor-%E2%9D%A4-pink?style=for-the-badge&logo=githubsponsors)](https://github.com/sponsors/rajit2004)

---

## 👨‍💻 Author

**Ranesh Rajit** — B.Tech CS Student, India

[![GitHub](https://img.shields.io/badge/GitHub-rajit2004-black?style=flat&logo=github)](https://github.com/rajit2004)
[![LinkedIn](https://img.shields.io/badge/LinkedIn-ranesh--kun-blue?style=flat&logo=linkedin)](https://linkedin.com/in/ranesh-kun)

---

> *v1.0 available at [ModedRepo_ParkingSystem](https://github.com/rajit2004/ModedRepo_ParkingSystem) — originally forked from [Rishav123918](https://github.com/Rishav123918/Parking_Application_C-).*