package main

import (
	"fmt"
	"math"
	"os"
	"strconv"
	"sync"
)

const (
	numSteps  = 200_000_000
	blockSize = 10_000
)

func worker(start, end int, wg *sync.WaitGroup, sumChan chan<- float64) {
	defer wg.Done()
	var localSum float64
	for i := start; i < end; i++ {
		localSum += 1.0 / (float64(i)*4.0 + 1.0)
		localSum -= 1.0 / (float64(i)*4.0 + 3.0)
	}
	sumChan <- localSum
}

func main() {
	if len(os.Args) < 2 {
		fmt.Printf("Usage: %s <num_threads>\n", os.Args[0])
		os.Exit(1)
	}

	numThreads, err := strconv.Atoi(os.Args[1])
	if err != nil || numThreads <= 0 {
		fmt.Println("Invalid number of threads")
		os.Exit(1)
	}

	var wg sync.WaitGroup
	sumChan := make(chan float64, numThreads)

	// вычисление количества блоков
	totalBlocks := (numSteps + blockSize - 1) / blockSize
	blocksPerThread := (totalBlocks + numThreads - 1) / numThreads

	for t := 0; t < numThreads; t++ {
		wg.Add(1)
		startBlock := t * blocksPerThread
		endBlock := (t + 1) * blocksPerThread
		if endBlock > totalBlocks {
			endBlock = totalBlocks
		}
		start := startBlock * blockSize
		end := endBlock * blockSize
		if end > numSteps {
			end = numSteps
		}
		go worker(start, end, &wg, sumChan)
	}

	go func() {
		wg.Wait()
		close(sumChan)
	}()

	var pi float64
	for sum := range sumChan {
		pi += sum
	}
	pi *= 4.0

	fmt.Printf("Computed Pi = %.15g\n", pi)
	fmt.Printf("Math Pi    = %.15g\n", math.Pi)
	fmt.Printf("Error      = %.15g\n", math.Abs(pi-math.Pi))
}
