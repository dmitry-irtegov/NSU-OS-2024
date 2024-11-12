package main

import (
	"fmt"
	parc "lab12/parceline"
	prom "lab12/promptline"
	"os"
	"os/exec"
	"strings"
	"syscall"
	"unsafe"
)

func main() { os.Exit(mainWithReturn()) }

func mainWithReturn() int {
	prompt := ([]byte)(fmt.Sprintf("[%s] ", os.Args[0]))

	/* PLACE SIGNAL CODE HERE */

	for {
		var line []byte
		line = prom.Promptline(prompt, line)
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
			if len(cmds[i].GetArgs()) == 0 {
				continue
			}
			if strings.Compare(cmds[i].GetArgs()[0], ("exit")) == 0 {
				return 0
			}

			binary, err := exec.LookPath(cmds[i].GetArgs()[0])
			if err != nil {
				fmt.Printf("command not found: %s\n", cmds[i].GetArgs()[0])
				continue
			}
			ppid, _, errno := syscall.Syscall(syscall.SYS_FORK, 0, 0, 0)
			pid := *(*int)(unsafe.Pointer(&ppid))
			if errno != 0 {
				err = errno
				fmt.Println("error during fork", err)
				continue
			}
			if pid == 0 {
				err := syscall.Exec(binary, cmds[i].GetArgs(), os.Environ())
				fmt.Println("error during exec, it doesnt return here ", err)
				return 1
			} else {
				var ws syscall.WaitStatus
				_, err = syscall.Wait4(pid, &ws, 0, nil)
				if err != nil {
					fmt.Println("error waiting for process to complete:", err)
				}
			}
		}
	}
	return 0
}
