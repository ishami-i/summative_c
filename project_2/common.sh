#!/usr/bin/env bash
# common.sh - shared helpers, sourced by every other script (not run directly).
# Supports Linux and macOS (bash 3.2+). Other Unix systems get partial support.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG_FILE="${LOG_FILE:-$SCRIPT_DIR/activity.log}"
REFRESH_INTERVAL=5          # seconds between monitoring samples

OS_TYPE="$(uname -s)"       # Linux, Darwin (macOS), ...
is_linux() { [[ "$OS_TYPE" == "Linux" ]]; }
is_macos() { [[ "$OS_TYPE" == "Darwin" ]]; }

# macOS ps prints the short process name with "ucomm"; Linux uses "comm"
if is_macos; then PS_COMM="ucomm"; else PS_COMM="comm"; fi

# ---------------------------------------------------------------- basic helpers
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

# ------------------------------------------------------------ system information
get_os_info() {
    if is_macos && command -v sw_vers >/dev/null 2>&1; then
        echo "$(sw_vers -productName) $(sw_vers -productVersion)"
    elif [[ -f /etc/os-release ]]; then
        grep '^PRETTY_NAME=' /etc/os-release | cut -d '"' -f 2
    else
        uname -sr
    fi
}

# format_duration SECONDS -> "2 days, 3 hours, 5 minutes"
format_duration() {
    awk -v s="$1" 'BEGIN {
        d = int(s / 86400); h = int((s % 86400) / 3600); m = int((s % 3600) / 60)
        out = ""
        if (d > 0) { sd = (d > 1) ? "s" : ""; out = d " day" sd ", " }
        if (h > 0 || d > 0) { sh = (h != 1) ? "s" : ""; out = out h " hour" sh ", " }
        sm = (m != 1) ? "s" : ""; out = out m " minute" sm
        print out }'
}

# get_uptime -> human-readable uptime (uptime -p does not exist on macOS)
get_uptime() {
    local secs="" boot now
    if is_linux && [[ -r /proc/uptime ]]; then
        secs=$(cut -d. -f1 /proc/uptime)
    elif is_macos; then
        boot=$(sysctl -n kern.boottime 2>/dev/null | sed -E 's/^\{ *sec = ([0-9]+),.*/\1/')
        now=$(date +%s)
        [[ "$boot" =~ ^[0-9]+$ ]] && secs=$((now - boot))
    fi
    if [[ "$secs" =~ ^[0-9]+$ ]]; then
        format_duration "$secs"
    else
        uptime | sed -E 's/^.*up +//; s/, +[0-9]+ users?.*//'      # generic fallback
    fi
}

# get_load_avg -> "0.52, 0.40, 0.35" (macOS prints "load averages:" with an s)
get_load_avg() { uptime | sed -E 's/.*load averages?: *//'; }

get_cpu_count() { getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1; }

print_cpu_info() {
    if is_macos; then
        echo "Model name: $(sysctl -n machdep.cpu.brand_string 2>/dev/null)"
        echo "Physical cores: $(sysctl -n hw.physicalcpu 2>/dev/null)"
        echo "Logical CPUs: $(sysctl -n hw.logicalcpu 2>/dev/null)"
    elif command -v lscpu >/dev/null 2>&1; then
        lscpu | grep -E 'Model name|^CPU\(s\)|Thread|Core|Socket|MHz'
    else
        echo "CPU(s): $(get_cpu_count)"
    fi
}

# --------------------------------------------------------------------- CPU usage
# Fallback for any Unix: sum of per-process %CPU divided by the number of CPUs
_cpu_usage_ps() {
    ps -A -o %cpu 2>/dev/null | awk -v n="$(get_cpu_count)" \
        'NR > 1 { s += $1 } END { v = s / n; if (v > 100) v = 100; printf "%.1f", v }'
}

_cpu_usage_linux() {
    local cpu u1 n1 s1 i1 w1 q1 sq1 st1 u2 n2 s2 i2 w2 q2 sq2 st2
    if [[ ! -r /proc/stat ]]; then _cpu_usage_ps; return; fi
    read -r cpu u1 n1 s1 i1 w1 q1 sq1 st1 _ < /proc/stat
    sleep 1
    read -r cpu u2 n2 s2 i2 w2 q2 sq2 st2 _ < /proc/stat
    awk -v t1="$((u1+n1+s1+i1+w1+q1+sq1+st1))" -v t2="$((u2+n2+s2+i2+w2+q2+sq2+st2))" \
        -v idle1="$((i1+w1))" -v idle2="$((i2+w2))" \
        'BEGIN { dt = t2 - t1; if (dt <= 0) print "0.0"; else printf "%.1f", (dt - (idle2 - idle1)) * 100 / dt }'
}

_cpu_usage_macos() {
    local line
    # two samples one second apart; the second one is the real measurement
    line=$(top -l 2 -n 0 -s 1 2>/dev/null | grep 'CPU usage' | tail -n 1)
    if [[ -z "$line" ]]; then _cpu_usage_ps; return; fi
    awk -v line="$line" 'BEGIN {
        if (match(line, /[0-9.]+% idle/)) { s = substr(line, RSTART, RLENGTH); sub(/%.*/, "", s); printf "%.1f", 100 - s }
        else print "0.0" }'
}

