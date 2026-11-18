# Linux System Health and Resource Monitoring Manager

An interactive Bash application that monitors CPU, memory, disk, processes and uptime, generates health reports, and keeps an activity log of everything the user does.

> **Platform:** Linux only. It relies on GNU/Linux tools (`/proc/stat`, `free`, `ps --sort`, `df --output`, `lscpu`, `uptime -p`). On macOS these are missing or behave differently, so run it in a Linux terminal, VM, WSL, or Docker container.

---

## 1. Setup — make every script executable (required)

Every script in the project folder must be executable, otherwise the menu cannot launch it. After copying the files, run this **once**, from inside the project folder:

```bash
chmod +x *.sh
```

Then start the application:

```bash
./main.sh
```
You can confirm the permissions with `ls -l`: every `.sh` file should show `-rwxr-xr-x`.

---

## 2. Project structure

| File | Purpose |
|---|---|
| `main.sh` | Interactive menu (options 1–10); validates input and launches the other scripts |
| `common.sh` | Shared functions sourced by every script (logging, validation, confirmation, CPU/memory helpers) |
| `system_info.sh` | Hostname, OS, kernel, uptime, current date/time |
| `cpu_monitor.sh` | Live CPU usage, top CPU processes, user-defined threshold warnings |
| `memory_monitor.sh` | Total/used/available memory, utilization %, threshold warnings |
| `disk_monitor.sh` | Per-filesystem usage, status flags (OK / APPROACHING / WARNING) |
| `process_management.sh` | `view` (list + search) and `terminate` (with protection and confirmation) |
| `health_report.sh` | Timestamped report saved to a user-selected directory |
| `activity_history.sh` | Displays the activity log |
| `clear_activity.sh` | Clears the activity log after confirmation |
| `activity.log` | Created automatically on first run |

---

## 3. Menu

```
====== Linux System Health and Resource Monitoring Manager ========
1. Display system information.
2. Monitor CPU utilization.
3. Monitor memory utilization.
4. Monitor disk utilization.
5. View and search running processes.
6. Terminate a process.
7. Generate a system health report.
8. View activity history.
9. Clear activity log.
10. Exit the application.
```

The menu loops until option 10 is chosen. Monitors (options 2–4) refresh every 5 seconds; press **Ctrl+C** to stop a monitor and return to the menu.

---

## 4. Functions

| Function | File | What it does |
|---|---|---|
| `timestamp` | common.sh | Returns the current date/time in a consistent format |
| `log_event LEVEL msg` | common.sh | Appends a line to `activity.log` (INFO / WARNING / ERROR) |
| `require_cmd` | common.sh | Checks that required utilities exist; prints a clear error if not |
| `is_valid_threshold` | common.sh | Accepts whole numbers 1–100 only |
| `read_threshold` | common.sh | Prompts repeatedly until a valid threshold is entered |
| `confirm` | common.sh | y/n confirmation prompt |
| `exceeds A B` | common.sh | Decimal-safe "A > B" test (uses `awk`, no `bc` needed) |
| `get_cpu_usage` | common.sh | Measures CPU busy % over 1 second from `/proc/stat` |
| `top_cpu_processes` | common.sh | Header + 5 heaviest processes by CPU |
| `get_memory_stats` | common.sh | Sets total/used/available (MB) and utilization % |
| `show_system_info` | system_info.sh | Prints the system summary |
| `monitor_cpu` / `monitor_memory` / `monitor_disk` | monitor scripts | Main monitoring loops |
| `check_disks` | disk_monitor.sh | Evaluates every real filesystem against the threshold |
| `list_processes` / `search_process` | process_management.sh | Lists top processes; searches by name or PID |
| `protection_reason` | process_management.sh | Decides whether a process is protected |
| `terminate_process` | process_management.sh | Validated, confirmed termination |
| `choose_directory` / `write_report` / `generate_report` | health_report.sh | Report directory validation and generation |
| `view_history` / `clear_log` | history / clear scripts | View or clear the log |
| `run_script`, `print_menu`, `main` | main.sh | Menu dispatch with file and permission checks |

---

## 5. Commands used

| Command | Used for |
|---|---|
| `/proc/stat` + `awk` | CPU utilization (two samples one second apart) |
| `ps -eo ... --sort=-%cpu` | Process lists, top CPU consumers, process details |
| `pgrep -x` | Exact process-name lookup |
| `free -m`, `free -h` | Memory statistics |
| `df -h --output=...` | Disk usage per filesystem (virtual filesystems excluded) |
| `uptime`, `uptime -p` | Load average and human-readable uptime |
| `uname`, `hostname`, `/etc/os-release` | Kernel, hostname, distribution name |
| `lscpu` | CPU model and core details for the report |
| `kill`, `kill -9` | Graceful (SIGTERM) and forced (SIGKILL) termination |
| `ps -p PID` | Checks that a PID exists before acting on it |
| `date`, `>>` | Timestamps and appending to the log |
| `awk`, `sed`, `grep`, `tr`, `xargs`, `paste`, `tail`, `less` | Text processing and display |

