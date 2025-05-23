#!/bin/bash

# Define source and destination directories
SOURCE_DIR="$HOME/Documents"
DEST_DIR="$HOME/backup"

# Check if source directory exists
if [ ! -d "$SOURCE_DIR" ]; then
    echo "Source directory $SOURCE_DIR does not exist."
    echo "Creating the directory $SOURCE_DIR."
    mkdir -p "$SOURCE_DIR"
fi

# Create backup directory if it doesn't exist
mkdir -p "$DEST_DIR"

# Create a timestamped backup file
BACKUP_FILE="$DEST_DIR/backup_$(date +%Y%m%d_%H%M%S).tar.gz"

# Perform the backup
tar -czf "$BACKUP_FILE" "$SOURCE_DIR"

# Check if backup was successful
if [ $? -eq 0 ]; then
    echo "Backup completed successfully: $BACKUP_FILE"
else
    echo "Backup failed. Please check for errors."
fi
