#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "server.h"
#include "log.h"

void write_log(int sock, char* secretcode, struct sockaddr_in *cli_addr, 
				char* clientmessage, int b, int m, int logval, FILE *fp){

	/* Lock mutex, this makes sure that both file writes and prints execute
	fully */
	pthread_mutex_lock(&lock);

	// Time calculations
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    // Write to log for client
    if (cli_addr != NULL){
    	// Print to server output
    	printf("[%d %d %d %d:%d:%d](%d.%d.%d.%d)(soc_id %d) ",
    	// Get time details
	    tm.tm_mday,
	    tm.tm_mon + 1,
	    tm.tm_year + 1900,
	    tm.tm_hour,
	    tm.tm_min,
	    tm.tm_sec,
	    // Convert IP to decimal
		(int)(cli_addr->sin_addr.s_addr&0xFF),
		(int)((cli_addr->sin_addr.s_addr&0xFF00)>>8),
		(int)((cli_addr->sin_addr.s_addr&0xFF0000)>>16),
		(int)((cli_addr->sin_addr.s_addr&0xFF000000)>>24),
		sock);
		// Write to log file
	    fprintf(fp, "[%d %d %d %d:%d:%d](%d.%d.%d.%d)(soc_id %d) ",
	    // Get time details
	    tm.tm_mday,
	    tm.tm_mon + 1,
	    tm.tm_year + 1900,
	    tm.tm_hour,
	    tm.tm_min,
	    tm.tm_sec,
	    // Convert IP to decimal
		(int)(cli_addr->sin_addr.s_addr&0xFF),
		(int)((cli_addr->sin_addr.s_addr&0xFF00)>>8),
		(int)((cli_addr->sin_addr.s_addr&0xFF0000)>>16),
		(int)((cli_addr->sin_addr.s_addr&0xFF000000)>>24),
		sock);
	}
	// Write to log, for server
	else{
		// Print to server output
		printf("[%d %d %d %d:%d:%d](0.0.0.0)(soc_id %d) ",
		// Get time details
	    tm.tm_mday,
	    tm.tm_mon + 1,
	    tm.tm_year + 1900,
	    tm.tm_hour,
	    tm.tm_min,
	    tm.tm_sec,
		sock);
		// Write to log file
	    fprintf(fp, "[%d %d %d %d:%d:%d](0.0.0.0)(soc_id %d) ",
	    // Get time details
	    tm.tm_mday,
	    tm.tm_mon + 1,
	    tm.tm_year + 1900,
	    tm.tm_hour,
	    tm.tm_min,
	    tm.tm_sec,
		sock);
	}

	// Print and write for client connected
    if (logval == CONNECT){
    	printf("client connected\n");
    	fprintf(fp, "client connected\n");
    }
    // Print and write for secret generated
    else if (logval == GENSECRET){
    	printf("server secret = %s\n", secretcode);
    	fprintf(fp, "server secret = %s\n", secretcode);
    }
    // Print and write for client guess
    else if (logval == CLIENTGUESS){
    	printf("client's guess = %s\n", clientmessage);
    	fprintf(fp, "client's guess = %s\n", clientmessage);
    }
    // Print and write for game over
    else if (logval == FAILURE){
    	printf("FAILURE game over\n");
    	fprintf(fp, "FAILURE game over\n");
    }
    // Print and write for client disconnected
    else if (logval == DISCONNECT){
    	printf("client disconnected\n");
    	fprintf(fp, "client disconnected\n");
    }
    // Print and write for response secret generated
    else if (logval == SERVERHINT){
    	printf("server's hint [%d,%d]\n",b,m);
    	fprintf(fp, "server's hint [%d,%d]\n",b,m);
    }
    // Print and write for correct guess
    else if (logval == CLIENTSUCCESS){
    	printf("SUCCESS game over\n");
    	fprintf(fp, "SUCCESS game over\n");
    }
    else if (logval == INVALID){
    	printf("INVALID guess not between [A,F]\n");
    	fprintf(fp, "INVALID guess not between [A,F]\n");
    }
    /* Flush buffer cache. File is not expected to be closed
    so files need to be written to immediately */ 
	fflush(fp);

	// Unlock mutex
	pthread_mutex_unlock(&lock);
}

/**
 * Write to file and stdout total connections and winners
 */
void write_connection_stats(int connections, int totalwins, FILE *fp){
	/* Lock mutex, this makes sure that both file writes and prints execute
	fully */
	pthread_mutex_lock(&lock);
	fprintf(fp, "Total winners: %d Total Connections: %d\n", totalwins, connections);
	printf("Total winners: %d Total Connections: %d\n", totalwins, connections);
	/* Flush buffer cache. File is not expected to be closed
    so files need to be written to immediately */ 
	fflush(fp);
	// Unlock mutex
	pthread_mutex_unlock(&lock);
}

/*
 * Write memory usage statistics to log
 */
void write_memory_usage(FILE *fp){
	// Get /proc usage status
	FILE* status = fopen( "/proc/self/status", "r" );
	if (status == NULL){
		fprintf(stderr, "ERROR, Could not retrieve usage statistics\n");
	}
	char temp;
	// Copy from status to log
	do {
      temp = fgetc(status);
      printf("%c", temp);
      fputc(temp, fp);
   	} while (temp != EOF);
   	/* Flush buffer cache. File is not expected to be closed
    so files need to be written to immediately */ 
	fflush(fp);
} 