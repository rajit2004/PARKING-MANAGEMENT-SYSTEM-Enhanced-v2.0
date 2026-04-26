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
