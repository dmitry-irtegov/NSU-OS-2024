package main

import (
	"fmt"
	"time"
)

func printTexts(s string) {
	for i := 0; i < 10; i++ {
		fmt.Println(s)
	}
}

func main() {
	go printTexts("Parent Thread")
	printTexts("Child Thread")
	time.Sleep(time.Second)
}
