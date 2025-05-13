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
	detailChan chan<- struct{},
	ctx context.Context,
	wg *sync.WaitGroup,
) {
	ticker := time.NewTicker(productionTime)
	defer func() {
		wg.Done()
		ticker.Stop()
	}()

	// log.Printf("[%s] Запущен", partName) // DEBUG

	for {
		select {
		case <-ticker.C:
			// log.Printf("[%s Producer] Деталь произведена...", partName) // DEBUG
			select {
			case detailChan <- struct{}{}:
				log.Printf("[%s Producer] -> Деталь отправлена.", partName)
			case <-ctx.Done():
				log.Printf("[%s Producer] Остановка...", partName)
				return
			}
		case <-ctx.Done():
			log.Printf("[%s Producer] Остановка...", partName)
			return
		}
	}
}

// moduleAssembler собирает модуль из деталей A и B, сигнализирует через семафор.
func moduleAssembler(
	id int,
	semModule chan<- struct{},
	ctx context.Context,
	parentWG *sync.WaitGroup,
) {
	assemblerName := fmt.Sprintf("Сборщик Модулей %d", id)
	// log.Printf("[%s] Запущен", assemblerName) // Для отладки
	var wg sync.WaitGroup

	defer func() {
		log.Printf("[%s] Остановка...", assemblerName)
		wg.Wait()
		log.Printf("[%s] Остановлен.", assemblerName)
		parentWG.Done()
	}()

	detailAChan := make(chan struct{})
	detailBChan := make(chan struct{})

	for i := 0; i < numProducersA; i++ {
		wg.Add(1)
		partID := fmt.Sprintf("Деталь A-%d-%d", id, i+1)
		go producer(partID, durationA, detailAChan, ctx, &wg)
	}
	for i := 0; i < numProducersB; i++ {
		wg.Add(1)
		partID := fmt.Sprintf("Деталь B-%d-%d", id, i+1)
		go producer(partID, durationB, detailBChan, ctx, &wg)
	}

	haveA, haveB := false, false

	for {

		/*
		var selectDetailAChan chan struct{}
		if !haveA {
			selectDetailAChan = detailAChan // Нужна деталь A
		} else {
			selectDetailAChan = nil // Деталь A уже есть, отключаем case
		}
		*/

		select {
		case <-ctx.Done():
			log.Printf("[%s] Остановка...", assemblerName)
			return
		case _, ok := <-detailAChan:
			if !ok {
				log.Printf("[%s] ОШИБКА: Канал A закрыт!", assemblerName)
				return
			}
			haveA = true
			// log.Printf("[%s] Получена Деталь A", assemblerName) // DEBUG
		case _, ok := <-detailBChan:
			if !ok {
				log.Printf("[%s] ОШИБКА: Канал B закрыт!", assemblerName)
				return
			}
			haveB = true
			// log.Printf("[%s] Получена Деталь B", assemblerName) // DEBUG
		}

		if (haveA && haveB) {
			select {
			case <-ctx.Done():
				log.Printf("[%s] Остановка...", assemblerName)
				return
			case semModule <- struct{}{}:
				log.Printf("[%s] -> Модуль отправлен", assemblerName)
				haveA, haveB = false, false
			}
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

	moduleChan := make(chan struct{})
	detailCChan := make(chan struct{})

	for i := 0; i < numProducersC; i++ {
		wg.Add(1)
		partID := fmt.Sprintf("Деталь C-%d-%d", id, i+1)
		go producer(partID, durationC, detailCChan, ctx, &wg)
	}

	for i := 0; i < numModuleAssemblers; i++ {
		wg.Add(1)
		go moduleAssembler(i+1, moduleChan, ctx, &wg)
	}

	haveModule, haveC := false, false

	for {
		select {
        case <-ctx.Done():
            log.Printf("[%s] Остановка...", assemblerName)
            return
        case _, ok := <-moduleChan:
			if !ok {
				log.Printf("[%s] ОШИБКА: Канал semModule закрыт!", assemblerName)
				return
			}
			haveModule = true
        case _, ok := <-detailCChan:
			if !ok {
				log.Printf("[%s] ОШИБКА: Канал C закрыт!", assemblerName)
				return
			}
			haveC = true
		}

		if haveModule && haveC {
			newCount := atomic.AddInt64(widgetCounter, 1)
			log.Printf("[%s] ===> Собран Винтик #%d", assemblerName, newCount)
			haveModule, haveC = false, false
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

	sig := <-shutdownRequestChan
	log.Printf("[Main] Получен запрос на остановку от обработчика сигналов (%s). Вызываю cancel().", sig)
	cancel()

	// Ожидание фактического завершения всех горутин
	log.Println("[Main] Ожидание завершения контроллера...")
	<-doneChan

	log.Printf( "[Main] Производственная линия остановлена")
	log.Printf( "[Main] Итого произведено винтиков: %d", atomic.LoadInt64(&widgetCounter))
}
