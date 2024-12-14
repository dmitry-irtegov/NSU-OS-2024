package tools

import (
	"fmt"
	"os"
	"os/user"
	"strings"
	"syscall"
	"unsafe"
)

type Job struct {
	Pid      int
	Status   string
	Cmdargs  []string
	Bkgrnd   bool
	Id       int
	PipeFlag bool
}

func Tcsetpgrp(fd uintptr, pgrp int) {
	_, _, err := syscall.Syscall(syscall.SYS_IOCTL, fd, uintptr(syscall.TIOCSPGRP), uintptr(unsafe.Pointer(&pgrp)))
	if err != 0 {
		return
	}
}

func RemoveByte(slice []byte, beginId int, endId int) []byte {
	newSlice := make([]byte, len(slice)+beginId-endId)
	if beginId != endId {
		slice = append(slice[:beginId], slice[endId:]...)
	}
	copy(newSlice, slice)
	return newSlice
}

func Promptline() error {
	customer, err := user.Current()
	if err != nil {
		customer = &user.User{Username: "username"}
	}

	hostname, err := os.Hostname()
	if err != nil {
		hostname = "hostname"
	}

	cwd, err := os.Getwd()
	if err != nil {
		cwd = "~"
	} else {
		homeDir, existDir := os.LookupEnv("HOME")
		if existDir && strings.HasPrefix(cwd, homeDir) {
			cwd = strings.Replace(cwd, homeDir, "~", 1)
		}
	}
	_, err = fmt.Printf("%s@%s:%s$ ", customer.Username, hostname, cwd)
	return err
}
