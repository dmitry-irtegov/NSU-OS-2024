package main

import (
	"fmt"
)

func printer(input string) {
	for i := 0; i < 10; i++ {
		fmt.Println(input + fmt.Sprint(i))
	}
}

func printerGoroutine(input string, done chan bool) {
	for i := 0; i < 10; i++ {
		fmt.Println(input + fmt.Sprint(i))
	}
	done <- true
}

func main() {
	done := make(chan bool)
	go printerGoroutine("thread", done)
	<-done
	printer("main")
}
