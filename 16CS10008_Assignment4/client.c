#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#include <arpa/inet.h>

#include <signal.h>
#include <unistd.h>

// defining maximum size of command
#define MAXCOMMAND 80
// define maximum size of the string
#define PSEUDO_BUFFER 83
#define MAXBUFFER 100

// success code to be send to the client in case needed
#define VALID_PORT 200
#define DIR_CHANGED 200
#define FILE_SEND_COMPLETE 250
#define CLOSE_CONNECTION_WITH_CLIENT 421
// error code to be send to the client in case needed
#define INVALID_ARGUMENTS 501
#define CANT_CHANGE_DIRECTORY 501
#define INVALID_COMMAND 502
#define NOT_FIRST_COMMAND 503
#define INVALID_PORT 550
#define FILE_CANNOT_SEND 550

int wordParsed(char *buffer, char **token){
  puts(buffer);
  int i=0;
  token[i] = strtok(buffer, " ");
  while(token[i]){
    // printf("while starting ...\n");
    i++;
    // printf("%s\n", token[i]);
    token[i] = strtok(NULL, " ");
  }
  return i;
}
//	for the put command
int sendFile(char *filename, int port){
	int			sockfd ;
	struct 	sockaddr_in	serv_addr;
  long int read_byte, send_byte, recv_byte, writ_byte;
	int i, reply_code, command_token_count;
	char message[PSEUDO_BUFFER];
	char buffer[MAXBUFFER];
	// creating TCP Socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Unable to create socket\n");
		exit(0);
	}
	wait(NULL);
	// Filling the server information
	serv_addr.sin_family		= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port = htons(port);
	wait(NULL);
	// connecting the TCP socket
	printf("CONNECTING TO THE SERVER for data transfer\n");
	if ((connect(sockfd, (struct sockaddr *) &serv_addr,
						sizeof(serv_addr))) < 0) {
		perror("Unable to connect to server\n");
		exit(0);
	}
	printf("CONNECTING TO THE SERVER for data transfer 2 times\n");
	int fd = open(filename, O_RDONLY, 0666);
	if(fd < 0){
		perror("Error creating a file | error creating the file\n");
		exit(EXIT_FAILURE);
	}
	do {
		bzero(buffer, sizeof(buffer));
		read_byte = read(fd, buffer, sizeof(buffer)-1);
		printf("readed bytes are : %ld, [%s] ", read_byte, buffer);
		send_byte = send(sockfd, buffer, sizeof(buffer), 0);
		printf("send bytes are : %ld, [%s] ", send_byte, buffer);
		if(send_byte <= 0){
			perror("sending failed ");
			close(sockfd);
			close(fd);
			return 0;
		}
	} while(read_byte > 0);
	return 1;
}
// for the get command
int recieveFile(int *fd, int port){
	int			sockfd ;
	struct 	sockaddr_in	serv_addr;
  long int read_byte, send_byte, recv_byte, writ_byte;
	int i, reply_code, command_token_count;
	char message[PSEUDO_BUFFER];
	char buffer[MAXBUFFER];
	// creating TCP Socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Unable to create socket\n");
		exit(0);
	}
	// Filling the server information
	serv_addr.sin_family		= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port = htons(50000);
	// connecting the TCP socket
	if ((connect(sockfd, (struct sockaddr *) &serv_addr,
						sizeof(serv_addr))) < 0) {
		perror("Unable to connect to server\n");
		exit(0);
	}
	do {
		recv_byte = send(sockfd, buffer, sizeof(buffer), 0);
		writ_byte = read(*fd, buffer, sizeof(buffer));
		if(writ_byte <= 0){
			perror("writing failed ");
			close(sockfd);
			return 0;
		}
	} while(recv_byte > 0);
	return 1;
}

