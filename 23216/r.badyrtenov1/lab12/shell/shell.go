package main

import (
	"fmt"
	"os"
	"os/exec"
	head "shell/header"
	parc "shell/parceline"
	prom "shell/promptline"
	"strings"
	"syscall"
)

func nullstrskip(line []string) int {
	var id int = 0
	for strings.Compare(line[id], "") != 0 {
		id++
	}
	return id
}

func main() {
	prompt := []byte(fmt.Sprintf("[%s] ", os.Args[0]))
	line := make([]byte, 1024)

	/* PLACE SIGNAL CODE HERE */

	for prom.Promptline(prompt, line) > 0 {
		ncmds := parc.Parceline(line)
		if ncmds <= 0 {
			continue
		}
		for i := 0; i < ncmds; i++ {
			if len(head.Cmds[i].Cmdargs) == 0 {
				continue
			}
			binary, err := exec.LookPath(head.Cmds[i].Cmdargs[0])
			if err != nil {
				fmt.Printf("Command not found: %s\n", head.Cmds[i].Cmdargs[0])
				continue
			}

			env := os.Environ()
			pid, err := syscall.ForkExec(binary, head.Cmds[i].Cmdargs[:nullstrskip(head.Cmds[i].Cmdargs)], &syscall.ProcAttr{
				Dir:   "",
				Env:   env,
				Files: []uintptr{0, 1, 2},
				Sys:   &syscall.SysProcAttr{},
			})

			if err != nil {
				fmt.Println("Error during ForkExec:", err)
				continue
			}

			var ws syscall.WaitStatus
			_, err = syscall.Wait4(pid, &ws, 0, nil)
			if err != nil {
				fmt.Println("Error waiting for process to complete:", err)
			}
		}
	}
}
