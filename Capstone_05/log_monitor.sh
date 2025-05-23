#!/bin/bash

LOG_FILE="/var/log/dpkg.log"
KEYWORDS=("error" "fail" "critical")

echo "Scanning logs for critical issues..."

> /tmp/log_alerts.txt  # Clear previous results

for keyword in "${KEYWORDS[@]}"; do
    grep -i "$keyword" "$LOG_FILE" >> /tmp/log_alerts.txt
done

if [[ -s /tmp/log_alerts.txt ]]; then
    echo "Alert: Critical issues found in logs!"
    cat /tmp/log_alerts.txt
else
    echo "No critical issues found in logs."
fi
