package util

import (
	"fmt"
	"io"
	"os"
	"syscall"
)

func Prompt() error {
	prompt := ([]byte)(fmt.Sprintf("[%s] ", os.Args[0]))
	_, err := syscall.Write(int(os.Stdout.Fd()), prompt)
	if err != nil {
		fmt.Println("write error")
		return err
	}
	return nil
}

func ReadLine() ([]byte, error) {
	str := make([]byte, 1024)
	var line []byte
	reader := io.Reader(os.Stdin)
	for {
		n, err := reader.Read(str)
		if err != nil {
			return nil, err
		} else if n == 0 {
			return nil, nil
		}
		line = append(line, str[:n]...)
		if len(line) >= 2 && line[len(line)-2] == '\\' && line[len(line)-1] == '\n' {
			line = line[:len(line)-2]
			fmt.Print("> ")
			continue
		}
		line[len(line)-1] = 0
		return line, nil
	}
}
