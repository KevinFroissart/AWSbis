#include <stdio.h>
#include <string.h>
#include <sys/types.h>  
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "socket.h"
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "http_parse.h"

http_request request;

pid_t fils;

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
	char msg[124];
	sprintf(msg, "HTTP/1.1 %d %s\r\n", code, reason_phrase);
	fprintf(client, msg);
}

void send_response(FILE *client, int code, const char *reason_phrase, const char *message_body){
	char msg[512];
	send_status(client, code, reason_phrase);
	sprintf(msg, "Connection: close\r\nContent-Length: %lu\r\n\r\n", strlen(message_body));	
	fprintf(client, msg);
	fprintf(client, message_body);
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
			const char * message_bienvenue = "Bonjour, bienvenue sur mon serveur\nCe serveur a ete cree par les soins de Maxence et Kevin\nCe n'est que le début mais il devrait vite y avoir des ameliorations\nVoici un passage d'Harry Potter en anglais\nCela vous permettra de travailler votre anglais\net aussi vous rappeler quelques souvenirs\n\" if you want to go back, I won’t blame you, \" he [Harry] said.\n\" You can take the Cloak, I won’t need it now. \"\n\" Don’t be stupid, \" said Ron.\n\" We’re coming, \" said Hermione.\n\n\n";

			char buff[8192];

			FILE * f1;
			if((f1 = fdopen(socket_client, "a+")) == NULL)
				perror("fdopen");

			fgets_or_exit(buff, sizeof(buff), f1);

			int bad_request = 0;

			if(parse_http_request(buff, &request) == 0)
				bad_request = 1;
			skip_headers(f1);

			if(bad_request)
				send_response(f1, 400, "Bad Request", "Bad request\r\n");
			else if(request.method == HTTP_UNSUPPORTED)
				send_response(f1, 405, "Method Not Allowed", "Method Not Allowed\r\n");
			else if(strcmp(request.target, "/" ) == 0)
				send_response(f1, 200, "OK", message_bienvenue);
			else
				send_response(f1, 404, "Not Found", "Not Found\r\n");
			fclose(f1);			
			exit(1);
		}
	}
	return 0;
}