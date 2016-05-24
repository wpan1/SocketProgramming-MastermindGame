#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 


int main(int argc, char**argv)
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    // Error if input is not correct
    if (argc < 3) 
    {
        fprintf(stderr,"ERROR, usage %s hostname port\n", argv[0]);
        exit(0);
    }
    portno = atoi(argv[2]);

    
    /* Translate host name into peer's IP address ;
     * This is name translation service by the operating system 
     */
    server = gethostbyname(argv[1]);
    // Error if host could not be found
    if (server == NULL) 
    {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    // Building data structures for socket
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
            (char *)&serv_addr.sin_addr.s_addr,
            server->h_length);
    serv_addr.sin_port = htons(portno);

    /* Create TCP socket -- active open 
    * Preliminary steps: Setup: creation of active open socket
    */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // Error if could not open socket
    if (sockfd < 0) 
    {
        perror("ERROR, opening socket");
        exit(0);
    }
    // Error if could not connect to server
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
    {
        perror("ERROR, connecting");
        exit(0);
    }

    char message[256] , server_reply[2048];
    int reply;
    /* Keep communicating with server until server sends
    stop alarm */
    while(1)
    {
        // Reset string
        bzero((char *) &server_reply, sizeof(server_reply));
        //Receive a reply from the server
        if(read(sockfd , server_reply , 2048 , 0) <= 0)
        {
            // Error if could not read
            printf("ERROR, Receiving message. No reply from server");
            break;
        }

        // Handle game over states, if alarm is last character
        if (server_reply[strlen(server_reply)-1] == '\a'){
            printf("%s\n", server_reply);
            break;
        }

        // Print reply if game is still ongoing
        printf("%s\n", server_reply);

        // Get user input
        printf("Enter guess : ");
        // Although spec says 4 characters are always entered, 
        // I always mess up and type 3/5 chars
        while (scanf("%s" , message) && strlen(message) != 4){
            printf("Enter 4 characters please\n");
            printf("Enter guess : ");
        }
         
        //Send guess to server
        if(send(sockfd , message , strlen(message) , 0) <= 0)
        {
            // Error if could not send message
            printf("ERROR, Send failed. Could not contact server");
            break;
        }
    }
    // Close connection
    close(sockfd);
    return 0;
}