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
			//const char * message_bienvenue = "Bonjour, bienvenue sur mon serveur\nCe serveur a ete cree par les soins de Maxence et Kevin\nCe n'est que le début mais il devrait vite y avoir des ameliorations\nVoici un passage d'Harry Potter en anglais\nCela vous permettra de travailler votre anglais\net aussi vous rappeler quelques souvenirs\n\" if you want to go back, I won’t blame you, \" he [Harry] said.\n\" You can take the Cloak, I won’t need it now. \"\n\" Don’t be stupid, \" said Ron.\n\" We’re coming, \" said Hermione.\n";

			char buff[8192];

			FILE * f1;
			if((f1 = fdopen(socket_client, "a+")) == NULL)
				perror("fdopen");

			fgets(buff, sizeof(buff), f1);

			if(strncmp(buff, "GET / HTTP/1.1\r\n", 18) == 0){
				char * msg = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Length: 17\r\n\r\n";
				fprintf(stdout, msg);
				fprintf(f1, msg);
			} else if(strncmp(buff, "GET /inexistant HTTP/1.1", 24) == 0){
				fprintf(f1, "q");
				fprintf(stdout, "q");
			} else {
				fprintf(f1,"HTTP/1.1 400 Bad Request\r\nConnection: close\r\nContent-Length: 17\r\n\r\n");
				fprintf(stdout,"HTTP/1.1 400 Bad Request\r\nConnection: close\r\nContent-Length: 17\r\n\r\n");
			}

			fprintf(f1, "<AWSBis>: %s", message_bienvenue);
			while(fgets(buff, sizeof(buff), f1) != NULL){
				fprintf(stdout, buff);
			}
			fclose(f1);
			exit(1);
		}
	}
	return 0;
}