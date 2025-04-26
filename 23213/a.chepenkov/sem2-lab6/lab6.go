package main

import (
    "bufio"
    "fmt"
    "os"
    "sync"
    "time"
)

const (
    MAX_LINES   = 100
    SLEEP_COEFF = 10
)

func sleepFunc(input string, wg *sync.WaitGroup) {
    defer wg.Done()
    time.Sleep(time.Duration(SLEEP_COEFF*len(input)) * time.Millisecond)
    fmt.Println(input)
}

func main() {
    var wg sync.WaitGroup
    var strings []string
    scanner := bufio.NewScanner(os.Stdin)
    var input string
    for i := 0; i < MAX_LINES && scanner.Scan(); i++ {
        input = scanner.Text()
        strings = append(strings, input)
    }
    fmt.Println()

    for i := 0; i < len(strings); i++ {
        wg.Add(1)
        go sleepFunc(strings[i], &wg)
    }

    wg.Wait()
}
