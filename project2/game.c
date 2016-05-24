#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>
#include "server.h"
#include "log.h"
#include "game.h"

/*
 * This will handle the game for each client
 */
void *game_handler(void *arguments){
	// Get argument struct
	struct arg_struct *args = arguments;
    // Get the socket descriptor
    int sock = *(int*)args->arg1, count = 0;
    // Get client descriptor
    struct sockaddr_in cli_addr = *(struct sockaddr_in*)args->arg2;
    // Get secret code
    char secretcode[5];
    strncpy(secretcode, (char*)args->arg3, 5);
    // If secretcode is empty, generate a secret code
    // Generate random seed
    srand(time(NULL));
    if (secretcode[0] == '\0'){
    	char randomletter;
    	int i;
		for (i = 0; i < 4; i++){
			randomletter = 'A' + (rand() % 6);
			secretcode[i] = randomletter;
		}
		secretcode[i] = '\0';
    }
    // Get File descriptor
    FILE *fp = args->arg4;
    // Size of string read from client
    int read_size;
    // Buffers for messages
    char *message , client_message[5];
    // Write to log, client connected
   	write_log(sock, NULL, &cli_addr, NULL, NULL, NULL, CONNECT, fp);
	// Write to log, secret code generated
	write_log(sock, secretcode, NULL, NULL, NULL, NULL, GENSECRET, fp);

    // Welcome message to client
    send_welcome(sock);

    // Receive a message from client
    while((read_size = recv(sock , client_message , 4 , 0)) > 0){
    	// Write to log, client guesses
    	write_log(sock, NULL, &cli_addr, client_message, NULL, NULL, CLIENTGUESS, fp);
		// Increment guess count
        count += 1;
        if (count != 10){
		    // End of string marker
			client_message[read_size] = '\0';
			// Send the message back to client
		    if (send_feedback(sock, secretcode, client_message, &count, fp)){
		    	// Edit shared total wins
		    	pthread_mutex_lock(&lock);
				*((int*)args->arg5) += 1;
				pthread_mutex_unlock(&lock);
		    	// If game ended, close socket
		    	write_log(sock, NULL, &cli_addr, NULL, NULL, NULL, CLIENTSUCCESS, fp);
		    	break;
		    }
			// clear the message buffer
			memset(client_message, 0, 4);
		}
		// Close connection after 10 attempts
    	else{
    		// Create message to send after game is over
    		char guess_message[45];
    		sprintf(guess_message, "Ten guesses are up, You lose. Code was: %s\a", secretcode);
    		// Send a alarm char to signal game over
    		write(sock, guess_message , 45);
    		// Write to log, game over
    		write_log(sock, NULL, &cli_addr, NULL, NULL, NULL, FAILURE, fp);
    		break;
    	}
    }
     
    // Get IP address of the client and print when client exits
    if(read_size == 0){
    	// Write to log, client disconnect (Ctrl+C) or server disconnect
    	write_log(sock, NULL, &cli_addr, NULL, NULL, NULL, DISCONNECT, fp);
    }
    else if(read_size == -1){
		fprintf(stderr,"ERROR, failed to read client data, CLOSING CONNECTION\n");
    }

    // Close connection
    close(sock);
    return 0;
}

/*
 * Print the welcome message from the text file
 */
void send_welcome(int sock){
	FILE *msg;
	// Read from welcome.txt
	msg = fopen("welcome.txt", "r");
	if (!msg){
		fprintf(stderr, "ERROR, Could not open message.txt\n");
		return;
	}
	char message[2048];
	// Copy message from welcome.txt into char array
	fread(message, 1, 2048, msg);
	// Send character array
    write(sock , message , strlen(message));
}

/*
 * Takes string and secret key and returnsa string and writes
 * to client
 */
int send_feedback(int sock, char* secretcode, char* client_message, int *count, FILE *fp){
	int i = 0, b = 0, m = 0;
	// Store characters of secret to ignore
	int ignoresecret[] = {1,1,1,1};
	// Store characters of guess to ignore
	int ignoreguess[] = {1,1,1,1};
	// Find correct places in the secret code
	for (i = 0; i < 4; i++){
		if (client_message[i] > 'F' || client_message[i] < 'A'){
			write(sock, "Value is not between A and F, Please enter again", 48);
			// Write invalid move log
			write_log(sock, NULL, NULL, NULL, NULL, NULL, INVALID, fp);
			*count -= 1;
			return 0;
		}
		// Check if guess is correct and in correct position
		if (secretcode[i] == client_message[i]){
			// Add to characters to ignore
			ignoreguess[i] = 0;
			ignoresecret[i] = 0;
			b++;
		}
	}
	// Find incorrect places in the secret code
	for (i = 0; i < 4; i++){
		// Ignore character if already correctly guessed
		if (!ignoreguess[i]){
			continue;
		}
		// Find character if found in secretcode
		int j = 0;
		for (j = 0; j < 4; j++){
			// Again, ignore if already guessed in secret
			if (ignoresecret[j] && (secretcode[j] == client_message[i])){
				ignoresecret[j] = 0;
				m++;
				break;
			}
		}
	}
	// If all correctly guessed, write sucess message to client
	if (b == 4){
		write(sock, "Congratulations, You guessed correctly\a", 39);
		return 1;
	}	
	// Create message out of computed values
	char message[15];
	sprintf(message, "Guess %d: [%d,%d]",*count,b,m);
	// Write to client
	write(sock, message, strlen(message));
	// Write to log, server's hint
	write_log(sock, NULL, NULL, NULL, b, m, SERVERHINT, fp);
    return 0;
}