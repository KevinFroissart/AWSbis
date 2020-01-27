#include "socket.h"

struct sockaddr_in saddr ;
saddr.sin_family = AF_INET ; /* Socket ipv4 */
saddr.sin_port = htons (8080); /* Port d ’é coute */
saddr.sin_addr.s_addr = INADDR_ANY; /* é coute sur toutes les interfaces */

int creer_serveur(int port){

	s_addr.sin_port = htons(port);

	int socket_serveur ;
	socket_serveur = socket(AF_INET, SOCK_STREAM, 0);
	
	if(socket_serveur == -1){ 
		perror("socket_serveur");
		return -1;
	}

	if(bind(socket_serveur, (struct sockaddr *)&saddr, sizeof(saddr)) == -1){
		perror("bind socker_serveur");
		return -1;
	}

	if(listen(socket_serveur, 10) == -1){
		perror("Listen socket_serveur");
		return -1;
	}

	int socket_client;
	socket_client = accept(socket_serveur, NULL, NULL);
	
	if(socket_client == -1){
		perror("accept");
		return -1;
	}
	
	return socket_serveur;
}