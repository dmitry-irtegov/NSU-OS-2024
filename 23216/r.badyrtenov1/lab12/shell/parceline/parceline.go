package parc

import (
	"bytes"
	"fmt"
)

type Command struct {
	cmdargs []string
	cmdflag byte
}

type Status struct {
	infile, outfile, appfile string
	bkgrnd                   bool
}

const (
	OUTPIP = 01
	INPIP  = 02
)

func GetBkgrndFlag(stat Status) bool {
	return stat.bkgrnd
}

func GetArgs(cmds Command) []string {
	return cmds.cmdargs
}

func skipSpaces(line []byte, id int) int {
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

	for i := 0; i < len(line); i++ {
		i = skipSpaces(line, i)
		if line[i] == 0 {
			break
		}

		switch sw := line[i]; sw {
		case '&':
			stat.bkgrnd = true

		case '|':
			if len(tmp.cmdargs) == 0 {
				fmt.Println("Syntax error: unexpected operator |")
				return nil, stat
			}
			tmp.cmdflag |= OUTPIP
			cmds = append(cmds, tmp)
			tmp.cmdargs = []string{}
			tmp.cmdflag = (0 | INPIP)

		case '<':
			i = skipSpaces(line, i)
			if line[i] == 0 {
				fmt.Println("Syntax error: missing input file name after '<'")
				return nil, stat
			}
			s := delimId(i, line)
			stat.infile = string(line[i:s])
			i = s

		case '>':
			i++
			if line[i] == '>' {
				aflg = true
				i++
			}
			i = skipSpaces(line, i)
			if line[i] == 0 {
				fmt.Println("Syntax error: missing output file name after '>'")
				return nil, stat
			}
			s := delimId(i, line)
			if aflg {
				stat.appfile = string(line[i:s])
			} else {
				stat.outfile = string(line[i:s])
			}
			i = s

		case ';':
			cmds = append(cmds, tmp)
			tmp.cmdflag = 0
			tmp.cmdargs = []string{}

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
