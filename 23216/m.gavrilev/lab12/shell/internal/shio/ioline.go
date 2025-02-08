package shio

import (
	"fmt"
	"io"
	"os"
)

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
		var quotestype byte
		for i := 0; i < len(line); i++ {
			if quotestype == 0 {
				if line[i] == '\'' || line[i] == '"' {
					quotestype = line[i]
				} else if line[i] == '\\' {
					i++
				}
			} else if quotestype == '"' {
				if line[i] == '"' {
					quotestype = 0
				} else if line[i] == '\\' && (line[i+1] == '"' || line[i+1] == '\\') {
					i++
				}
			} else if quotestype == '\'' {
				if line[i] == '\'' {
					quotestype = 0
				}
			}
		}
		if len(line) >= 2 && line[len(line)-2] == '\\' && line[len(line)-1] == '\n' {

			if quotestype != '\'' {
				line = line[:len(line)-2]
			}
			fmt.Print("> ")
			continue
		}
		if len(line) >= 1 && line[len(line)-1] != '\n' {
			line = append(line, '\n')
		}
		if quotestype != 0 {
			fmt.Print("> ")
			continue
		}
		if len(line) >= 1 && line[len(line)-1] == '\n' {
			line[len(line)-1] = 0
		}
		return line, nil
	}
}

func Prompt() error {
	prompt := (fmt.Sprintf("[%s] ", os.Args[0]))
	_, err := fmt.Print(prompt)
	if err != nil {
		fmt.Println("write error")
		return err
	}
	return nil
}
