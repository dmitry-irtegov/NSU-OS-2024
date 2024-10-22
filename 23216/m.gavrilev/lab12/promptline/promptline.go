package promptline

import (
	"syscall"
)

func Promptline(prompt []byte, line []byte, sizline int64) int64 {

	var len int64 = 0
	tmpLine := make([]byte, 1024)
	_, err := syscall.Write(1, prompt)
	if err != nil {
		panic("Write Problem")
	}
	for {
		n, err := syscall.Read(0, tmpLine)
		line = append(line[:len], tmpLine[:n]...)
		if err != nil {
			panic("Read Problem")
		}
		b := int64(n)
		len += b
		if line[len-2] == '\\' && line[len-1] == '\n' {
			line[len-1] = ' '
			line[len-2] = ' '
			continue
		}
		line[len-1] = 0
		return (len)
	}
}
