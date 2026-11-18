#!/bin/bash
# Generates a timestamped system health report in a user-selected directory.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

# choose_directory -> sets $REPORT_DIR (validated, created if needed, writable)
choose_directory() {
    local dir
    read -r -p "Enter the directory to save the report (default: current directory): " dir || return 1
    dir="${dir:-$(pwd)}"
    dir="${dir/#\~/$HOME}"                       # expand a leading ~

    if [[ ! -d "$dir" ]]; then
        echo "Directory '$dir' does not exist."
        confirm "Create it?" || { echo "Report cancelled."; return 1; }
        if ! mkdir -p "$dir" 2>/dev/null; then
            echo "Error: cannot create '$dir' (permission denied?)."
            log_event ERROR "Report directory '$dir' could not be created"
            return 1
        fi
    fi
    if [[ ! -w "$dir" ]]; then
        echo "Error: '$dir' is not writable."
        log_event ERROR "Report directory '$dir' is not writable"
        return 1
    fi
    REPORT_DIR="$dir"
}

write_report() {
    echo "====== System Health Report ========"
    echo "Generated on: $(timestamp)"
    echo "Hostname: $(hostname)"
    echo "------------------------------------"
    echo "CPU Information:"
    if command -v lscpu >/dev/null 2>&1; then
        lscpu | grep -E 'Model name|^CPU\(s\)|Thread|Core|Socket|MHz'
    else
        echo "(lscpu not available)"
    fi
    echo "Current CPU usage: $(get_cpu_usage)%"
    echo "Load average:$(uptime | awk -F'load average:' '{print $2}')"
    echo "Top 5 CPU-consuming processes:"
    top_cpu_processes
    echo "------------------------------------"
    echo "Memory Information:"
    free -h
    get_memory_stats && echo "Memory utilization: ${MEM_PCT}%"
    echo "------------------------------------"
    echo "Disk Usage Information:"
    df -h -x tmpfs -x devtmpfs -x squashfs --output=source,fstype,size,used,avail,pcent,target
    echo "------------------------------------"
    echo "System Uptime: $(uptime -p 2>/dev/null || uptime)"
    echo "===================================="
}

generate_report() {
    require_cmd free df uptime ps awk || return 1
    choose_directory || return 1
    local file="$REPORT_DIR/health_report_$(date '+%Y%m%d_%H%M%S').txt"
    echo "Generating health report at $file ..."
    if write_report > "$file" 2>/dev/null; then
        echo "Report saved: $file"
        log_event INFO "Health report generated: $file"
    else
        echo "Error: failed to write the report."
        log_event ERROR "Health report generation failed: $file"
        return 1
    fi
}

generate_report
exit $?