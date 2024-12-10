package parseline

import (
	"bytes"
	"fmt"
	"strings"

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

func (cmd *Command) blankSkip(line []byte, id int) int {
	for id+1 < len(line) && line[id] == ' ' {
		id++
	}
	return id
}

// func (cmd *Command) delimId(line []byte, id int) int {
// 	s := bytes.IndexAny(line[id:], " |&<>;")
// 	if s == -1 {
// 		return len(line) - 1
// 	}
// 	return id + s
// }

func (cmd *Command) argParser(line []byte, beginId int) (string, int) {
	str := make([]byte, 0)
	var nextId int = beginId
	var quotestype byte = 0
argParserLoop:
	for ; quotestype != 0 || !strings.Contains(" |&<>;", string(line[beginId])); beginId++ {
		if quotestype != 0 {
			nextId = bytes.IndexAny(line[beginId:], ("\\" + string(quotestype)))
			// if nextId == -1 {
			// 	// недосягаемо, уже обработано при чтении файла
			//  // все квотации точно закрыты
			// 	return "", -1
			// }
			nextId += beginId
			str = append(str, line[beginId:nextId]...)
			// на nextid сейчас либо (\) либо текущий маркер квотации (', ")
			beginId = nextId
			if line[beginId] == '\\' {
				beginId++
				str = append(str, line[beginId])
			} else if line[beginId] == quotestype {
				quotestype = 0
			}
		} else {
			nextId = bytes.IndexAny(line[beginId:], "\\ |&<>;'\"")
			if nextId == -1 {
				nextId = len(line) - 1
				str = append(str, line[beginId:nextId]...)
				beginId = nextId
				break argParserLoop
			}
			nextId += beginId
			str = append(str, line[beginId:nextId]...)
			beginId = nextId
			if line[beginId] == '\\' {
				beginId++
			} else if strings.Contains("'\"", string(line[beginId])) {
				quotestype = line[beginId]
			} else if strings.Contains(" |&<>;", string(line[beginId])) {
				break argParserLoop
			}

		}
		if beginId >= len(line)-1 {
			break
		}
	}
	return string(str), beginId
}

func (cmd *Command) Parceline(line []byte) []Command {
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
			i++
			i = cmd.blankSkip(line, i)
			newline, s := cmd.argParser(line, i)
			if i == s {
				fmt.Println("syntax error near unexpected token `newline'")
				return nil
			}
			tmp.infile = newline
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
			newline, s := cmd.argParser(line, i)
			if aflg {
				tmp.appfile = newline
			} else {
				tmp.outfile = newline
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
			newline, s := cmd.argParser(line, i)
			tmp.cmdargs = append(tmp.cmdargs, newline)
			i = s
		}
	}
	if len(tmp.cmdargs) != 0 {
		cmds = append(cmds, tmp)
	}
	return cmds
}
