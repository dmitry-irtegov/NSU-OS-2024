package jobs

import (
	"container/list"
	"fmt"
	"os"
	"os/signal"
	"shell/internal/tools"
	"strings"
	"sync"
	"syscall"
)

type JobManager struct {
	Jobs      *list.List
	IdLastJob int
	jobsMutex sync.Mutex
}

func (jm *JobManager) Init() {
	jm.Jobs = list.New()
}

func (jm *JobManager) WaitForBackground(pid int) {
	go func() {
		var ws syscall.WaitStatus
		_, err := syscall.Wait4(pid, &ws, 0, nil)
		if err != nil {
			return
		}
		jm.Update(pid, "Done")
	}()
}

func (jm *JobManager) WaitForForeground(pid int, fgPid *int) {
	var ws syscall.WaitStatus
	*fgPid = pid
	_, err := syscall.Wait4(pid, &ws, syscall.WUNTRACED, nil)
	if err != nil {
		return
	}
	if ws.Stopped() || ws.Signaled() {
		for *fgPid != 0 {
		}
		return
	}
	jm.Update(pid, "Done")
	*fgPid = 0
}

func (jm *JobManager) WriteDoneJobs() {
	for i := 1; i <= jm.IdLastJob; i++ {
		for e := jm.Jobs.Front(); e != nil; e = e.Next() {
			if e.Value.(tools.Job).Id == i {
				if e.Value.(tools.Job).Status == "Done" {
					jm.Write(e.Value.(tools.Job).Pid)
				}
				break
			}
		}
	}
}

func (jm *JobManager) Add(pid int, cmdargs []string, flag bool) {
	jm.jobsMutex.Lock()
	defer jm.jobsMutex.Unlock()
	jm.IdLastJob++
	if jm.Jobs.Front() == nil {
		jm.Jobs.PushBack(tools.Job{Pid: pid, Status: "Running", Cmdargs: cmdargs, Bkgrnd: flag, Id: jm.IdLastJob})
		if flag {
			fmt.Printf("[%d] %d\n", jm.IdLastJob, pid)
		}
	} else {
		for e := jm.Jobs.Front(); e != nil; e = e.Next() {
			job := e.Value.(tools.Job)
			if flag {
				if job.Status == "Stopped" {
					jm.Jobs.InsertBefore(tools.Job{Pid: pid, Status: "Running", Cmdargs: cmdargs, Bkgrnd: flag, Id: jm.IdLastJob}, e)
					fmt.Printf("[%d] %d\n", jm.IdLastJob, pid)
					break
				} else if e.Next() == nil {
					jm.Jobs.PushBack(tools.Job{Pid: pid, Status: "Running", Cmdargs: cmdargs, Bkgrnd: flag, Id: jm.IdLastJob})
					fmt.Printf("[%d] %d\n", jm.IdLastJob, pid)
					break
				}
			} else {
				if job.Bkgrnd || job.Status == "Stopped" {
					jm.Jobs.InsertBefore(tools.Job{Pid: pid, Status: "Running", Cmdargs: cmdargs, Bkgrnd: flag, Id: jm.IdLastJob}, e)
					break
				} else if e.Next() == nil {
					jm.Jobs.PushBack(tools.Job{Pid: pid, Status: "Running", Cmdargs: cmdargs, Bkgrnd: flag, Id: jm.IdLastJob})
					break
				}
			}
		}
	}
	if cmdargs[0] == "cat" && flag {
		jm.jobsMutex.Unlock()
		jm.Update(pid, "Running")
		jm.jobsMutex.Lock()
	}
}

func (jm *JobManager) Write(pid int) {
	jm.jobsMutex.Lock()
	defer jm.jobsMutex.Unlock()
	for elem := jm.Jobs.Front(); elem != nil; elem = elem.Next() {
		job := elem.Value.(tools.Job)
		if pid == job.Pid {
			stat := " "
			if elem.Next() == nil {
				stat = "+"
			} else if elem.Next().Next() == nil {
				stat = "-"
			}
			if job.Bkgrnd {
				fmt.Printf("[%d]%s    %s    %s &\n", job.Id, stat, job.Status, strings.Join(job.Cmdargs, " "))
			} else if job.Status != "Done" {
				fmt.Printf("[%d]%s    %s    %s\n", job.Id, stat, job.Status, strings.Join(job.Cmdargs, " "))
			}
			if job.Status == "Done" {
				if job.Id == jm.IdLastJob {
					maxId := 0
					for e := jm.Jobs.Front(); e != nil; e = e.Next() {
						if e.Value.(tools.Job).Status != "Done" && maxId < e.Value.(tools.Job).Id {
							maxId = e.Value.(tools.Job).Id
						}
					}
					jm.IdLastJob = maxId
				}
				jm.Jobs.Remove(elem)
			}
			break
		}
	}
}

