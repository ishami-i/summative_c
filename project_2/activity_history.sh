#!/usr/bin/env bash
# Displays the activity log (last 50 entries, or everything on request).
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

view_history() {
    if [[ ! -s "$LOG_FILE" ]]; then
        echo "The activity log is empty or does not exist yet."
        return 0
    fi
    if [[ ! -r "$LOG_FILE" ]]; then
        echo "Error: cannot read $LOG_FILE (permission denied)."
        return 1
    fi
    echo "====== Activity History (last 50 entries) ======"
    tail -n 50 "$LOG_FILE"
    echo "================================================"
    if [[ $(wc -l < "$LOG_FILE") -gt 50 ]] && confirm "Show the full log?"; then
        less "$LOG_FILE" 2>/dev/null || cat "$LOG_FILE"
    fi
    log_event INFO "Viewed activity history"
}

view_history