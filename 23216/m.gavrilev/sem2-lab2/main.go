package main

import (
	"fmt"
)

func main() {
	done := make(chan bool)
	go func() {
		for i := 0; i < 10; i++ {
			fmt.Println("goroutine - " + fmt.Sprint(i))
		}
		done <- true
	}()
	<-done
	for i := 0; i < 10; i++ {
		fmt.Println("main - " + fmt.Sprint(i))
	}

}
