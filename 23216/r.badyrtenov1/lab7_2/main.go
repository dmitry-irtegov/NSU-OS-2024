package main

import (
	"errors"
	"fmt"
	"io"
	"log"
	"os"
	"path/filepath"
	"sync"
	"syscall"
	"time"
)

func openWithRetry(path string, flags int, mode os.FileMode) (*os.File, error) {
	var pe *os.PathError

	for {
		fd, err := os.OpenFile(path, flags, mode)
		if err == nil {
			return fd, nil
		}

		if errors.As(err, &pe) && errors.Is(pe.Err, syscall.EMFILE) {
			time.Sleep(100 * time.Millisecond)
			continue
		}

		return nil, err
	}
}

func createDir(path string, fi os.FileInfo) error {
	if err := os.Mkdir(path, fi.Mode()); err != nil && !os.IsExist(err) {
		return err
	}
	return os.Chmod(path, fi.Mode())
}

func copyFile(src, dst string, fi os.FileInfo) {
	srcFile, err := openWithRetry(src, syscall.O_RDONLY, 0)
	if err != nil {
		fmt.Println(err)
		return
	}
	defer srcFile.Close()

	dstFile, err := openWithRetry(dst, syscall.O_WRONLY|syscall.O_CREAT|syscall.O_TRUNC, fi.Mode())
	if err != nil {
		fmt.Println(err)
		return
	}
	defer dstFile.Close()

	if _, err = io.Copy(dstFile, srcFile); err != nil {
		fmt.Println(err)
	}
}

func processDir(src, dst string) {
	dir, err := openWithRetry(src, syscall.O_RDONLY, 0)
	if err != nil {
		log.Println(err)
		return
	}
	defer dir.Close()

	var wg sync.WaitGroup
	var entries []os.FileInfo

	for {
		entries, err = dir.Readdir(1)
		if err == io.EOF {
			break
		}
		if err != nil {
			fmt.Println(err)
			break
		}

		entry := entries[0]
		srcPath := filepath.Join(src, entry.Name())
		dstPath := filepath.Join(dst, entry.Name())

		if entry.IsDir() {
			if err = createDir(dstPath, entry); err != nil {
				fmt.Println(err)
				continue
			}

			wg.Add(1)
			go func(srcP, dstP string) {
				defer wg.Done()
				processDir(srcP, dstP)
			}(srcPath, dstPath)
		} else if entry.Mode().IsRegular() {
			wg.Add(1)
			go func(srcP, dstP string, e os.FileInfo) {
				defer wg.Done()
				copyFile(srcP, dstP, e)
			}(srcPath, dstPath, entry)
		}
	}

	wg.Wait()
}

func main() {
	if len(os.Args) < 3 {
		log.Fatal("Usage: go run main.go <source> <destination>")
	}

	source := os.Args[1]
	dest := os.Args[2]

	srcInfo, err := os.Stat(source)
	if err != nil {
		fmt.Println(err)
		return
	}
	if !srcInfo.IsDir() {
		fmt.Println("cp " + source + ": not a directory")
		return
	}

	if _, err = os.Stat(dest); !errors.Is(err, os.ErrNotExist) {
		dest = filepath.Join(dest, filepath.Base(source))
	}

	if err = os.MkdirAll(dest, srcInfo.Mode()); err != nil {
		fmt.Println(err)
		return
	}

	processDir(source, dest)
}
