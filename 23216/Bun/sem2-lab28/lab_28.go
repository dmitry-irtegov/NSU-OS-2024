package main

import (
	"bufio"
	"fmt"
	"golang.org/x/sys/unix"
	"net"
	"os"
	"strings"
)

const maxLines = 25

func main() {
	if len(os.Args) < 2 {
		fmt.Println("Usage: go run lab_28.go <URL>")
		os.Exit(1)
	}
	url := os.Args[1]

	host, path := parseURL(url)
	//connecting server
	conn, err := net.Dial("tcp", host+":80")
	if err != nil {
		fmt.Println("Error connecting:", err)
		os.Exit(1)
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
		os.Exit(1)
	}
	err = rawConn.Control(func(s uintptr) {
		fd = s
	})
	if err != nil {
		fmt.Println("Error of func Control", err)
		os.Exit(1)
	}

	lineCount := 0
	for {
		// preparing fd for select()
		fds := &unix.FdSet{}
		fds.Bits[fd/64] |= 1 << (fd % 64)
		fds.Bits[0] |= 1 << 0

		_, err := unix.Select(int(fd)+1, fds, nil, nil, nil)
		if err != nil {
			fmt.Println("\nError selecting fd:", err)
			os.Exit(1)
		}

		if fds.Bits[fd/64]&(1<<(fd%64)) == 0 {
			panic("Error selecting fd")
		}
		line, err := reader.ReadString('\n')
		if err != nil {
			fmt.Println("Connection closed.")
			break
		}

		fmt.Print(line)
		lineCount++

		if lineCount >= maxLines {
			fmt.Print("Press space to scroll down... ")
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
			lineCount = 0
			fmt.Print("\r\033[K")
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
