# Makefile for Proxy Lab 
#
# You may modify this file any way you like (except for the handin
# rule). You instructor will type "make" on your specific Makefile to
# build your proxy from sources.

CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread

all: echoserveri

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

echo.o: echo.c csapp.h
	$(CC) $(CFLAGS) -c echo.c

echoclient.o: echoclient.c csapp.h
	$(CC) $(CFLAGS) -c echoclient.c

echoclient: echoclient.o csapp.o
	$(CC) $(CFLAGS) echoclient.o csapp.o -o echoclient $(LDFLAGS)

echoserveri.o: echoserveri.c csapp.h
	$(CC) $(CFLAGS) -c echoserveri.c

echoserveri: echoserveri.o echo.o csapp.o
	$(CC) $(CFLAGS) echoserveri.o echo.o csapp.o -o echoserveri $(LDFLAGS)

handin:
	(make clean; cd ..; tar cvf $(USER)-proxylab-handin.tar proxylab-handout --exclude tiny --exclude nop-server.py --exclude proxy --exclude driver.sh --exclude port-for-user.pl --exclude free-port.sh --exclude ".*")

clean:
	rm -f *~ *.o echoclient echoserveri core *.tar *.zip *.gzip *.bzip *.gz

