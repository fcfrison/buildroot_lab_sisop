#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>
 
#define BUFLEN	1024	//Max length of buffer
#define PORT	12345	//The port on which to listen for incoming data
char* templateFormatter(char* content, char* template);
char* generateHtml(char* fileName, char* titleName, char** keyWords, char nrKeywords);
char* generateResponseSystemInfo();
char* generateHtmlCpuUsage(char* fileName, char* titleName, char** keyWords, char nrKeywords);
void die(char *s)
{
	perror(s);
	exit(1);
}

char http_error[] = "HTTP/1.0 400 Bad Request\r\nContent-type: text/html\r\nServer: Test\r\n\r\n";
char http_ok[] = "HTTP/1.0 200 OK\r\nContent-type: text/html\r\nServer: Test\r\nConnection: close\r\n\r\n";
int main(void){
    struct sockaddr_in si_me, si_other;
	int s, i, slen = sizeof(si_other) , recv_len, conn;
	char buf[BUFLEN];
	if ((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
		die("socket");
	memset((char *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(s, (struct sockaddr*)&si_me, sizeof(si_me)) == -1)
		die("bind");
	if (listen(s, 10) == -1)
		die("listen");
	while (1) {
		memset(buf, 0, sizeof(buf));
		printf("Waiting a connection...");
		fflush(stdout);
		conn = accept(s, (struct sockaddr *) &si_other, &slen);
        printf("Chegou request..");
		if (conn < 0)
			die("accept");
		printf("Client connected: %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
		recv_len = read(conn, buf, BUFLEN);
		if (recv_len < 0)
			die("read");
		printf("Data: %s\n" , buf);

		if (strstr(buf, "GET")) {
            char* response = generateResponseSystemInfo();
            if (write(conn, http_ok, strlen(http_ok)) < 0)
				die("write");
			if (write(conn, response, strlen(response)) < 0){
				die("write");
            }
            free(response);
		} else {
			if (write(conn, http_error, strlen(http_error)) < 0)
				die("write");
		}
		close(conn);
	}
	close(s);
    return 0;
}
char* generateResponseSystemInfo(){
    // Keywords
    char* cpuUsageKeyWords[] = {"cpu"};
    char* cpuInfoKewWords[] = {"processor","model name","cpu MHz"};
    char* rtcTime[] = {"rtc_time", "rtc_date"};
    char* memorykeywords[] = {"MemTotal","MemFree"};
    
    char* cpuUsage = generateHtmlCpuUsage("/proc/stat","Capacidade ocupada do processador em %",cpuUsageKeyWords, 1);
    char* systemVersion = generateHtml("/proc/version","Info - Sistema",NULL,0);
    char* cpuInfo = generateHtml("/proc/cpuinfo", "Cpu - Info", cpuInfoKewWords, 3);
    //char* time = generateHtml("/proc/driver/rtc", "Data e hora do sistema", rtcTime, 2);
    char* uptime = generateHtml("/proc/uptime", "Uptime", NULL, 0);
    char* netRoute = generateHtml("/proc/net/route","Net Route",NULL,0);
    char* memory = generateHtml("/proc/meminfo","Info - memoria",memorykeywords,2);
    char* finalTemplate = "<html>\n<head>\n<title>\nTest page\n</title>\n</head>\n\t<body>\n%s\n\t</body>\n</html>";
    unsigned short cpuInfoSize = strlen(cpuInfo);
    //unsigned short rtcTimeSize = strlen(time);
    unsigned short uptimeSize = strlen(uptime);
    unsigned short netRouteSize = strlen(netRoute);
    unsigned short memorySize = strlen(memory);
    unsigned short systemVersionSize = strlen(systemVersion);
    unsigned short cpuUsageSize = strlen(cpuUsage);
    unsigned int total = cpuInfoSize + uptimeSize + netRouteSize
                         + memorySize + systemVersionSize + cpuUsageSize;//+rtcTimeSize
    char* body = calloc(total+1,sizeof(char));
    strcat(body,cpuInfo);
    //strcat(body,time);
    strcat(body,uptime);
    strcat(body,netRoute);
    strcat(body,memory);
    strcat(body,systemVersion);
    strcat(body,cpuUsage);
    char* finalHtml = templateFormatter(body,finalTemplate);
    free(body);
    free(cpuInfo);
    //free(time);
    free(uptime);
    free(netRoute);
    free(cpuUsage);
    return finalHtml;
}
char* generateHtml(char* fileName, char* titleName, char** keyWords, char nrKeywords){
    FILE* fp = fopen(fileName,"r");
    const int fixedArraySize = 12000;
    int bufferSize = 500;
    //setvbuf(fp,NULL,_IOFBF,BUFSIZ);
    char* buffer = (char*) calloc(bufferSize,sizeof(char));
    char* h3 = "\t<h3>%s</h3>\n";
    char* divOne = "\t<div>%s</div>\n";
    char* divTwo = "<div>\n%s</div>\n";
    char* tempHtml = (char*) calloc(fixedArraySize,sizeof(char));
    char* title = templateFormatter(titleName,h3);
    strcat(tempHtml,title);
    if(keyWords){
        while(fgets(buffer,bufferSize,fp)){
            for(char i=0;i<nrKeywords;i++){
                if(strstr(buffer,keyWords[i])){
                    buffer[strlen(buffer)-1]='\0';
                    char* formattedString = templateFormatter(buffer,divOne);
                    strcat(tempHtml,formattedString);
                    free(formattedString);
                }
            }
        }
    }else{
        while(fgets(buffer,bufferSize,fp)){
            buffer[strlen(buffer)-1]='\0';
            char* formattedString = templateFormatter(buffer,divOne);
            strcat(tempHtml,formattedString);
            free(formattedString);
        }
    }
    free(buffer);
    fclose(fp);
    char* finalHtml = templateFormatter(tempHtml,divTwo);
    printf("%s",finalHtml);
    free(tempHtml);
    free(title);
    return finalHtml;
}
char* templateFormatter(char* content, char* template){
    int bufSize = snprintf(NULL, 0, template, content);
    char* buffer = (char*)calloc(bufSize+1,sizeof(char));
    if (buffer == NULL) {
        return NULL;
    }
    sprintf(buffer, template, content);
    buffer[bufSize]='\0';
    return buffer;
}

char* generateHtmlCpuUsage(char* fileName, char* titleName, char** keyWords, char nrKeywords){
    FILE* fp = fopen(fileName,"r");
    const int fixedArraySize = 12000;
    int bufferSize = 500;
    char cpu_string[6];
    int numbers[4];
    char* buffer = (char*) calloc(bufferSize,sizeof(char));
    char* h3 = "\t<h3>%s</h3>\n";
    char* divOne = "\t<div>%s</div>\n";
    char* divTwo = "<div>\n%s</div>\n";
    char* tempHtml = (char*) calloc(fixedArraySize,sizeof(char));
    char* title = templateFormatter(titleName,h3);
    strcat(tempHtml,title);
    while(fgets(buffer,bufferSize,fp)){
        for(char i=0;i<nrKeywords;i++){
            if(strstr(buffer,keyWords[i])){
                buffer[strlen(buffer)-1]='\0';
                int result = sscanf(buffer, "%5s %d %d %d %d", 
                    cpu_string, 
                    &numbers[0], &numbers[1], &numbers[2], &numbers[3]);
                int totalUsage = numbers[0]+numbers[1]+numbers[2]+numbers[3];
                double usage = (numbers[0]+numbers[1]+numbers[2])/(double)totalUsage;
                usage *=100;
                char* usageNumber = (char*)calloc(6,sizeof(char));
                snprintf(usageNumber, 6, "%2f", usage);
                int lineSize = strlen(cpu_string) + 8;
                char* line = (char*) calloc(lineSize,sizeof(char));
                strcat(line,cpu_string),
                strcat(line," ");
                strcat(line,usageNumber);
                strcat(line,"%");
                char* formattedString = templateFormatter(line,divOne);
                strcat(tempHtml,formattedString);
                free(formattedString);
                free(usageNumber);
                free(line);
            }
        }
        }
    free(buffer);
    fclose(fp);
    char* finalHtml = templateFormatter(tempHtml,divTwo);
    printf("%s",finalHtml);
    free(tempHtml);
    free(title);
    return finalHtml;
}