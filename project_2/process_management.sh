#!/bin/bash
# Process management. Usage: ./process_management.sh view | terminate
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

# Processes that must never be terminated by this tool
PROTECTED_NAMES=(systemd init kthreadd sshd dbus-daemon login Xorg gdm lightdm NetworkManager)

# Show the 10 busiest processes
list_processes() {
    echo "====== Running Processes (top 10 by CPU) ========"
    ps -eo pid,ppid,user,%cpu,%mem,comm --sort=-%cpu | head -n 11
    echo "=================================================="
    log_event INFO "Viewed running processes"
}

# Search by PID or (case-insensitive) name; repeats until the user presses Enter
search_process() {
    local term lower matches
    while true; do
        read -r -p "Search by process name or PID (press Enter to return): " term || return 0
        [[ -z "$term" ]] && return 0

        if [[ "$term" =~ ^[0-9]+$ ]]; then
            if ps -p "$term" >/dev/null 2>&1; then
                ps -p "$term" -o pid,ppid,user,%cpu,%mem,etime,comm
                log_event INFO "Searched for PID $term (found)"
            else
                echo "No process found with PID $term."
                log_event WARNING "Searched for PID $term (not found)"
            fi
        else
            lower=$(tr '[:upper:]' '[:lower:]' <<< "$term")
            matches=$(ps -eo pid,ppid,user,%cpu,%mem,etime,comm |
                awk -v t="$lower" 'NR==1 || (index(tolower($7), t) && $7 != "awk" && $7 != "ps")')
            if (( $(wc -l <<< "$matches") > 1 )); then
                echo "$matches"
                log_event INFO "Searched for process name '$term' (found)"
            else
                echo "No process found matching '$term'."
                log_event WARNING "Searched for process name '$term' (not found)"
            fi
        fi
    done
}

# protection_reason PID -> prints why a process is protected and returns 0; returns 1 if it is safe to kill
protection_reason() {
    local pid="$1" name uid ppid n
    name=$(ps -p "$pid" -o comm= 2>/dev/null | xargs)
    uid=$(ps -p "$pid" -o uid= 2>/dev/null | xargs)
    ppid=$(ps -p "$pid" -o ppid= 2>/dev/null | xargs)

    if (( pid <= 2 )); then
        echo "PID $pid is a core system process"
    elif (( pid == $$ || pid == PPID )); then
        echo "PID $pid is this application (or its parent)"
    elif [[ "$ppid" == "2" ]]; then
        echo "PID $pid is a kernel thread"
    elif [[ "$uid" == "0" && $EUID -ne 0 ]]; then
        echo "PID $pid is owned by root and you do not have permission"
    elif [[ "$uid" != "$EUID" && $EUID -ne 0 ]]; then
        echo "PID $pid belongs to another user and you do not have permission"
    else
        for n in "${PROTECTED_NAMES[@]}"; do
            if [[ "$name" == "$n" ]]; then
                echo "'$name' is a protected system process"
                return 0
            fi
        done
        return 1
    fi
    return 0
}

terminate_process() {
    local input pid name reason matches
    read -r -p "Enter the PID or process name to terminate: " input || return 1
    if [[ -z "$input" ]]; then
        echo "Error: no input provided."
        return 1
    fi

    if [[ "$input" =~ ^[0-9]+$ ]]; then
        pid=$((10#$input))
    else
        matches=$(pgrep -x "$input")
        if [[ -z "$matches" ]]; then
            echo "No process found with the exact name '$input'."
            log_event WARNING "Terminate requested for '$input' (no match)"
            return 1
        fi
        echo "Processes named '$input':"
        ps -p "$(paste -sd, <<< "$matches")" -o pid,ppid,user,%cpu,%mem,comm
        read -r -p "Enter the exact PID to terminate: " input || return 1
        if [[ ! "$input" =~ ^[0-9]+$ ]]; then
            echo "Error: '$input' is not a valid PID."
            return 1
        fi
        pid=$((10#$input))
    fi

    if ! ps -p "$pid" >/dev/null 2>&1; then
        echo "Error: no process found with PID $pid."
        log_event WARNING "Terminate requested for PID $pid (does not exist)"
        return 1
    fi

    if reason=$(protection_reason "$pid"); then
        echo "Refused: $reason."
        log_event WARNING "Termination of PID $pid blocked: $reason"
        return 1
    fi

    name=$(ps -p "$pid" -o comm= | xargs)
    ps -p "$pid" -o pid,ppid,user,%cpu,%mem,etime,comm
    if ! confirm "Terminate '$name' (PID $pid)?"; then
        echo "Termination cancelled."
        log_event INFO "Termination of $name (PID $pid) cancelled by user"
        return 0
    fi

    if kill "$pid" 2>/dev/null; then
        sleep 2
        if ps -p "$pid" >/dev/null 2>&1; then
            echo "Process $pid is still running after SIGTERM."
            if confirm "Force kill with SIGKILL?"; then
                if kill -9 "$pid" 2>/dev/null; then
                    echo "Process $pid force killed."
                    log_event INFO "Process $name (PID $pid) force killed (SIGKILL)"
                else
                    echo "Error: could not force kill PID $pid."
                    log_event ERROR "SIGKILL failed for $name (PID $pid)"
                fi
            else
                log_event INFO "Process $name (PID $pid) ignored SIGTERM; force kill declined"
            fi
        else
            echo "Process $name (PID $pid) terminated."
            log_event INFO "Process $name (PID $pid) terminated (SIGTERM)"
        fi
    else
        echo "Error: failed to signal PID $pid (insufficient permissions?)."
        log_event ERROR "kill failed for $name (PID $pid)"
        return 1
    fi
}

require_cmd ps pgrep awk xargs || exit 1
case "${1:-}" in
    view)      list_processes; search_process ;;
    terminate) terminate_process ;;
    *)         echo "Usage: $0 {view|terminate}"; exit 1 ;;
esac