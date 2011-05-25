#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <time.h>

int           error(const char *format, ...);
int           TCPdaytimed(int fd);
int           passiveTCP(const char *service, int qlen);

#define QLEN     5

/*
 *-----------------------------------------------------
 * main - iterative TCP server for Daytime service
 *-----------------------------------------------------
 */

int
main(int argc, char *argv[])
{
  struct  sockaddr_in fsin;   /* from the client  */
  char    *service = "daytime";
  int     msock, ssock;       /*  master and slave sockets */
  socklen_t alen;               /*  sender address length   */

  switch (argc) {
  case  1:
    break;
  case  2:
    service = argv[1];
    break;
  default:
    error("usage:  TCPdaytimed [port]\n");
  }
      

  msock = passiveTCP(service, QLEN);

  while (1) {
     ssock = accept(msock, (struct sockaddr *)&fsin, &alen);
     if (ssock < 0)
        error("accept failed: %s\n", strerror(errno));
     (void) TCPdaytimed(ssock);
     (void) close(ssock);
 }
}
/*
 *-----------------------------------------------------
 *  TCPdaytimed  - get daytime and send
 *-----------------------------------------------------
 */
int
TCPdaytimed(int fd)
{
  char     *pts;             /* pointer to time string  */
  time_t   now;              /* actual time  */
  char     *ctime();
  
  (void)  time(&now);
  pts = ctime(&now);
  (void) write(fd, pts, strlen(pts));
  return(0);
}

int
parseRequest(char* buffer)
{
	
}