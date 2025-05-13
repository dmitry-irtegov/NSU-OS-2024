package main

import (
	"bufio"
	"context"
	"fmt"
	"io"
	"math/rand"
	"os"
	"strings"
	"sync"
	"time"
)

const (
	sortObservationDelay time.Duration = 50 * time.Millisecond
	numSorters                         = 10
	maxLength                          = 80
	minSortInterval                    = 3 * time.Second
	intervalRange                      = int64(12 * time.Second)
)

type Node struct {
	data string
	next *Node
	mu   sync.RWMutex
}

type LinkedList struct {
	head   *Node
	headMu sync.RWMutex
}

func (l *LinkedList) Add(data string) {
	l.headMu.Lock()
	defer l.headMu.Unlock()
	newNode := &Node{data: data, next: l.head}
	l.head = newNode
}

func (l *LinkedList) Print() {
	fmt.Println("--- Текущее состояние списка ---")
	l.headMu.RLock()
	current := l.head
	l.headMu.RUnlock()
	if current == nil {
		fmt.Println("Список пуст.")
		fmt.Println("--------------------------------")
		return
	}
	for i := 0; current != nil; i++ {
		current.mu.RLock()
		nodeData := current.data
		nextNode := current.next
		current.mu.RUnlock()
		fmt.Printf("[%d] -> %s\n", i, nodeData)
		current = nextNode
	}
	fmt.Println("--------------------------------")
}

func (l *LinkedList) Sort() {
	l.headMu.RLock()
	if l.head == nil {
		l.headMu.RUnlock()
		return
	}
	l.head.mu.RLock()
	headNext := l.head.next
	l.head.mu.RUnlock()
	l.headMu.RUnlock()

	if headNext == nil {
		return
	}

	for swapped := true; swapped; {
		swapped = false
		var current *Node
		var next_node *Node
		var prevNode *Node

		l.headMu.Lock()
		current = l.head
		if current == nil {
			l.headMu.Unlock()
			continue
		}
		current.mu.Lock()
		next_node = current.next
		if next_node == nil {
			current.mu.Unlock()
			l.headMu.Unlock()
			continue
		}
		next_node.mu.Lock()

		currentData := current.data
		nextNodeData := next_node.data

		if sortObservationDelay > 0 {
			time.Sleep(sortObservationDelay)
		}

		if currentData > nextNodeData {
			current.next = next_node.next
			next_node.next = current
			l.head = next_node
			swapped = true
			next_node.mu.Unlock()
			current.mu.Unlock()
			l.headMu.Unlock()
			prevNode = next_node
		} else {
			next_node.mu.Unlock()
			current.mu.Unlock()
			l.headMu.Unlock()
			prevNode = current
		}

		for {
			prevNode.mu.Lock()
			current = prevNode.next
			if current == nil {
				prevNode.mu.Unlock()
				break
			}
			current.mu.Lock()
			next_node = current.next
			if next_node == nil {
				current.mu.Unlock()
				prevNode.mu.Unlock()
				break
			}
			next_node.mu.Lock()

			currentData = current.data
			nextNodeData = next_node.data

			if sortObservationDelay > 0 {
				time.Sleep(sortObservationDelay)
			}

			if currentData > nextNodeData {
				current.next = next_node.next
				next_node.next = current
				prevNode.next = next_node
				swapped = true
				next_node.mu.Unlock()
				current.mu.Unlock()
				prevNode.mu.Unlock()
				prevNode = next_node
				// было  1  2 1  3
				// стало 1  1 2  3
				//       p  c n n.n
				//            p
			} else {
				next_node.mu.Unlock()
				current.mu.Unlock()
				prevNode.mu.Unlock()
				prevNode = current
				// было  1  1 2  1  3
				// стало 1  1 2  1  3
				//       p  c n n.n
				//          p
			}
		}
	}
}

func sorter(ctx context.Context, wg *sync.WaitGroup, id int, list *LinkedList, sleepTime time.Duration) {
	ticker := time.NewTicker(sleepTime)
	defer func() {
		ticker.Stop()
		wg.Done()
	}()
	fmt.Printf("[Sorter %d] Запущен и спит по %d (секунд).\n", id, sleepTime/time.Second)

	for {
		select {
		case <-ticker.C:
		case <-ctx.Done():
			fmt.Printf("[Sorter %d] Завершаю работу...\n", id)
			return
		}
		// fmt.Printf("[Sorter %d] Начинаю сортировку...\n", id)
		// startTime := time.Now()
		
		list.Sort()

		// duration := time.Since(startTime)
		// fmt.Printf("[Sorter %d] Сортировка завершена за %v.\n", id, duration)

		// fmt.Printf("[Sorter %d] Следующая сортировка через ~%v\n", id, sleepDuration)
	}
}

func main() {
	rand.Seed(time.Now().UnixNano())

	list := &LinkedList{}
	ctx, cancel := context.WithCancel(context.Background())
	var wg sync.WaitGroup

	fmt.Printf("Запуск %d горутин сортировщиков...\n", numSorters)
	for i := 0; i < numSorters; i++ {
		wg.Add(1)
		randomOffset := time.Duration(rand.Int63n(intervalRange))
		sleepDuration := minSortInterval + randomOffset

		go sorter(ctx, &wg, i, list, sleepDuration)
	}
	fmt.Println("Сортировщики запущены.")
	time.Sleep(100 * time.Millisecond)

	fmt.Println("\nПрограмма запущена, введите строки (пустая строка для вывода списка, Ctrl+D для выхода)")
	reader := bufio.NewReader(os.Stdin)

	for {
		fmt.Print("> ")
		input, err := reader.ReadString('\n')
		if err != nil {
			if err == io.EOF {
				fmt.Println("\nПолучен EOF (Ctrl+D), начинаем завершение...")
				break
			}
			fmt.Println("Ошибка чтения ввода:", err)
			break
		}

		line := strings.TrimSpace(input)
		if line == "" {
			list.Print()
		} else {
			start := 0
			for start < len(line) {
				end := start + maxLength
				if end > len(line) {
					end = len(line)
				}
				chunk := line[start:end]
				chunkBytes := []byte(chunk)
				chunkCopy := string(chunkBytes)
				list.Add(chunkCopy)
				start = end
			}
		}
	}

	fmt.Println("Отправка сигнала отмены сортировщикам...")
	cancel()
	fmt.Println("Ожидание завершения всех горутин сортировщиков...")
	wg.Wait()
	fmt.Println("Все сортировщики завершили работу.")
	fmt.Println("\nФинальное состояние списка:")
	list.Print()
	fmt.Println("Программа завершена.")
}
