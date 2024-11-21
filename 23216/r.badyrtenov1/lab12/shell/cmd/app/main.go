package main

import (
	"fmt"
	"shell/internal/jobs"
	"shell/internal/parserline"
	"shell/internal/signals"
	"shell/internal/tools"
)

func main() {
	var ch signals.Channels
	var parser pars.Parser
	var jm jobs.JobManager
	ch.Init()
	ch.SignalHandler(&jm)
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
			cmds[i].ForkAndExec(&jm, &ch)
		}
	}
}
