package exec

import (
	"fmt"
	"os"
	"os/exec"
	"shell/internal/jobs"
	"shell/internal/signals"
	"syscall"
)

type Command struct {
	Cmdargs                  []string
	Infile, Outfile, Appfile string
	Bkgrnd                   bool
}

func (cmd *Command) Init() {
	cmd.Cmdargs = nil
	cmd.Infile = ""
	cmd.Outfile = ""
	cmd.Appfile = ""
	cmd.Bkgrnd = false
}

func (cmd *Command) ForkAndExec(jm *jobs.JobManager, ch *signals.Channels) {
	if len(cmd.Cmdargs) == 0 {
		return
	}

	if cmd.Cmdargs[0] == "jobs" {
		if len(cmd.Cmdargs) == 1 {
			for i := 0; i < len(jm.Jobs); i++ {
				if jm.Jobs[i].Status == "Done" {
					jm.Write(jm.Jobs[i].Pid)
					i--
				} else {
					jm.Write(jm.Jobs[i].Pid)
				}
			}
		} else {
			fmt.Println("jobs: Too many arguments")
		}
		return
	}

	binary, err := exec.LookPath(cmd.Cmdargs[0])
	if err != nil {
		fmt.Println("Command not found:", cmd.Cmdargs[0])
		return
	}

	stdin := os.Stdin
	if cmd.Infile != "" {
		stdin, err = os.Open(cmd.Infile)
		if err != nil {
			fmt.Println("No such file or directory:", cmd.Infile)
			return
		}
		defer func(stdin *os.File) {
			err := stdin.Close()
			if err != nil {
				fmt.Println("Error closing input file")
				return
			}
		}(stdin)
	}

	stdout := os.Stdout
	if cmd.Appfile != "" {
		stdout, err = os.OpenFile(cmd.Appfile, os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
		if err != nil {
			fmt.Println("Error opening append file")
			return
		}
		defer func(stdout *os.File) {
			err := stdout.Close()
			if err != nil {
				fmt.Println("Error closing append file")
				return
			}
		}(stdout)
	}
	if cmd.Outfile != "" {
		stdout, err = os.Create(cmd.Outfile)
		if err != nil {
			fmt.Println("Error opening output file")
			return
		}
		defer func(stdout *os.File) {
			err := stdout.Close()
			if err != nil {
				fmt.Println("Error closing output file")
				return
			}
		}(stdout)
	}

	pid, err := syscall.ForkExec(binary, cmd.Cmdargs, &syscall.ProcAttr{
		Dir:   "",
		Files: []uintptr{stdin.Fd(), stdout.Fd(), os.Stderr.Fd()},
		Sys: &syscall.SysProcAttr{
			Setpgid: cmd.Bkgrnd,
		},
	})
	if err != nil {
		fmt.Println("Error during ForkExec")
		return
	}

	jm.Add(pid, cmd.Cmdargs, cmd.Bkgrnd)
	var ws syscall.WaitStatus
	if cmd.Bkgrnd {
		go func() {
			_, err := syscall.Wait4(pid, &ws, 0, nil)
			if err != nil {
				fmt.Println("Error waiting for process")
			} else {
				jm.Update(pid, "Done")
			}
		}()
		return
	}

	ch.FgPidChan <- pid
	_, err = syscall.Wait4(pid, &ws, syscall.WUNTRACED, nil)
	if err != nil {
		fmt.Println("Error waiting for process")
		return
	}
	if !ws.Stopped() {
		jm.Update(pid, "Done")
	}
	if len(ch.FgPidChan) != 0 {
		_ = <-ch.FgPidChan
	}

	for i := 0; i < len(jm.Jobs); i++ {
		if jm.Jobs[i].Status == "Done" {
			jm.Write(jm.Jobs[i].Pid)
			i--
		}
	}
}
