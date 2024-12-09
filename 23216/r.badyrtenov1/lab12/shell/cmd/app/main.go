package main

import (
	"fmt"
	"os"
	"shell/internal/jobs"
	"shell/internal/parserline"
	"shell/internal/tools"
)

func main() {
	var parser pars.Parser
	var jm jobs.JobManager
	var readPipe *os.File
	var tmpPipe *os.File
	var writePipe *os.File
	var fgPid int
	signChan := make(chan os.Signal, 1)
	jm.Init()
	jm.SignalHandler(signChan, &fgPid)
	for {
		err := tools.Promptline()
		if err != nil {
			fmt.Println("Error in Prompt")
			return
		}
		line, err := parser.Readline()
		if err != nil {
			fmt.Println("Error in Parser")
			return
		}
		if len(line) == 0 {
			fmt.Println("\nexit")
			return
		}

		cmds := parser.Parserline(line)
		for i := 0; i < len(cmds); i++ {
			cmds[i].ForkAndExec(&jm, &fgPid, readPipe, tmpPipe, writePipe)
		}
		tmpPipe = nil
	}
}
