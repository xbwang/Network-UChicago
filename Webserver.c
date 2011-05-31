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
void sysDisk(int* _disk, char* _path);
void sysMemory(int* _memory);
void sysLoad(double* _load);
void sysUptime(int* _uptime);

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
	int		uptime[4], memory[5], diskAll[8], diskMe[8];	
	double	load[3];
	FILE 	*pFile;
	
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
										
	request = (char *)malloc(sizeof(char)*RQT_SIZE);
	buffer = (char *)malloc(sizeof(char)*BUF_SIZE);
	result = (char *)malloc(sizeof(char)*RST_SIZE);
	type = (char *)malloc(sizeof(char)*RQT_SIZE);									
	while(1) {
		slSock = accept(msSock, (struct sockaddr *)&fsin, &addrLen);
		if(slSock < 0)
			error("accept failed: %s\n", strerror(errno));
		
		memset(buffer, '\0', BUF_SIZE);
		memset(result, '\0', RST_SIZE);
		memset(request, '\0', RQT_SIZE);
		memset(type, '\0', RQT_SIZE);
		
		dataLen = recv(slSock, buffer, BUF_SIZE, 0);
		rqstParse(buffer, dataLen, request);
		if(request[0] == '\0'){
			sysDisk(diskAll, "/");
			sysDisk(diskMe, "/home/xbwang");
			sysMemory(memory);
			sysLoad(load);
			sysUptime(uptime);
			
			sprintf(result, "[disk space]\t%dG used, %dG available, %d%% usage\n"
								"[uptime]\t%d days, %d hours, %d minutes, %d seconds\n"
								"[load avg.]\t%.2f in 1min, %.2f in 5mins, %.2f in 15mins\n[memory]\t%dK avaiable, %dK used\n"
								, diskAll[0], diskAll[1], diskAll[2], uptime[0], uptime[1], uptime[2], uptime[3],
								load[0], load[1], load[2], memory[0], memory[1]);
			(void) write(slSock, result, strlen(result));
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
				sprintf(result, "HTTP/1.1 404 File Not Found\nServer: cspp54015\nContent-type: %s\n\n"
						"Could not find the file requested: %s\n\n", type, request);
				(void) write(slSock, result, strlen(result));
			}
		}
		(void) close(slSock);
	}
}

void
rqstParse(char *_buf, int _len, char *_request)
{
	int	i, j;
	
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
	int	i;
	
	for(i = 0; i < _len; i++){
		if(_request[i] == '.')
			break;
	}
	strcpy(_type, &_request[i+1]);
	if(strcmp(_type, "html") == 0){
		strcpy(_type, "text/html");
	}else if(strcmp(_type, "gif") == 0){
		strcpy(_type, "image/gif");
	}else{
		strcpy(_type, "text/plain");
	}
}

void
sysUptime(int* _uptime)
{
	struct 	sysinfo sys_info;
	
	if(sysinfo(&sys_info) != 0)
		error("sysinfo failed: %s\n", strerror(errno));
	_uptime[0] = sys_info.uptime/86400; //days
	_uptime[1] = (sys_info.uptime/3600) - (_uptime[0]*24); //hours
	_uptime[2] = (sys_info.uptime/60) - (_uptime[0]*1440) -(_uptime[1]*60); //mins
	_uptime[3] = sys_info.uptime%60; //seconds
}

void
sysLoad(double* _load)
{
	struct 	sysinfo sys_info;
	
	if(sysinfo(&sys_info) != 0)
		error("sysinfo failed: %s\n", strerror(errno));
		
	_load[0] = (double)sys_info.loads[0]/65536.0; //avg within 1 min
	_load[1] = (double)sys_info.loads[1]/65536.0; //avg within 5 mins
	_load[2] = (double)sys_info.loads[2]/65536.0; //avg within 15 mins
}

void
sysMemory(int* _memory)
{
	struct 	sysinfo sys_info;
	
	if(sysinfo(&sys_info) != 0)
		error("sysinfo failed: %s\n", strerror(errno));
		
	//in term of k
	_memory[0] = sys_info.freeram/1024; //free memory
	_memory[1] = sys_info.totalram/1024 - _memory[0]; //used memory
	_memory[2] = sys_info.totalram/1024; //total memory
	_memory[3] = sys_info.sharedram/1024; //shared memory
	_memory[4] = sys_info.bufferram/1024; //memory used by buffer
}

void
sysDisk(int* _disk, char* _path)
{
	struct statvfs diskData;
	char* path = _path;
	int total, cvt;
	
	if(statvfs(path, &diskData) < 0)
		error("statvfs failed: %s\n", strerror(errno));
	
	cvt = 1024*1024*1024;
	_disk[0] = diskData.f_bsize; //in term of k
	_disk[1] = diskData.f_blocks*diskData.f_bsize/1024; //1k blocks
	_disk[2] = (double)diskData.f_blocks*diskData.f_frsize/cvt + 0.5; //total space
	_disk[3] = (double)diskData.f_bfree*diskData.f_bsize/cvt + 0.5; //available disk space
	_disk[4] = (double)(diskData.f_blocks*diskData.f_frsize - diskData.f_bfree*diskData.f_bsize)/cvt + 0.5; //used disk space
	_disk[5] = ((double)_disk[0]/total+0.005)*100; //usage percentage
	_disk[6] = diskData.f_fsid;
	_disk[7] = diskData.f_namemax;
}