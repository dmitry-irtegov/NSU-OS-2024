package main

import (
	"fmt"
	"math"
	"os"
	"os/signal"
	"sync"
	"sync/atomic"
	"syscall"
)

const (
	maxSteps  = 2_000_000_000 
	blockSize = 10_000        
)
var (
	globalIndex int64
	interrupted int32
)

func worker(partialSum *float64, wg *sync.WaitGroup) {
	defer wg.Done()
	var localSum float64

	for {
		start := int(atomic.AddInt64(&globalIndex, blockSize)) - blockSize
		if start >= maxSteps || atomic.LoadInt32(&interrupted) == 1 {
			break
		}

		end := start + blockSize
		if end > maxSteps {
			end = maxSteps
		}

		for i := start; i < end; i++ {
			localSum += 1.0 / (float64(i)*4.0 + 1.0)
			localSum -= 1.0 / (float64(i)*4.0 + 3.0)
		}
	}

	*partialSum = localSum
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

	
	sigChan := make(chan os.Signal, 1)
	signal.Notify(sigChan, syscall.SIGINT)

	go func() {
		<-sigChan
		fmt.Println("\nReceived SIGINT, stopping gracefully...")
		atomic.StoreInt32(&interrupted, 1)
	}()

	var wg sync.WaitGroup
	partialSums := make([]float64, numThreads)

	wg.Add(numThreads)
	for i := 0; i < numThreads; i++ {
		go worker(&partialSums[i], &wg)
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
