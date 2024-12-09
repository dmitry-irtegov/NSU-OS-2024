package main

import (
	"fmt"
	"io"
	"log"
	"net/http"
	_ "net/http/pprof"
	"os"
	"shell/constants"
	"shell/internal/execute"
	"shell/internal/jobs"
	"shell/internal/parceline"
	"shell/util"
	"strings"
)

func main() {
	//signal.Ignore(syscall.SIGINT, syscall.SIGTSTP, syscall.SIGQUIT, syscall.SIGTTOU, syscall.SIGTTIN) // sigchild not working
	// notify -> pid signal.kill (signal that was given to fg one process)
	// prompt := ([]byte)(fmt.Sprintf("[%s] ", os.Args[0]))
	go func() {
		log.Println(http.ListenAndServe("localhost:6060", nil))
	}()
	var parser parceline.Command
	var executer execute.Exec

	shell_terminal := os.Stdin
	// var last_pgid int
out:
	for {
		err := util.Prompt()
		if err != nil {
			fmt.Println("prompt error", err)
			return
		}
		line, err := util.ReadLine()
		if err != nil {
			if err == io.EOF {
				fmt.Println()
				break out
			}
			fmt.Println("parser error", err)
			return
		}
		cmds := parser.Parceline(line) //a

		var read *os.File
		var readNext *os.File
		var write *os.File
		for i := 0; i < len(cmds); i++ {
			if strings.Compare(cmds[i].GetArgs()[0], ("exit")) == 0 {
				jobs.Tcsetpgrp(int(shell_terminal.Fd()), 5000)
				break out
			}
			fmt.Println(strings.Compare(cmds[i].GetArgs()[0], ("fg")))
			if cmds[i].GetArgs()[0] == "fg" {
				jobs.Tcsetpgrp(int(shell_terminal.Fd()), 5000)
			}
			if cmds[i].GetCmdFlag()&constants.OUTPIP != 0 {
				readNext, write, err = os.Pipe()
				if err != nil {
					fmt.Println("pipe creation error", err)
				}
			}

			executer.ForkAndExec(cmds[i], read, write)

			if read != nil {
				err = read.Close()
				if err != nil {
					fmt.Println("closing read pipe error", err)
				}
			}
			if write != nil {
				err = write.Close()
				if err != nil {
					fmt.Println("closing write pipe error", err)
				}
			}
			write = nil
			read = readNext
			readNext = nil
		}
	}
	fmt.Println("exit")
	return
}
