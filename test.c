#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>

int main()
{
	struct 	sysinfo sys_info;
	
	if(sysinfo(&sys_info) != 0)
		printf("haha\n");
		
	printf("%f\n", (double)sys_info.loads[0]/65536.0); //avg within 1 min
	printf("%f\n", (double)sys_info.loads[1]/65536.0); //avg within 5 mins
	printf("%f\n", (double)sys_info.loads[2]/65536.0); //avg within 15 mins
}

/*
int main(int argc, char *argv[])
{
	struct statvfs fiData;
	char fnPath[128];
	int perc, total, free, used;
	
	
	strcpy(fnPath, argv[1]);
	if((statvfs(fnPath, &fiData)) < 0){
		printf("haha\n");
	}
	total = (double)fiData.f_blocks*fiData.f_frsize/(1024*1024*1024)+0.5;
	free = (double)fiData.f_bfree*fiData.f_bsize/(1024*1024*1024)+0.5;
	used = (double)(fiData.f_blocks*fiData.f_frsize - fiData.f_bfree*fiData.f_bsize)/(1024*1024*1024)+0.5;
	perc = ((double)used/total+0.005)*100;
	
	printf("disk total: %uG\n", total);
	printf("disk free: %uG\n", free);
	printf("disk used: %uG\n", used);
	printf("disk used: %u%\n", perc);			
	return 0;
	
}
*/
/*
int
main()
{
	int days, hours, mins;
	struct sysinfo sys_info;
	
	if(sysinfo(&sys_info) != 0)
		perror("sysinfo");
	days = sys_info.uptime / 86400;
	hours = (sys_info.uptime/3600) - (days*24);
	mins = (sys_info.uptime/60) - (days*1440) -(hours*60);

	printf("%d days, %d hours, %d minutes, %d seconds\n", days, hours, mins, sys_info.uptime % 60);

	return 0;
}
*/

/*
char *parse(char *buf, int len)
{
	int i, j, n;
	char *file;
	
	for(i = 0; i < len; i++){
		if(buf[i] == ' '){
			for(j = i+1; j < len; j++){
				if(buf[j] == ' '){
					break;
				}
			}
			file = (char *)malloc(sizeof(char)*(j-i-1));
			strncpy(file, &buf[i+1], j-i-1);
			break;
		}
	}
	
	return file;
}

int main()
{
	char *buffer = "GET ./index.html HTTP/1.0";
	char *rst = parse(buffer, strlen(buffer));
	printf("%s\n", rst);
	
	return 0;
}
*/
