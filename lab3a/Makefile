# NAME: Charlotte McGinn
# EMAIL: charlotte.mcginn@engineering.ucla.edu
# ID: 104-601-417 

# Makefile for lab4b

# Variables
CC=gcc
CFLAGS=-g -Wall -Wextra -std=gnu99

# Build the executable
build: main.c
	$(CC) $(CFLAGS) main.c -o lab3a

# Deletes all makefile-created files
clean:
	rm -f lab3a *.o *.tar.gz *.txt *~
	

# Build the distribution tarball
dist: 
	tar -cvzf lab3a-104601417.tar.gz main.c Makefile README ext2_fs.h