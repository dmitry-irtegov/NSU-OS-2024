package main

import (
	"context"
	"fmt"
	"os"
	"os/signal"
	"strconv"
	"sync"
	"syscall"
)

const checkInterval = 1000000

func leibnizPi(start, step int, result chan<- float64, wg *sync.WaitGroup, ctx context.Context) {
	defer wg.Done()
	partialSum := 0.0
	i := start

loop:
	for {
		for j := 0; j < checkInterval; j++ {
			partialSum += 1.0 / (float64(i)*4 + 1)
			partialSum -= 1.0 / (float64(i)*4 + 3)
			i += step
		}

		select {
		case <-ctx.Done():
			break loop
		}
	}
	result <- partialSum
}

func main() {
	if len(os.Args) < 2 {
		fmt.Println("Usage: go run main.go <num_threads>")
		return
	}
	amtThreads, err := strconv.Atoi(os.Args[1])
	if err != nil || amtThreads <= 0 {
		fmt.Println("Invalid number of threads")
		return
	}

	ctx, stop := signal.NotifyContext(context.Background(), os.Interrupt, syscall.SIGINT)
	defer stop()

	result := make(chan float64, amtThreads)
	var wg sync.WaitGroup

	for i := 0; i < amtThreads; i++ {
		wg.Add(1)
		go leibnizPi(i, amtThreads, result, &wg, ctx)
	}

	wg.Wait()
	close(result)

	sum := 0.0
	for partialSum := range result {
		sum += partialSum
	}
	fmt.Printf("\nApproximated pi = %.15f\n", sum*4.0)
}
