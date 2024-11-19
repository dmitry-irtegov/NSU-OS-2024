package main

import (
	"fmt"
	"shell/internal/jobs"
	"shell/internal/parserline"
	"shell/internal/tools"
)

func main() {
	var parser pars.Parser
	var jm jobs.JobManager
	for {
		err := tools.Promptline()
		if err != nil {
			fmt.Println("Prompt problem")
			return
		}
		line, err := parser.Readline()
		if err != nil {
			fmt.Println("Parser problem")
			return
		}
		if len(line) == 0 {
			return
		}
		cmds := parser.Parserline(line)
		for i := 0; i < len(cmds); i++ {
			cmds[i].ForkAndExec(&jm)
		}
	}
}
