#!/bin/bash

PROXY_HOST="127.0.0.1"
PROXY_PORT=8080

NUM_REQUESTS=100

for ((i=0; i<$NUM_REQUESTS; i++))
do
    echo "Sending request $i"
    curl -x http://localhost:65430 \
     --http1.0 -i \
     -H "Host: github.com" \
     -H "Cache-Control: no-cache" \
     http://github.com/

done