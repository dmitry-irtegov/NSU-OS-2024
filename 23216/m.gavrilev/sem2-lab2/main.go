package main

import (
	"fmt"
)

func printer(msg string) {
	for i := 0; i < 10; i++ {
		fmt.Println(msg + " - " + fmt.Sprint(i))
	}
}

func main() {
	done := make(chan struct{})
	go func() {
		printer("goroutine")
		done <- struct{}{}
	}()
	<-done
	printer("main")

}
