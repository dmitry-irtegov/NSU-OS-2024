package main

import (
	"fmt"
	"os"
	"strconv"
	"sync"
)

const iterations = 200000000 // Количество итераций

func leibnizPi(start, step int, result chan<- float64, wg *sync.WaitGroup) {
	defer wg.Done()
	partialSum := 0.0
	for i := start; i < iterations; i += step {
		partialSum += 1.0 / (float64(i)*4.0 + 1.0)
		partialSum -= 1.0 / (float64(i)*4.0 + 3.0)
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

	result := make(chan float64, amtThreads)
	var wg sync.WaitGroup
	for i := 0; i < amtThreads; i++ {
		wg.Add(1)
		go leibnizPi(i, amtThreads, result, &wg)
	}

	wg.Wait()
	close(result)

	sum := 0.0
	for partialSum := range result {
		sum += partialSum
	}
	fmt.Printf("Approximated pi = %.15f\n", sum*4.0)
}
