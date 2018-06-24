EXTRAE_HOME = /home/users/uncomauser04/usr/local3
CFLAGS = -O3 -g -I $(EXTRAE_HOME)/include -L $(EXTRAE_HOME)/lib -Wall -lmpitrace -fopenmp
CC = mpicc

all: juego

juego:  juego.c
        $(CC) $(CFLAGS) juego.c -o juego

clean:
        rm -fr TRACE.sym TRACE.mpits set-0 juego
