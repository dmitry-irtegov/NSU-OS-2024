package main

import (
	"fmt"
	"sync"
)

func printTexts(s string, cond *sync.Cond, turn *bool, myTurn bool) {
	for i := 1; i <= 10; i++ {
		cond.L.Lock()
		for *turn != myTurn {
			cond.Wait()
		}
		fmt.Println(s, i)
		*turn = !(*turn)
		cond.L.Unlock()
		cond.Signal()
	}
}

func main() {
	var wg sync.WaitGroup
	var lock sync.Mutex
	cond := sync.NewCond(&lock)
	flagThread := false

	wg.Add(1)
	go func() {
		printTexts("Child Thread", cond, &flagThread, true)
		wg.Done()
	}()

	printTexts("Parent Thread", cond, &flagThread, false)
	wg.Wait()
}
