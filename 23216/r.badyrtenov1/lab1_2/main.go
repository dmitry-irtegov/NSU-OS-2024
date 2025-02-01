package main

import (
	"fmt"
	"sync"
)

func printTexts(s string) {
	for i := 0; i < 10; i++ {
		fmt.Println(s)
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
