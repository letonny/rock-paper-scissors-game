#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <iostream>

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10	 // how many pending connections queue will hold

#define MAXDATASIZE 300

using namespace std;

char getComputerMove();

int getResults(char player, char computer);

void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on
    int numbytes;
    char buf2[MAXDATASIZE];
    char buf3[MAXDATASIZE];
    char status[MAXDATASIZE];
    char statusOk[] = "Status: 200 OK";
    char statusError[] = "Status: 400 ERROR";
    char statusRedirect[] = "Status: 303 REDIRECT";
    char sendComputer[MAXDATASIZE];
    char move[MAXDATASIZE];
    string cScore;
    string pScore;
    char charCScore[MAXDATASIZE];
    char charPScore[MAXDATASIZE];
    bool end = false;
    bool endGame = false;
    int compScore = 0;
    int playerScore = 0;
    


	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address 

	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr,
                        &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        
        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);
        printf("server: got connection from %s\n", s);
        
        
        if (!fork()) { // this is the child process
            //close(sockfd); // child doesn't need the listener
            while(!end)
            {
                if ((numbytes = recv(new_fd, buf2, MAXDATASIZE -1, 0)) == -1) {
                    perror("recv");
                    exit(1);
                }
                buf2[numbytes] = '\0';
                
                //HELLO command
                if(strcmp(buf2, "HELLO") == 0)
                {
                    strcpy(status, statusOk);
                    strcpy(buf3, "conversation has been initiated");
                }
                
                //CLOSE command
                else if(strcmp(buf2, "CLOSE") == 0)
                {
                    strcpy(status, statusOk);
                    strcpy(buf3, "connection has ended");
                    end = true;
                }
                
                //CMDS command
                else if(strcmp(buf2, "CMDS") == 0)
                {
                    strcpy(status, statusOk);
                    strcpy(buf3, "\nCMDS - displays all commands\n");
                    strcat(buf3, "HELLO - initiates conversation\n");
                    strcat(buf3, "CLOSE - closes connection\n");
                    strcat(buf3, "PLAY - starts a game of rock, paper, scissors\n");
                    strcat(buf3, "MOVE <choice> - When in game, enter R, S, or P to make your move\n");
                    strcat(buf3, "SCORE - displays current score\n");
                }
                
                //Misplaced MOVE command
                else if((buf2[0] == 'M') && (buf2[1] == 'O') && (buf2[2] == 'V') && (buf2[3] == 'E'))
                {
                    strcpy(status, statusRedirect);
                    strcpy(buf3, "You can only use this command within a game");
                }
                
                //SCORE command
                else if(strcmp(buf2, "SCORE") == 0)
                {
                    cout << "COMPUTER SCORE: " << compScore << endl;
                    cout << "PLAYER SCORE: " << playerScore << endl;
                    strcpy(status, statusOk);
                    cScore = to_string(compScore);
                    pScore = to_string(playerScore);
                    strcpy(charCScore, cScore.c_str());
                    strcpy(charPScore, pScore.c_str());
                    strcpy(buf3, "\nSCORE: \nComputer: ");
                    strcat(buf3, charCScore);
                    strcat(buf3, "\nPlayer: ");
                    strcat(buf3, charPScore);
                }
                
                //PLAY command
                else if(strcmp(buf2, "PLAY") == 0)
                {
                    endGame = false;
                    cout << "Initiating game...\n";
                    strcpy(status, statusOk);
                    if (send(new_fd, status, MAXDATASIZE-1, 0) == -1)
                        perror("send");
                    strcpy(buf3, "ACK: PLAY");
                    if (send(new_fd, buf3, MAXDATASIZE-1, 0) == -1)
                        perror("send");
                    while(!endGame)
                    {
                        cout << "Start of game" << endl << buf2 << endl;
                        //strcpy(buf2, "");

                        if ((numbytes = recv(new_fd, buf2, MAXDATASIZE -1, 0)) == -1) {
                                perror("recv");
                                exit(1);
                        }
                        

                        cout << "player:" << buf2 << endl;
                        
                        if(strcmp(buf2, "endGame") == 0)
                        {
                            endGame = true;
                        }
                         
                        
                        else
                        {
                            if(strcmp(buf2, "MOVE R") == 0)
                            {
                                strcpy(move, "r");
                            }
                            else if(strcmp(buf2, "MOVE P") == 0)
                            {
                                strcpy(move, "p");
                            }
                            else
                            {
                                strcpy(move, "s");
                            }
                            // Get the computer's move
                            char computer = getComputerMove();
                            strcpy(sendComputer, "");
                            sendComputer[0] = computer;
                            
                            cout << "Computer: " << computer << endl;

                            // Determine the result of the game
                            
                            int result = getResults(move[0], computer);
                            
                            if(result == -1)
                            {
                                compScore++;
                                cout << "COMPUTER SCORE: " << compScore << endl;
                            }
                            if(result == 1)
                            {
                                playerScore++;
                                cout << "PLAYER SCORE: " << playerScore << endl;
                            }
                            
                            snprintf(buf3, sizeof(buf3), "Result: %d", result);
                            
                            if (send(new_fd, sendComputer, MAXDATASIZE-1, 0) == -1)
                                perror("send");
                            
                            if (send(new_fd, buf3, MAXDATASIZE-1, 0) == -1)
                                perror("send");
                        }
                    }
                }
                
                
                else
                {
                    strcpy(status, statusError);
                    strcpy(buf3, "Unkown command");
                    
                }

                
                printf("Server sent '%s'\n", buf3);
                
                if (send(new_fd, status, MAXDATASIZE-1, 0) == -1)
                    perror("send");
                
                if (send(new_fd, buf3, MAXDATASIZE-1, 0) == -1)
                    perror("send");
                   
            }
            
            
            close(new_fd);
            exit(0);
        }
    }

	return 0;
}

char getComputerMove()
{
    int randNum;
    char p = 'p';
    char s = 's';
    char r = 'r';
    char move;

    // generating random number between 0 - 2
    srand(time(NULL));
    randNum = rand() % 3;
  
    // returning move based on the random number generated
    if (randNum == 0) {
        move = p;
    }
    else if (randNum == 1) {
        move = s;
    }
    else {
        move = r;
    }
    return move;
}

int getResults(char player, char computer)
{
    if (player == computer)
    {
        return 0;
    }

    if (player == 's' && computer == 'p')
    {
        return 1;
    }

    if (player == 's' && computer == 'r')
    {
        return -1;
    }

    if (player == 'p' && computer == 'r')
    {
        return 1;
    }

    if (player == 'p' && computer == 's')
    {
        return -1;
    }

    if (player == 'r' && computer == 'p')
    {
        return -1;
    }
    
    if (player == 'r' && computer == 's')
    {
        return 1;
    }
      
    return 0;
}
