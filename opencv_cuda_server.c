#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <time.h>
#include <signal.h>

//define fixed message size
#define MSG_SIZE 40

//create server/client structure
struct sockaddr_in serv_addr, cli_addr;
socklen_t serv_len, cli_len;

void error(char *msg){
	perror(msg);
	exit(1);
}

void read_msg(int sockfd)
{
    int n;
    char buffer[MSG_SIZE];

    //clear buffer
    bzero(buffer, MSG_SIZE);
	n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&cli_addr, &cli_len);
	if (n < 0) error("Error on receiving message.\n");
    
    //if client socket closed
    if (n == 0)
    {
        printf("Client closed.\n");
        exit(0);    
    }
    else
    {
        printf("Target @ %s\n", buffer);
    }
}


int main(int argc, char *argv[]) {

	int sockfd, newsockfd, n, boolval = 1, pid;
	int vote = 0, max_vote = 0;
	int master_flag = 0;
	char buffer[MSG_SIZE];
	char mymsg[MSG_SIZE];

	//check if the user specify a port number
	if (argc < 2){
		printf("Error: port number not specified.\n");
		exit(1);
	}

	//open a socket using stream - connection based
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd<0) error("Error on opening socket.\n");

	//initialize server address
	serv_len = sizeof(struct sockaddr_in);
	bzero(&serv_addr, serv_len);

	//initialize client address
	cli_len = sizeof(struct sockaddr_in);
	bzero(&serv_addr, serv_len);

	//set parameters for server socket
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[1]));
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	//bind server socket with the address
	if(bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
		error("Error on binding.\n");

    //listen to the socket for connection
    listen(sockfd,0);

    //allow the server to handle multiple simultaneous connections in case using more than one cameras to get coordinates
    while (1)
    {
        //blocks until a client connects
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &cli_len);
        if (newsockfd < 0) error("Error on accepting.\n");

        pid = fork();
        if (pid < 0) error("Error on forking.\n");
        
        //child process
        if (pid == 0) 
        {
            close(sockfd);
            while (1)
            {
                read_msg(newsockfd);
            }
        }
        //parent
        else 
        {
            close(newsockfd);
            //ensure processes that are done are closed
            signal(SIGCHLD, SIG_IGN);
        }
    }

    close(sockfd);
    return 0;
}
