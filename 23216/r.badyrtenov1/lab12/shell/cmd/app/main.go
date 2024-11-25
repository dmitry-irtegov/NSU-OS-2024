package main

import (
	"fmt"
	"os"
	"shell/internal/jobs"
	"shell/internal/parserline"
	"shell/internal/tools"
)

func main() {
	var fgPidChan int
	var parser pars.Parser
	var jm jobs.JobManager
	signChan := make(chan os.Signal, 1)
	jm.SignalHandler(signChan, &fgPidChan)
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
			cmds[i].ForkAndExec(&jm, &fgPidChan)
		}
	}
}
