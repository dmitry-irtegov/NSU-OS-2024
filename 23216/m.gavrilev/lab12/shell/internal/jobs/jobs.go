package jobs

// #include <stdlib.h>
// #include <stdio.h>
// #include <unistd.h>
import "C"
import (
	"fmt"
	"shell/internal/utils"
	"syscall"
)

type JobMaster struct {
	Jobs      []utils.Job
	LastJobId int
}

func (jobmaster *JobMaster) Tcsetpgrp(fd int, pgid int32) (err error) {
	a := C.int(fd)
	b := C.int(pgid)
	errC := C.tcsetpgrp(a, b)
	if errC != 0 {
		return syscall.Errno(-errC)
	}
	return nil
}

func (jobmaster *JobMaster) Add(pid int, cmdargs []string, bkgrnd bool) {
	jobmaster.LastJobId++
	jobmaster.Jobs = append(jobmaster.Jobs, utils.Job{Pid: pid, Status: "Running", Cmdargs: cmdargs, Bkgrnd: bkgrnd, JobId: jobmaster.LastJobId})
	fmt.Printf("[%d] %d\n", len(jobmaster.Jobs), pid)
}

func (jobmaster *JobMaster) Write(pid int) {
	for i, job := range jobmaster.Jobs {
		if pid == job.Pid {
			fmt.Printf("[%d] %d %s %s\n", i+1, jobmaster.Jobs[i].Pid, jobmaster.Jobs[i].Status, jobmaster.Jobs[i].Cmdargs)
			if jobmaster.Jobs[i].Status == "Done" {
				jobmaster.Jobs = append(jobmaster.Jobs[:i], jobmaster.Jobs[i+1:]...)
			}
			break
		}
	}
}

func (jobmaster *JobMaster) Update(pid int, status string) {
	for i, job := range jobmaster.Jobs {
		if pid == job.Pid {
			jobmaster.Jobs[i].Status = status
			break
		}
	}
}

func (jobmaster *JobMaster) removeJob(slice []utils.Job, beginId int, endId int) []utils.Job {
	newSlice := make([]utils.Job, len(slice)+beginId-endId)
	if beginId != endId {
		slice = append(slice[:beginId], slice[endId:]...)
	}
	copy(newSlice, slice)
	return newSlice
}

func (jobmaster *JobMaster) WaitJobInfo(pid int) {
	var ws syscall.WaitStatus
	_, err := syscall.Wait4(pid, &ws, syscall.WNOHANG, nil)
	if err != nil {
		fmt.Println("error waiting for job", err)
		return
	}
	jobmaster.Update(pid, "Running")
}

// func (jobmaster *JobMaster) WaitForegroundJob(pid int) {
// 	var ws syscall.WaitStatus
//  _, err = syscall.Wait4(pid, &ws, 0, syscall.WUNTRACED)
// }
