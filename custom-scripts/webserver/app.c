#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>

#define BUFLEN	1024	//Max length of buffer
#define PORT	12345	//The port on which to listen for incoming data
#define MAX_PATH 1024
#define STR_BUFF 50000
char* templateFormatter(char* content, char* template);
char* generateHtml(char* fileName, char* titleName, char** keyWords, char nrKeywords);
void generateResponseSystemInfo();
char* generateHtmlCpuUsage(char* fileName, char* titleName, char** keyWords, char nrKeywords);
char* generateHtmlListRunningProcesses();
int is_numeric(const char *str);

void generateResponseSystemInfo(){
    // Keywords
    char* cpuUsageKeyWords[] = {"cpu"};
    char* cpuInfoKewWords[] = {"processor","model name","cpu MHz"};
    char* rtcTime[] = {"rtc_time", "rtc_date"};
    char* memorykeywords[] = {"MemTotal","MemFree"};

    char* runningProcesses = generateHtmlListRunningProcesses();
    char* cpuUsage = generateHtmlCpuUsage("/proc/stat","Capacidade ocupada do processador em %",cpuUsageKeyWords, 1);
    char* systemVersion = generateHtml("/proc/version","Info - Sistema",NULL,0);
    char* cpuInfo = generateHtml("/proc/cpuinfo", "Cpu - Info", cpuInfoKewWords, 3);
    //char* time = generateHtml("/proc/driver/rtc", "Data e hora do sistema", rtcTime, 2);
    char* uptime = generateHtml("/proc/uptime", "Uptime", NULL, 0);
    char* netRoute = generateHtml("/proc/net/route","Net Route",NULL,0);
    char* memory = generateHtml("/proc/meminfo","Info - memoria",memorykeywords,2);
    char* diskUnits = generateHtml("/proc/partitions", "Lista de unidades de disco, com capacidade total de cada unidade",NULL,0);
    char* finalTemplate = "<html>\n<head>\n<title>\nTest page\n</title>\n<meta http-equiv=\"refresh\" content=\"1; http://127.0.0.1:12345/index.html\"></head>\n\t<body>\n%s\n\t</body>\n</html>";
    int cpuInfoSize = strlen(cpuInfo);
    //unsigned short rtcTimeSize = strlen(time);
    int uptimeSize = strlen(uptime);
    int netRouteSize = strlen(netRoute);
    int memorySize = strlen(memory);
    int systemVersionSize = strlen(systemVersion);
    int cpuUsageSize = strlen(cpuUsage);
    int listRunningProcessesSize = strlen(runningProcesses);
    int diskUnitsSize = strlen(diskUnits);
    unsigned int total = cpuInfoSize + uptimeSize + netRouteSize
                         + memorySize + systemVersionSize + cpuUsageSize
                         + listRunningProcessesSize + diskUnitsSize;//+rtcTimeSize
    char* body = calloc(total+1,sizeof(char));
    strcat(body,cpuInfo);
    //strcat(body,time);
    strcat(body,uptime);
    strcat(body,netRoute);
    strcat(body,memory);
    strcat(body,systemVersion);
    strcat(body,cpuUsage);
    strcat(body,runningProcesses);
    strcat(body,diskUnits);
    char* finalHtml = templateFormatter(body,finalTemplate);
    FILE* filePointer = fopen("index.html","w");
    free(body);
    fwrite(finalHtml, sizeof(char), strlen(finalHtml), filePointer);
    free(cpuInfo);
    free(uptime);
    free(netRoute);
    free(cpuUsage);
    free(runningProcesses);
    free(finalHtml);
    fclose(filePointer);
}
char* generateHtml(char* fileName, char* titleName, char** keyWords, char nrKeywords){
    FILE* fp = fopen(fileName,"r");
    char* buffer = (char*) calloc(BUFLEN,sizeof(char));
    char* h3 = "\t<h3>%s</h3>\n";
    char* divOne = "\t<div>%s</div>\n";
    char* divTwo = "<div>\n%s</div>\n";
    char* tempHtml = (char*) calloc(STR_BUFF,sizeof(char));
    char* title = templateFormatter(titleName,h3);
    strcat(tempHtml,title);
    if(keyWords){
        while(fgets(buffer,BUFLEN,fp)){
            for(char i=0;i<nrKeywords;i++){
                if(strstr(buffer,keyWords[i])){
                    char* formattedString = templateFormatter(buffer,divOne);
                    strcat(tempHtml,formattedString);
                    free(formattedString);
                }
            }
        }
    }else{
        while(fgets(buffer,BUFLEN,fp)){
            buffer[strlen(buffer)-1]='\0';
            char* formattedString = templateFormatter(buffer,divOne);
            strcat(tempHtml,formattedString);
            free(formattedString);
        }
    }
    free(buffer);
    fclose(fp);
    char* finalHtml = templateFormatter(tempHtml,divTwo);
    //printf("%s",finalHtml);
    free(tempHtml);
    free(title);
    return finalHtml;
}
char* templateFormatter(char* content, char* template){
    unsigned int bufSize = (unsigned int)snprintf(NULL, 0, template, content);
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
    int bufferSize = 500;
    char cpu_string[6];
    int* numbers = (int*) calloc(7,sizeof(int));
    char* buffer = (char*) calloc(bufferSize,sizeof(char));
    char* h3 = "\t<h3>%s</h3>\n";
    char* divOne = "\t<div>%s</div>\n";
    char* divTwo = "<div>\n%s</div>\n";
    char* tempHtml = (char*) calloc(STR_BUFF,sizeof(char));
    char* title = templateFormatter(titleName,h3);
    strcat(tempHtml,title);
    while(fgets(buffer,bufferSize,fp)){
        for(char i=0;i<nrKeywords;i++){
            if(strstr(buffer,keyWords[i])){
                int result = sscanf(buffer, "%5s %d %d %d %d %d %d %d", 
                    cpu_string, 
                    &numbers[0], &numbers[1], &numbers[2], &numbers[3],
                    &numbers[4], &numbers[5], &numbers[6]);
                long totalUsage = 0;
                long busyUsage = 0;
                for(unsigned char i=0;i<7;i++){
                    totalUsage += numbers[i];
                    busyUsage += i!=3?numbers[i]:0; 
                }
                double busyRate = busyUsage/(double)totalUsage;
                busyRate *=100;
                char* usageNumber = (char*)calloc(6,sizeof(char));
                snprintf(usageNumber, 6, "%2f", busyRate);
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
    free(numbers);
    free(buffer);
    fclose(fp);
    char* finalHtml = templateFormatter(tempHtml,divTwo);
    //printf("%s",finalHtml);
    free(tempHtml);
    free(title);
    return finalHtml;
}
int is_numeric(const char *str) {
    while (*str) {
        if (!isdigit(*str))
            return 0;
        str++;
    }
    return 1;
}
char* generateHtmlListRunningProcesses() {
    DIR* proc_dir;
    FILE* fp;
    struct dirent* entry;
    char path[MAX_PATH];
    char line[MAX_PATH];
    char name[MAX_PATH];
    int bufferSize = 500;
    char* buffer = (char*) calloc(bufferSize,sizeof(char));
    char* h3 = "\t<h3>%s</h3>\n";
    char* divOne = "\t<div>%s</div>\n";
    char* divTwo = "<div>\n%s</div>\n";
    char* tempHtml = (char*) calloc(STR_BUFF,sizeof(char));
    char* title = templateFormatter("Running processes",h3);
    strcat(tempHtml,title);
    proc_dir = opendir("/proc");
    if (proc_dir == NULL) {
        perror("Unable to open /proc");
        return NULL;
    }
    while ((entry = readdir(proc_dir)) != NULL) {
        if (is_numeric(entry->d_name)) {
            snprintf(path, sizeof(path), "/proc/%s/status", entry->d_name);
            fp = fopen(path, "r");
            if (fp != NULL) {
                if (fgets(line, sizeof(line), fp) != NULL) {
                    sscanf(line, "Name:\t%s", name);
                    //printf("%s\t%s\n", entry->d_name, name);
                    int tempBufferSize = strlen(entry->d_name) + strlen(name) + 2;
                    char* tempBuffer = calloc(tempBufferSize,sizeof(char));
                    strcat(tempBuffer,entry->d_name);
                    strcat(tempBuffer,"\t");
                    strcat(tempBuffer,name);
                    char* formattedString = templateFormatter(tempBuffer    ,divOne);
                    strcat(tempHtml,formattedString);
                    free(tempBuffer);
                }
                fclose(fp);
            }
        }
    }

    char* finalHtml = templateFormatter(tempHtml,divTwo);
    closedir(proc_dir);
    free(tempHtml);
    free(title);
    return finalHtml;
}
int main(){
    while(1){
        printf("Gera novo index.html\n");
        generateResponseSystemInfo();
        sleep(1);
    }

    return 0;
}