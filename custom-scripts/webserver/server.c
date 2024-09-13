#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>
 
#define BUFLEN	1024
#define PORT	12345

char* readFromTextFile(char* fileName);

void die(char *s)
{
	perror(s);
	exit(1);
}

int main(void){
	char http_error[] = "HTTP/1.0 400 Bad Request\r\nContent-type: text/html\r\nServer: Test\r\n\r\n";
	char http_ok[] = "HTTP/1.0 200 OK\r\nContent-type: text/html\r\nServer: Test\r\nConnection: close\r\n\r\n";
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
		if (conn < 0)
			die("accept");
		printf("Client connected: %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
		recv_len = read(conn, buf, BUFLEN);
		if (recv_len < 0)
			die("read");
		printf("Data: %s\n" , buf);
		if (strstr(buf, "GET /index.html")) {
            FILE* fp = fopen("index.html","r");
            char* response = readFromTextFile("index.html");
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
char* readFromTextFile(char* fileName){
    char* buffer;
    long length;
    FILE* pfile = fopen(fileName,"r");
    if(pfile==NULL){
        return NULL;
    }
    setvbuf(pfile,NULL,_IOFBF,BUFSIZ);
    fseek (pfile, 0, SEEK_END);
    length = ftell(pfile);
    fseek (pfile, 0, SEEK_SET);
    buffer = (char*) calloc(length+1,sizeof(char));
    if(!buffer){
        return NULL;
    }
    fread(buffer, 1, length, pfile);
    fclose (pfile);
    return buffer;
}