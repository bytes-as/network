/*
			NETWORK PROGRAMMING WITH SOCKETS

In this program we illustrate the use of Berkeley sockets for interprocess
communication across the network. We show the communication between a server
process and a client process.


*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* The following three files must be included for network programming */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Code to convert the long int to the binary character string
void getBinaryString(long int dist, char buf[]){
  long int bin = 0;
  int count = 1, i;
  while(dist){
  bin += count*(dist%2);
  count *= 10;
  // printf("%d\n", count);
  dist /= 2;
  // printf("%ld :: %ld\n",bin , dist);
  }
	// printf("the binary conversion would be : \n%ld\n", bin);
	// char *buf;
  // buf = (char *)malloc(8 * sizeof(char));
  for(i=0; i<7; i++){
    if(bin%10 == 1)
      buf[6-i] = '1';
    else
      buf[6-i] = '0';
    bin /= 10;
  }
  buf[7] = '\0';
  // printf("the binary string would be : %s\n", buf);
  // return buf;
}

			/* THE SERVER PROCESS */
int main(int argc, char const *argv[]) {
  int    sockfd, newsockfd;
  int    clilen;
  struct sockaddr_in cli_addr, serv_addr;

  long int read_byte, send_byte, recv_byte;

  int    i;
  char   buf[100];
  char   buf_length[8], buf_message[93];

  /* The following system call opens a socket. The first parameter
	   indicates the family of the protocol to be followed. For internet
	   protocols we use AF_INET. For TCP sockets the second parameter
	   is SOCK_STREAM. The third parameter is set to 0 for user
	   applications.
	*/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Cannot create socket\n");
		exit(0);
	}

  /* The structure "sockaddr_in" is defined in <netinet/in.h> for the
     internet family of protocols. This has three main fields. The
     field "sin_family" specifies the family and is therefore AF_INET
     for the internet family. The field "sin_addr" specifies the
     internet address of the server. This field is set to INADDR_ANY
     for machines having a single IP address. The field "sin_port"
     specifies the port number of the server.
  */
  serv_addr.sin_family		= AF_INET;
  serv_addr.sin_addr.s_addr	= INADDR_ANY;
  serv_addr.sin_port		= htons(20000);

  /* With the information provided in serv_addr, we associate the server
     with its port using the bind() system call.
  */
  if (bind(sockfd, (struct sockaddr *) &serv_addr,
    sizeof(serv_addr)) < 0) {
  	perror("Unable to bind local address\n");
  	exit(0);
  }

  listen(sockfd, 5); /* This specifies that up to 5 concurrent client
  		      requests will be queued up while the system is
  		      executing the "accept" system call below.
  		   */
    /* In this program we are illustrating an iterative server -- one
     which handles client connections one by one.i.e., no concurrency.
     The accept() system call returns a new socket descriptor
     which is used for communication with the server. After the
     communication is over, the process comes back to wait again on
     the original socket descriptor.
  */
  while (1) {
    /* The accept() system call accepts a client connection.
       It blocks the server until a client request comes.

       The accept() system call fills up the client's details
       in a struct sockaddr which is passed as a parameter.
       The length of the structure is noted in clilen. Note
       that the new socket descriptor returned by the accept()
       system call is stored in "newsockfd".
    */
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
      perror("Accept Error\n");
      exit(0);
    }

    /*          Testing the connection
     We initialize the buffer, copy the message to it,
      and send the message to the client.
    */
    strcpy(buf,"[SERVER] : Enter the file path");
    printf("%s\n", buf);
    if(send(newsockfd, buf, strlen(buf) + 1, 0) == -1)
      perror("sending the \"Enter the file path\" : ");

      /* We now receive a message from the client. For this example
    		   we make an assumption that the entire message sent from the
    		   client will come together. In general, this need not be true
  		   for TCP sockets (unlike UDPi sockets), and this program may not
  		   always work (for this example, the chance is very low as the
  		   message is very short. But in general, there has to be some
  	   	   mechanism for the receiving side to know when the entire message
  		  is received. Look up the return value of recv() to see how you
  		  can do this.
  		*/
      for(i=0; i<100; i++)  buf[i] = '\0';
      for(i=0; i<8; i++)    buf_length[i] = '\0';
      for(i=0; i<93; i++)   buf_message[i] = '\0';
      recv(newsockfd, buf_message,100,0);
      // recieving the file naem
      printf("[CLIENT] : %s\n", buf_message);
      int fd = open(buf_message, O_RDONLY | O_EXCL);
  		if(fd == -1){
        printf("buffer contains : %s\n", buf_message);
  			perror("opening the file in read-only mode");
  			exit(1);
  		}
      printf("[SERVER] : Reading file...\n");
      while (1) {
        for(i=0; i<8; i++)    buf_length[i] = '\0';
        for(i=0; i<93; i++)   buf_message[i] = '\0';
        read_byte = read(fd, buf_message, 92);
        getBinaryString(read_byte, buf_length);
        // Just testing the buffer messages
        printf("\n[SERVER] : %s", strcat(buf_length, buf_message));
        if(send(newsockfd,  buf_length, read_byte+8, 0) == -1)
          perror("sending text from file : ");
        // All these print messages are for the time delay if not included the server colsese the socket first and sends a empty string which has 0 bytes in it which would generate the unwanted results in the output.
        printf("\n");
printf("\n");printf("\n");printf("\n");
        printf("\n");
        printf("\n");
        printf("\n");
        printf("\n"); // this printf is not making any sense but while loop last iteration is not occuring
printf("\n");printf("\n");printf("\n");
printf("\n");
printf("\n");printf("\n");printf("\n");
printf("\n");
printf("\n");
printf("\n");
printf("\n"); // this printf is not making any sense but while loop last iteration is not occuring
printf("\n");printf("\n");printf("\n");
        // without any printf statement So I added a dummy printf statement.
        // printf("\n::%s\n", buf_length);
        if(read_byte == 0){
          for(i=0; i<100; i++)  buf[i] = '\0';
          recv(newsockfd, buf_message,100,0);
          printf("[CLIENT] : %s\n", buf);
          break;
        }
      }


      if(close(newsockfd) == -1)
  			perror("Closing the new-socket created\n");
  	}
  return 0;
}
