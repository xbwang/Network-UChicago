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

#include <sys/sysinfo.h>
#include <sys/statvfs.h>

int error(const char *format, ...);
int passiveTCP(const char* service, int qlen);

void rqstParse(char *_buf, int _len, char *_request);
void typeParse(char *_request, int _len, char *_type);
void sysDisk(char* _path, int* _sock);
void sysMemory(int* _sock);
void sysLoad(int* _sock);
void sysUptime(int* _sock);
void sysBasic(int* _sock);
void sysActv(int* _sock);
void netStat(int* _sock);

#define QLEN	5
#define BUF_SIZE 1024*10
#define RST_SIZE 1024*10
#define RQT_SIZE 1024
/*
 *-----------------------------------------------------
 * main - iterative CGI web server
 *-----------------------------------------------------
 */

int
main(int argc, char *argv[])
{
	struct sockaddr_in fsin;
	socklen_t addrLen;
	char	*service = "daytime", *request, *buffer, *result, *type;
	int		msSock, slSock, dataLen;
	FILE	*pFile;
	
	request = (char *)malloc(sizeof(char)*RQT_SIZE);
	buffer = (char *)malloc(sizeof(char)*BUF_SIZE);
	result = (char *)malloc(sizeof(char)*RST_SIZE);
	type = (char *)malloc(sizeof(char)*RQT_SIZE);
	
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
	msSock = passiveTCP(service, QLEN);									
	while(1) {
		memset(buffer, '\0', BUF_SIZE);
		memset(result, '\0', RST_SIZE);
		memset(request, '\0', RQT_SIZE);
		memset(type, '\0', RQT_SIZE);
		
		slSock = accept(msSock, (struct sockaddr *)&fsin, &addrLen);
		if(slSock < 0)
			error("accept failed: %s\n", strerror(errno));
		dataLen = recv(slSock, buffer, BUF_SIZE, 0);
		rqstParse(buffer, dataLen, request);
		if(request[0] == '\0'){
			sprintf(result, "HTTP/1.1 200 / follows\nServer: cspp54015\nContent-type: text/plain\n\n");
			(void) write(slSock, result, strlen(result));
			sysBasic(&slSock);
			sysDisk("/", &slSock);
			sysDisk("/home/xbwang", &slSock);
			sysMemory(&slSock);
			sysLoad(&slSock);
			sysUptime(&slSock);
			netStat(&slSock);
			sysActv(&slSock);
		}else{
			typeParse(request, strlen(request), type);
			pFile = fopen(request,"rb");
			if(pFile != NULL){
				sprintf(result, "HTTP/1.1 200 /%s follows\nServer: cspp54015\nContent-type: %s\n\n", request, type);
				while(!feof(pFile)){
					memset(result, '\0', RST_SIZE);
					dataLen = fread(result, sizeof(char), RST_SIZE, pFile);
					(void) write(slSock, result, dataLen);
				}
			}else{
				sprintf(result, "HTTP/1.1 404 File Not Found\nServer: cspp54015\nContent-type: text/plain\n\n"
						"Could not find the file requested: %s\n\n", request);
				(void) write(slSock, result, strlen(result));
			}
		}
		(void) close(slSock);
	}
}

void
rqstParse(char *_buf, int _len, char *_request)
{
	int i, j;
	
	for(i = 0; i < _len; i++){
		if(_buf[i] == ' '){
			for(j = i+1; j < _len; j++){
				if(_buf[j] == ' '){
					break;
				}
			}
			break;
		}
	}
	strncpy(_request, &_buf[i+2], j-i-2);
}

void
typeParse(char *_request, int _len, char *_type)
{
	int i;

	for(i = 0; i < _len; i++){
		if(_request[i] == '.')
			break;
	}
	strcpy(_type, &_request[i+1]);
	if(strcmp(_type, "html") == 0){
		strcpy(_type, "text/html");
	}else if(strcmp(_type, "gif") == 0){
		strcpy(_type, "image/gif");
	}else if(strcmp(_type, "jpg") == 0){
		strcpy(_type, "image/jpeg");
	}else{
		strcpy(_type, "text/plain");
	}
}

void
sysUptime(int* _sock)
{
	struct	sysinfo sys_info;
	char *result;
	int uptime[4];

	if(sysinfo(&sys_info) != 0)
		error("sysinfo failed: %s\n", strerror(errno));
	uptime[0] = sys_info.uptime/86400; //days
	uptime[1] = (sys_info.uptime/3600) - (uptime[0]*24); //hours
	uptime[2] = (sys_info.uptime/60) - (uptime[0]*1440) -(uptime[1]*60); //mins
	uptime[3] = sys_info.uptime%60; //seconds
	
	result = (char *)malloc(sizeof(char)*RST_SIZE);
	sprintf(result, "\nUptime:\n\t%d days, %d hours, %d minutes, %d seconds\n"
				,uptime[0], uptime[1], uptime[2], uptime[3]);
	(void) write(*_sock, result, strlen(result));
}

void
sysLoad(int *_sock)
{
	struct	sysinfo sys_info;
	char *result;
	double load[3];
	
	if(sysinfo(&sys_info) != 0)
		error("sysinfo failed: %s\n", strerror(errno));
		
	load[0] = (double)sys_info.loads[0]/65536.0; //avg within 1 min
	load[1] = (double)sys_info.loads[1]/65536.0; //avg within 5 mins
	load[2] = (double)sys_info.loads[2]/65536.0; //avg within 15 mins

	result = (char *)malloc(sizeof(char)*RST_SIZE);
	sprintf(result, "\nLoad Average:\n\t[1min]%.2f, [5mins]%.2f, [15mins]%.2f\n"
			,load[0], load[1], load[2]);
	(void) write(*_sock, result, strlen(result));
}