---

## 6. Error handling

| Situation | Handling |
|---|---|
| Invalid menu choice (letters, 0, 11, empty) | Error message, menu is shown again |
| Invalid threshold (text, 0, >100) | Re-prompts until a whole number from 1 to 100 is given |
| Missing utility (`free`, `ps`, `df`, ...) | `require_cmd` prints which command is missing and the task is aborted gracefully |
| Not a Linux system (`/proc/stat` missing) | Clear error message instead of a crash |
| Script missing or not executable | `main.sh` reports it and suggests `chmod +x *` |
| Non-numeric or non-existent PID | Error message, warning written to the log |
| No process with the given name | "No process found" message, logged |
| Protected process (PID ≤ 2, kernel threads, `systemd`, `init`, `sshd`, `dbus-daemon`, `login`, `Xorg`, `gdm`, `lightdm`, `NetworkManager`, this script and its parent) | Termination refused with the reason, logged |
| Process owned by root/another user without privileges | Termination refused (permission check before sending a signal) |
| Terminate / clear log | Always asks for y/n confirmation first |
| Process ignores SIGTERM | After 2 seconds the user is offered a confirmed SIGKILL |
| Report directory missing | Offers to create it |
| Report directory not writable / cannot be created | Error message, logged, no report written |
| Log file not writable | Error message printed to stderr |
| Ctrl+C during monitoring | Monitor stops cleanly, logs the stop, and the menu continues |
| Ctrl+D / end of input | Application exits cleanly |

---

## 7. How utilization is calculated

- **CPU %** = (total time delta − idle time delta) ÷ total time delta × 100, measured over 1 second from `/proc/stat`.
- **Memory %** = used ÷ total × 100, using `free -m` (the *used* column excludes reclaimable cache on modern systems; *available* is shown separately).
- **Disk %** = the `pcent` column of `df`. A filesystem is flagged **WARNING** above the threshold and **APPROACHING** when within 10 percentage points of it.

---

## 8. Test plan and evidence (screenshots to capture)

Run on Linux, after `chmod +x *`, from `./main.sh`:

| # | Deliverable | What to do | Expected result |
|---|---|---|---|
| 1 | System info | Option 1 | Hostname, OS, kernel, uptime, date/time |
| 2 | CPU monitoring | Option 2 → threshold `1` → wait → Ctrl+C | Usage %, top 5 processes, WARNING line |
| 3 | Memory monitoring | Option 3 → threshold `1` → Ctrl+C | Total/used/available, utilization %, WARNING |
| 4 | Disk monitoring | Option 4 → threshold `30` → Ctrl+C | Table with Status column, WARNING row |
| 5 | Invalid input | Enter `abc`, `11`; threshold `0`, `150` | Rejected with messages |
| 6 | Process search | Option 5 → search `bash`, then a PID, then `99999999` | Matches shown; "not found" for the bad PID |
| 7 | Process termination | In another terminal run `sleep 300 &` and note its PID → option 6 → PID → `y` | Process terminated, logged |
| 8 | Protected process | Option 6 → `1` | Refused: core system process |
| 9 | Cancel termination | Option 6 → valid PID → `n` | Cancelled, logged |
| 10 | Health report | Option 7 → a new directory → `y` | Report saved with timestamp |
| 11 | Bad directory | Option 7 → `/proc/nope` | Error message |
| 12 | Activity log | Option 8 | Log entries displayed |
| 13 | Clear log | Option 9 → `n`, then option 9 → `y` | Cancelled, then cleared |
| 14 | Permission check | `chmod -x system_info.sh`, choose option 1 | Error suggesting `chmod +x *` |

> Tip: to see a disk WARNING on a healthy system, set the disk threshold low (for example 5 or 10).

---

## 9. Project requirements coverage

| Requirement | Where it is met |
|---|---|
| 1. System information | `system_info.sh` |
| 2. CPU monitoring with threshold | `cpu_monitor.sh` |
| 3. Memory monitoring with threshold | `memory_monitor.sh` |
| 4. Disk monitoring with threshold | `disk_monitor.sh` |
| 5. Process management (search, terminate, protection, confirmation) | `process_management.sh` |
| 6. Health report in a chosen directory with timestamp | `health_report.sh` |
| 7. Activity logging, view, clear with confirmation | `common.sh` (`log_event`), `activity_history.sh`, `clear_activity.sh` |
| 8. Interactive menu | `main.sh` |
| 9. Validation and error handling | Section 6 above |
| 10. Code organization | Shared functions in `common.sh`, one function per operation, commented code |
