package main

import (
	"fmt"
	"sync"
)

func printTexts(s string) {
	for i := 1; i <= 10; i++ {
		fmt.Println(s, i)
	}
}

func main() {
	var wg sync.WaitGroup
	wg.Add(1)
	go func() {
		printTexts("Child Thread")
		wg.Done()
	}()

	printTexts("Parent Thread")
	wg.Wait()
}
