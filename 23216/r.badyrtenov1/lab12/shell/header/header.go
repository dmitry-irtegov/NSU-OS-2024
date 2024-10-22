package head

type Command struct {
	Cmdargs []string
	Cmdflag byte
}

const (
	MAXARGS = 256
	MAXCMDS = 50
	OUTPIP  = 01
	INPIP   = 02
)

var Cmds []Command = make([]Command, MAXCMDS)
var Infile, Outfile, Appfile string
var Bkgrnd bool
