#include <stdio.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h> 
#include <string.h> 
#include <unistd.h> 
#include <stdlib.h> 
#define MAX_SIZE 2048 
int 
main(int argc, char * argv[]) 
{ 
    FILE *fp; 
    struct hostent *hp; 
    struct sockaddr_in sin; 
    char *host, *port; 
    char buf[MAX_SIZE]; 
    int s; 
    int len; 
    host = "127.0.0.1"; 
    port = "12000";
    /* translate host name into peer's IP address */  
    hp = gethostbyname(host); 
    if (!hp) { 
        fprintf(stderr, "%s: unknown host\n", host);  exit(1); 
    } 
    /* build address data structure */ 
    bzero((char *)&sin, sizeof(sin)); 
    sin.sin_family = AF_INET; 
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);  sin.sin_port = htons(atoi(port)); 
    /* active open */ 
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {  perror("simplex-talk: socket"); 
        exit(1); 
    } 
     if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)  {
        perror("simplex-talk: connect");  close(s); 
        exit(1); 
    } 
    int size; 
    while(fgets(buf, MAX_SIZE, stdin) ) {  
        if(send(s, buf, strlen(buf), 0) < 0) { 
            perror("client: send"); 
        }
        if(buf[strlen(buf)-1]=='\n')
            break;
    } 
    close(s);
 // Done, close the s socket descriptor  close(s); 
}
