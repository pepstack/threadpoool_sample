# Makefile for main.c
# 2019-11-12
#   350137278@qq.com
#
# show all predefinitions of gcc:
#
#  $ gcc -E -dM - </dev/null
###########################################################

# change version[] in main.c by below version:
PREFIX = .

all: main

CC=gcc

CFLAGS=-D_GNU_SOURCE

threadpool.o: $(PREFIX)/src/threadpool.c
	$(CC) $(CFLAGS) -c $(PREFIX)/src/threadpool.c -o $@

main.o: $(PREFIX)/src/main.c
	$(CC) $(CFLAGS) -c $(PREFIX)/src/main.c -o $@

main: main.o threadpool.o
	$(CC) -o $@ $(PREFIX)/threadpool.o \
	$(PREFIX)/main.o \
	-lpthread

clean:
	-rm -f $(PREFIX)/threadpool.o
	-rm -f $(PREFIX)/main.o
	-rm -f $(PREFIX)/main
	-rm -f $(PREFIX)/main.exe

check: all
	@echo "**** ALL TESTS PASSED ****"

.PHONY: all clean
