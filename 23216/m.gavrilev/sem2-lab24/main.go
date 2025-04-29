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

	totalWidgetsToProduce int64 = 15

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

		select {
		case <-ctx.Done():
			log.Printf("[%s] Остановка производителя (после отправки)...", partName)
			return
		default:
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
		select {
		case <-ctx.Done():
			log.Printf("[Сборщик Модулей %d] Остановка...", id)
			return
		default:
		}

		log.Printf("[Сборщик Модулей %d] Ожидает деталь A...", id)
		select {
		case <-semA:
			log.Printf("[Сборщик Модулей %d] Взял деталь A.", id)
		case <-ctx.Done():
			log.Printf("[Сборщик Модулей %d] Остановка во время ожидания A...", id)
			return
		}

		log.Printf("[Сборщик Модулей %d] Ожидает деталь B...", id)
		select {
		case <-semB:
			log.Printf("[Сборщик Модулей %d] Взял деталь B.", id)
		case <-ctx.Done():
			log.Printf("[Сборщик Модулей %d] Остановка во время ожидания B...", id)
			return
		}

		log.Printf("[Сборщик Модулей %d] -> Модуль собран.", id)
		select {
		case semModule <- struct{}{}:
			log.Printf("[Сборщик Модулей %d] Модуль отправлен на склад.", id)
		case <-ctx.Done():
			log.Printf("[Сборщик Модулей %d] Остановка во время отправки Модуля...", id)
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
	target int64,
	cancel context.CancelFunc,
	ctx context.Context,
	wg *sync.WaitGroup,
) {
	defer wg.Done()
	for {
		currentCount := atomic.LoadInt64(widgetCounter)
		if currentCount >= target {
			log.Printf("[Сборщик Винтиков %d] Цель (%d) уже достигнута. Остановка.", id, target)
			return
		}
		select {
		case <-ctx.Done():
			log.Printf("[Сборщик Винтиков %d] Остановка (начало цикла)...", id)
			return
		default:
		}

		log.Printf("[Сборщик Винтиков %d] Ожидает Модуль...", id)
		select {
		case <-semModule:
			log.Printf("[Сборщик Винтиков %d] Взял Модуль.", id)
		case <-ctx.Done():
			log.Printf("[Сборщик Винтиков %d] Остановка во время ожидания Модуля...", id)
			return
		}

		log.Printf("[Сборщик Винтиков %d] Ожидает деталь C...", id)
		select {
		case <-semC:
			log.Printf("[Сборщик Винтиков %d] Взял деталь C.", id)
		case <-ctx.Done():
			log.Printf("[Сборщик Винтиков %d] Остановка во время ожидания C...", id)
			return
		}

		newCount := atomic.AddInt64(widgetCounter, 1)
		log.Printf("[Сборщик Винтиков %d] ===> Винтик #%d собран! <===", id, newCount)

		if newCount >= target {
			log.Printf("[Сборщик Винтиков %d] !!! ЦЕЛЬ ДОСТИГНУТА (%d/%d) !!! Инициирую общую остановку.", id, newCount, target)
			cancel() // Сигнал остановки для всей линии
			return
		}
	}
}

func main() {
	log.Println("--- Запуск производственной линии ---")
	log.Printf("Цель: %d винтиков", totalWidgetsToProduce)
	log.Printf("Параметры: A:%ds(%d), B:%ds(%d), C:%ds(%d), МодульСборщики:%d, ВинтикСборщики:%d",
		durationA/time.Second, numProducersA,
		durationB/time.Second, numProducersB,
		durationC/time.Second, numProducersC,
		numModuleAssemblers, numWidgetAssemblers)
	log.Printf("Размеры буферов: A:%d, B:%d, C:%d, Модуль:%d",
		bufferSizeA, bufferSizeB, bufferSizeC, bufferSizeModule)
	log.Println("------------------------------------")

	ctx, cancel := context.WithCancel(context.Background())
	var wg sync.WaitGroup
	var widgetCounter int64

	semA := make(chan struct{}, bufferSizeA)
	semB := make(chan struct{}, bufferSizeB)
	semC := make(chan struct{}, bufferSizeC)
	semModule := make(chan struct{}, bufferSizeModule)

	// Запуск производителей
	for i := 0; i < numProducersA; i++ {
		wg.Add(1)
		go producer(fmt.Sprintf("A-%d", i+1), durationA, semA, ctx, &wg)
	}
	for i := 0; i < numProducersB; i++ {
		wg.Add(1)
		go producer(fmt.Sprintf("B-%d", i+1), durationB, semB, ctx, &wg)
	}
	for i := 0; i < numProducersC; i++ {
		wg.Add(1)
		go producer(fmt.Sprintf("C-%d", i+1), durationC, semC, ctx, &wg)
	}

	// Запуск сборщиков модулей
	for i := 0; i < numModuleAssemblers; i++ {
		wg.Add(1)
		go moduleAssembler(i+1, semA, semB, semModule, ctx, &wg)
	}

	// Запуск сборщиков винтиков
	for i := 0; i < numWidgetAssemblers; i++ {
		wg.Add(1)
		go widgetAssembler(i+1, semModule, semC, &widgetCounter, totalWidgetsToProduce, cancel, ctx, &wg)
	}

	// Обработка сигнала ОС для грациозной остановки
	sigChan := make(chan os.Signal, 1)
	signal.Notify(sigChan, syscall.SIGINT, syscall.SIGTERM)

	// Ожидание завершения
	select {
	case <-ctx.Done():
		log.Println("--- Сигнал остановки получен (цель достигнута) ---")
	case sig := <-sigChan:
		log.Printf("--- Сигнал ОС получен (%s), инициирую остановку ---", sig)
		cancel()
	}

	// Ожидание фактического завершения всех горутин
	log.Println("--- Ожидание завершения всех процессов... ---")
	wg.Wait()

	log.Println("------------------------------------")
	log.Printf("--- Производственная линия остановлена ---")
	log.Printf("--- Итого произведено винтиков: %d ---", atomic.LoadInt64(&widgetCounter))
	log.Println("------------------------------------")
}