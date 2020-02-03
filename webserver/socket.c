#include "socket.h"
#include <sys/types.h>  
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>


int creer_serveur(int port){

	struct sockaddr_in saddr;
	saddr.sin_port = htons(port);
	saddr.sin_family = AF_INET ; /* Socket ipv4 */
	saddr.sin_addr.s_addr = INADDR_ANY; /* écoute sur toutes les interfaces */

	int socket_serveur ;
	socket_serveur = socket(AF_INET, SOCK_STREAM, 0);


	int optval = 1;
	
	if(setsockopt(socket_serveur, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1)
	perror("Can not set SO_REUSEADDR option");
	
	if(socket_serveur == -1){ 
		perror("socket_serveur");
		return -1;
	}

	if(bind(socket_serveur, (struct sockaddr *) &saddr, sizeof(saddr)) == -1){
		perror("bind socker_serveur");
		return -1;
	}

	if(listen(socket_serveur, 10) == -1){
		perror("Listen socket_serveur");
		return -1;
	}

	return socket_serveur;
}