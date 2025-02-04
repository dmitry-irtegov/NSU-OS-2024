package main

import (
	"fmt"
	"sync"
)

func printer(input string) {
	for i := 0; i < 10; i++ {
		fmt.Println(input + " - " + fmt.Sprint(i))
	}
}

func printerGoroutine(input string, wg *sync.WaitGroup) {
	defer wg.Done()
	for i := 0; i < 10; i++ {
		fmt.Println(input + " - " + fmt.Sprint(i))
	}
}

func main() {
	var wg sync.WaitGroup
	wg.Add(1)
	go printerGoroutine("thread", &wg)
	printer("main")
	wg.Wait()
}
