package main

import (
	"fmt"
	"os"
	"strconv"
)

func calculatePi(start int, number_of_thread int, limit int, chanPi chan float64) {

	pi := 0

	for i := start; i < limit; i += number_of_thread {

		pi += float64(1.0 / (float64(i*4.0) + 1.0))
		pi -= float64(1.0 / (float64(i*4.0) + 3.0))
	}

	pi = pi * 4.0
	// fmt.Println("part of pi - ", pi)
	chanPi <- pi
}

func main() {
	num_steps := 20000000
	var wg sync.WaitGroup

	if len(os.Args) < 2 {
		fmt.Println("write number of threads")
		return
	}

	number_of_thread, err := strconv.Atoi(os.Args[1])
	if err != nil {
		fmt.Println("error with parsing to int: number_of_thread", err)
		return
	}
	if number_of_thread < 1 {
		fmt.Println("illegal argument for number_of_threads (write more than 0)")
		return
	}
	chanPi := make(chan float64, number_of_thread)

	for i := 0; i < number_of_thread; i++ {
		wg.Add(1)
		go func(i int) {
			defer wg.Done()
			calculatePi(i, number_of_thread, num_steps, chanPi)
		}(i)
	}

	var pi float64

	wg.Wait()

	for {
		select {
		case tmp := <-chanPi:
			pi += tmp
		case <-chanPi:
			break
		}
	}

	fmt.Println("pi done - ", pi)
}
