#!/bin/bash
# Displays hostname, OS, kernel, uptime and current date/time.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

show_system_info() {
    local os os_info uptime_info
    os=$(uname -s)
    if [[ "$os" == "Linux" ]]; then
        if [[ -f /etc/os-release ]]; then
            os_info=$(grep '^PRETTY_NAME=' /etc/os-release | cut -d '"' -f 2)
        else
            os_info=$(uname -o)
        fi
        uptime_info=$(uptime -p 2>/dev/null | sed 's/^up //')
        [[ -z "$uptime_info" ]] && uptime_info=$(uptime)
    else
        os_info="$os (non-Linux: some features unavailable)"
        uptime_info=$(uptime)
    fi

    echo "====== System Summary ========"
    echo "Hostname: $(hostname)"
    echo "Operating System: $os_info"
    echo "Kernel Version: $(uname -r)"
    echo "System Uptime: $uptime_info"
    echo "Current Date & Time: $(timestamp)"
    echo "=============================="
    log_event INFO "Displayed system information"
}

show_system_info