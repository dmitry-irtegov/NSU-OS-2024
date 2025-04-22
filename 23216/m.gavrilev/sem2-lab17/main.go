package main

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"strings"
	"sync"
	"time"
)

type Node struct {
	data string // строка <= 80 символов
	next *Node
}

type LinkedList struct {
	head *Node
	mu   sync.Mutex
}

func (l *LinkedList) Add(data string) {
	l.mu.Lock()
	defer l.mu.Unlock()

	newNode := &Node{
		data: data,
		next: l.head,
	}
	l.head = newNode
}

func (l *LinkedList) Print() {
	l.mu.Lock()
	defer l.mu.Unlock()

	fmt.Println("--- Текущее состояние списка ---")
	if l.head == nil {
		fmt.Println("Список пуст.")
		fmt.Println("--------------------------------")
		return
	}

	current := l.head
	i := 0
	for current != nil {
		fmt.Printf("[%d] -> %s\n", i, current.data)
		current = current.next
		i++
	}
	fmt.Println("--------------------------------")
}

func (l *LinkedList) Sort() {
	l.mu.Lock()
	defer l.mu.Unlock()

	if l.head == nil || l.head.next == nil {
		return
	}

	for swapped := true; swapped; {
		swapped = false

		p := &l.head

		for (*p) != nil && (*p).next != nil {
			current := *p
			next_node := current.next

			if current.data > next_node.data {
				current.next = next_node.next
				next_node.next = current
				*p = next_node

				swapped = true
			}
			p = &(*p).next
		}
	}
}

func sorter(list *LinkedList) {
	ticker := time.NewTicker(5 * time.Second)
	defer ticker.Stop()

	fmt.Println("Sorter: Горутина сортировки запущена")

	for range ticker.C {
		// fmt.Println("Sorter: Сортирую")
		list.Sort()
	}
}

func main() {
	list := &LinkedList{}

	go sorter(list)

	time.Sleep(100 * time.Millisecond)

	fmt.Println()
	fmt.Println("Программа запущена, введите строки")

	reader := bufio.NewReader(os.Stdin)
	const maxLength = 80

	for {
		fmt.Print("> ")
		input, err := reader.ReadString('\n')
		if err != nil {
			if err == io.EOF {
				break
			}
			fmt.Println("Ошибка чтения ввода:", err)
			break
		}

		line := strings.TrimSpace(input)

		if line == "" {
			list.Print()
		} else {
			// fmt.Printf("Получена")

			start := 0
			for start < len(line) {
				end := start + maxLength
				if end > len(line) {
					end = len(line)
				}

				chunkSlice := line[start:end]

				chunkBytes := []byte(chunkSlice)
				chunkCopy := string(chunkBytes)

				list.Add(chunkCopy)

				start = end
			}
			// fmt.Println("Добавлена")
		}
	} // for

	fmt.Println()
	fmt.Println("Вмер")
}
