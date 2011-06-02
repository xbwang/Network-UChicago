/********************
 * CSPP54015 Web Server
 * Xiangbo Wang
 * June 2nd, 2011
 ********************/
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

void html(char* _string, int* _sock);
void navigator(int *_sock);

void rqstParse(char *_buf, int _len, char *_request);
void typeParse(char *_request, int _len, char *_type);

void sysDisk(int* _sock);
void sysMemory(int* _sock);
void sysLoad(int* _sock);
void sysUptime(int* _sock);
void sysBasic(int* _sock);
void sysActv(int* _sock);
void netStat(int* _sock);
void ioStat(int* _sock);

#define QLEN    5
#define BUF_SIZE 1024*10
#define RST_SIZE 1024*10
#define RQT_SIZE 1024


/*
 *-----------------------------------------------------
 * main - web server
 *-----------------------------------------------------
 */
int
main(int argc, char *argv[])
{
    struct sockaddr_in fsin;
    socklen_t addrLen;
    char    *service = "daytime", *request, *buffer, *result, *type;
    int msSock, slSock, dataLen;
    FILE    *pFile;
    
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
            sprintf(result, "HTTP/1.1 200 / follows\nServer: cspp54015\nContent-type: text/html\n\n");
            (void) write(slSock, result, strlen(result));
            navigator(&slSock);
            sysBasic(&slSock);
            sysUptime(&slSock);
            sysLoad(&slSock);
            sysMemory(&slSock);
            sysDisk(&slSock);
            ioStat(&slSock);
            netStat(&slSock);
            sysActv(&slSock);
            html("</div>", &slSock);
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
                fclose(pFile);
            }else{
                sprintf(result, "HTTP/1.1 404 File Not Found\nServer: cspp54015\nContent-type: text/plain\n\n"
                        "Could not find the file requested: %s\n\n", request);
                (void) write(slSock, result, strlen(result));
            }
        }
        (void) close(slSock);
    }
}

/*
 *-----------------------------------------------------
 * html formatting - web server
 *-----------------------------------------------------
 */
void
html(char* _string, int* _sock)
{
    write(*_sock, _string, strlen(_string));
}

void
header(char* _name, int* _sock)
{
    char* result;
    
    result = (char *)malloc(sizeof(char)*RST_SIZE);
    memset(result, '\0', RST_SIZE);
    sprintf(result, "<a name='%s'></a><font color = 'blue'><h2>%s:</h2></font>"
        "<a href='#main'>Top</a><div id='line' style = 'width:1000px'><hr/></div>"
        , _name, _name);
    write(*_sock, result, strlen(result));
}
void
navigator(int *_sock)
{
    html("<body style= 'font-family:Tahoma, Geneva, sans-serif;background-color:#FFEAA8;'>", _sock);
    html("<title>CSPP51045 Web Server</title>", _sock);
    html("<a name='main'></a><a href='#System Basic Info'><b>System Basic Info</b></a>&nbsp;", _sock);
    html("<a href='#Uptime'><b>Uptime</b></a>&nbsp;", _sock);
    html("<a href='#Load Average'><b>Load Average</b></a>&nbsp;", _sock);
    html("<a href='#Memory'><b>Memory</b></a>&nbsp;", _sock);
    html("<a href='#Disk Space'><b>Disk Space</b></a>&nbsp;", _sock);
    html("<a href='#IO status'><b>IO status</b></a>&nbsp;", _sock);
    html("<a href='#Network Status'><b>Network Status</b></a>&nbsp;", _sock);
    html("<a href='#Disk Activity'><b>Disk Activity</b></a>&nbsp;", _sock);
}
/*
 *-----------------------------------------------------
 * request parsing - web server
 *-----------------------------------------------------
 */
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
/*
 *-----------------------------------------------------
 * system info functions - web server
 *-----------------------------------------------------
 */
void
container(char** _result)
{
    *_result = (char *)malloc(sizeof(char)*RST_SIZE);
    memset(*_result, '\0', RST_SIZE);
}

void
sysBasic(int* _sock)
{
    FILE *pCmd;
    char *result;

    container(&result);
    header("System Basic Info", _sock);
    write(*_sock, "<p>", strlen("<p>"));
    pCmd = popen("uname -a", "r");
    while(!feof(pCmd)){
        memset(result, '\0', RST_SIZE);
        fgets(result, RST_SIZE, pCmd);
        write(*_sock, result, strlen(result));
    }
    write(*_sock, "</p>", strlen("</p>"));
    pclose(pCmd);
}

void
sysUptime(int* _sock)
{
    struct  sysinfo sys_info;
    char *result;
    int uptime[4];

    if(sysinfo(&sys_info) != 0)
        error("sysinfo failed: %s\n", strerror(errno));
    uptime[0] = sys_info.uptime/86400; //days
    uptime[1] = (sys_info.uptime/3600) - (uptime[0]*24); //hours
    uptime[2] = (sys_info.uptime/60) - (uptime[0]*1440) -(uptime[1]*60); //mins
    uptime[3] = sys_info.uptime%60; //seconds
    
    container(&result);
    header("Uptime", _sock);
    sprintf(result, "<p>%d days, %d hours, %d minutes, %d seconds<br/></p>\n"
            , uptime[0], uptime[1], uptime[2], uptime[3]);
    (void) write(*_sock, result, strlen(result));
}

