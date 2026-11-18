#!/usr/bin/env bash
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
    echo "Operating System: $(get_os_info)"
    echo "------------------------------------"
    echo "CPU Information:"
    print_cpu_info
    echo "Current CPU usage: $(get_cpu_usage)%"
    echo "Load average: $(get_load_avg)"
    echo "Top 5 CPU-consuming processes:"
    top_cpu_processes
    echo "------------------------------------"
    echo "Memory Information:"
    if get_memory_stats; then
        echo "Total: ${MEM_TOTAL}MB | Used: ${MEM_USED}MB | Available: ${MEM_AVAIL}MB"
        echo "Memory utilization: ${MEM_PCT}%"
    else
        echo "(memory statistics unavailable)"
    fi
    echo "------------------------------------"
    echo "Disk Usage Information:"
    printf '%-28s %8s %8s %8s %5s  %s\n' Filesystem Size Used Avail 'Use%' Mount
    list_filesystems | awk -F'\t' '{ printf "%-28s %8s %8s %8s %4s%%  %s\n", $1, $2, $3, $4, $5, $6 }'
    echo "------------------------------------"
    echo "System Uptime: $(get_uptime)"
    echo "===================================="
}

generate_report() {
    require_cmd df uptime ps awk || return 1
    choose_directory || return 1
    local file
    file="$REPORT_DIR/health_report_$(date '+%Y%m%d_%H%M%S').txt"
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