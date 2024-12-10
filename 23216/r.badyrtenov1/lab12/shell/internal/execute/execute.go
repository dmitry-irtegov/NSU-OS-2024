package exec

import (
	"fmt"
	"os"
	"os/exec"
	"shell/internal/jobs"
	"shell/internal/tools"
	"strconv"
	"syscall"
)

type Command struct {
	Cmdargs                  []string
	Infile, Outfile, Appfile string
	Bkgrnd                   bool
	Cmdflag                  byte
}

func (cmd *Command) Init() {
	cmd.Cmdargs = nil
	cmd.Infile = ""
	cmd.Outfile = ""
	cmd.Appfile = ""
	cmd.Bkgrnd = false
}

func (cmd *Command) ForkAndExec(jm *jobs.JobManager, fgPid *int, readPipe *os.File, tmpPipe *os.File, writePipe *os.File) {
	if len(cmd.Cmdargs) == 0 {
		return
	}

	if cmd.Cmdflag == 2 {
		var err error
		tmpPipe, writePipe, err = os.Pipe()
		if err != nil {
			fmt.Println("Error creating pipe")
		}
	}

	if cmd.Cmdargs[0] == "jobs" {
		if len(cmd.Cmdargs) == 1 {
			for i := 1; i <= jm.IdLastJob; i++ {
				for elem := jm.Jobs.Front(); elem != nil; elem = elem.Next() {
					if elem.Value.(tools.Job).Id == i {
						jm.Write(elem.Value.(tools.Job).Pid)
						break
					}
				}
			}
		} else {
			for i := 1; i < len(cmd.Cmdargs); i++ {
				var flag bool
				for elem := jm.Jobs.Front(); elem != nil; elem = elem.Next() {
					if strconv.Itoa(elem.Value.(tools.Job).Id) == cmd.Cmdargs[i] {
						jm.Write(elem.Value.(tools.Job).Pid)
						flag = true
						break
					}
				}
				if !flag {
					fmt.Println("jobs: No such job:", cmd.Cmdargs[i])
				}
			}
		}
		return
	}
	if cmd.Cmdargs[0] == "cd" {
		if len(cmd.Cmdargs) == 2 {
			err := os.Chdir(cmd.Cmdargs[1])
			if err != nil {
				fmt.Println("cd: No such file or directory:", cmd.Cmdargs[1])
			}
		} else if len(cmd.Cmdargs) != 1 {
			fmt.Println("cd: Too many arguments")
		}
		return
	}
	if cmd.Cmdargs[0] == "fg" {
		var pid int
		if len(cmd.Cmdargs) == 1 {
			if jm.Jobs.Back() == nil {
				fmt.Println("fg: No such job: current")
				return
			} else if jm.Jobs.Back().Value.(tools.Job).Status == "Done" {
				fmt.Println("fg: Job has terminated")
				return
			} else {
				pid = jm.Jobs.Back().Value.(tools.Job).Pid
				jm.Fg(pid)
			}
		} else {
			var flag bool
			for elem := jm.Jobs.Front(); elem != nil; elem = elem.Next() {
				if strconv.Itoa(elem.Value.(tools.Job).Id) == cmd.Cmdargs[1] {
					if elem.Value.(tools.Job).Status == "Done" {
						fmt.Println("fg: Job has terminated")
						return
					}
					pid = elem.Value.(tools.Job).Pid
					jm.Fg(pid)
					flag = true
					break
				}
			}
			if !flag {
				fmt.Println("fg: No such job:", cmd.Cmdargs[1])
				return
			}
		}

		jm.WaitForForeground(pid, fgPid)
		return
	}
	if cmd.Cmdargs[0] == "bg" {
		var pid int
		if len(cmd.Cmdargs) == 1 {
			if jm.Jobs.Back() == nil {
				fmt.Println("bg: No such job: current")
				return
			} else if jm.Jobs.Back().Value.(tools.Job).Status == "Done" {
				fmt.Println("bg: Job has terminated")
				return
			} else if jm.Jobs.Back().Value.(tools.Job).Bkgrnd {
				fmt.Println("bg: Job", jm.Jobs.Back().Value.(tools.Job).Id, "already in background")
				return
			} else {
				pid = jm.Jobs.Back().Value.(tools.Job).Pid
				jm.Bg(pid)
			}
		} else {
			for i := 1; i < len(cmd.Cmdargs); i++ {
				var flag bool
				for elem := jm.Jobs.Front(); elem != nil; elem = elem.Next() {
					if strconv.Itoa(elem.Value.(tools.Job).Id) == cmd.Cmdargs[i] {
						if elem.Value.(tools.Job).Status == "Done" {
							fmt.Println("bg: Job has terminated")
							return
						}
						if elem.Value.(tools.Job).Bkgrnd {
							fmt.Println("bg: Job", cmd.Cmdargs[i], "already in background")
							flag = true
							return
						}
						pid = elem.Value.(tools.Job).Pid
						jm.Bg(pid)
						flag = true
						break
					}
				}
				if !flag {
					fmt.Println("bg: No such job:", cmd.Cmdargs[i])
					return
				}
			}
		}

		jm.WaitForBackground(pid)
		return
	}

	binary, err := exec.LookPath(cmd.Cmdargs[0])
	if err != nil {
		fmt.Println("Command not found:", cmd.Cmdargs[0])
		return
	}

	stdin := os.Stdin
	if cmd.Cmdflag == 1 {
		stdin = readPipe
	}
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
	if cmd.Cmdflag == 2 {
		stdout = writePipe
	}
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

	if readPipe != nil {
		err = readPipe.Close()
		if err != nil {
			fmt.Println("Error closing readPipe")
		}
	}
	if writePipe != nil {
		err = writePipe.Close()
		if err != nil {
			fmt.Println("Error closing writePipe")
		}
	}
	writePipe = nil
	readPipe = tmpPipe
	tmpPipe = nil

	jm.Add(pid, cmd.Cmdargs, cmd.Bkgrnd)
	if cmd.Bkgrnd {
		jm.WaitForBackground(pid)
		return
	}
	jm.WaitForForeground(pid, fgPid)
}
