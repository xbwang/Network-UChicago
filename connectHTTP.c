#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* connectHTTP.c - connect to HTTP server */

int connectTCP(const char *host, const char *service );

/*------------------------------------------------------------------------
 * connectHTTP - connect to a specified HTTP port service on a specified host
 *------------------------------------------------------------------------
 */

int
connectHTTP( char * host, int port, char * request)
{
  char port_string[ 20 ];   /* Used to convert numeric port to string */
  const char * buffer;      /* message to be sent is formed here */
  int fd;                   /* Socket file descriptor */
  int n;                    /* bytes written to socket */

  /* connectTCP accepts port as a string */
  sprintf( port_string, "%d", port );

  /* Connect to web server */
  fd = connectTCP( host, port_string );

  if ( fd < 0 ) {
    return fd;
  }

  /* Send http request */
  buffer = "GET ";
  n = write( fd, buffer, strlen( buffer ));
  if ( n < 0 ) {
    return -1;
  }

  n = write( fd, request, strlen( request ));
  if ( n < 0 ) {
    return -1;
  }

  /* Send trailing CR/LF sequence twice */
  buffer = "\015\012\015\012";
  
  n = write( fd, buffer, strlen( buffer ) );
  if ( n < 0 ) {
    return -1;
  }

  /* Return fd so caller can get the data */
  return fd;
}