int main()
{
	int			sockfd ;
	struct 	sockaddr_in	serv_addr;

  long int read_byte, send_byte, recv_byte;

	int i, reply_code, command_token_count, client_file_transfer_port;
	char buffer[MAXCOMMAND];
	char **token = (char **)malloc(20 * sizeof(char *));
	// creating TCP Socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Unable to create socket\n");
		exit(0);
	}

	// Filling the server information
	serv_addr.sin_family		= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port = htons(50000);

	// connecting the TCP socket
	if ((connect(sockfd, (struct sockaddr *) &serv_addr,
						sizeof(serv_addr))) < 0) {
		perror("Unable to connect to server\n");
		exit(0);
	}

	// // getting the first message from the server
	// for(i=0; i < 100; i++) buffer[i] = '\0';
	// read(sockfd, (char *)buffer, sizeof(buffer));
  // printf("[SERVER] : %s\n", buffer);

	// recieving the words from the file that server is reading.
  bzero(buffer, sizeof(buffer));
  printf("[CLIENT] : ");
	scanf("%[^\n]%*c", buffer);
	// sending the first command to the server
	// supposed to be the 'port xxxxxx'
	send(sockfd, buffer, sizeof(buffer), 0);
	// printf("SEnding the first command successfull\n");
	// recieving the code
	recv(sockfd, &reply_code, sizeof(reply_code), 0);
	reply_code = ntohl(reply_code);
	printf("[SERVER] : %d \n", reply_code);
	// if(i == NOT_FIRST_COMMAND || i == INVALID_PORT ||
	// 		i == INVALID_COMMAND || i == INVALID_ARGUMENTS || i == FILE_CANNOT_SEND)
	if(reply_code == NOT_FIRST_COMMAND || reply_code == INVALID_ARGUMENTS ||
		reply_code == INVALID_COMMAND || reply_code == INVALID_PORT)	{
		printf("[SERVER] : error \'%d\' | INVALID FIRST COMMAND\n", NOT_FIRST_COMMAND);
		close(sockfd);
	}
	else	if(reply_code == VALID_PORT){
		command_token_count = wordParsed(buffer, token);
		client_file_transfer_port = atoi(token[1]);
		while(1)	{
			printf("[SERVER] : Enter the command...\n");
			printf("[CLIENT] : ");
			bzero(buffer, sizeof(buffer));
			scanf("%[^\n]%*c", buffer);
			send(sockfd, &buffer, sizeof(buffer), 0);
			command_token_count = wordParsed(buffer, token);
			// recieve the reply code for the command send above
			if(strcmp(token[0], "put") && strcmp(token[0], "get") && command_token_count != 2)
				recv(sockfd, &reply_code, sizeof(reply_code), 0);
			reply_code = ntohl(reply_code);

			if(reply_code == 502)	{
				printf("[SERVER] : \'%d\' | INVALID COMMAND...\n", reply_code);
				continue;
			}

			for(i=0; i<command_token_count; i++)
				printf("%d :: %s\n",i, token[i]);

			// invalid argument check has to be impleented here
			if((strcmp(token[0], "cd") || command_token_count != 2) && reply_code == 501){
				printf("[SERVER] : error \'%d\' || INVLALID ARGUMENTs\n", reply_code);
				continue;
			}
			// check for the command 'quit'
			if(reply_code == 421)	{
				printf("[SERVER] : \'%d\' | CLOSING CONNECTION...\n", reply_code);
				close(sockfd);
				break;
			}
			// check for the command 'cd'
			if(!strcmp(token[0], "cd") && reply_code == CANT_CHANGE_DIRECTORY)	{
				printf("[SERVER] : error \'%d\' | CAN\'T CHANGE THE DIRECTORY\n", reply_code);
				continue;
			}	else	if (!strcmp(token[0], "cd") && reply_code == DIR_CHANGED) {
				printf("[SERVER] : \'%d\' | DIRECTORY HAS BEEN CHANGED SUCCESSFULLY\n", reply_code);
				continue;
			}
			// command left are 'get' , 'put'
			if(!strcmp(token[0], "get")){
				pid_t pid = fork();
				if(pid == 0)	{
					int filefd = open(token[1], O_WRONLY | O_CREAT, 0666);
					if(filefd < 0){
						perror("Error creating a file\n");
						continue;
					}
					recieveFile(&filefd, client_file_transfer_port);
					close(filefd);
				}	else	{
					recv(sockfd, &reply_code, sizeof(reply_code), 0);
					reply_code = ntohl(reply_code);
					if(reply_code == FILE_CANNOT_SEND)	{
						printf("[SERVER] : error \'%d\' | couldn't fetch file\n", FILE_CANNOT_SEND);
						kill(pid, SIGKILL);
						continue;
					}	else	if(reply_code == FILE_SEND_COMPLETE)	{
						printf("[SERVER] : \'%d\' | FILE TRANSFER COMPLETE\n", FILE_SEND_COMPLETE);
						kill(pid, SIGKILL);
						continue;
					}
				}
			}
			if(!strcmp(token[0], "put")){
				pid_t pid = fork();
				if(pid == 0)	{
					sendFile(token[1], client_file_transfer_port);
				}	else	{
					recv(sockfd, &reply_code, sizeof(reply_code), 0);
					reply_code = ntohl(reply_code);
					if(reply_code == FILE_CANNOT_SEND)	{
						printf("[SERVER] : error \'%d\' | couldn't send file\n", FILE_CANNOT_SEND);
						kill(pid, SIGKILL);
						continue;
					}	else	if(reply_code == FILE_SEND_COMPLETE){
						printf("[SERVER] : \'%d\' | FILE TRANSFER COMPLETE\n", FILE_SEND_COMPLETE);
						kill(pid, SIGKILL);
						continue;
					}
				}
			}
			// just for testing purpose implementing sanitisation for pwd

			// if(!strcmp(token[0], "pwd"))	{
			// 	printf("[SERVER] : DIRECTORY HAS BEEN CHANGED SUCCESFULLY\n");
			// 	continue;
			// }
		}
	}
	return 0;
}
