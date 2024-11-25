package jobs

import (
	"fmt"
	"os"
	"os/signal"
	"shell/internal/tools"
	"strings"
	"sync"
	"syscall"
)

type JobManager struct {
	Jobs      []tools.Job
	IdLastJob int
	jobsMutex sync.Mutex
}

func (jm *JobManager) Add(pid int, cmdargs []string, flag bool) {
	jm.jobsMutex.Lock()
	defer jm.jobsMutex.Unlock()
	jm.IdLastJob++
	jm.Jobs = append(jm.Jobs, tools.Job{Pid: pid, Status: "Running", Cmdargs: cmdargs, Bkgrnd: flag, Id: jm.IdLastJob})
	if flag {
		fmt.Printf("[%d] %d\n", jm.IdLastJob, pid)
	}
}

func (jm *JobManager) Write(pid int) {
	jm.jobsMutex.Lock()
	defer jm.jobsMutex.Unlock()
	for i, job := range jm.Jobs {
		if pid == job.Pid {
			if job.Bkgrnd {
				fmt.Printf("[%d]    %d    %s    %s &\n", job.Id, job.Pid, job.Status, strings.Join(job.Cmdargs, " "))
			} else if job.Status != "Done" {
				fmt.Printf("[%d]    %d    %s    %s\n", job.Id, job.Pid, job.Status, strings.Join(job.Cmdargs, " "))
			}
			if job.Status == "Done" {
				if job.Id == jm.IdLastJob {
					jm.IdLastJob = 0
					for j := len(jm.Jobs) - 2; j >= 0; j-- {
						if jm.Jobs[j].Status != "Done" {
							jm.IdLastJob = jm.Jobs[j].Id
							break
						}
					}
				}
				jm.Jobs = tools.RemoveJob(jm.Jobs, i, i+1)
			}
			break
		}
	}
}

func (jm *JobManager) Update(pid int, status string) {
	jm.jobsMutex.Lock()
	defer jm.jobsMutex.Unlock()
	for i, job := range jm.Jobs {
		if pid == job.Pid {
			jm.Jobs[i].Status = status
			break
		}
	}
}

func (jm *JobManager) SignalHandler(signChan chan os.Signal, fgPidChan *int) {
	signal.Notify(signChan, syscall.SIGINT, syscall.SIGQUIT, syscall.SIGTSTP)
	go func() {
		for sig := range signChan {
			if sig == syscall.SIGINT {
				fmt.Println()
				if *fgPidChan != 0 {
					err := syscall.Kill(*fgPidChan, syscall.SIGINT)
					if err != nil {
						fmt.Println("Error killing process")
					}
				} else {
					err := tools.Promptline()
					if err != nil {
						fmt.Println("Error in Prompt")
					}
				}
			} else if sig == syscall.SIGTSTP {
				fmt.Println()
				if *fgPidChan != 0 {
					jm.Update(*fgPidChan, "Stopped")
					jm.Write(*fgPidChan)
					err := syscall.Kill(*fgPidChan, syscall.SIGSTOP)
					if err != nil {
						fmt.Println("Error stopping process")
					}
					*fgPidChan = 0
				} else {
					err := tools.Promptline()
					if err != nil {
						fmt.Println("Error in Prompt")
					}
				}
			}
		}
	}()
}
