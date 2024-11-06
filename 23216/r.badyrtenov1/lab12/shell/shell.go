package main

import (
	"fmt"
	"os"
	"os/exec"
	parc "shell/parceline"
	prom "shell/promptline"
	"syscall"
)

func main() {
	prom.SetPrompt([]byte(fmt.Sprintf("[%s] ", os.Args[0])))

	/* PLACE SIGNAL CODE HERE */

	for {
		var line []byte
		line = prom.Promptline(line)
		if len(line) <= 0 {
			break
		}
		var stat parc.Status
		var cmds []parc.Command
		cmds, stat = parc.Parceline(line, cmds, stat)
		if len(cmds) <= 0 {
			continue
		}
		for i := 0; i < len(cmds); i++ {
			if len(parc.GetArgs(cmds[i])) == 0 {
				continue
			}

			binary, err := exec.LookPath(parc.GetArgs(cmds[i])[0])
			if err != nil {
				fmt.Printf("Command not found: %s\n", parc.GetArgs(cmds[i])[0])
				continue
			}

			pid, err := syscall.ForkExec(binary, parc.GetArgs(cmds[i]), &syscall.ProcAttr{
				Dir:   "",
				Files: []uintptr{os.Stdin.Fd(), os.Stdout.Fd(), os.Stderr.Fd()},
				Sys:   &syscall.SysProcAttr{},
			})

			if err != nil {
				fmt.Println("Error during ForkExec:", err)
				continue
			}

			if parc.GetBkgrndFlag(stat) {
				fmt.Println(pid)
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
