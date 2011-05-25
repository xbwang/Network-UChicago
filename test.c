#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>

extern int	errno;

int	test(const char *host, const char *service);
int	error(const char *format, ...);
int	connectTCP(const char *host, const char *service);

#define	LINELEN		128

/*------------------------------------------------------------------------
 * main - timecat arg parsing
 *------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
	char	*host = "localhost";	/* host to use if none supplied	*/
	char	*service = "daytime";	/* default service name		*/

	switch (argc) {
	case 1:
		host = "localhost";
		break;
	case 3:
		service = argv[2];
		/* FALL THROUGH */
	case 2:
		host = argv[1];
		break;
	default:
		fprintf(stderr, "usage: timecat [host [port]]\n");
		exit(1);
	}
	test(host, service);
	exit(0);
}

/*------------------------------------------------------------------------
 * timecat  -- get time from remote host
 *------------------------------------------------------------------------
 */
int
test(const char *host, const char *service)
{
	char	*buf;//[LINELEN+1];		/* buffer for one line of text	*/
	int	s;//, n;			/* socket descriptor, read count*/

	s = connectTCP(host, service);
	buf = "GET /index.html HTTP/1.0 Connection: Keep-Alive User-Agent: Mozilla/2.02Gold (WinNT; I) Host: www.ora.com Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, */*\r\r";
	(void) write(s, buf, strlen(buf));
	/*
	while( (n = read(s, buf, LINELEN)) > 0) {
          buf[n] = '\0';    
	  (void) fputs (buf, stdout );
	}
	*/
	return 0;
}
