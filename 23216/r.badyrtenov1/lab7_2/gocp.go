package main

import (
	"errors"
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

func copyFile(src, dst string, fi os.FileInfo, wg *sync.WaitGroup) {
	defer wg.Done()

	srcFile, err := openWithRetry(src, syscall.O_RDONLY, 0)
	if err != nil {
		log.Println(err)
		return
	}
	defer srcFile.Close()

	dstFile, err := openWithRetry(dst, syscall.O_WRONLY|syscall.O_CREAT|syscall.O_TRUNC, fi.Mode())
	if err != nil {
		log.Println(err)
		return
	}
	defer dstFile.Close()

	if _, err = io.Copy(dstFile, srcFile); err != nil {
		log.Println(err)
	}

	if err = os.Chmod(dst, fi.Mode()); err != nil {
		log.Printf("Warning: can't set permissions for %s: %v", dst, err)
	}
}

func processDir(src, dst string, wg *sync.WaitGroup) {
	defer wg.Done()

	dir, err := openWithRetry(src, syscall.O_RDONLY, 0)
	if err != nil {
		log.Println(err)
		return
	}
	defer dir.Close()

	var entries []os.FileInfo
	for {
		entries, err = dir.Readdir(1)
		if err == io.EOF {
			break
		}
		if err != nil {
			log.Println(err)
			break
		}

		entry := entries[0]
		srcPath := filepath.Join(src, entry.Name())
		dstPath := filepath.Join(dst, entry.Name())

		if entry.IsDir() {
			if err = createDir(dstPath, entry); err != nil {
				log.Println(err)
				continue
			}

			wg.Add(1)
			go processDir(srcPath, dstPath, wg)
		} else if entry.Mode().IsRegular() {
			wg.Add(1)
			go copyFile(srcPath, dstPath, entry, wg)
		}
	}
}

func main() {
	if len(os.Args) != 3 {
		log.Fatal("Usage: gocp <source> <destination>")
	}

	var wg sync.WaitGroup
	source := os.Args[1]
	dest := os.Args[2]

	srcInfo, err := os.Stat(source)
	if err != nil {
		log.Fatal(err)
	}

	if !srcInfo.IsDir() {
		log.Fatal("Source is not a directory")
	}

	if err = os.MkdirAll(dest, srcInfo.Mode()); err != nil {
		log.Fatal(err)
	}

	if err = os.Chmod(dest, srcInfo.Mode()); err != nil {
		log.Printf("Warning: can't set permissions for %s: %v", dest, err)
	}

	wg.Add(1)
	go processDir(source, dest, &wg)

	wg.Wait()
}
