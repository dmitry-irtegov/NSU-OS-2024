package prom

import (
	"fmt"
	"os"
	"syscall"
)

var prompt []byte

func SetPrompt(newPrompt []byte) {
	prompt = newPrompt
}

func Promptline(line []byte) []byte {
	_, err := syscall.Write(int(os.Stdout.Fd()), prompt)
	if err != nil {
		fmt.Println("Write problem")
		os.Exit(1)
	}
	str := make([]byte, 1024)
	var quotestype byte
	var i int
	for {
		n, err := syscall.Read(int(os.Stdin.Fd()), str)
		if err != nil {
			fmt.Println("Read problem")
			os.Exit(2)
		}
		line = append(line, str[:n]...)
		for ; i < len(line); i++ {
			if quotestype == 0 && line[i] == '\\' && line[i+1] != '\n' {
				line = append(line[:i], line[i+1:]...)
			} else if line[i] == '\\' && line[i+1] == quotestype {
				line = append(line[:i], line[i+1:]...)
			} else if quotestype == 0 && (line[i] == '\'' || line[i] == '"') {
				quotestype = line[i]
				line = append(line[:i], line[i+1:]...)
				i--
			} else if line[i] == quotestype {
				quotestype = 0
				line = append(line[:i], line[i+1:]...)
				i--
			}
		}
		if len(line) >= 2 && line[len(line)-2] == '\\' && line[len(line)-1] == '\n' {
			line = line[:len(line)-2]
			i -= 2
			fmt.Print("> ")
			continue
		}
		if quotestype != 0 {
			fmt.Print("> ")
			continue
		}
		line[len(line)-1] = 0
		return line
	}
}
