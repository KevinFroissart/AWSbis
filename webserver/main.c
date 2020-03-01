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

http_request request;

pid_t fils;

struct stat stats;

void traitement_signal(int sig)
{
	fprintf(stderr, "Signal %d reçu \n", sig);
	waitpid(fils, NULL, WNOHANG);
}

void initialiser_signaux(void) 
{
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

void send_response(FILE *client, int code, const char *reason_phrase, const char *message_body){
	char msg[128];
	send_status(client, code, reason_phrase);
	sprintf(msg, "Connection: close\r\nContent-Length: %lu\r\n\r\n", strlen(message_body));	
	fprintf(client, "%s", msg);
	fprintf(client, "%s", message_body);
}

char *rewrite_target(char *target) {
	if(strcmp(target, "/" ) == 0)
		strcat(target, "/index.html");
	char *ptr;
	ptr = strchr(target, '?');
	if(ptr != NULL)
	    *ptr = '\0';
	return ptr;
}

FILE *check_and_open(const char *target, const char *document_root) {
	char * targeted_document = malloc(sizeof(document_root));
	targeted_document = strdup(document_root);
	targeted_document = realloc(targeted_document, sizeof(targeted_document) + sizeof(target) + 500*sizeof(char));
	strcat(targeted_document, "/site");
	strcat(targeted_document, target);
	return fopen(targeted_document, "r");
}

int get_file_size(int fd) {
	if(fstat(fd, &stats) != -1)
		return stats.st_size;
	return -1;	
}

int copy(FILE *in, FILE *out) {
	int return_value = -1;
	int fd = fileno(in);
	get_file_size(fd);
	char *buffer = malloc(stats.st_size);
	if(buffer){
		fread(buffer, 1, stats.st_size, in);
		return_value = 0;
	}
	fclose(in);
	send_response(out, 200, "OK", buffer);
	return return_value;
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
			//const char * message_bienvenue = "Bonjour, bienvenue sur mon serveur\nCe serveur a ete cree par les soins de Maxence et Kevin\nCe n'est que le début mais il devrait vite y avoir des ameliorations\nVoici un passage d'Harry Potter en anglais\nCela vous permettra de travailler votre anglais\net aussi vous rappeler quelques souvenirs\n\" if you want to go back, I won’t blame you, \" he [Harry] said.\n\" You can take the Cloak, I won’t need it now. \"\n\" Don’t be stupid, \" said Ron.\n\" We’re coming, \" said Hermione.\n\n\n";

			char buff[8192];

			FILE * f1;
			if((f1 = fdopen(socket_client, "a+")) == NULL)
				perror("fdopen");

			fgets_or_exit(buff, sizeof(buff), f1);

			skip_headers(f1);

    		if(parse_http_request(buff, &request) == 0){
    			if(request.method == HTTP_UNSUPPORTED)
    		  		send_response(f1, 405, "Method Not Allowed", "Method Not Allowed\r\n");
    			else
    		  		send_response(f1, 400, "Bad Request", "Bad request\r\n");
    		} 
			else if(parse_http_request(buff, &request) == 1) {
				if(!(request.http_major == 1 && (request.http_minor == 1 || request.http_minor == 0)))
					send_response(f1, 505, "HTTP Version Not Supported", "HTTP Version Not Supported\r\n");
    			else if(strcmp(request.target, "/") == 0)
    		  		send_response(f1, 200, "OK", message_bienvenue);
    			else {
					char cwd[1024];
					getcwd(cwd, sizeof(cwd));
					rewrite_target(request.target);
					FILE * fichier = check_and_open(request.target, cwd);

					if(fichier == NULL) 
						send_response(f1, 404, "Not Found", "Not Found\r\n");
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