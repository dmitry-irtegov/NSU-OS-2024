package execute

import (
	"fmt"
	"os"
	"os/exec"
	"shell/constants"
	"shell/internal/jobs"
	pars "shell/internal/parseline"
	"strconv"
	"syscall"
)

type Exec struct {
}

func (c *Exec) ForkAndExec(cmd *pars.Command, jobmaster *jobs.JobMaster, pipeRead *os.File, pipeWrite *os.File) {

	args := cmd.GetArgs()
	infile := cmd.GetInfile()
	outfile := cmd.GetOutfile()
	appfile := cmd.GetAppfile()
	backgroundflag := cmd.GetBackgroundFlag()
	cmdflag := cmd.GetCmdFlag()

	if len(args) == 0 {
		return
	}

	// if strings.Compare(args[0], ("exit")) == 0 {
	// 	return
	// }
	if args[0] == "jobs" {
		if len(args) == 1 {
			for i := 1; i <= jobmaster.LastJobId; i++ {
				for k := 0; k < len(jobmaster.Jobs); k++ {
					elem := jobmaster.Jobs[k]
					if elem.JobId == i {
						jobmaster.Write(elem.Pid)
						break
					}
				}
			}
		} else {
			for i := 1; i < len(args); i++ {
				var flag bool
				for k := 0; k < len(jobmaster.Jobs); k++ {
					elem := jobmaster.Jobs[k]
					if strconv.Itoa(elem.JobId) == args[i] {
						jobmaster.Write(elem.Pid)
						flag = true
						break
					}
				}
				if !flag {
					fmt.Println("jobs: No such job:", args[i])
				}
			}
		}
		return
	}

	binary, err := exec.LookPath(args[0])
	if err != nil {
		fmt.Println("command not found:", args[0])
		return
	}

	stdin := os.Stdin
	stdout := os.Stdout

	if infile != "" {
		stdin, err = os.Open(infile)
		if err != nil {
			fmt.Println("no such file or directory:", infile)
			return
		}
		defer func() {
			err := stdin.Close()
			if err != nil {
				fmt.Println("error closing input file")
				return
			}
		}()
	} else if cmdflag&constants.INPIP != 0 {
		stdin = pipeRead
	}

	if outfile != "" {
		stdout, err = os.Create(outfile)
		if err != nil {
			fmt.Println("error opening output file")
			return
		}
		defer func() {
			err := stdout.Close()
			if err != nil {
				fmt.Println("error closing output file")
				return
			}
		}()
	} else if appfile != "" {
		stdout, err = os.OpenFile(appfile, os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
		if err != nil {
			fmt.Println("error opening append file")
			return
		}
		defer func() {
			err := stdout.Close()
			if err != nil {
				fmt.Println("error closing append file")
				return
			}
		}()
	} else if cmdflag&constants.OUTPIP != 0 {
		stdout = pipeWrite
	}

	pid, err := syscall.ForkExec(binary, args, &syscall.ProcAttr{
		Dir:   "",
		Files: []uintptr{stdin.Fd(), stdout.Fd(), os.Stderr.Fd()},
		Sys: &syscall.SysProcAttr{
			Setpgid: backgroundflag,
		},
	})
	if err != nil {
		fmt.Println("error during ForkExec")
		return
	}
	jobmaster.Add(pid, args, backgroundflag)
	if backgroundflag {
		jobmaster.WaitJobInfo(pid)
		return
	} else {
		var ws syscall.WaitStatus
		//fmt.Println(pid, &ws, syscall.WEXITED, syscall.WUNTRACED, syscall.WNOHANG, nil)
		_, err = syscall.Wait4(pid, &ws, 0, nil)
		if err != nil {
			fmt.Println("error waiting for process", err)
			return
		}
	}

}
