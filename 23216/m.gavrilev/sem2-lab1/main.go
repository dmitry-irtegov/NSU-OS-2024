package main

import (
	"fmt"
	"sync"
)

func main() {
	var wg sync.WaitGroup
	wg.Add(1)
	go func() {
		for i := 0; i < 10; i++ {
			fmt.Println("goroutine - " + fmt.Sprint(i))
		}
		wg.Done()
	}()
	for i := 0; i < 10; i++ {
		fmt.Println("main - " + fmt.Sprint(i))
	}
	wg.Wait()
}
