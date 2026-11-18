#!/bin/bash
# Monitors disk utilization for mounted filesystems and flags those near capacity.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

DF_COLUMNS="source,fstype,size,used,avail,pcent,target"
APPROACH_MARGIN=10     # "approaching" = within 10 percentage points of the threshold

check_disks() {
    local source fstype size used avail pcent target pct status
    echo "$(timestamp) - Disk Usage (threshold ${THRESHOLD}%)"
    printf '%-26s %-8s %7s %7s %7s %6s  %-12s %s\n' Filesystem Type Size Used Avail 'Use%' Status Mount
    # Skip virtual filesystems so only real drives are shown
    while read -r source fstype size used avail pcent target; do
        pct=${pcent%\%}
        [[ "$pct" =~ ^[0-9]+$ ]] || continue
        if (( pct > THRESHOLD )); then
            status="WARNING"
            echo "WARNING: Disk utilization above ${THRESHOLD}% on $source ($target): ${pct}%"
            log_event WARNING "Disk utilization above ${THRESHOLD}% on $source ($target): ${pct}%"
        elif (( pct >= 1 && pct >= THRESHOLD - APPROACH_MARGIN )); then
            status="APPROACHING"
            log_event INFO "Disk $source ($target) approaching capacity: ${pct}%"
        else
            status="OK"
        fi
        printf '%-26s %-8s %7s %7s %7s %6s  %-12s %s\n' "$source" "$fstype" "$size" "$used" "$avail" "$pcent" "$status" "$target"
    done < <(df -h -x tmpfs -x devtmpfs -x squashfs --output=$DF_COLUMNS 2>/dev/null | tail -n +2)
    echo "----------------------------------------"
}

monitor_disk() {
    require_cmd df awk || return 1
    if ! df --output=source >/dev/null 2>&1; then
        echo "Error: this 'df' does not support --output (GNU coreutils required)." >&2
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