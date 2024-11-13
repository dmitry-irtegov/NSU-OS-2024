package pars

import (
	"fmt"
	"os"
	"shell/internal/execute"
	"shell/internal/tools"
	"strings"
	"syscall"
)

type Parser struct {
	Line []byte
}

func (parc *Parser) SkipSpaces(id int) int {
	for id+1 < len(parc.Line) && parc.Line[id] == ' ' {
		id++
	}
	return id
}

func (parc *Parser) QuotesHandling(beginId int) (string, int) {
	str := make([]byte, 0)
	var quotestype byte
	for quotestype != 0 || !strings.Contains(" |&<>;", string(parc.Line[beginId])) {
		if quotestype == 0 && parc.Line[beginId] == '\\' {
			beginId++
		} else if parc.Line[beginId] == '\\' && quotestype == '"' && parc.Line[beginId+1] == quotestype {
			beginId++
		} else if quotestype == 0 && (parc.Line[beginId] == '\'' || parc.Line[beginId] == '"') {
			quotestype = parc.Line[beginId]
			beginId++
			continue
		} else if quotestype != 0 && parc.Line[beginId] == quotestype {
			quotestype = 0
			beginId++
			continue
		}
		if beginId >= len(parc.Line)-1 {
			break
		}
		str = append(str, parc.Line[beginId])
		beginId++
	}
	return string(str), beginId - 1
}

func (parc *Parser) Readline() error {
	str := make([]byte, 1024)
	for {
		n, err := syscall.Read(int(os.Stdin.Fd()), str)
		if err != nil {
			return err
		}
		parc.Line = append(parc.Line, str[:n]...)
		var quotestype byte
		for i := 0; i < len(parc.Line); i++ {
			if quotestype != '\'' && parc.Line[i] == '\\' && parc.Line[i+1] != '\'' {
				i++
			} else if quotestype == 0 && (parc.Line[i] == '\'' || parc.Line[i] == '"') {
				quotestype = parc.Line[i]
			} else if parc.Line[i] == quotestype {
				quotestype = 0
			}
		}
		if len(parc.Line) >= 2 && parc.Line[len(parc.Line)-2] == '\\' && parc.Line[len(parc.Line)-1] == '\n' {
			parc.Line = tools.RemoveByte(parc.Line, len(parc.Line)-2, len(parc.Line))
			fmt.Print("> ")
			continue
		}
		if quotestype != 0 {
			fmt.Print("> ")
			continue
		}
		parc.Line[len(parc.Line)-1] = 0
		return nil
	}
}

func (parc *Parser) Parserline() []execute.Command {
	var cmds []execute.Command
	var tmp execute.Command
	var tmpStr string
	var aflg bool

	for i := 0; i < len(parc.Line); i++ {
		i = parc.SkipSpaces(i)
		if parc.Line[i] == 0 {
			break
		}

		switch parc.Line[i] {
		case '&':
			if len(tmp.Cmdargs) == 0 {
				fmt.Println("Syntax error: missing command before '&'")
				return nil
			}
			tmp.Bkgrnd = true

		case '|':
			if len(tmp.Cmdargs) == 0 {
				fmt.Println("Syntax error: missing command before '|'")
				return nil
			}
			tmp.Cmdflag |= tools.OUTPIP
			cmds = append(cmds, tmp)
			tmp.Clear()
			tmp.Cmdflag |= tools.INPIP

		case '<':
			i = parc.SkipSpaces(i + 1)
			if parc.Line[i] == 0 {
				fmt.Println("Syntax error: missing input file name after '<'")
				return nil
			}
			tmpStr, i = parc.QuotesHandling(i)
			tmp.Infile = tmpStr

		case '>':
			if parc.Line[i+1] == '>' {
				aflg = true
				i++
			}
			i = parc.SkipSpaces(i + 1)
			if parc.Line[i] == 0 {
				fmt.Println("Syntax error: missing output file name after '>'")
				return nil
			}
			tmpStr, i = parc.QuotesHandling(i)
			if aflg {
				tmp.Appfile = tmpStr
			} else {
				tmp.Outfile = tmpStr
			}

		case ';':
			if len(tmp.Cmdargs) == 0 {
				fmt.Println("Syntax error: missing command before ';'")
				return nil
			}
			cmds = append(cmds, tmp)
			tmp.Clear()

		default:
			tmpStr, i = parc.QuotesHandling(i)
			tmp.Cmdargs = append(tmp.Cmdargs, tmpStr)
		}
	}
	if len(tmp.Cmdargs) != 0 {
		cmds = append(cmds, tmp)
		tmp.Clear()
	}
	return cmds
}
