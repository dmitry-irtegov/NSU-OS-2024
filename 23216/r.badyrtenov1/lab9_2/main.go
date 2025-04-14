package main

import (
	"fmt"
	"os"
	"os/signal"
	"strconv"
	"sync"
	"syscall"
)

const checkInterval = 1000000 // Интервал проверки прерывания

type InterruptFlag struct {
	mu    sync.Mutex
	value bool
}

func (f *InterruptFlag) Set(value bool) {
	f.mu.Lock()
	defer f.mu.Unlock()
	f.value = value
}

func (f *InterruptFlag) Get() bool {
	f.mu.Lock()
	defer f.mu.Unlock()
	return f.value
}

func waitSignal(f *InterruptFlag, sigChan <-chan os.Signal) {
	<-sigChan
	f.Set(true)
}

func leibnizPi(start, step int, result chan<- float64, wg *sync.WaitGroup, f *InterruptFlag) {
	defer wg.Done()
	partialSum := 0.0
	count, i := 1, start
	for {
		partialSum += 1.0 / (float64(i)*4.0 + 1.0)
		partialSum -= 1.0 / (float64(i)*4.0 + 3.0)
		if count%checkInterval == 0 && f.Get() {
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
	var f InterruptFlag
	var wg sync.WaitGroup

	go waitSignal(&f, sigChan)
	for i := 0; i < amtThreads; i++ {
		wg.Add(1)
		go leibnizPi(i, amtThreads, result, &wg, &f)
	}

	wg.Wait()
	close(result)

	sum := 0.0
	for partialSum := range result {
		sum += partialSum
	}
	fmt.Printf("\nApproximated pi = %.15f\n", sum*4.0)
}
