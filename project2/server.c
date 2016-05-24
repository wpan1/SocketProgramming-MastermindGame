/* A simple server in the internet domain using TCP
The port number is passed as an argument 


 To compile: gcc server.c -o server -lsocket -lnsl
 			(-l links required on csse Unix machines)	
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include  <setjmp.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include "server.h"
#include "log.h"
#include "game.h"

// Storing data used for signal handler
FILE *fp; // File descriptor for log.txt writing
int connections = 0; // Total number of connections
int totalwins = 0; // Total number of winners
int sockfd; // TCP socket for server

/**
 * Main function for hosting server
 */
int main(int argc, char **argv){
	// Initialize mutex
	if (pthread_mutex_init(&lock, NULL) != 0){
		fprintf(stderr,"ERROR, could not initialize mutex\n");
		exit(1);
    }
	// Signal handler
	if (signal(SIGINT, sig_handler) == SIG_ERR){
		fprintf(stderr,"ERROR, could not initialize signal hander\n");
		exit(1);
	}

	int newsockfd, portno, clilen;
	char secretcode[5];
	// Clear secretcode
	bzero((char *) &secretcode, 5);
	struct sockaddr_in serv_addr, cli_addr;

	// Initialise the log file
    fp = fopen("log.txt", "w+");
    if (fp == NULL){
		fprintf(stderr,"ERROR, could not open/create log file\n");
		exit(1);
    }
    // Error if no port is entered
	if (argc < 2){
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	// Get port number to use
	portno = atoi(argv[1]);
	// Use secret code from input if provided
	if (argc == 3){
		if (strlen(argv[2]) != 4){
			fprintf(stderr, "ERROR, secretcode not length 4\n");
			exit(1);
		}
		strncpy(secretcode, argv[2], 4);
	}

	// Create TCP socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0){
		fprintf(stderr,"ERROR, cannot open socket\n");
		exit(1);
	}
	// Clear server_addr
	bzero((char *) &serv_addr, sizeof(serv_addr));
	/* Create address we're going to listen on (given port number)
	 - converted to network byte order & any IP address for 
	 this machine */
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);  // store in machine-neutral format

	// Bind address to the socket
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
		fprintf(stderr,"ERROR, could not bind address to socket\n");
		exit(1);
	}
	
	/* Listen on socket - means we're ready to accept connections - 
	 incoming connection requests will be queued */
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);
	pthread_t thread_id;

	// Print start status, debugging purposes only
	printf("Sever Started\n");

	/* Accept a connection - block until a connection is ready to
	 be accepted. Get back a new file descriptor to communicate on. */
	while ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen))){
		/* Create a new thread to handle client */
		struct arg_struct args;
		args.arg1 = (void*)&newsockfd;
		args.arg2 = (void*)&cli_addr;
		args.arg3 = (void*)&secretcode;
		args.arg4 = fp;
		args.arg5 = (void*)&totalwins;
		// Increment number of connections
		connections += 1;

		// Send error if thread cannot be created
		if(pthread_create(&thread_id , NULL ,  game_handler , (void*)&args) < 0){
			fprintf(stderr,"ERROR, cannot create thread\n");
			exit(1);
        }
	}
	// Send error if client connection error
	if (newsockfd < 0){
		fprintf(stderr,"ERROR, could not accept client\n");
		exit(1);
	}

	/* Should not reach this position since accept() will always block
	the process from continuing */
	return 0; 
}

/**
 * Handle ctrl-C exit from server  
 */
void sig_handler(int signo)
{
  if (signo == SIGINT || signo == SIGTERM){
  	// Write winning and connection statistics
  	write_connection_stats(connections, totalwins, fp);
  	// Write memory usage for process
  	write_memory_usage(fp);
  	// Close file
	fclose(fp);
	// Close socket
	close(sockfd);
  	exit(0);
  }
}
