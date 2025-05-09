package main

import (
	"fmt"
	"sync"
)

func printTexts(s string, lock *sync.Mutex, turn *bool, myTurn bool) {
	for i := 1; i <= 10; i++ {
		for {
			lock.Lock()
			if *turn == myTurn {
				fmt.Println(s, i)
				*turn = !(*turn)
				lock.Unlock()
				break
			}
			lock.Unlock()
		}
	}
}

func main() {
	var wg sync.WaitGroup
	var lock sync.Mutex
	flagThread := false

	wg.Add(1)
	go func() {
		printTexts("Child Thread", &lock, &flagThread, true)
		wg.Done()
	}()

	printTexts("Parent Thread", &lock, &flagThread, false)
	wg.Wait()
}
