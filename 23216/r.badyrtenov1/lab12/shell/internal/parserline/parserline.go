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
}

func (parc *Parser) SkipSpaces(line []byte, id int) int {
	for id+1 < len(line) && line[id] == ' ' {
		id++
	}
	return id
}

func (parc *Parser) QuotesHandling(line []byte, beginId int) (string, int) {
	str := make([]byte, 0)
	var quotestype byte
	for ; quotestype != 0 || !strings.Contains(" |&<>;", string(line[beginId])); beginId++ {
		if line[beginId] == '\\' && (quotestype == 0 || (quotestype == '"' && (line[beginId+1] == '"' || line[beginId+1] == '\\'))) {
			beginId++
		} else if quotestype == 0 && (line[beginId] == '\'' || line[beginId] == '"') {
			quotestype = line[beginId]
			continue
		} else if quotestype != 0 && line[beginId] == quotestype {
			quotestype = 0
			continue
		}
		if beginId >= len(line)-1 {
			break
		}
		str = append(str, line[beginId])
	}
	return string(str), beginId - 1
}

func (parc *Parser) Readline() ([]byte, error) {
	str := make([]byte, 1024)
	var line []byte
	for {
		n, err := syscall.Read(int(os.Stdin.Fd()), str)
		if err != nil {
			return nil, err
		}
		line = append(line, str[:n]...)
		var quotestype byte
		for i := 0; i < len(line); i++ {
			if line[i] == '\\' && (quotestype == 0 || (quotestype == '"' && (line[i+1] == '"' || line[i+1] == '\\'))) {
				i++
			} else if quotestype == 0 && (line[i] == '\'' || line[i] == '"') {
				quotestype = line[i]
			} else if line[i] == quotestype {
				quotestype = 0
			}
		}
		if len(line) >= 2 && line[len(line)-2] == '\\' && line[len(line)-1] == '\n' {
			if quotestype != '\'' {
				line = tools.RemoveByte(line, len(line)-2, len(line))
			}
			fmt.Print("> ")
			continue
		}
		if len(line) >= 1 && line[len(line)-1] != '\n' {
			continue
		}
		if quotestype != 0 {
			fmt.Print("> ")
			continue
		}
		if len(line) >= 1 && line[len(line)-1] == '\n' {
			line[len(line)-1] = 0
		}
		return line, nil
	}
}

func (parc *Parser) Parserline(line []byte) []exec.Command {
	var cmds []exec.Command
	var tmp exec.Command
	var tmpStr string
	var aflg bool

	for i := 0; i < len(line); i++ {
		i = parc.SkipSpaces(line, i)
		if line[i] == 0 {
			break
		}

		switch line[i] {
		case '&':
			if len(tmp.Cmdargs) == 0 {
				fmt.Println("Syntax error: missing command before '&'")
				return nil
			}
			tmp.Bkgrnd = true
			cmds = append(cmds, tmp)
			tmp.Init()

		case '|':
			if len(tmp.Cmdargs) == 0 {
				fmt.Println("Syntax error: missing command before '|'")
				return nil
			}
			tmp.Cmdflag += 2
			cmds = append(cmds, tmp)
			tmp.Init()
			tmp.Cmdflag += 1

		case '<':
			i = parc.SkipSpaces(line, i+1)
			if line[i] == 0 {
				fmt.Println("Syntax error: missing input file name after '<'")
				return nil
			}
			tmpStr, i = parc.QuotesHandling(line, i)
			tmp.Infile = tmpStr

		case '>':
			if line[i+1] == '>' {
				aflg = true
				i++
			}
			i = parc.SkipSpaces(line, i+1)
			if line[i] == 0 {
				fmt.Println("Syntax error: missing output file name after '>'")
				return nil
			}
			tmpStr, i = parc.QuotesHandling(line, i)
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
			tmp.Init()

		default:
			tmpStr, i = parc.QuotesHandling(line, i)
			tmp.Cmdargs = append(tmp.Cmdargs, tmpStr)
		}
	}
	if len(tmp.Cmdargs) != 0 {
		cmds = append(cmds, tmp)
	}
	return cmds
}
