package terminal

import (
	"fmt"
	"syscall"
	"unsafe"
)

type State *syscall.Termios

const (
	ioctl_TCGETS = 0x540D
	ioctl_TCSETS = 0x540E
)

func IsTerminal(fd int) bool {
	var termios syscall.Termios
	_, _, errno := syscall.Syscall(
		syscall.SYS_IOCTL,
		uintptr(fd),
		uintptr(ioctl_TCGETS),
		uintptr(unsafe.Pointer(&termios)),
	)
	return errno == 0
}

func MakeRaw(fd int) (State, error) {
	originalState := &syscall.Termios{}
	_, _, errno := syscall.Syscall(
		syscall.SYS_IOCTL,
		uintptr(fd),
		uintptr(ioctl_TCGETS),
		uintptr(unsafe.Pointer(originalState)),
	)
	if errno != 0 {
		return nil, fmt.Errorf("error getting terminal state (TCGETS): %w", errno)
	}

	rawState := *originalState

	rawState.Iflag &^= (syscall.IGNBRK | syscall.BRKINT | syscall.PARMRK | syscall.ISTRIP |
		syscall.INLCR | syscall.IGNCR | syscall.ICRNL | syscall.IXON)
	rawState.Oflag &^= syscall.OPOST
	rawState.Lflag &^= (syscall.ECHO | syscall.ECHONL | syscall.ICANON | syscall.ISIG | syscall.IEXTEN)
	rawState.Cflag &^= (syscall.CSIZE | syscall.PARENB)
	rawState.Cflag |= syscall.CS8
	rawState.Cc[syscall.VMIN] = 1
	rawState.Cc[syscall.VTIME] = 0

	_, _, errno = syscall.Syscall(
		syscall.SYS_IOCTL,
		uintptr(fd),
		uintptr(ioctl_TCSETS),
		uintptr(unsafe.Pointer(&rawState)),
	)
	if errno != 0 {
		return nil, fmt.Errorf("error setting raw terminal state (TCSETS): %w", errno)
	}

	return originalState, nil
}

func Restore(fd int, state State) error {
	originalState := state
	if originalState == nil {
		return nil
	}
	_, _, errno := syscall.Syscall(
		syscall.SYS_IOCTL,
		uintptr(fd),
		uintptr(ioctl_TCSETS),
		uintptr(unsafe.Pointer(originalState)),
	)
	if errno != 0 {
		return fmt.Errorf("error restoring terminal state (TCSETS): %w", errno)
	}
	return nil
}

func GetState(fd int) (State, error) {
	state := &syscall.Termios{}
	_, _, errno := syscall.Syscall(
		syscall.SYS_IOCTL,
		uintptr(fd),
		uintptr(ioctl_TCGETS),
		uintptr(unsafe.Pointer(state)),
	)
	if errno != 0 {
		return nil, fmt.Errorf("error getting terminal state (TCGETS): %w", errno)
	}
	return state, nil
}
