package jobs

import (
	"syscall"
	"unsafe"
)

func Tcsetpgrp(fd int, pgid int32) (err error) {
	_, _, errno := syscall.Syscall6(syscall.SYS_IOCTL, uintptr(fd), uintptr(syscall.TIOCSPGRP), uintptr(unsafe.Pointer(&pgid)), 0, 0, 0)
	if errno != 0 {
		return errno
	}
	return nil
}
