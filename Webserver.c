#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <time.h>

int error(const char *format, ...);
int passiveTCP(const char* service, int qlen);
int rqstParse();
int nofileResonse();
int statResponse();

#define QLEN	5
#define BUFFER_SIZE 1024*5
#define TEMP_SIZE 1024

/*
 *-----------------------------------------------------
 * main - iterative CGI web server
 *-----------------------------------------------------
 */

int
main(int argc, char *argv[])
{
	struct	sockaddr_in fsin;
	char	*service = "daytime", *buffer, *buf;
	int		msSock, slSock, dataLen; //msSock for master socket, and slSock for slave socket
	socklen_t addrLen;
	
	switch (argc) {
		case 1:
			error("usage:  Webserver [port]\n");
			break;
		case 2:
			service = argv[1];
			break;
		default:
			error("usage:  Webserver [port]\n");
	}
	
	msSock = passiveTCP(service, QLEN);	//create a TCP server socket
										//bind the port *service* on that socket
										//set the queue to be length of QLEN
	
	buffer = (char *)malloc(sizeof(char)*BUFFER_SIZE);
	memset(buffer, '\0', BUFFER_SIZE);
	buffer = "";
	buf = (char *)malloc(sizeof(char)*(TEMP_SIZE+1));
	memset(buf, '\0', TEMP_SIZE);
	while(1) {
		slSock = accept(msSock, (struct sockaddr *)&fsin, &addrLen);
		if(slSock < 0)
			error("accept failed: %s\n", strerror(errno));
		
		while(dataLen = read(slSock, buf, TEMP_SIZE) > 0){
			//(void) fputs(buffer, stdout);
			strcat(buffer, buf);
			memset(buf, '\0', TEMP_SIZE); //reset the buffer
		}
		(void) fputs(buffer, stdout);
		(void) fputs("\n", stdout);
		/*
		if(dataLen < 0)
			error("receive failed: %s\n", strerror(errno));
		
		(void) fputs(buffer, stdout);
		(void) fputs("\n", stdout);
		*/
	}
}

int rqstParse(char *buffer)
{
	return 0;
}

int nofileResonse()
{
	return 0;
}

int statResponse()
{
	return 0;
}