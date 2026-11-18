#!/bin/bash
# common.sh - shared helpers, sourced by every other script (not run directly).

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG_FILE="${LOG_FILE:-$SCRIPT_DIR/activity.log}"
REFRESH_INTERVAL=5          # seconds between monitoring samples

# Current date/time in a consistent format
timestamp() { date '+%Y-%m-%d %H:%M:%S %Z'; }

# log_event LEVEL message...  -> appends one line to the activity log
log_event() {
    local level="$1"; shift
    if ! echo "$(timestamp) [$level] $*" >> "$LOG_FILE" 2>/dev/null; then
        echo "Error: cannot write to log file $LOG_FILE" >&2
        return 1
    fi
}

# require_cmd cmd1 cmd2 ... -> fails with a clear message if a utility is missing
require_cmd() {
    local missing=0 cmd
    for cmd in "$@"; do
        if ! command -v "$cmd" >/dev/null 2>&1; then
            echo "Error: required command '$cmd' is not installed or not in PATH." >&2
            missing=1
        fi
    done
    return $missing
}

# A valid threshold is a whole number from 1 to 100
is_valid_threshold() { [[ "$1" =~ ^[0-9]{1,3}$ ]] && (( 10#$1 >= 1 && 10#$1 <= 100 )); }

# read_threshold "CPU" -> prompts until valid; result stored in $THRESHOLD
read_threshold() {
    local label="$1" input
    while true; do
        read -r -p "Enter $label utilization threshold percentage (1-100, e.g. 80): " input || { echo; return 1; }
        if is_valid_threshold "$input"; then
            THRESHOLD=$((10#$input))
            return 0
        fi
        echo "Invalid threshold '$input'. Please enter a whole number between 1 and 100."
    done
}

# confirm "Question?" -> returns 0 only for y / yes
confirm() {
    local answer
    read -r -p "$1 (y/n): " answer || return 1
    [[ "$answer" =~ ^[Yy]([Ee][Ss])?$ ]]
}

# exceeds A B -> true when A > B (works with decimals)
exceeds() { awk -v a="$1" -v b="$2" 'BEGIN { exit !(a > b) }'; }

# get_cpu_usage -> prints CPU busy % measured over 1 second from /proc/stat
get_cpu_usage() {
    local cpu u1 n1 s1 i1 w1 q1 sq1 st1 u2 n2 s2 i2 w2 q2 sq2 st2
    if [[ ! -r /proc/stat ]]; then
        echo "Error: /proc/stat is not readable (is this a Linux system?)." >&2
        return 1
    fi
    read -r cpu u1 n1 s1 i1 w1 q1 sq1 st1 _ < /proc/stat
    sleep 1
    read -r cpu u2 n2 s2 i2 w2 q2 sq2 st2 _ < /proc/stat
    awk -v t1="$((u1+n1+s1+i1+w1+q1+sq1+st1))" -v t2="$((u2+n2+s2+i2+w2+q2+sq2+st2))" \
        -v idle1="$((i1+w1))" -v idle2="$((i2+w2))" \
        'BEGIN { dt = t2 - t1; if (dt <= 0) print "0.0"; else printf "%.1f", (dt - (idle2 - idle1)) * 100 / dt }'
}

# top_cpu_processes -> header + 5 heaviest processes by CPU
top_cpu_processes() { ps -eo pid,ppid,comm,%mem,%cpu --sort=-%cpu | head -n 6; }

# get_memory_stats -> sets MEM_TOTAL MEM_USED MEM_AVAIL (MB) and MEM_PCT (%)
get_memory_stats() {
    read -r MEM_TOTAL MEM_USED MEM_AVAIL <<< "$(free -m | awk '/^Mem:/ {print $2, $3, $7}')"
    if [[ -z "$MEM_TOTAL" || "$MEM_TOTAL" -eq 0 ]]; then
        echo "Error: could not read memory information from 'free'." >&2
        return 1
    fi
    [[ -z "$MEM_AVAIL" ]] && MEM_AVAIL=$((MEM_TOTAL - MEM_USED))   # very old 'free' has no 'available'
    MEM_PCT=$(awk -v u="$MEM_USED" -v t="$MEM_TOTAL" 'BEGIN { printf "%.1f", u * 100 / t }')
}