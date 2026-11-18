#!/usr/bin/env bash
# Monitors memory utilization and warns above a user-defined threshold.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

monitor_memory() {
    require_cmd awk || return 1
    get_memory_stats || return 1            # fail early on unsupported systems
    read_threshold "memory" || return 1

    log_event INFO "Memory monitoring started (threshold ${THRESHOLD}%)"
    echo "Monitoring memory utilization. Press [CTRL+C] to stop."
    trap 'echo; echo "Memory monitoring stopped."; log_event INFO "Memory monitoring stopped"; exit 0' INT

    local status
    while true; do
        get_memory_stats || return 1
        status="Total: ${MEM_TOTAL}MB | Used: ${MEM_USED}MB | Available: ${MEM_AVAIL}MB | Utilization: ${MEM_PCT}%"
        echo "$(timestamp) - Memory: $status"
        log_event INFO "Memory: $status"

        if exceeds "$MEM_PCT" "$THRESHOLD"; then
            echo "WARNING: Memory utilization is above ${THRESHOLD}%! Current usage: ${MEM_PCT}%"
            log_event WARNING "Memory utilization above ${THRESHOLD}% (current ${MEM_PCT}%)"
        fi
        sleep "$REFRESH_INTERVAL"
    done
}

monitor_memory
exit $?