package main

import (
	"fmt"
	"shell/internal/jobs"
	"shell/internal/parserline"
	"shell/internal/tools"
)

func main() {
	var jm jobs.JobManager
	for {
		err := tools.Promptline()
		if err != nil {
			fmt.Println("Prompt problem")
			return
		}
		var parser pars.Parser
		err = parser.Readline()
		if err != nil {
			fmt.Println("Readline problem")
			return
		}
		if len(parser.Line) == 0 {
			break
		}
		cmds := parser.Parserline()
		if len(cmds) == 0 {
			continue
		}
		for i := 0; i < len(cmds); i++ {
			cmds[i].ForkAndExec(&jm)
		}
	}
}
