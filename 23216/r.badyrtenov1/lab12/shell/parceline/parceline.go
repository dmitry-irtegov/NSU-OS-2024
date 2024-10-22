package parc

import (
	"bytes"
	head "shell/header"
)

func blankskip(line []byte, id int) int {
	for line[id] == ' ' {
		id++
	}
	return (id)
}

func min(a int, b int) int {
	if a == -1 {
		return b
	}
	if b == -1 {
		return a
	}
	if a < b {
		return a
	}
	return b
}

func Parceline(line []byte) int {
	var nargs int = 0
	var ncmds int = 0
	var rval int = 0
	var i int = 0
	var delim string = " \t|&<>;\n"
	var delimbyte byte = 0
	var aflg bool = false
	head.Bkgrnd = false
	for i := 0; i < head.MAXCMDS; i++ {
		head.Cmds[i].Cmdflag = 0
	}

	for line[i] != 0 {
		i = blankskip(line, i)
		if line[i] == 0 {
			break
		}
		switch sw := line[i]; sw {
		case '&':
			head.Bkgrnd = true
			line[i] = 0
			i++
		case '|':
			if nargs == 0 {
				panic("syntax error\n")
			}
			head.Cmds[ncmds].Cmdflag |= head.OUTPIP
			ncmds++
			head.Cmds[ncmds].Cmdflag |= head.INPIP
			line[i] = 0
			i++
			nargs = 0
		case '<':
			line[i] = 0
			i++
			i = blankskip(line, i)
			if line[i] == 0 {
				panic("syntax error\n")
			}
			head.Infile = string(line[i:])
			i = bytes.IndexAny(line[i:], delim)
			if i == -1 {
				i = len(line) - 1
			}
			if line[i] == ' ' {
				line[i] = 0
				i++
			}
		case '>':
			if line[i+1] == '>' {
				aflg = true
				line[i] = 0
				i++
			}
			line[i] = 0
			i++
			i = blankskip(line, i)
			if line[i] == 0 {
				panic("syntax error\n")
			}
			if aflg {
				head.Appfile = string(line[i : i+min(bytes.IndexAny(line[i:], delim), bytes.IndexByte(line[i:], delimbyte))])
			} else {
				head.Outfile = string(line[i : i+min(bytes.IndexAny(line[i:], delim), bytes.IndexByte(line[i:], delimbyte))])
			}
			i = bytes.IndexAny(line[i:], delim)
			if i == -1 {
				i = len(line) - 1
			}
			if line[i] == ' ' {
				line[i] = 0
				i++
			}
		case ';':
			line[i] = 0
			i++
			ncmds++
			nargs = 0
		default:
			if nargs == 0 {
				rval = ncmds + 1
				head.Cmds[ncmds].Cmdargs = make([]string, head.MAXARGS)
			}
			head.Cmds[ncmds].Cmdargs[nargs] = string(line[i : i+min(bytes.IndexAny(line[i:], delim), bytes.IndexByte(line[i:], delimbyte))])
			nargs++
			i = bytes.IndexAny(line[i:], delim)
			if i == -1 {
				i = len(line) - 1
			}
			if line[i] == ' ' {
				line[i] = 0
				i++
			}
		}
	}
	if ncmds != 0 && (head.Cmds[ncmds-1].Cmdflag&head.OUTPIP) != 0 {
		if nargs == 0 {
			panic("syntax error\n")
		}
	}
	return rval
}
