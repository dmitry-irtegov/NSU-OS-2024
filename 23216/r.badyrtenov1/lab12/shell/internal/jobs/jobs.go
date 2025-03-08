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
	"unsafe"
)

type JobManager struct {
	Jobs      *list.List
	IdLastJob int
	jobsMutex sync.Mutex
}

func (jm *JobManager) Init() {
	jm.Jobs = list.New()
}

func (jm *JobManager) SignalHandler(fgPid *int) {
	go func() {
		signChan := make(chan os.Signal, 1)
		signal.Notify(signChan, syscall.SIGINT, syscall.SIGQUIT, syscall.SIGTSTP)
		for sig := range signChan {
			switch sig {
			case syscall.SIGINT:
				fmt.Println()
				if *fgPid != 0 {
					jm.Update(*fgPid, "Done")
					err := syscall.Kill(jm.PgId(*fgPid), syscall.SIGINT)
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
				if *fgPid != 0 {
					jm.Update(*fgPid, "Stopped")
					jm.Write(*fgPid)
					err := syscall.Kill(jm.PgId(*fgPid), syscall.SIGSTOP)
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

func (jm *JobManager) FgWait(pid int) {
	var ws syscall.WaitStatus
	for {
		_, err := syscall.Wait4(jm.PgId(pid), &ws, syscall.WUNTRACED, nil)
		if ws.Stopped() {
			fmt.Println()
			jm.Update(pid, "Stopped")
			jm.Write(pid)
			err = syscall.Kill(jm.PgId(pid), syscall.SIGSTOP)
			if err != nil {
				fmt.Println("Error stopping process")
			}
			return
		}
		if ws.Signaled() {
			jm.Update(pid, "Done")
			err = syscall.Kill(jm.PgId(pid), syscall.SIGINT)
			if err != nil {
				fmt.Println()
				return
			}
		}
		if err != nil {
			break
		}
	}
	jm.Update(pid, "Done")
}

func (jm *JobManager) WaitForForeground(pid int, fgPid *int) {
	*fgPid = pid
	var ws syscall.WaitStatus
	for {
		_, err := syscall.Wait4(jm.PgId(pid), &ws, syscall.WUNTRACED, nil)
		if ws.Stopped() {
			for *fgPid != 0 {
			}
			return
		}
		if ws.Signaled() {
			for *fgPid != 0 {
			}
			if err != nil {
				return
			}
		}
		if err != nil {
			break
		}
	}
	jm.Update(pid, "Done")
	*fgPid = 0
}

func (jm *JobManager) WaitForBackground(pid int) {
	go func() {
		var ws syscall.WaitStatus
		for {
			_, err := syscall.Wait4(jm.PgId(pid), &ws, syscall.WUNTRACED, nil)
			if ws.Stopped() {
				return
			}
			if ws.Signaled() {
				if jm.PgId(pid) >= 0 {
					fmt.Println()
				}
				return
			}
			if err != nil {
				break
			}
		}
		jm.Update(pid, "Done")
	}()
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

func (jm *JobManager) Add(pid int, cmdargs []string, flag bool, pipeFlag bool) {
	jm.jobsMutex.Lock()
	defer jm.jobsMutex.Unlock()
	jm.IdLastJob++
	if jm.Jobs.Front() == nil {
		jm.Jobs.PushBack(tools.Job{Pid: pid, Status: "Running", Cmdargs: cmdargs, Bkgrnd: flag, Id: jm.IdLastJob, PipeFlag: pipeFlag})
		if flag {
			fmt.Printf("[%d] %d\n", jm.IdLastJob, pid)
		}
	} else {
		for e := jm.Jobs.Front(); e != nil; e = e.Next() {
			job := e.Value.(tools.Job)
			if flag {
				if job.Status == "Stopped" {
					jm.Jobs.InsertBefore(tools.Job{Pid: pid, Status: "Running", Cmdargs: cmdargs, Bkgrnd: flag, Id: jm.IdLastJob, PipeFlag: pipeFlag}, e)
					fmt.Printf("[%d] %d\n", jm.IdLastJob, pid)
					break
				} else if e.Next() == nil {
					jm.Jobs.PushBack(tools.Job{Pid: pid, Status: "Running", Cmdargs: cmdargs, Bkgrnd: flag, Id: jm.IdLastJob, PipeFlag: pipeFlag})
					fmt.Printf("[%d] %d\n", jm.IdLastJob, pid)
					break
				}
			} else {
				if job.Bkgrnd || job.Status == "Stopped" {
					jm.Jobs.InsertBefore(tools.Job{Pid: pid, Status: "Running", Cmdargs: cmdargs, Bkgrnd: flag, Id: jm.IdLastJob, PipeFlag: pipeFlag}, e)
					break
				} else if e.Next() == nil {
					jm.Jobs.PushBack(tools.Job{Pid: pid, Status: "Running", Cmdargs: cmdargs, Bkgrnd: flag, Id: jm.IdLastJob, PipeFlag: pipeFlag})
					break
				}
			}
		}
	}
	if (cmdargs[0] == "cat" || cmdargs[0] == "vim") && flag {
		jm.jobsMutex.Unlock()
		jm.Update(pid, "Running")
		pgId := jm.PgId(pid)
		jm.jobsMutex.Lock()
		err := syscall.Kill(pgId, syscall.SIGSTOP)
		if err != nil {
			fmt.Println("Error stopping process")
		}
	}
}

func (jm *JobManager) PgId(pid int) int {
	jm.jobsMutex.Lock()
	defer jm.jobsMutex.Unlock()
	for elem := jm.Jobs.Front(); elem != nil; elem = elem.Next() {
		job := elem.Value.(tools.Job)
		if pid == job.Pid {
			if job.PipeFlag {
				return -pid
			}
			break
		}
	}
	return pid
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
			pgId := jm.PgId(pid)
			jm.jobsMutex.Lock()
			if job.Cmdargs[0] == "cat" || job.Cmdargs[0] == "vim" {
				err := syscall.Kill(pgId, syscall.SIGSTOP)
				if err != nil {
					fmt.Println("Error stopping process")
				}
				return
			}
			err := syscall.Kill(pgId, syscall.SIGCONT)
			if err != nil {
				fmt.Println("Error continuing job")
				return
			}
			jm.jobsMutex.Unlock()
			jm.WaitForBackground(pid)
			jm.jobsMutex.Lock()
			break
		}
	}
}

func (jm *JobManager) Fg(pid int, fgPid *int) {
	signal.Ignore(syscall.SIGTTOU)
	defer signal.Reset(syscall.SIGTTOU)
	jm.jobsMutex.Lock()
	defer jm.jobsMutex.Unlock()
	for elem := jm.Jobs.Front(); elem != nil; elem = elem.Next() {
		job := elem.Value.(tools.Job)
		if job.Pid == pid {
			bkgrndFlag := job.Bkgrnd
			job.Bkgrnd = false
			elem.Value = job
			jm.jobsMutex.Unlock()
			jm.Update(pid, "Running")
			pgId := jm.PgId(pid)
			jm.jobsMutex.Lock()
			fmt.Println(strings.Join(job.Cmdargs, " "))
			err := syscall.Kill(pgId, syscall.SIGCONT)
			if err != nil {
				fmt.Println("Error continuing job")
				return
			}
			jm.jobsMutex.Unlock()
			if bkgrndFlag {
				_, _, err = syscall.Syscall(syscall.SYS_IOCTL, os.Stdin.Fd(), uintptr(syscall.TIOCSPGRP), uintptr(unsafe.Pointer(&pid)))
				if err != nil {
				}
				jm.FgWait(pid)
			} else {
				jm.WaitForForeground(pid, fgPid)
			}
			jm.jobsMutex.Lock()
			sysPid := os.Getpid()
			_, _, err = syscall.Syscall(syscall.SYS_IOCTL, os.Stdin.Fd(), uintptr(syscall.TIOCSPGRP), uintptr(unsafe.Pointer(&sysPid)))
			if err != nil {
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
			if (job.Cmdargs[0] == "cat" || job.Cmdargs[0] == "vim") && job.Bkgrnd && status == "Running" {
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
