CC = g++
STDFLAG = -std=c++11
CFLAGS = -g -c -Wall -pedantic
LINKFLAGS = -pthread 

default: MCIStest
build-default: MCIStest

MCIStest :  MCIS_config.o MCIS_fileio.o discreteMath.o MCIS_MDA.o MCIStest.o
	$(CC) $(LINKFLAGS)  MCIS_config.o MCIS_fileio.o discreteMath.o MCIS_MDA.o MCIStest.o -o MCIStest
    
MCIStest.o : MCIS_config.h discreteMath.h MCIStest.cpp 
	$(CC) $(STDFLAG) $(CFLAGS) MCIStest.cpp -o MCIStest.o
    
discreteMath.o : discreteMath.h discreteMath.cpp 
	$(CC) $(STDFLAG) $(CFLAGS) discreteMath.cpp -o discreteMath.o

MCIS_config.o : MCIS_config.h MCIS_config.cpp 
	$(CC) $(STDFLAG) $(CFLAGS) MCIS_config.cpp -o MCIS_config.o

MCIS_fileio.o : discreteMath.h MCIS_fileio.h MCIS_fileio.cpp 
	$(CC) $(STDFLAG) $(CFLAGS) MCIS_fileio.cpp -o MCIS_fileio.o

MCIS_MDA.o : discreteMath.h MCIS_config.h MCIS_MDA.h MCIS_MDA.cpp 
	$(CC) $(STDFLAG) $(CFLAGS) MCIS_MDA.cpp -o MCIS_MDA.o
    

    
.PHONY : clean
clean : 
	-rm *.o MCIStest
