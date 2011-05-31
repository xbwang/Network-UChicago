#
# Simple web server project
#	

all: Webserver

DEFS =
CFLAGS = -g -Wall

# Client Objects 

OBJ = connectsock.o error.o

# Sever Objects

SOBJ = passivesock.o 


libtcp.a: $(OBJ) $(SOBJ)
	$(AR) -r $@ $(OBJ) $(SOBJ)
 
Send: libtcp.a Send.o
		$(CC) -o $@ $(CFLAGS) $@.o libtcp.a -lnsl

Webserver: libtcp.a Webserver.o
	$(CC) -o $@ $(CFLAGS) $@.o libtcp.a -lnsl 

clean: 
	rm -f a.out core Webserver *.o *.a *~
