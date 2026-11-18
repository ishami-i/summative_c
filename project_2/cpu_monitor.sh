#!/usr/bin/env bash
# Monitors CPU utilization, lists heavy processes, warns above a user threshold.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

monitor_cpu() {
    require_cmd ps awk uptime sort || return 1
    read_threshold "CPU" || return 1

    log_event INFO "CPU monitoring started (threshold ${THRESHOLD}%)"
    echo "Monitoring CPU utilization. Press [CTRL+C] to stop."
    trap 'echo; echo "CPU monitoring stopped."; log_event INFO "CPU monitoring stopped"; exit 0' INT

    local usage load ts
    while true; do
        usage=$(get_cpu_usage) || return 1
        load=$(get_load_avg)
        ts=$(timestamp)

        echo "$ts - CPU Usage: ${usage}% | Load Average: $load"
        echo "Top 5 CPU-consuming processes:"
        top_cpu_processes
        echo "----------------------------------------"
        log_event INFO "CPU usage ${usage}%, load average: $load"

        if exceeds "$usage" "$THRESHOLD"; then
            echo "WARNING: CPU utilization is above ${THRESHOLD}%! Current usage: ${usage}%"
            log_event WARNING "CPU utilization above ${THRESHOLD}% (current ${usage}%). Top processes:"
            top_cpu_processes | sed 's/^/        /' >> "$LOG_FILE"
        fi
        sleep "$REFRESH_INTERVAL"
    done
}

monitor_cpu
exit $?