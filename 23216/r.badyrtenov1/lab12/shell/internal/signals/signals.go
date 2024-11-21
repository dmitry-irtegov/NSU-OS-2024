package signals

import (
	"fmt"
	"os"
	"os/signal"
	"shell/internal/jobs"
	"shell/internal/tools"
	"syscall"
)

type Channels struct {
	SignChan  chan os.Signal
	FgPidChan chan int
}

func (ch *Channels) Init() {
	ch.SignChan = make(chan os.Signal, 1)
	ch.FgPidChan = make(chan int, 1)
}

func (ch *Channels) SignalHandler(jm *jobs.JobManager) {
	signal.Notify(ch.SignChan, syscall.SIGINT, syscall.SIGQUIT, syscall.SIGTSTP)
	go func() {
		for sig := range ch.SignChan {
			if sig == syscall.SIGINT {
				fmt.Println()
				select {
				case foregroundPid := <-ch.FgPidChan:
					err := syscall.Kill(foregroundPid, syscall.SIGINT)
					if err != nil {
					}
				default:
					err := tools.Promptline()
					if err != nil {
						fmt.Println("Error in Prompt")
					}
				}
			} else if sig == syscall.SIGTSTP {
				fmt.Println()
				select {
				case pid := <-ch.FgPidChan:
					err := syscall.Kill(pid, syscall.SIGSTOP)
					if err != nil {
						fmt.Println("Error stopping process")
					}
					jm.Update(pid, "Stopped")
					jm.Write(pid)
				default:
					err := tools.Promptline()
					if err != nil {
						fmt.Println("Error in Prompt")
					}
				}
			}
		}
	}()
}
