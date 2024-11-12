package parceline

import (
	"bytes"
	"fmt"
)

const (
	OUTPIP = 01
	INPIP  = 02
)

type Command struct {
	cmdargs []string
	cmdflag byte
}

type Status struct {
	infile, outfile, appfile string
	bkgrnd                   bool
}

func (s *Status) GetInfile() string {
	return s.infile
}

func (s *Status) GetOutfile() string {
	return s.outfile
}

func (s *Status) GetAppfile() string {
	return s.appfile
}

func (s *Status) GetBkgrndFlag() bool {
	return s.bkgrnd
}

func (c *Command) GetArgs() []string {
	return c.cmdargs
}

func blankSkip(line []byte, id int) int {
	for id+1 < len(line) && line[id] == ' ' {
		id++
	}
	return id
}

func delimId(id int, line []byte) int {
	s := bytes.IndexAny(line[id:], " |&<>;")
	if s == -1 {
		return len(line) - 1
	}
	return id + s
}

func Parceline(line []byte, cmds []Command, stat Status) ([]Command, Status) {
	var tmp Command
	var aflg bool

	for i := 0; i < len(line); {
		i = blankSkip(line, i)
		if line[i] == 0 {
			break
		}
		switch sw := line[i]; sw {
		case '&':
			if len(tmp.cmdargs) == 0 {
				fmt.Println("syntax error near unexpected token `&'")
				return nil, stat
			}
			stat.bkgrnd = true
			cmds = append(cmds, tmp)
			tmp.cmdflag = 0
			tmp.cmdargs = []string{}
			i++

		case '|':
			if len(tmp.cmdargs) == 0 {
				fmt.Println("syntax error near unexpected token `|'")
				return nil, stat
			}
			tmp.cmdflag |= OUTPIP
			cmds = append(cmds, tmp)
			tmp.cmdargs = []string{}
			tmp.cmdflag = (0 | INPIP)
			i++

		case '<':
			i = blankSkip(line, i)
			if i < len(line) { // TODO: < ____ catch this error
				fmt.Println("syntax error near unexpected token `<'")
				return nil, stat
			}
			s := delimId(i, line)
			stat.infile = string(line[i:s])
			i = s
			i++

		case '>':
			i++
			if line[i] == '>' {
				aflg = true
				i++
			}
			i = blankSkip(line, i)
			if len(tmp.cmdargs) == 0 {
				fmt.Println("syntax error near unexpected token `>'")
				return nil, stat
			}
			s := delimId(i, line)
			if aflg {
				stat.appfile = string(line[i:s])
			} else {
				stat.outfile = string(line[i:s])
			}
			i = s
			i++

		case ';':
			if len(tmp.cmdargs) == 0 {
				fmt.Println("syntax error near unexpected token `;'")
				return nil, stat
			}
			cmds = append(cmds, tmp)
			tmp.cmdflag = 0
			tmp.cmdargs = []string{}
			i++

		default:
			s := delimId(i, line)
			tmp.cmdargs = append(tmp.cmdargs, string(line[i:s]))
			i = s
		}
	}
	if len(tmp.cmdargs) != 0 {
		cmds = append(cmds, tmp)
	}
	return cmds, stat
}
