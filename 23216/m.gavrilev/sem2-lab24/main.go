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

	numProducersA       = 2
	numProducersB       = 1
	numProducersC       = 3
	numModuleAssemblers = 2
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
	// log.Printf("[%s] Запущен", partName) // DEBUG

	for {
		// log.Printf("[%s] Засыпаю...", partName)
		time.Sleep(productionTime)
		// log.Printf("[%s] Проснулся...", partName)

		select {
		case <-ctx.Done():
			log.Printf("[%s] Остановка производителя...", partName)
			return
		case semaphore <- struct{}{}:
			log.Printf("[%s] -> Деталь произведена.", partName)
		}
			
	}
}

// moduleAssembler собирает модуль из деталей A и B, сигнализирует через семафор.
func moduleAssembler(
	id int,
	semModule chan<- struct{},
	ctx context.Context,
	wgMain *sync.WaitGroup,
) {
	assemblerName := fmt.Sprintf("Сборщик Модулей %d", id)
	// log.Printf("[%s] Запущен", assemblerName) // Для отладки
	var wg sync.WaitGroup

	defer func() {
		log.Printf("[%s] Остановка...", assemblerName)
		wg.Wait()
		log.Printf("[%s] Остановлен.", assemblerName)
		wgMain.Done()
	}()

	semA := make(chan struct{})
	semB := make(chan struct{})

	for i := 0; i < numProducersA; i++ {
		wg.Add(1)
		partID := fmt.Sprintf("Деталь A-%d-%d", id, i+1)
		go producer(partID, durationA, semA, ctx, &wg)
	}
	for i := 0; i < numProducersB; i++ {
		wg.Add(1)
		partID := fmt.Sprintf("Деталь B-%d-%d", id, i+1)
		go producer(partID, durationB, semB, ctx, &wg)
	}

	selectSemA, selectSemB := semA, semB
	var selectSemModule chan<- struct{}

	// selectSemModule := semModule
	// selectSemModule = nil

	for {

		select {
		case <-ctx.Done():
			log.Printf("[%s] Остановка (контекст отменен)...", assemblerName)
			return
		case _, ok := <-selectSemA:
			if !ok {
				log.Printf("[%s] ОШИБКА: Канал A закрыт!", assemblerName)
				return
			} else {
				// log.Printf("[%s] Получена Деталь A", assemblerName) // DEBUG
				selectSemA = nil
			}
		case _, ok := <-selectSemB:
			if !ok {
				log.Printf("[%s] ОШИБКА: Канал B закрыт!", assemblerName)
				return
			} else {
				// log.Printf("[%s] Получена Деталь B", assemblerName) // DEBUG
				selectSemB = nil
			}
		case selectSemModule <- struct{}{}:
			log.Printf("[%s] -> Модуль отправлен", assemblerName)

			selectSemA = semA
			selectSemB = semB

			selectSemModule = nil
		}
		if selectSemA == nil && selectSemB == nil && selectSemModule == nil {
			// log.Printf("[%s] Готов отправить модуль", assemblerName) // DEBUG
			selectSemModule = semModule
		}
	}
}

// Собирает винтик из Модуля и детали C.
// Также управляет счетчиком и инициирует остановку линии при достижении цели.
func widgetAssembler(
	id int,
	widgetCounter *int64,
	ctx context.Context,
	doneChan chan struct{},
) {
	assemblerName := fmt.Sprintf("Сборщик Винтиков %d", id)
	var wg sync.WaitGroup
	// log.Printf("[%s] Запущен", assemblerName) // Для отладки

	defer func() {
		log.Printf("[%s] Ожидание остановки дочерних горутин...", assemblerName)
		wg.Wait()
		log.Printf("[%s] Остановлен.", assemblerName)
		close(doneChan)
	}()

	semModule := make(chan struct{})
	semC := make(chan struct{})

	for i := 0; i < numProducersC; i++ {
		wg.Add(1)
		partID := fmt.Sprintf("Деталь C-%d-%d", id, i+1)
		go producer(partID, durationC, semC, ctx, &wg)
	}

	for i := 0; i < numModuleAssemblers; i++ {
		wg.Add(1)
		go moduleAssembler(i+1, semModule, ctx, &wg)
	}

	selectSemModule, selectSemC := semModule, semC

	for { // like tee-channel

		select {
        case <-ctx.Done():
            log.Printf("[%s] Остановка (контекст отменен)...", assemblerName)
            return
        case _, ok := <-selectSemModule:
			if !ok {
				log.Printf("[%s] ОШИБКА: Канал semModule закрыт!", assemblerName)
				return
			} else {
				selectSemModule = nil
			}
        case _, ok := <-selectSemC:
			if !ok {
				log.Printf("[%s] ОШИБКА: Канал C закрыт!", assemblerName)
				return
			} else {
				selectSemC = nil
			}
        }
		if selectSemModule == nil && selectSemC == nil {
            newCount := atomic.AddInt64(widgetCounter, 1)
            log.Printf("[%s] ===> Собран Винтик #%d", assemblerName, newCount)

            selectSemModule = semModule
            selectSemC = semC
        }
	}
}

func handleSignals(shutdownRequestChan chan<- os.Signal) {
	osSignalChan := make(chan os.Signal, 1)
	signal.Notify(osSignalChan, syscall.SIGINT, syscall.SIGTERM)
	// log.Printf("[Обработчик Сигналов] Ожидание SIGINT/SIGTERM...") // DEBUG

	defer func() {
		signal.Stop(osSignalChan)
		close(shutdownRequestChan)
		log.Println("[Обработчик сигналов] Завершил работу")
	}()
	
	sig := <-osSignalChan
	log.Printf("[Обработчик Сигналов] Получен сигнал (%s). Отправляю запрос на остановку в Main...", sig)

	shutdownRequestChan <- sig
    log.Println("[Обработчик Сигналов] Запрос на остановку успешно отправлен.")
}

func main() {
	log.Printf("[Main] Запуск производственной линии...")
	log.Printf("[Main] Параметры: A:%ds(%d), B:%ds(%d), C:%ds(%d), МодульСборщики:%d, ВинтикСборщики:1",
		durationA/time.Second, numProducersA,
		durationB/time.Second, numProducersB,
		durationC/time.Second, numProducersC,
		numModuleAssemblers)

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	var widgetCounter int64
	doneChan := make(chan struct{})

	shutdownRequestChan := make(chan os.Signal, 1)
	go handleSignals(shutdownRequestChan)

	// Запуск сборщиков винтиков
	go widgetAssembler(1, &widgetCounter, ctx, doneChan)

	select {
		// Ждем сигнала от обработчика сигналов
		case sig := <-shutdownRequestChan:
			log.Printf("Main: Получен запрос на остановку от обработчика сигналов (%s). Вызываю cancel().", sig)
			cancel()
	
		case <-doneChan:
			log.Println("Main: Получен сигнал завершения от widgetAssembler (doneChan) до сигнала ОС.")
			// cancel() // Можно и тут?
		}

	// Ожидание фактического завершения всех горутин
	log.Println("[Main] Ожидание завершения контроллера...")
	<-doneChan

	log.Printf( "Производственная линия остановлена")
	log.Printf( "Итого произведено винтиков: %d", atomic.LoadInt64(&widgetCounter))
}
