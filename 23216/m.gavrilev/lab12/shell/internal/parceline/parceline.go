package parceline

import (
	"bytes"
	"fmt"

	"shell/constants"
)

type Command struct {
	cmdargs                  []string
	cmdflag                  byte
	infile, outfile, appfile string
	backgroundflag           bool
}

func (cmd *Command) GetInfile() string {
	return cmd.infile
}

func (cmd *Command) GetOutfile() string {
	return cmd.outfile
}

func (cmd *Command) GetAppfile() string {
	return cmd.appfile
}

func (cmd *Command) GetBackgroundFlag() bool {
	return cmd.backgroundflag
}

func (cmd *Command) GetArgs() []string {
	return cmd.cmdargs
}

func (cmd *Command) GetCmdFlag() byte {
	return cmd.cmdflag
}

func (cmd *Command) Clear() {
	cmd.cmdflag = 0
	cmd.cmdargs = nil
	cmd.infile = ""
	cmd.outfile = ""
	cmd.appfile = ""
	cmd.backgroundflag = false
}

func (cmd Command) blankSkip(line []byte, id int) int {
	for id+1 < len(line) && line[id] == ' ' {
		id++
	}
	return id
}

func (cmd Command) delimId(id int, line []byte) int {
	s := bytes.IndexAny(line[id:], " |&<>;")
	if s == -1 {
		return len(line) - 1
	}
	return id + s
}

func (cmd Command) Parceline(line []byte) []Command {
	var cmds []Command
	var tmp Command
	var aflg bool

	for i := 0; i < len(line); {
		i = cmd.blankSkip(line, i)
		if line[i] == 0 {
			break
		}
		switch sw := line[i]; sw {
		case '&':
			if len(tmp.cmdargs) == 0 {
				fmt.Println("syntax error near unexpected token `&'")
				return nil
			}
			tmp.backgroundflag = true
			cmds = append(cmds, tmp)
			tmp.Clear()
			i++

		case '|':
			if len(tmp.cmdargs) == 0 {
				fmt.Println("syntax error near unexpected token `|'")
				return nil
			}
			tmp.cmdflag |= constants.OUTPIP
			cmds = append(cmds, tmp)
			tmp.Clear()
			tmp.cmdflag |= constants.INPIP
			i++

		case '<':
			i = cmd.blankSkip(line, i)
			if i < len(line) { // TODO: < ____ catch this error
				fmt.Println("syntax error near unexpected token `<'")
				return nil
			}
			s := cmd.delimId(i, line)
			tmp.infile = string(line[i:s])
			i = s
			i++

		case '>':
			i++
			if line[i] == '>' {
				aflg = true
				i++
			}
			i = cmd.blankSkip(line, i)
			if len(tmp.cmdargs) == 0 {
				fmt.Println("syntax error near unexpected token `>'")
				return nil
			}
			s := cmd.delimId(i, line)
			if aflg {
				tmp.appfile = string(line[i:s])
			} else {
				tmp.outfile = string(line[i:s])
			}
			i = s
			i++

		case ';':
			if len(tmp.cmdargs) == 0 {
				fmt.Println("syntax error near unexpected token `;'")
				return nil
			}
			cmds = append(cmds, tmp)
			tmp.Clear()
			i++

		default:
			s := cmd.delimId(i, line)
			tmp.cmdargs = append(tmp.cmdargs, string(line[i:s]))
			i = s
		}
	}
	if len(tmp.cmdargs) != 0 {
		cmds = append(cmds, tmp)
	}
	return cmds
}
