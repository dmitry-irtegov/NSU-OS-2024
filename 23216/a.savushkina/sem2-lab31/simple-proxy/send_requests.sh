#!/bin/bash

PROXY_HOST="127.0.0.1"
PROXY_PORT=8080
REQUESTS=(
    "GET / HTTP/1.0\r\nHost: example.com\r\nCache-Control: max-age=60\r\n\r\n"
    # "GET / HTTP/1.0\r\nHost: github.com\r\nCache-Control: max-age=60\r\n\r\n"
    # "GET / HTTP/1.0\r\nHost: speedtest.tele2.net\r\nCache-Control: no-cache\r\n\r\n"
    "GET / HTTP/1.0\r\nHost: philohome.com\r\nCache-Control: max-age=60\r\n\r\n"
)

NUM_REQUESTS=100

for ((i=0; i<$NUM_REQUESTS; i++))
do
    echo "\nREQUEST $((i + 1))"
    echo "\nRequest:"
    echo -e "${REQUESTS[$((i % ${#REQUESTS[@]}))]}"
    echo "\nResponse:"
    {
        exec 3<>/dev/tcp/$PROXY_HOST/$PROXY_PORT
        echo -e "${REQUESTS[$((i % ${#REQUESTS[@]}))]}" >&3
        while read -r line <&3; do
            echo "$line"
            [[ "$line" == $'\r' ]] && break
        done
        echo "Response received"
        echo -e "CLOSE_CONNECTION" >&3
        exec 3<&-
        exec 3>&-
    } || {
        echo "Failed to connect to proxy"
    }
done