package main

import (
	"fmt"
	"time"
)

func main() {
	aChan := make(chan struct{})
	bChan := make(chan struct{})
	cChan := make(chan struct{})
	moduleChan := make(chan struct{})

	go produceA(aChan)
	go produceB(bChan)
	go produceC(cChan)
	go assembleModule(aChan, bChan, moduleChan)
	go assembleWidget(moduleChan, cChan)

	select {}
}

func produceA(ch chan<- struct{}) {
	for {
		time.Sleep(1 * time.Second)
		fmt.Println("Produced A")
		ch <- struct{}{}
	}
}

func produceB(ch chan<- struct{}) {
	for {
		time.Sleep(2 * time.Second)
		fmt.Println("Produced B")
		ch <- struct{}{}
	}
}

func produceC(ch chan<- struct{}) {
	for {
		time.Sleep(3 * time.Second)
		fmt.Println("Produced C")
		ch <- struct{}{}
	}
}

func assembleModule(aChan <-chan struct{}, bChan <-chan struct{}, moduleChan chan<- struct{}) {
	for {
		<-aChan 
		<-bChan 
		fmt.Println("Module assembled")
		moduleChan <- struct{}{}
	}
}

func assembleWidget(moduleChan <-chan struct{}, cChan <-chan struct{}) {
	for {
		<-moduleChan
		<-cChan
		fmt.Println("Widget assembled")
	}
}
