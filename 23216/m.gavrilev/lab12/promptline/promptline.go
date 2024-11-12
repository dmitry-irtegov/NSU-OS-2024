package promptline

import (
	"fmt"
	"os"
	"syscall"
)

func Promptline(prompt []byte, line []byte) []byte {
	_, err := syscall.Write(int(os.Stdout.Fd()), prompt)
	if err != nil {
		fmt.Println("write problem")
		os.Exit(1)
	}
	str := make([]byte, 1024)
	for {
		n, err := syscall.Read(int(os.Stdin.Fd()), str)
		if err != nil {
			fmt.Println("read problem")
			os.Exit(2)
		}
		line = append(line, str[:n]...)
		if len(line) >= 2 && line[len(line)-2] == '\\' && line[len(line)-1] == '\n' {
			line = line[:len(line)-2]
			fmt.Print("> ")
			continue
		}
		line[len(line)-1] = 0
		return line
	}
}
