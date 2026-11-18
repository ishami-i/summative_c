#!/usr/bin/env bash
# Monitors disk utilization for mounted filesystems and flags those near capacity.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

APPROACH_MARGIN=10     # "approaching" = within 10 percentage points of the threshold

check_disks() {
    local source size used avail pct mount status
    echo "$(timestamp) - Disk Usage (threshold ${THRESHOLD}%)"
    printf '%-28s %8s %8s %8s %5s  %-12s %s\n' Filesystem Size Used Avail 'Use%' Status Mount
    while IFS=$'\t' read -r source size used avail pct mount; do
        [[ "$pct" =~ ^[0-9]+$ ]] || continue
        if (( pct > THRESHOLD )); then
            status="WARNING"
            echo "WARNING: Disk utilization above ${THRESHOLD}% on $source ($mount): ${pct}%"
            log_event WARNING "Disk utilization above ${THRESHOLD}% on $source ($mount): ${pct}%"
        elif (( pct >= 1 && pct >= THRESHOLD - APPROACH_MARGIN )); then
            status="APPROACHING"
            log_event INFO "Disk $source ($mount) approaching capacity: ${pct}%"
        else
            status="OK"
        fi
        printf '%-28s %8s %8s %8s %4s%%  %-12s %s\n' "$source" "$size" "$used" "$avail" "$pct" "$status" "$mount"
    done < <(list_filesystems)
    echo "----------------------------------------"
}

monitor_disk() {
    require_cmd df awk || return 1
    if [[ -z "$(list_filesystems)" ]]; then
        echo "Error: could not read filesystem information from 'df'." >&2
        return 1
    fi
    read_threshold "disk" || return 1

    log_event INFO "Disk monitoring started (threshold ${THRESHOLD}%)"
    echo "Monitoring disk utilization. Press [CTRL+C] to stop."
    trap 'echo; echo "Disk monitoring stopped."; log_event INFO "Disk monitoring stopped"; exit 0' INT
    while true; do
        check_disks
        sleep "$REFRESH_INTERVAL"
    done
}

monitor_disk
exit $?