CC = g++
STDFLAG = -std=c++11
CFLAGS = -g -c -Wall -pedantic
LINKFLAGS = -pthread 

default: MCIStest nettest
build-default: MCIStest

MCIStest :  MCIS_config.o MCIS_fileio.o discreteMath.o MCIS_MDA.o MCIStest.o
	$(CC) $(LINKFLAGS)  MCIS_config.o MCIS_fileio.o discreteMath.o MCIS_MDA.o MCIStest.o -o MCIStest

nettest :  MCIS_config.o MCIS_fileio.o discreteMath.o MCIS_MDA.o MCIS_xplane_sock.o nettest.o
	$(CC) $(LINKFLAGS)  MCIS_config.o MCIS_fileio.o discreteMath.o MCIS_MDA.o MCIS_xplane_sock.o nettest.o -o nettest
    
MCIStest.o : MCIS_config.h discreteMath.h MCIS_fileio.h MCIStest.cpp 
	$(CC) $(STDFLAG) $(CFLAGS) MCIStest.cpp -o MCIStest.o

nettest.o : MCIS_config.h discreteMath.h MCIS_xplane_sock.h MCIS_fileio.h nettest.cpp 
	$(CC) $(STDFLAG) $(CFLAGS) nettest.cpp -o nettest.o
    
discreteMath.o : discreteMath.h discreteMath.cpp 
	$(CC) $(STDFLAG) $(CFLAGS) discreteMath.cpp -o discreteMath.o

MCIS_config.o : MCIS_config.h MCIS_config.cpp 
	$(CC) $(STDFLAG) $(CFLAGS) MCIS_config.cpp -o MCIS_config.o

MCIS_fileio.o : discreteMath.h MCIS_fileio.h MCIS_fileio.cpp 
	$(CC) $(STDFLAG) $(CFLAGS) MCIS_fileio.cpp -o MCIS_fileio.o

MCIS_MDA.o : discreteMath.h MCIS_config.h MCIS_MDA.h MCIS_MDA.cpp 
	$(CC) $(STDFLAG) $(CFLAGS) MCIS_MDA.cpp -o MCIS_MDA.o

MCIS_xplane_sock.o : discreteMath.h MCIS_xplane_sock.h MCIS_config.h MCIS_xplane_sock.cpp 
	$(CC) $(STDFLAG) $(CFLAGS) MCIS_xplane_sock.cpp -o MCIS_xplane_sock.o
    

    
.PHONY : clean
clean : 
	-rm *.o MCIStest nettest