# get_cpu_usage -> prints CPU busy % (measured over about 1 second)
get_cpu_usage() {
    case "$OS_TYPE" in
        Linux)  _cpu_usage_linux ;;
        Darwin) _cpu_usage_macos ;;
        *)      _cpu_usage_ps ;;
    esac
}

# ------------------------------------------------------------------- processes
# ps_sorted -> header + all processes sorted by CPU (works with GNU and BSD ps)
ps_sorted() {
    local out
    out=$(ps -A -o pid,ppid,user,%cpu,%mem,$PS_COMM 2>/dev/null) || return 1
    echo "$out" | head -n 1 | sed 's/UCOMM/COMMAND/'
    echo "$out" | tail -n +2 | sort -k4 -rn
}

# top_cpu_processes -> header + 5 heaviest processes by CPU
top_cpu_processes() { ps_sorted | head -n 6; }

# ps_detail PID[,PID...] -> detailed rows for the given PIDs
ps_detail() { ps -p "$1" -o pid,ppid,user,%cpu,%mem,etime,$PS_COMM 2>/dev/null | sed '1s/UCOMM/COMMAND/'; }

# ---------------------------------------------------------------------- memory
_mem_linux() {
    if [[ ! -r /proc/meminfo ]]; then
        echo "Error: /proc/meminfo is not readable." >&2
        return 1
    fi
    read -r MEM_TOTAL MEM_AVAIL <<< "$(awk '
        /^MemTotal:/ { t = $2 } /^MemAvailable:/ { a = $2 } /^MemFree:/ { f = $2 }
        /^Buffers:/ { b = $2 } /^Cached:/ { c = $2 }
        END { if (a == "") a = f + b + c; printf "%d %d", t / 1024, a / 1024 }' /proc/meminfo)"
}

_mem_macos() {
    require_cmd vm_stat sysctl || return 1
    local bytes
    bytes=$(sysctl -n hw.memsize 2>/dev/null)
    MEM_TOTAL=$(( ${bytes:-0} / 1048576 ))
    # available = free + inactive + speculative pages
    MEM_AVAIL=$(vm_stat | awk '
        /page size of/ { for (i = 1; i <= NF; i++) if ($i ~ /^[0-9]+$/) ps = $i }
        /^Pages free/ { f = $3 + 0 } /^Pages inactive/ { n = $3 + 0 } /^Pages speculative/ { s = $3 + 0 }
        END { printf "%d", (f + n + s) * ps / 1048576 }')
}

# get_memory_stats -> sets MEM_TOTAL MEM_USED MEM_AVAIL (MB) and MEM_PCT (%)
get_memory_stats() {
    case "$OS_TYPE" in
        Linux)  _mem_linux  || return 1 ;;
        Darwin) _mem_macos  || return 1 ;;
        *) echo "Error: memory statistics are not supported on $OS_TYPE." >&2; return 1 ;;
    esac
    if [[ -z "$MEM_TOTAL" || "$MEM_TOTAL" -eq 0 ]]; then
        echo "Error: could not read memory information." >&2
        return 1
    fi
    (( MEM_AVAIL > MEM_TOTAL )) && MEM_AVAIL=$MEM_TOTAL
    MEM_USED=$((MEM_TOTAL - MEM_AVAIL))
    MEM_PCT=$(awk -v u="$MEM_USED" -v t="$MEM_TOTAL" 'BEGIN { printf "%.1f", u * 100 / t }')
}

# ----------------------------------------------------------------------- disks
# list_filesystems -> tab-separated rows: source size used avail pct mountpoint
# Uses POSIX "df -Pk" (identical on Linux and macOS) and skips virtual filesystems.
list_filesystems() {
    df -Pk 2>/dev/null | awk '
        function hum(k,   i, v) { v = k; i = 1; while (v >= 1024 && i < 5) { v /= 1024; i++ }
                                  return sprintf("%.1f%s", v, substr("KMGTP", i, 1)) }
        NR == 1 { next }
        {
            c = 0
            for (i = 2; i <= NF; i++) if ($i ~ /^[0-9]+%$/) { c = i; break }
            if (c < 5) next
            src = $1; for (j = 2; j <= c - 4; j++) src = src " " $j
            mnt = $(c + 1); for (j = c + 2; j <= NF; j++) mnt = mnt " " $j
            if (src ~ /^(tmpfs|devtmpfs|devfs|squashfs|udev|map|none|efivarfs)/) next
            if (mnt ~ /^\/(dev|proc|sys)(\/|$)/) next
            if (mnt ~ /^\/System\/Volumes\// && mnt != "/System/Volumes/Data") next   # macOS helper volumes
            pct = $c; sub(/%/, "", pct)
            printf "%s\t%s\t%s\t%s\t%s\t%s\n", src, hum($(c - 3)), hum($(c - 2)), hum($(c - 1)), pct, mnt
        }'
}