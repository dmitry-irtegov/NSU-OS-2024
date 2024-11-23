#!/bin/bash

mode=$1
n=$2
input_file=$3

if [ "$mode" -eq 1 ]; then
    for ((i=1; i<n; i++)); do
        { while read line; do
            echo "$line"
            sleep 0.5
        done < "$input_file"; } | ./client &
    done

elif [ "$mode" -eq 2 ]; then
    for ((i=1; i<n; i++)); do
        ./client < "$input_file" &
    done
fi
