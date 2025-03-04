package main

import (
	"bufio"
	"fmt"
	"os"
	"sync"
	"time"
)

type Node struct {
	data string
	next *Node
}

type LinkedList struct {
	head *Node
	mu   sync.Mutex
}

func (list *LinkedList) Insert(data string) {
	list.mu.Lock()
	defer list.mu.Unlock()

	NewNode := &Node{data: data, next: list.head}
	list.head = NewNode
}

func (list *LinkedList) Bubble() {
	list.mu.Lock()
	defer list.mu.Unlock()

	if list.head == nil {
		return
	}

	sortable := true
	for sortable {
		sortable = false
		current := list.head
		for current.next != nil {
			if current.data > current.next.data {
				
				buf := current.data
				current.data = current.next.data
				current.next.data = buf
				sortable = true
			}
			current = current.next
		}
	}
}

func (list *LinkedList) PrintList() {
	list.mu.Lock()
	defer list.mu.Unlock()

	current := list.head
	if current == nil {
		fmt.Println("The list is empty.")
		return
	}

	for current != nil {
		fmt.Println(current.data)
		current = current.next
	}
}

func main() {
	list := &LinkedList{}
	scanner := bufio.NewScanner(os.Stdin)

	done := make(chan struct{})

	go func() {
		for {
			select {
			case <-done:
				return 
			default:
				time.Sleep(5 * time.Second)
				list.Bubble()
			}
		}
	}()

	for {
		fmt.Print("Enter a string (or 'exit' to quit): ")
		if !scanner.Scan() {
			fmt.Println("Error reading input.")
			close(done)
			break
		}
		input := scanner.Text()

		if input == "" {
			fmt.Println("Current state: ")
			list.PrintList()
			continue
		}

		if input == "exit" {
			fmt.Println("Exiting...")
			close(done) 
			break
		}

		for len(input) > 80 {
			list.Insert(input[:80])
			input = input[80:]
		}
		list.Insert(input)
	}
    if err := scanner.Err(); err != nil {
		fmt.Println("Error reading input:", err)
	}

	fmt.Println("Program finished.")
}
