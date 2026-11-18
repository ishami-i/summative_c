#!/bin/bash
# Linux System Health and Resource Monitoring Manager - interactive menu.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

# Ctrl+C stops a running monitor (which handles it itself) without closing the menu
trap 'echo' INT

print_menu() {
    echo
    echo "====== Linux System Health and Resource Monitoring Manager ========"
    echo "1. Display system information."
    echo "2. Monitor CPU utilization."
    echo "3. Monitor memory utilization."
    echo "4. Monitor disk utilization."
    echo "5. View and search running processes."
    echo "6. Terminate a process."
    echo "7. Generate a system health report."
    echo "8. View activity history."
    echo "9. Clear activity log."
    echo "10. Exit the application."
}

# run_script name [args...] -> runs a sibling script with existence/permission checks
run_script() {
    local name="$1"; shift
    local path="$SCRIPT_DIR/$name"
    if [[ ! -f "$path" ]]; then
        echo "Error: $name was not found in $SCRIPT_DIR."
        return 1
    fi
    if [[ ! -x "$path" ]]; then
        echo "Error: $name is not executable. Run:  chmod +x *"
        return 1
    fi
    "$path" "$@"
}

pause() { read -r -p "Press Enter to return to the menu..." _ || true; }

main() {
    log_event INFO "Application started"
    local choice
    while true; do
        print_menu
        read -r -p "Enter your choice (1-10): " choice || { echo; break; }

        if [[ ! "$choice" =~ ^[0-9]+$ ]] || (( 10#$choice < 1 || 10#$choice > 10 )); then
            echo "Invalid choice '$choice'. Please enter a number from 1 to 10."
            continue
        fi
        choice=$((10#$choice))
        [[ $choice -ne 10 ]] && log_event INFO "Menu option $choice selected"

        case $choice in
            1)  run_script system_info.sh ;;
            2)  run_script cpu_monitor.sh ;;
            3)  run_script memory_monitor.sh ;;
            4)  run_script disk_monitor.sh ;;
            5)  run_script process_management.sh view ;;
            6)  run_script process_management.sh terminate ;;
            7)  run_script health_report.sh ;;
            8)  run_script activity_history.sh ;;
            9)  run_script clear_activity.sh ;;
            10) break ;;
        esac
        pause
    done
    log_event INFO "Application exited"
    echo "Exiting the application. Goodbye!"
}

main