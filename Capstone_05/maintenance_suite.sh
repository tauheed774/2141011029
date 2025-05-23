#!/bin/bash

while true; do
    echo ""
    echo "===== System Maintenance Menu ====="
    echo "1. Backup Files"
    echo "2. System Update and Cleanup"
    echo "3. Monitor System Logs"
    echo "4. Exit"
    echo "======================================"
    read -p "Enter your choice [1-4]: " choice

    case $choice in
        1) bash backup.sh ;;
        2) bash update_cleanup.sh ;;
        3) bash log_monitor.sh ;;
        4) echo "Exiting Maintenance Suite. Goodbye!" ; exit ;;
        *) echo "Invalid option. Please try again." ;;
    esac
done
