#!/bin/bash
# Clears the activity log after confirmation.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

clear_log() {
    if [[ ! -s "$LOG_FILE" ]]; then
        echo "The activity log is already empty."
        return 0
    fi
    if ! confirm "This will permanently delete all activity log entries. Continue?"; then
        echo "Clear cancelled."
        return 0
    fi
    if : > "$LOG_FILE" 2>/dev/null; then
        echo "Activity log cleared."
        log_event INFO "Activity log cleared by user"
    else
        echo "Error: cannot clear $LOG_FILE (permission denied)."
        return 1
    fi
}

clear_log