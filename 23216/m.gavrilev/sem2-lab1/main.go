package main

import (
	"fmt"
	"sync"
)

func printer(msg string) {
	for i := 0; i < 10; i++ {
		fmt.Println(msg + " - " + fmt.Sprint(i))
	}
}

func main() {
	var wg sync.WaitGroup
	wg.Add(1)
	go func() {
		defer wg.Done()
		printer("goroutine")
	}()
	printer("main")
	wg.Wait()
}
