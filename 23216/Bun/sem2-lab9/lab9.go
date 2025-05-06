package main

import (
	"context"
	"fmt"
	"math"
	"os"
	"os/signal"
	"sync"
	"sync/atomic"
	"syscall"
)

const blockSize = 10_000

var globalIndex int64

func worker(ctx context.Context, partialSum *float64, wg *sync.WaitGroup) {
	defer wg.Done()
	var localSum float64

	for {
		select {
		case <-ctx.Done():
			*partialSum = localSum
			return
		default:
			start := int(atomic.AddInt64(&globalIndex, blockSize)) - blockSize
			end := start + blockSize

			for i := start; i < end; i++ {
				localSum += 1.0 / (float64(i)*4.0 + 1.0)
				localSum -= 1.0 / (float64(i)*4.0 + 3.0)
			}
		}
	}
}

func main() {
	if len(os.Args) < 2 {
		fmt.Printf("Usage: %s <num_threads>\n", os.Args[0])
		os.Exit(1)
	}

	var numThreads int
	_, err := fmt.Sscanf(os.Args[1], "%d", &numThreads)
	if err != nil || numThreads <= 0 {
		fmt.Println("Invalid number of threads")
		os.Exit(1)
	}

	ctx, cancel := context.WithCancel(context.Background())

	sigChan := make(chan os.Signal, 1)
	signal.Notify(sigChan, syscall.SIGINT)

	go func() {
		<-sigChan
		fmt.Println("\nReceived SIGINT, stopping gracefully...")
		cancel()
	}()

	var wg sync.WaitGroup
	partialSums := make([]float64, numThreads)

	for i := 0; i < numThreads; i++ {
		wg.Add(1)
		go worker(ctx, &partialSums[i], &wg)
	}

	wg.Wait()

	
	var pi float64
	for _, sum := range partialSums {
		pi += sum
	}
	pi *= 4.0

	fmt.Printf("Computed Pi = %.15g\n", pi)
	fmt.Printf("Math Pi    = %.15g\n", math.Pi)
	fmt.Printf("Error      = %.15g\n", math.Abs(pi-math.Pi))
}
