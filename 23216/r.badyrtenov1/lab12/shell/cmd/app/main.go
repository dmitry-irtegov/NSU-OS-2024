package main

import (
	"fmt"
	"os"
	"shell/internal/jobs"
	"shell/internal/parserline"
	"shell/internal/tools"
)

func main() {
	var cmdPipe []string
	var tmpPipe *os.File
	var readPipe *os.File
	var writePipe *os.File
	var parser pars.Parser
	var jm jobs.JobManager
	var groupPid int
	var fgPid int
	jm.Init()
	jm.SignalHandler(&fgPid)
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
		if len(cmds) == 0 {
			jm.WriteDoneJobs()
		}
		for i := 0; i < len(cmds); i++ {
			cmds[i].ForkAndExec(&jm, &cmdPipe, &groupPid, &fgPid, &readPipe, &tmpPipe, &writePipe)
		}
	}
}