void
sysLoad(int *_sock)
{
    struct  sysinfo sys_info;
    char *result;
    double load[3];
    
    if(sysinfo(&sys_info) != 0)
        error("sysinfo failed: %s\n", strerror(errno));
        
    load[0] = (double)sys_info.loads[0]/65536.0; //avg within 1 min
    load[1] = (double)sys_info.loads[1]/65536.0; //avg within 5 mins
    load[2] = (double)sys_info.loads[2]/65536.0; //avg within 15 mins

    container(&result);
    header("Load Average", _sock);
    sprintf(result, "<p><table border=\"1\"><tr><th>1mins</th><th>5mins</th><th>15mins</th></tr>"
        "<tr><td>%.2f</td><td>%.2f</td><td>%.2f</td></tr></table></p>"
        , load[0], load[1], load[2]);
    (void) write(*_sock, result, strlen(result));
}

void
sysMemory(int* _sock)
{
    struct  sysinfo sys_info;
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

    container(&result);
    header("Memory", _sock);
    sprintf(result, "<p><table border=\"1\"><tr>"
        "<th>total memory</th><th>avaiable memory</th><th>used memory</th><th>shared memory</th><th>buffer used memory</th></tr>"
        "<tr><td>%d K</td><td>%d K</td><td>%d K</td><td>%d K</td><td>%d K</td></tr></table></p>"
        ,memory[2], memory[0], memory[1], memory[3], memory[4]);
    (void) write(*_sock, result, strlen(result));
}

void
outCmd(char* _name, char* _cmd, int* _sock, int _option)
{   
    char* result;
    FILE *pCmd;
    
    container(&result);
    sprintf(result, "</pre><b>[%s]</b><pre>", _name);
    html(result, _sock);
    if(_option){
        pCmd = popen(_cmd, "r");
    }else{
        pCmd = fopen(_cmd, "rb");
    }   
    while(!feof(pCmd)){
        memset(result, '\0', RST_SIZE);
        fgets(result, RST_SIZE, pCmd);
        write(*_sock, result, strlen(result));
    }
    if(_option){
        pclose(pCmd);
    }else{
        fclose(pCmd);
    }
}

void
sysDisk(int* _sock)
{
    FILE *pCmd;
    char *result;
    
    container(&result);
    header("Disk Space", _sock);
    html("<p><pre>", _sock);
    pCmd = popen("df", "r");
    while(!feof(pCmd)){
        memset(result, '\0', RST_SIZE);
        fgets(result, RST_SIZE, pCmd);;
        write(*_sock, result, strlen(result));
    }
    html("</pre></p>", _sock);
    pclose(pCmd);   
}


void
sysActv(int* _sock)
{
    FILE *pCmd;
    char *result;
    int count = 2;
    
    container(&result);
    header("Disk Activity", _sock);
    html("<p><b>[General]</b><pre>", _sock);
    pCmd = popen("top -bn 1", "r");
    fgets(result, RST_SIZE, pCmd);
    while(count--){
        memset(result, '\0', RST_SIZE);
        fgets(result, RST_SIZE, pCmd);;
        write(*_sock, result, strlen(result));
    }
    count = 3;
    while(count--){
        fgets(result, RST_SIZE, pCmd);
    }
    html("</pre><b>[Detailed]</b><pre>", _sock);
    while(!feof(pCmd)){
        memset(result, '\0', RST_SIZE);
        fgets(result, RST_SIZE, pCmd);
        write(*_sock, result, strlen(result));
    }
    html("</pre></p>", _sock);
    pclose(pCmd);
}

void
netStat(int* _sock)
{
    header("Network Status", _sock);
    html("<p><pre>", _sock);
    outCmd("ARP", "/proc/net/arp", _sock, 0);
    outCmd("User Connected", "w", _sock, 1);
    outCmd("Device(including errors/collisions)", "/proc/net/dev", _sock, 0);
    outCmd("Router Status", "netstat -rn", _sock, 1);
    outCmd("Connection Status", "netstat -t -u", _sock, 1);
    outCmd("Protocol Status", "netstat -s", _sock, 1);
    html("</pre></p>", _sock);

}

void
ioStat(int* _sock)
{
    FILE *pCmd;
    char *result;
    int count = 2;

    container(&result);
    header("IO status", _sock);
    html("<p><pre>", _sock);
    pCmd = popen("iostat -m", "r");
    while(count--){
        fgets(result, RST_SIZE, pCmd);
    }
    while(!feof(pCmd)){
        memset(result, '\0', RST_SIZE);
        fgets(result, RST_SIZE, pCmd);
        write(*_sock, result, strlen(result));
    }
    html("</pre></p>", _sock);  
    pclose(pCmd);
}
