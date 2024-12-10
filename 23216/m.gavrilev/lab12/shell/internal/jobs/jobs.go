package jobs

// #include <stdlib.h>
// #include <stdio.h>
// #include <unistd.h>
// static void some( int a, int b)
// {
//		printf("%d %d\n", a, b);
// }
import "C"

func Tcsetpgrp(fd int, pgid int32) (err error) {
	a := C.int(fd)
	b := C.int(pgid)
	C.tcsetpgrp(a, b)
	//fmt.Println(C.some())
	//var ne _Ctype_int
	//C.tcsetpgrp(fd, pgid)
	// _, _, errno := syscall.Syscall6(syscall.SYS_IOCTL, uintptr(fd), uintptr(syscall.TIOCSPGRP), uintptr(unsafe.Pointer(&pgid)), 0, 0, 0)
	// if errno != 0 {
	// 	return errno
	// }
	return nil
}
