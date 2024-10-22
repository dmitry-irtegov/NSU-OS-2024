package prom

import (
	"syscall"
)

func Promptline(prompt []byte, line []byte) int {
	_, err := syscall.Write(1, prompt)
	if err != nil {
		panic("write problem\n")
	}
	len := 0
	for {
		n, err := syscall.Read(0, line[len:cap(line)])
		if err != nil {
			panic("read problem\n")
		}
		len += n
		if line[len-2] == '\\' && line[len-1] == '\n' {
			line[len-1] = ' '
			line[len-2] = ' '
			continue
		}
		line[len-1] = 0
		return (len)
	}
}
