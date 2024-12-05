#!/bin/ksh

N_CLIENTS=100
successful=0
failed=0

echo "Starting client test..."

i=1
while [ $i -le 8 ]
do
    echo "TEST MESSAGE FROM CLIENT $i" | ./client &
    i=$((i + 1))
done

sleep 0.5

while [ $i -le $N_CLIENTS ]
do
    echo "TEST MESSAGE FROM CLIENT $i" | ./client &
    sleep 0.1
    i=$((i + 1))
done

wait

echo
echo "Test completed:"
echo "Total clients attempted: $N_CLIENTS"
