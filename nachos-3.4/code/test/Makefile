# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# if you are cross-compiling, you need to point to the right executables
# and change the flags to ld and the build procedure for as

GCCDIR = ../../../gnu-decstation-ultrix/decstation-ultrix/2.95.3/

LDFLAGS = -T script -N
ASFLAGS = -mips2
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)

PATH = $(GCCDIR):/lib:/usr/bin:/bin

CC = $(GCCDIR)gcc -B../../../gnu-decstation-ultrix/
AS = $(GCCDIR)as
LD = $(GCCDIR)ld

CPP = gcc -E
INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)

all: halt shell matmult sort testexec test2 testexechalf testjoin test4 test5 testexit test7 testjoinhalf testexithalf

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.c > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

shell.o: shell.c
	$(CC) $(CFLAGS) -c shell.c
shell: shell.o start.o
	$(LD) $(LDFLAGS) start.o shell.o -o shell.coff
	../bin/coff2noff shell.coff shell

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

testexec.o: testexec.c
	$(CC) $(CFLAGS) -c testexec.c
testexec: testexec.o start.o
	$(LD) $(LDFLAGS) start.o testexec.o -o testexec.coff
	../bin/coff2noff testexec.coff testexec
	
test2.o: test2.c
	$(CC) $(CFLAGS) -c test2.c
test2: test2.o start.o
	$(LD) $(LDFLAGS) start.o test2.o -o test2.coff
	../bin/coff2noff test2.coff test2
	
testexechalf.o: testexechalf.c
	$(CC) $(CFLAGS) -c testexechalf.c
testexechalf: testexechalf.o start.o
	$(LD) $(LDFLAGS) start.o testexechalf.o -o testexechalf.coff
	../bin/coff2noff testexechalf.coff testexechalf
	
testjoin.o: testjoin.c
	$(CC) $(CFLAGS) -c testjoin.c
testjoin: testjoin.o start.o
	$(LD) $(LDFLAGS) start.o testjoin.o -o testjoin.coff
	../bin/coff2noff testjoin.coff testjoin
	
testjoinhalf.o: testjoinhalf.c
	$(CC) $(CFLAGS) -c testjoinhalf.c
testjoinhalf: testjoinhalf.o start.o
	$(LD) $(LDFLAGS) start.o testjoinhalf.o -o testjoinhalf.coff
	../bin/coff2noff testjoinhalf.coff testjoinhalf
	
test4.o: test4.c
	$(CC) $(CFLAGS) -c test4.c
test4: test4.o start.o
	$(LD) $(LDFLAGS) start.o test4.o -o test4.coff
	../bin/coff2noff test4.coff test4
	
test5.o: test5.c
	$(CC) $(CFLAGS) -c test5.c
test5: test5.o start.o
	$(LD) $(LDFLAGS) start.o test5.o -o test5.coff
	../bin/coff2noff test5.coff test5
	
testexit.o: testexit.c
	$(CC) $(CFLAGS) -c testexit.c
testexit: testexit.o start.o
	$(LD) $(LDFLAGS) start.o testexit.o -o testexit.coff
	../bin/coff2noff testexit.coff testexit
	
testexithalf.o: testexithalf.c
	$(CC) $(CFLAGS) -c testexithalf.c
testexithalf: testexithalf.o start.o
	$(LD) $(LDFLAGS) start.o testexithalf.o -o testexithalf.coff
	../bin/coff2noff testexithalf.coff testexithalf
	
test7.o: test7.c
	$(CC) $(CFLAGS) -c test7.c
test7: test7.o start.o
	$(LD) $(LDFLAGS) start.o test7.o -o test7.coff
	../bin/coff2noff test7.coff test7