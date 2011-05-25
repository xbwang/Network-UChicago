#
# Simple web server project
#	

all: Webserver test

DEFS =
CFLAGS = -g -Wall

# Client Objects 

OBJ = connectTCP.o connectUDP.o connectsock.o error.o connectHTTP.o 

# Sever Objects

SOBJ = passiveTCP.o passiveUDP.o passivesock.o


libtcp.a: $(OBJ) $(SOBJ)
	$(AR) -r $@ $(OBJ) $(SOBJ)
 
test: libtcp.a test.o
		$(CC) -o $@ $(CFLAGS) $@.o libtcp.a -lnsl

Webserver: libtcp.a Webserver.o
	$(CC) -o $@ $(CFLAGS) $@.o libtcp.a -lnsl 

clean: 
	rm -f a.out core test Webserver *.o *.a *~