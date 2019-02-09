#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <fcntl.h>

#define MAXBUFFER 100
#define MAXBLOCK 20
#define PORT 50000

int main()	{
	int sockfd;
	struct sockaddr_in serv_addr;
	ssize_t send_byte, recv_byte;

	int i;
	char *buffer = (char *)malloc(MAXBUFFER * sizeof(char));
	char *filename = (char *)malloc(50 * sizeof(char));
	bzero(filename, sizeof(filename));
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)	{
		perror("UNABLE TO CREATE A SOCKET");
		exit(EXIT_FAILURE);
	}

	serv_addr.sin_family = AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port = htons(PORT);

	if((connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0)	{
		perror("UNABLE TO CONNECT TO THE SERVER");
		exit(EXIT_FAILURE);
	}
	// recieving the message from the server asking file name
	for(i=0; i<MAXBUFFER; i++)	buffer[i] = '\0';
	recv(sockfd, buffer, MAXBUFFER, 0);
	printf("[SERVER] : %s\n", buffer);
	// client is prompted to enter the file name
	for(i=0; i<MAXBUFFER; i++)	buffer[i] = '\0';
	printf("[CLIENT] : ");
	scanf("%[^\n]%*c", buffer);
	strcpy(filename, buffer);
	// send the file name
	send(sockfd, buffer, strlen(buffer)+1, 0);
	// recieving the response code from the server
	for(i=0; i<MAXBUFFER; i++)	buffer[i] = '\0';
	recv(sockfd, buffer, MAXBUFFER, 0);
	printf("[SERVER] : %s\n", buffer);
	if(!strcmp(buffer, "L"))	{
		// char *block = (char 	*)malloc(MAXBLOCK * sizeof(char));
		char block[MAXBLOCK];
		printf("memory allocated : %ld\n", sizeof(block));
		// creating a new file to store the data to be recieved from server
		int filefd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0666);
		if (filefd < 0)	{
			perror("Error creating file\n");
			exit(EXIT_FAILURE);
		}
		// getting file size from the server;
		off_t file_size;
		recv(sockfd, &file_size, sizeof(file_size), MSG_WAITALL);
		printf("[SERVER] : %ld | size of the file\n", file_size);
		// maintining the size of recieved data
		long int byte_recieved = 0;
		// maintaining the number of blocks recieved
		int block_number = 0;
		// expected byte
		long int expected_byte = sizeof(block);
		// loop to recieve a block of the file
		while (1) {
			if(block_number == (file_size/(long int)sizeof(block)))
				expected_byte = file_size % sizeof(block);
			recv_byte = recv(sockfd, block, expected_byte, MSG_WAITALL);
			// printf("block number = %d | recieved byte = %ld | %ld = expected byte\n",block_number, recv_byte, expected_byte);
			printf("block : %d | bytes send = %ld\n", block_number+1, recv_byte);
			write(filefd, block, expected_byte);
			byte_recieved += recv_byte;
			block_number++;
			if(recv_byte < (long int)sizeof(block))
				break;
		}
		for(i=0; i<MAXBUFFER; i++)	buffer[i] = '\0';
		strcpy(buffer, "The file transfer is successfull");
		send(sockfd, buffer, strlen(buffer)+1, 0);
		printf("[CLIENT] : %s.\nTotal number of blocks recieved = %d, Last block size = %ld.\n", buffer, block_number, recv_byte);
	}
	close(sockfd);
	return 0;
}
