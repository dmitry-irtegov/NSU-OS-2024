package prom

import (
	"fmt"
	"os"
	"syscall"
	_ "net/http/pprof"
	"log"
	"net/http"
)

var prompt []byte

func SetPrompt(newPrompt []byte) {
	prompt = newPrompt
}

func Func() {
	log.Println(http.ListenAndServe("localhost:6060", nil))
}

func Promptline(line []byte) ([]byte, error) {
	_, err := syscall.Write(int(os.Stdout.Fd()), prompt)
	if err != nil {
		return nil, err
	}
	str := make([]byte, 1024)
	var quotestype byte
	var i int
	for {
		n, err := syscall.Read(int(os.Stdin.Fd()), str)
		if err != nil {
			return nil, err
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
		return line, err
	}
}
