#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include "socket.h"
#include "http_parse.h"

#define BUFFER_SIZE 4096

http_request request;

pid_t fils;

struct stat stats;

char mime_type[1024];

void traitement_signal(int sig){
	fprintf(stderr, "Signal %d reçu \n", sig);
	waitpid(fils, NULL, WNOHANG);
}

void initialiser_signaux(void) {
	if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		perror ("signal");

	struct sigaction sa;
	sa.sa_handler = traitement_signal;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	if (sigaction(SIGCHLD, &sa, NULL) == -1)
		perror("sigaction(SIGCHLD)");
}

char * fgets_or_exit(char *buffer, int size, FILE *stream){
	char * res = "";
	if((res = fgets(buffer, size, stream)) == NULL)
		exit(-1);
	return res;
}

void skip_headers(FILE *client){
	char uselessBuffer[8216];
	while(fgets_or_exit(uselessBuffer, sizeof(uselessBuffer), client) != NULL){
		if(strncmp(uselessBuffer, "\r\n", 2) == 0 || strncmp(uselessBuffer, "\n", 1) == 0)
			break;
	}
}

void send_status(FILE *client, int code, const char *reason_phrase){
	char msg[256];
	sprintf(msg, "HTTP/1.1 %d %s\r\n", code, reason_phrase);
	fprintf(client, "%s", msg);
}

void send_response(FILE *client, int code, const char *reason_phrase, const char *message_body, int content_type){
	char msg[128];
	send_status(client, code, reason_phrase);	
	if(content_type){
		char * mime_parsed = strtok(mime_type, ":");
		mime_parsed = strtok(NULL, ":");
		sprintf(msg, "Content-Length: %lu\n\rContent-Type:%s\rConnection: close\r\n\r\n", strlen(message_body), mime_parsed);
	}
	else
		sprintf(msg, "Content-Length: %lu\n\rConnection: close\r\n\r\n", strlen(message_body));
	fprintf(stdout, "%s", msg);
	fprintf(client, "%s", msg);
	fprintf(client, "%s", message_body);
}

char *rewrite_target(char *target) {
	if(strcmp(target, "/" ) == 0)
		strcat(target, "index.html");
	char *ptr;
	ptr = strchr(target, '?');
	if(ptr != NULL)
	    *ptr = '\0';
	return ptr;
}

char * define_mime_type(const char * document){
	char fileCommand[1024];
	sprintf(fileCommand, "file -i %s", document);
	FILE *fp = popen(fileCommand, "r");
	while(fgets(mime_type, sizeof(mime_type), fp) != NULL){}
	pclose(fp);
	return mime_type;
}

FILE *check_and_open(const char *target, const char *document_root) {
	char * targeted_document = strdup(document_root);
	targeted_document = realloc(targeted_document, sizeof(targeted_document) + sizeof(target) + 500*sizeof(char));
	strcat(targeted_document, "/site");
	strcat(targeted_document, target);
	char buffer[BUFFER_SIZE];
	while((fread(buffer, 1, BUFFER_SIZE, in)) > 0){
		fwrmpo*/);
	return fopen(targeted_document, "r");
}

int get_file_size(int fd) {
	if(fstat(fd, &stats) != -1)
		return stats.st_size;
	return -1;	
}

void send_ok(FILE *client){
	char msg[256];
	send_status(client, 200, "OK");
	char * mime_parsed = strtok(mime_type, ":");
	mime_parsed = strtok(NULL, ":");
	sprintf(msg, "Content-Length: %d\n\rContent-Type:%s\rConnection: close\r\n\r\n", (int)stats.st_size, mime_parsed);
	fprintf(client, "%s", msg);
}

void copy(FILE *in, FILE *out) {
	int fd = fileno(in);
	get_file_size(fd);
	send_ok(out);
	char buffer[BUFFER_SIZE];
	while((fread(buffer, 1, BUFFER_SIZE, in)) > 0){
		fwrite(buffer, 1, BUFFER_SIZE, out);
	}
	fclose(in);
}

int main (void)
{
	initialiser_signaux();
	int socket_serveur;

	if((socket_serveur = creer_serveur(8080)) == -1) {
		fprintf(stderr, "Erreur lors de la création du serveur");
		return -1;
	}

	while(1) {

		int socket_client;
		
		if((socket_client = accept(socket_serveur, NULL, NULL)) == -1){
			perror("accept");
			return -1;
		}

		if((fils = fork()) != 0) {
			close(socket_client);

		} else {
			char buff[8192];

			FILE * f1;
			if((f1 = fdopen(socket_client, "a+")) == NULL)
				perror("fdopen");

			fgets_or_exit(buff, sizeof(buff), f1);

			skip_headers(f1);

    		if(parse_http_request(buff, &request) == 0){
    			if(request.method == HTTP_UNSUPPORTED)
    		  		send_response(f1, 405, "Method Not Allowed", "Method Not Allowed\r\n", 0);
    			else
    		  		send_response(f1, 400, "Bad Request", "Bad request\r\n", 0);
    		} 
			else if(parse_http_request(buff, &request) == 1) {
				if(!(request.http_major == 1 && (request.http_minor == 1 || request.http_minor == 0)))
					send_response(f1, 505, "HTTP Version Not Supported", "HTTP Version Not Supported\r\n", 0);
    			else if(strstr(request.target, "/../"))
					send_response(f1, 403, "Forbidden", "Forbidden\r\n", 0);
				else {
					char cwd[8192];
					getcwd(cwd, sizeof(cwd));
					rewrite_target(request.target);
					FILE * fichier = check_and_open(request.target, cwd);

					if(fichier == NULL) 
						send_response(f1, 404, "Not Found", "Not Found\r\n", 0);
					else
						copy(fichier, f1);
				}
    		}
			fclose(f1);			
			exit(1);
		}
	}
	return 0;
}