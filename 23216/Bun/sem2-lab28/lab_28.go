package main

import (
	"bufio"
	"fmt"
	"golang.org/x/sys/unix"
	"golang.org/x/term"
	"net"
	"os"
	"strings"
)

const maxLines = 25

func main() {
	if len(os.Args) < 2 {
		fmt.Println("Usage: go run lab_28.go <URL>")
		return
	}
	url := os.Args[1]

	host, path := parseURL(url)
	//connecting server
	conn, err := net.Dial("tcp", host+":80")
	if err != nil {
		fmt.Println("Error connecting:", err)
		return
	}
	defer func(conn net.Conn) {
		err := conn.Close()
		if err != nil {
			fmt.Println("Error closing connection:", err)
			return
		}
	}(conn)

	// send HTTP-request
	request := fmt.Sprintf("GET %s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", path, host)
	_, err = conn.Write([]byte(request))
	if err != nil {
		fmt.Println("Error sending request:", err)
		return
	}

	// create reader for reading response
	reader := bufio.NewReader(conn)

	// getting sockets fd
	var fd uintptr
	rawConn, err := conn.(*net.TCPConn).SyscallConn()
	if err != nil {
		fmt.Println("Error getting raw connection:", err)
		return
	}
	err = rawConn.Control(func(s uintptr) {
		fd = s
	})
	if err != nil {
		fmt.Println("Error of func Control", err)
		return
	}

	lineCount := 0
	for {
		// preparing fd for select()
		fds := &unix.FdSet{}
		fds.Bits[fd/64] |= 1 << (fd % 64) // socket
		fds.Bits[0] |= 1 << 0             // stdin

		_, err := unix.Select(int(fd)+1, fds, nil, nil, nil)
		if err != nil {
			fmt.Println("Error selecting fd:", err)
			return
		}

		if fds.Bits[fd/64]&(1<<(fd%64)) != 0 {
			line, err := reader.ReadString('\n')
			if err != nil {
				fmt.Println("\nConnection closed.")
				break
			}

			fmt.Print(line)
			lineCount++

			if lineCount >= maxLines {
				fmt.Print("Press space to scroll down... ")
				
				oldState, err := term.MakeRaw(int(os.Stdin.Fd()))
				// waiting enter space
				for {
					input := make([]byte, 1)
					_, err := os.Stdin.Read(input)
					if err != nil {
						fmt.Println("Error reading input:", err)
						return
					}
					if input[0] == ' ' {
						break
					}
				}
				err = term.Restore(int(os.Stdin.Fd()), oldState)
				if err != nil {
					fmt.Println("Error restoring terminal:", err)
					return
				}
				lineCount = 0
				fmt.Print("\r\033[K")
			}
		}
	}
	fmt.Println("Success.")
}

func parseURL(url string) (string, string) {
	if strings.HasPrefix(url, "https://") {
		url = url[len("https://"):]
	} else if strings.HasPrefix(url, "http://") {
		url = url[len("http://"):]
	}
	parts := strings.SplitN(url, "/", 2)
	host := parts[0]
	path := "/"
	if len(parts) > 1 {
		path += parts[1]
	}
	return host, path
}
