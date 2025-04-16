##################################
# Makefile
# Lily Brongo
# COSC 3750, Spring 2025
# Homework 8
# April 8th, 2025
# This Makefile is used to compile and run wyshell.c
##################################

CC=gcc
CFLAGS=-ggdb -Wall -Wno-unused-function
# needed additive from assignment sheet


# .PHONY targets
.PHONY: tidy clean

# all: wyshell
wyshell: wyshell.c wyscanner.c
	$(CC) $(CFLAGS) wyshell.c wyscanner.c -o wyshell

# tidy
tidy:
	/bin/rm -f a.out core.*

# clean
clean: tidy
	/bin/rm -f wyshell


#ReadMe: Ishita Patel, Rhiannon Kilduff, Alex Bryan, Daniel Collins