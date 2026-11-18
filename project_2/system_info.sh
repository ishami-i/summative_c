#!/usr/bin/env bash
# Displays hostname, OS, kernel, uptime and current date/time.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

show_system_info() {
    echo "====== System Summary ========"
    echo "Hostname: $(hostname)"
    echo "Operating System: $(get_os_info)"
    echo "Kernel Version: $(uname -r)"
    echo "System Uptime: $(get_uptime)"
    echo "Current Date & Time: $(timestamp)"
    echo "=============================="
    log_event INFO "Displayed system information"
}

show_system_info