void
sysMemory(int* _sock)
{
	struct	sysinfo sys_info;
	char *result;
	int memory[5];

	if(sysinfo(&sys_info) != 0)
		error("sysinfo failed: %s\n", strerror(errno));
		
	//in term of k
	memory[0] = sys_info.freeram/1024; //free memory
	memory[1] = sys_info.totalram/1024 - memory[0]; //used memory
	memory[2] = sys_info.totalram/1024; //total memory
	memory[3] = sys_info.sharedram/1024; //shared memory
	memory[4] = sys_info.bufferram/1024; //memory used by buffer

	result = (char *)malloc(sizeof(char)*RST_SIZE);
	sprintf(result, "\nMemory:\n\t[total memory]%dK, [avaiable memory]%dK, [used memory]%dK, [shared memory]%dK, [buffer used memory]%dK\n"
			,memory[2], memory[0], memory[1], memory[3], memory[4]);
	(void) write(*_sock, result, strlen(result));
}

void
sysDisk(char* _path, int* _sock)
{
	struct statvfs diskData;
	char* path = _path;
	int cvt, disk[8];
	char *result;

	if(statvfs(path, &diskData) < 0)
		error("statvfs failed: %s\n", strerror(errno));
	
	cvt = 1024*1024*1024;
	disk[0] = diskData.f_bsize; //in term of k
	disk[1] = diskData.f_blocks*diskData.f_bsize/1024; //1k blocks
	disk[2] = (double)diskData.f_blocks*diskData.f_frsize/cvt + 0.5; //total space
	disk[3] = (double)diskData.f_bfree*diskData.f_bsize/cvt + 0.5; //available disk space
	disk[4] = (double)(diskData.f_blocks*diskData.f_frsize - diskData.f_bfree*diskData.f_bsize)/cvt + 0.5; //used disk space
	disk[5] = ((double)disk[4]/disk[2]+0.005)*100; //usage percentage
	disk[6] = diskData.f_fsid;
	disk[7] = diskData.f_namemax;

	result = (char *)malloc(sizeof(char)*RST_SIZE);
	sprintf(result, "\nDisk Space Mount at '%s':\n\t[block size]%dK, [num.of.1Kblocks]%d, [total space]%dG, [available space]%dG, [used space]%dG, [usage]%d%%\n"
			,path ,disk[0], disk[1], disk[2], disk[3], disk[4], disk[5]);
	(void) write(*_sock, result, strlen(result));
}

void
sysActv(int* _sock)
{
	FILE *pCmd;
	char *result;
	int count = 2;
	pCmd = popen("top -bn 1", "r");
	result = (char *)malloc(sizeof(char)*RST_SIZE);

	fgets(result, RST_SIZE, pCmd);
	count = 2;
	write(*_sock, "\nDisk Activity(General):\n\t", strlen("\nDisk Activity(General):\n\t"));
	while(count--){
		memset(result, '\0', RST_SIZE);
		fgets(result, RST_SIZE, pCmd);
		strcat(result, "\t");
		write(*_sock, result, strlen(result));
	}
	count = 3;
	while(count--){
		fgets(result, RST_SIZE, pCmd);
	}
	write(*_sock, "\nDisk Activity(Detailed):\n\t", strlen("\nDisk Activity(Detailed):\n\t"));
	while(!feof(pCmd)){
		memset(result, '\0', RST_SIZE);
		fgets(result, RST_SIZE, pCmd);
		strcat(result, "\t");
		write(*_sock, result, strlen(result));
	}
	pclose(pCmd);
}

void
sysBasic(int* _sock)
{
	FILE *pCmd;
	char *result;

	pCmd = popen("uname -a", "r");
	result = (char *)malloc(sizeof(char)*RST_SIZE);
	write(*_sock, "System Basic Info:\n\t", strlen("System Basic Info:\n\t"));
	while(!feof(pCmd)){
		memset(result, '\0', RST_SIZE);
		fgets(result, RST_SIZE, pCmd);
		strcat(result, "\t");
		write(*_sock, result, strlen(result));
	}
	pclose(pCmd);
}

void
netStat(int* _sock)
{
	FILE *pf, *pCmd;
	char *result;

	result = (char *)malloc(sizeof(char)*RST_SIZE);
	pf = fopen("/proc/net/arp", "rb");
	write(*_sock, "\nNetwork Status:\n\t", strlen("\nNetwork Status:\n\t"));
	write(*_sock, "[ARP]:\n", strlen("[ARP]:\n"));
	while(!feof(pf)){
		memset(result, '\0', RST_SIZE);
		fgets(result, RST_SIZE, pf);
		write(*_sock, "\t", strlen("\t"));
		write(*_sock, result, strlen(result));
	}
	
	pf = fopen("/proc/net/dev", "rb");
	write(*_sock, "[Device Info(including errors/collisions)]:\n", strlen("[Device Info(including errors/collisions)]:\n"));
	while(!feof(pf)){
		memset(result, '\0', RST_SIZE);
		fgets(result, RST_SIZE, pf);
		write(*_sock, "\t", strlen("\t"));
		write(*_sock, result, strlen(result));
	}
	
	pCmd = popen("netstat -t -u", "r");
	write(*_sock, "[Connection status]:\n", strlen("[Connection status]:\n"));
	while(!feof(pCmd)){
		memset(result, '\0', RST_SIZE);
		fgets(result, RST_SIZE, pCmd);
		write(*_sock, "\t", strlen("\t"));
		write(*_sock, result, strlen(result));
	}
	fclose(pf);
	pclose(pCmd);
}