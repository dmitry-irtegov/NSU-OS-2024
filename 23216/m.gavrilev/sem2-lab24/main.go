package main

import (
	"context"
	"fmt"
	"log"
	"os"
	"os/signal"
	"sync"
	"sync/atomic"
	"syscall"
	"time"
)

// Конфигурация производственной линии
const (
	durationA = 1 * time.Second
	durationB = 2 * time.Second
	durationC = 3 * time.Second

	bufferSizeA       = 5
	bufferSizeB       = 5
	bufferSizeC       = 5
	bufferSizeModule  = 5

	numProducersA       = 2
	numProducersB       = 1
	numProducersC       = 1
	numModuleAssemblers = 2
	numWidgetAssemblers = 2
)

func init() {
	log.SetFlags(0) // Убираем префиксы времени/даты из логов
}

// producer производит детали, имитируя задержку, и сигнализирует через семафор.
func producer(
	partName string,
	productionTime time.Duration,
	semaphore chan<- struct{},
	ctx context.Context,
	wg *sync.WaitGroup,
) {
	defer wg.Done()
	for {
		time.Sleep(productionTime)

		select {
		case semaphore <- struct{}{}:
			log.Printf("[%s] -> Деталь произведена.", partName)
		case <-ctx.Done():
			log.Printf("[%s] Остановка производителя...", partName)
			return
		}
	}
}

// moduleAssembler собирает модуль из деталей A и B, сигнализирует через семафор.
func moduleAssembler(
	id int,
	semA <-chan struct{},
	semB <-chan struct{},
	semModule chan<- struct{},
	ctx context.Context,
	wg *sync.WaitGroup,
) {
	defer wg.Done()
	for {
		<- semA
		<- semB

		select {
		case semModule <- struct{}{}:
			log.Printf("[Сборщик Модулей %d] -> Модуль собран.", id)
		case <-ctx.Done():
			log.Printf("[Сборщик Модулей %d] Остановка...", id)
			return
		}
	}
}

// собирает винтик из Модуля и детали C.
// Также управляет счетчиком и инициирует остановку линии при достижении цели.
func widgetAssembler(
	id int,
	semModule <-chan struct{},
	semC <-chan struct{},
	widgetCounter *int64,
	ctx context.Context,
	wg *sync.WaitGroup,
) {
	defer wg.Done()
	for {
		<- semModule
		<- semC

		newCount := atomic.AddInt64(widgetCounter, 1)
		log.Printf("[Сборщик Винтиков %d] ===> Винтик #%d собран! <===", id, newCount)

		select {
		case <-ctx.Done():
			log.Printf("[Сборщик Винтиков %d] Остановка (начало цикла)...", id)
			return
		default:
		}
	}
}

func controller (
	ctx context.Context,
	widgetCounter *int64,
	wgMain *sync.WaitGroup,
) {
	defer wgMain.Done()

	var wgProd sync.WaitGroup
	var wgCons sync.WaitGroup

	semA := make(chan struct{}, bufferSizeA)
	semB := make(chan struct{}, bufferSizeB)
	semC := make(chan struct{}, bufferSizeC)
	semModule := make(chan struct{}, bufferSizeModule)

	// Запуск производителей
	for i := 0; i < numProducersA; i++ {
		wgProd.Add(1)
		go producer(fmt.Sprintf("A-%d", i+1), durationA, semA, ctx, &wgProd)
	}
	for i := 0; i < numProducersB; i++ {
		wgProd.Add(1)
		go producer(fmt.Sprintf("B-%d", i+1), durationB, semB, ctx, &wgProd)
	}
	for i := 0; i < numProducersC; i++ {
		wgProd.Add(1)
		go producer(fmt.Sprintf("C-%d", i+1), durationC, semC, ctx, &wgProd)
	}

	// Запуск сборщиков модулей
	for i := 0; i < numModuleAssemblers; i++ {
		wgCons.Add(1)
		go moduleAssembler(i+1, semA, semB, semModule, ctx, &wgCons)
	}

	// Запуск сборщиков винтиков
	for i := 0; i < numWidgetAssemblers; i++ {
		wgCons.Add(1)
		go widgetAssembler(i+1, semModule, semC, widgetCounter, ctx, &wgCons)
	}

	wgProd.Wait()
	wgCons.Wait()
}

func main() {
	log.Println("--- Запуск производственной линии ---")
	log.Printf("Параметры: A:%ds(%d), B:%ds(%d), C:%ds(%d), МодульСборщики:%d, ВинтикСборщики:%d",
		durationA/time.Second, numProducersA,
		durationB/time.Second, numProducersB,
		durationC/time.Second, numProducersC,
		numModuleAssemblers, numWidgetAssemblers)
	log.Printf("Размеры буферов: A:%d, B:%d, C:%d, Модуль:%d",
		bufferSizeA, bufferSizeB, bufferSizeC, bufferSizeModule)
	log.Println("------------------------------------")

	ctx, cancel := context.WithCancel(context.Background())
	var widgetCounter int64
	var wg sync.WaitGroup

	go controller(ctx, &widgetCounter, &wg)
	
	// Обработка сигнала ОС для грациозной остановки
	sigChan := make(chan os.Signal, 1)
	signal.Notify(sigChan, syscall.SIGINT, syscall.SIGTERM)

	// Ожидание завершения
	sig := <-sigChan
		log.Printf("--- Сигнал ОС получен (%s), инициирую остановку ---", sig)
		cancel()

	// Ожидание фактического завершения всех горутин
	log.Println("--- Ожидание завершения контроллера... ---")
	wg.Wait()

	log.Println("------------------------------------")
	log.Printf("--- Производственная линия остановлена ---")
	log.Printf("--- Итого произведено винтиков: %d ---", atomic.LoadInt64(&widgetCounter))
	log.Println("------------------------------------")
}