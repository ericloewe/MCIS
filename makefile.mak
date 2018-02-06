CC = g++
STDFLAG = -std=c++11
CFLAGS = -g -c -Wall -pedantic
LINKFLAGS = -pthread 

default: tcas
build-default: tcas

limitstest : discreteMath.o limitstest.o
	$(CC) $(LINKFLAGS) discreteMath.o limitstest.o -o limitstest
    
limitstest.o : discreteMath.h limitstest.cpp 
	$(CC) $(STDFLAG) $(CFLAGS) limitstest.cpp -o limitstest.o
    

    
.PHONY : clean
clean : 
	-rm *.o limitstest
