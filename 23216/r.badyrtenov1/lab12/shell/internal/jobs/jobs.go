package jobs

import (
	"fmt"
	"shell/internal/tools"
	"sync"
)

type JobManager struct {
	Jobs      []tools.Job
	jobsMutex sync.Mutex
}

func (jm *JobManager) Add(pid int, cmd string) {
	jm.jobsMutex.Lock()
	defer jm.jobsMutex.Unlock()
	jm.Jobs = append(jm.Jobs, tools.Job{Pid: pid, Status: "Running", Cmd: cmd})
	fmt.Printf("[%d] %d\n", len(jm.Jobs), pid)
}

func (jm *JobManager) Write(pid int) {
	jm.jobsMutex.Lock()
	defer jm.jobsMutex.Unlock()
	for i, job := range jm.Jobs {
		if pid == job.Pid {
			fmt.Printf("[%d] %d %s %s\n", i+1, jm.Jobs[i].Pid, jm.Jobs[i].Status, jm.Jobs[i].Cmd)
			if jm.Jobs[i].Status == "Done" {
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
