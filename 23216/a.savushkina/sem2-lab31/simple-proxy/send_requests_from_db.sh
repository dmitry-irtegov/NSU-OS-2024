#!/bin/bash

PROXY_HOST="127.0.0.1"
PROXY_PORT=8080
DB_FILE="requests_db.txt"

if [[ ! -f "$DB_FILE" ]]; then
    echo "Database file '$DB_FILE' not found!"
    exit 1
fi

while true; do
    echo "Available requests:"
    mapfile -t requests < "$DB_FILE"
    for i in "${!requests[@]}"; do
        echo "$i) ${requests[$i]}"
    done

    echo "Enter the number of the request to send (or 'q' to quit):"
    read -r choice

    if [[ "$choice" == "q" ]]; then
        echo "Exiting..."
        break
    fi

    if ! [[ "$choice" =~ ^[0-9]+$ ]] || (( choice < 0 || choice >= ${#requests[@]} )); then
        echo "Invalid choice. Please try again."
        continue
    fi

    request="${requests[$choice]}"
    echo "Sending request:"
    echo -e "$request"
    echo "Response:"
    {
        exec 3<>/dev/tcp/$PROXY_HOST/$PROXY_PORT
        echo -e "$request" >&3
        while read -r line <&3; do
            echo "$line"
            [[ "$line" == $'\r' ]] && break
        done
        echo "Response received"
        exec 3<&-
        exec 3>&-
    } || {
        echo "Failed to connect to proxy"
    }
    sleep 1
done
