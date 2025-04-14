package main

import (
	"fmt"
	"os"
	"os/signal"
	"strconv"
	"sync"
	"sync/atomic"
	"syscall"
)

const checkInterval = 1000000 // Интервал проверки прерывания

func waitSignal(interruptFlag *atomic.Bool, sigChan <-chan os.Signal) {
	<-sigChan
	interruptFlag.Store(true)
}

func leibnizPi(start, step int, result chan<- float64, wg *sync.WaitGroup, interruptFlag *atomic.Bool) {
	defer wg.Done()
	partialSum := 0.0
	count, i := 1, start
	for {
		partialSum += 1.0 / (float64(i)*4.0 + 1.0)
		partialSum -= 1.0 / (float64(i)*4.0 + 3.0)
		if count%checkInterval == 0 && interruptFlag.Load() {
			break
		}
		count++
		i += step
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

	sigChan := make(chan os.Signal, 1)
	signal.Notify(sigChan, os.Interrupt, syscall.SIGINT)
	result := make(chan float64, amtThreads)
	var interruptFlag atomic.Bool
	var wg sync.WaitGroup

	go waitSignal(&interruptFlag, sigChan)
	for i := 0; i < amtThreads; i++ {
		wg.Add(1)
		go leibnizPi(i, amtThreads, result, &wg, &interruptFlag)
	}

	wg.Wait()
	close(result)

	sum := 0.0
	for partialSum := range result {
		sum += partialSum
	}
	fmt.Printf("\nApproximated pi = %.15f\n", sum*4.0)
}
