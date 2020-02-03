#include <stdio.h>
#include <string.h>
#include <sys/types.h>  
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "socket.h"
#include <signal.h>

void initialiser_signaux(void) 
{
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
	{
		perror ("signal");
	}
}

int main (void)
{
	int socket_serveur;
	socket_serveur = creer_serveur(8080);
	if (socket_serveur == -1) {
		printf("Erreur lors de la création du serveur");
		return -1;
	}

	int socket_client;

	socket_client = accept(socket_serveur, NULL, NULL);

	if(socket_client == -1){
		perror("accept");
		return -1;
	}

	const char * message_bienvenue = "Bonjour, bienvenue sur mon serveur\n Ce serveur a ete cree par les soins de Maxence et Kevin\n Ce n'est que le début mais il devrait vite y avoir des ameliorations\n Voici un passage d'Harry Potter en anglais\n Cela vous permettra de travailler votre anglais\n et aussi vous rappeler quelques souvenirs\n\" if you want to go back, I won’t blame you, \" he [Harry] said.\n\" You can take the Cloak, I won’t need it now. \"\n\" Don’t be stupid, \" said Ron.\n\" We’re coming, \" said Hermione.\n";
	char buff[256];

	write(socket_client, message_bienvenue, strlen(message_bienvenue));
	memset(buff, 0, sizeof(buff));

	while(read(socket_client, buff, sizeof(buff))){
		write(socket_client, buff, sizeof(buff));
		memset(buff, 0, sizeof(buff));
	}

	initialiser_signaux();

	return 0;
}

