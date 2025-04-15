package main

import (
	"bufio"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"net/http"
	"os"
	"sem2-lab28/terminal"
	"sync"
)

var (
	errTermInputRead        = errors.New("terminal input read error")
	errTermFailedToRestore  = errors.New("failed to restore terminal state")
	errTermInterruptedByUser = errors.New("interrupted by user")
)

const (
	linesPerPage        = 25
	networkBufferSize   = 4096
	errorResponseLimit  = 512
	dataChannelBuffer   = 100
	scannerMaxCapacity  = 1024 * 1024
	asciiCtrlC          = 3
	paginationClearLine = "\r                               \r"
)

func networkReader(resp *http.Response, dataChan chan<- []byte, wg *sync.WaitGroup) {
	defer wg.Done()
	defer resp.Body.Close()
	defer close(dataChan)

	buffer := make([]byte, networkBufferSize)
	for {
		n, err := resp.Body.Read(buffer)
		if n > 0 {
			dataCopy := make([]byte, n)
			copy(dataCopy, buffer[:n])
			dataChan <- dataCopy
		}
		if err == io.EOF {
			break
		}
		if err != nil {
			fmt.Fprintf(os.Stderr, "\n[Error reading response body: %v]\n", err)
			break
		}
	}
}

func handlePagination(stdinFd int, isInteractive bool, linesPrinted *int) (keepScanning bool, err error) {
	if !isInteractive || *linesPrinted < linesPerPage {
		return true, nil
	}

	initialState, makeRawErr := terminal.MakeRaw(stdinFd)
	if makeRawErr != nil {
		fmt.Fprintf(os.Stderr, "\n[Warning: cannot set raw mode: %v]\n--- Press Enter to continue ---\n", makeRawErr)
		bufio.NewReader(os.Stdin).ReadBytes('\n')
		fmt.Print(paginationClearLine)
		*linesPrinted = 0
		return true, nil
	}

	defer func() {
		restoreErr := terminal.Restore(stdinFd, initialState)
		if restoreErr != nil {
			fmt.Fprintf(os.Stderr, "\n[Critical: failed to restore terminal state in defer: %v]\n", restoreErr)
			if err == nil {
				err = fmt.Errorf("%w: %w", errTermFailedToRestore, restoreErr)
			}
		}
	}()

	fmt.Print("--- Press space to scroll down ---")

	keyBytes := make([]byte, 1)
	keepScanning = true

	for {
		n, readErr := os.Stdin.Read(keyBytes[:])

		if readErr != nil {
			fmt.Fprintf(os.Stderr, "\n[Error reading pagination input: %v]\n", readErr)
			keepScanning = false
			err = fmt.Errorf("%w: %w", errTermInputRead, readErr)
			break
		}

		if n == 1 {
			switch keyBytes[0] {
			case ' ':
				fmt.Print(paginationClearLine)
				*linesPrinted = 0
				keepScanning = true
				break
			case 'q', 'Q', asciiCtrlC:
				fmt.Println("\n[Interrupted by user]")
				keepScanning = false                  
				err = errTermInterruptedByUser        
				break
			default:
				continue
			}
		} else {
			fmt.Fprintf(os.Stderr, "\n[Error reading pagination input: unexpected read result (n=%d)]\n", n)
			keepScanning = false
			err = fmt.Errorf("%w: unexpected read result (n=%d)", errTermInputRead, n)
			break
		}

		break
	}

	return keepScanning, err
}

func userInteractor(dataChan <-chan []byte, wg *sync.WaitGroup) {
	defer wg.Done()

	linesPrinted := 0
	stdinFd := int(os.Stdin.Fd())
	stdoutFd := int(os.Stdout.Fd())
	isInteractive := terminal.IsTerminal(stdinFd) && terminal.IsTerminal(stdoutFd)

	pipeReader, pipeWriter := io.Pipe()
	scanner := bufio.NewScanner(pipeReader)
	buf := make([]byte, bufio.MaxScanTokenSize)
	scanner.Buffer(buf, scannerMaxCapacity)

	go func() {
		defer pipeWriter.Close()
		for dataChunk := range dataChan {
			if _, writeErr := pipeWriter.Write(dataChunk); writeErr != nil {
				return
			}
		}
	}()

	keepScanning := true
	var loopErr error

	for keepScanning && scanner.Scan() {
		fmt.Println(scanner.Text())
		linesPrinted++

		var paginationErr error
		keepScanning, paginationErr = handlePagination(stdinFd, isInteractive, &linesPrinted)

		if paginationErr != nil {
			loopErr = paginationErr
			break
		}
	}

	finalErr := loopErr
	if finalErr == nil {
		finalErr = scanner.Err()
	}

	if finalErr != nil {
		if errors.Is(finalErr, errTermInterruptedByUser) {
		} else if errors.Is(finalErr, errTermInputRead) {
		} else if errors.Is(finalErr, errTermFailedToRestore) {
		} else if errors.Is(finalErr, bufio.ErrTooLong) {
			fmt.Fprintf(os.Stderr, "\n[Error scanning response: Line too long...]\n")
		} else if errors.Is(finalErr, io.EOF) || errors.Is(finalErr, io.ErrClosedPipe) {
		} else {
			fmt.Fprintf(os.Stderr, "\n[Error during interaction: %v]\n", finalErr)
		}
	}
}

func main() {
	if len(os.Args) != 2 {
		fmt.Fprintf(os.Stderr, "Usage: %s <URL>\n", os.Args[0])
		os.Exit(1)
	}
	url := os.Args[1]

	fmt.Fprintf(os.Stderr, "[Fetching %s ...]\n", url)
	resp, err := http.Get(url)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error fetching URL %s: %v\n", url, err)
		os.Exit(1)
	}

	if resp.StatusCode != http.StatusOK {
		fmt.Fprintf(os.Stderr, "Error: Received non-200 status code: %d %s\n", resp.StatusCode, resp.Status)
		limitReader := io.LimitReader(resp.Body, errorResponseLimit)
		bodyBytes, readErr := ioutil.ReadAll(limitReader)
		resp.Body.Close()
		if readErr != nil {
			fmt.Fprintf(os.Stderr, "Additionally, error reading response body for details: %v\n", readErr)
		} else if len(bodyBytes) > 0 {
			fmt.Fprintf(os.Stderr, "Response body (partial): %s\n", string(bodyBytes))
		}
		os.Exit(1)
	}

	dataChan := make(chan []byte, dataChannelBuffer)
	var wg sync.WaitGroup
	wg.Add(2)

	go networkReader(resp, dataChan, &wg)
	go userInteractor(dataChan, &wg)

	wg.Wait()

	fmt.Fprintf(os.Stderr, "\n[Done]\n")
}