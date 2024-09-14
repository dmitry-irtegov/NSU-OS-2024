package main

import (
	"fmt"
	"time"
)

func main() {
	loc, _ := time.LoadLocation("PST8PDT")
	fmt.Println(time.Now().In(loc))
}
