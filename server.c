#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>

#include <fcntl.h>
#include <unistd.h>

#define MAXBUFFER 100
#define MAXBLOCK 20
#define PORT 50000

int main()	{
	int sockfd, newsockfd, clilen, i;
	struct sockaddr_in cli_addr, serv_addr;
	ssize_t recv_byte, send_byte;

	char *buffer;
	buffer = (char *)malloc(MAXBUFFER * sizeof(char));

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("CANNOT CREATE SOCKET\n");
		exit(EXIT_FAILURE);
	}
	int on = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)	{
		perror("SETSOCKOPT failure\n");
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(PORT);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("UNABLE TO BIND THE LOCAL ADDRESS\n");
		exit(EXIT_FAILURE);
	}

	listen(sockfd, 5);
	printf("WAITING FOR CONNECTION...\n");
	while (1) {
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if(newsockfd < 0)	{
			perror("ACCEPT ERROR\n");
			exit(EXIT_FAILURE);
		}
		// sending the message to the client to input the file name
		// for(i=0; i<MAXBUFFER; i++)	buffer[i] = '\0';
		strcpy(buffer, "Enter the file name : ");
		send(newsockfd, buffer, strlen(buffer) + 1, 0);
		printf("[SERVER] : %s\n", buffer);
		// getting the file name form the client
		for(i=0; i<MAXBUFFER; i++)	buffer[i] = '\0';
		recv_byte = recv(newsockfd, buffer, MAXBUFFER, 0);
		printf("[CLIENT] : \"%s\"\n", buffer);

		// opening file
		int filefd = open(buffer, O_RDONLY, 0666);
		// send the appropriate code back to client
		// printf("response code is in process... \n");
		if (filefd < 0) {
			for(i=0; i<MAXBUFFER; i++)	buffer[i] = '\0';
			strcpy(buffer, "E");
			send_byte = send(newsockfd, buffer, strlen(buffer) + 1, 0);
			printf("[SERVER] : %s\n", buffer);
		}	else	{
			// char *block = (char *)malloc(MAXBLOCK * sizeof(char));
			char block[MAXBLOCK];
			// printf("memory allocated : %ld\n", sizeof(block));
			off_t file_size;
			// craeting stat structure to access size of the file
			struct stat st;
			// getting the file size;
			if (stat(buffer, &st) == 0)
				file_size = st.st_size;
			// if file is found then send the code 'L' and then send the file size
			for(i=0; i<MAXBUFFER; i++)	buffer[i] = '\0';
			strcpy(buffer, "L");
			send(newsockfd, buffer, strlen(buffer) + 1, 0);
			printf("[SERVER] : %s\n", buffer);
			send(newsockfd, &file_size, sizeof(file_size), 0);
			printf("[SERVER] : %ld | size of the file\n", file_size);

			long int byte_to_be_send = sizeof(block);
			printf("sizeofthe block = %ld\n", byte_to_be_send);
			int block_number = 0;
			while(1)	{
				// printf("%d :: ", block_number);
				if(block_number == (file_size/(long int)(sizeof(block))) )
					byte_to_be_send = file_size % ((long int)sizeof(block));
				read(filefd, block, byte_to_be_send);
				// printf("file readed text = %ld\n", strlen(block));
				send_byte = send(newsockfd, block, byte_to_be_send, 0);
				// printf("byte send = %ld | %ld = byte to be send\n",send_byte, byte_to_be_send);
				printf("block : %d | bytes send = %ld\n", block_number+1, send_byte);
				block_number++;
				// if(block_number == (file_size/(long int)(sizeof(block)+1)))
				if(send_byte < (long int)sizeof(block))
					break;
			}
			for(i=0; i<MAXBUFFER; i++)	buffer[i] = '\0';
			recv_byte = recv(newsockfd, buffer, MAXBUFFER, 0);
			printf("[CLIENT] : \"%s\"\n", buffer);
		}
		close(filefd);
		close(newsockfd);
		printf("CLOSING CONNECTION WITH CURRENT CLIENT\nWAITING FOR NEXT CLIENT...\n");
	}
	close(sockfd);
	return 0;
}