func (jm *JobManager) Bg(pid int) {
	jm.jobsMutex.Lock()
	defer jm.jobsMutex.Unlock()
	for elem := jm.Jobs.Front(); elem != nil; elem = elem.Next() {
		job := elem.Value.(tools.Job)
		if job.Pid == pid {
			job.Bkgrnd = true
			elem.Value = job
			stat := " "
			if elem.Next() == nil {
				stat = "+"
			} else if elem.Next().Next() == nil {
				stat = "-"
			}
			fmt.Printf("[%d]%s    %s &\n", job.Id, stat, strings.Join(job.Cmdargs, " "))
			jm.jobsMutex.Unlock()
			jm.Update(pid, "Running")
			jm.jobsMutex.Lock()
			if job.Cmdargs[0] == "cat" {
				err := syscall.Kill(pid, syscall.SIGSTOP)
				if err != nil {
					fmt.Println("Error stopping process")
				}
				return
			}
			err := syscall.Kill(pid, syscall.SIGCONT)
			if err != nil {
				fmt.Println("Error continuing job", pid)
				return
			}
			break
		}
	}
}

func (jm *JobManager) Fg(pid int) {
	jm.jobsMutex.Lock()
	defer jm.jobsMutex.Unlock()
	for elem := jm.Jobs.Front(); elem != nil; elem = elem.Next() {
		job := elem.Value.(tools.Job)
		if job.Pid == pid {
			job.Bkgrnd = false
			elem.Value = job
			jm.jobsMutex.Unlock()
			jm.Update(pid, "Running")
			jm.jobsMutex.Lock()
			fmt.Println(strings.Join(job.Cmdargs, " "))
			err := syscall.Kill(pid, syscall.SIGCONT)
			if err != nil {
				fmt.Println("Error continuing job", pid)
				return
			}
			break
		}
	}
}

func (jm *JobManager) Update(pid int, status string) {
	jm.jobsMutex.Lock()
	defer jm.jobsMutex.Unlock()
	for elem := jm.Jobs.Front(); elem != nil; elem = elem.Next() {
		job := elem.Value.(tools.Job)
		if pid == job.Pid {
			if job.Bkgrnd && status == "Running" && job.Cmdargs[0] == "cat" {
				job.Status = "Stopped"
			} else {
				job.Status = status
			}
			elem.Value = job
			switch job.Status {
			case "Running":
				for e := jm.Jobs.Front(); e != nil; e = e.Next() {
					if e.Value.(tools.Job).Bkgrnd || e.Value.(tools.Job).Status == "Stopped" {
						jm.Jobs.MoveBefore(elem, e)
						break
					} else if e.Next() == nil {
						jm.Jobs.MoveToBack(elem)
						break
					}
				}
			case "Stopped":
				jm.Jobs.MoveToBack(elem)
			}
			break
		}
	}
}

func (jm *JobManager) SignalHandler(signChan chan os.Signal, fgPid *int) {
	signal.Notify(signChan, syscall.SIGINT, syscall.SIGQUIT, syscall.SIGTSTP)
	go func() {
		for sig := range signChan {
			switch sig {
			case syscall.SIGINT:
				fmt.Println()
				if *fgPid > 0 {
					jm.Update(*fgPid, "Done")
					jm.Write(*fgPid)
					err := syscall.Kill(*fgPid, syscall.SIGINT)
					if err != nil {
					}
					*fgPid = 0
				} else {
					err := tools.Promptline()
					if err != nil {
						fmt.Println("Error in Prompt")
					}
				}
			case syscall.SIGTSTP:
				fmt.Println()
				if *fgPid > 0 {
					jm.Update(*fgPid, "Stopped")
					jm.Write(*fgPid)
					err := syscall.Kill(*fgPid, syscall.SIGSTOP)
					if err != nil {
						fmt.Println("Error stopping process")
					}
					*fgPid = 0
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
