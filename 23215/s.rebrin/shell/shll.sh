#!/bin/bash
gcc -Wall -o shell shell.c shell.h jobs.c parseline.c -lreadline -g && ./shell
