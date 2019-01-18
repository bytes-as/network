
/*    THE CLIENT PROCESS */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// int is_word_continuing(char *buffer, char last_w){
//
//     if(!(buffer[0] == '.' || buffer[0] == ';' || buffer[0] == ':' || buffer[0] == ',' || buffer[0] == '\n' || buffer[0] == ' ' )){
//         if(!(last_w == '.' || last_w == ';' || last_w == ':' || last_w == ',' || last_w == '\n' || last_w == ' ')){
//             return 1;
//         }
//     }
//     return 0;
// }

// int word_count(char *buffer){
//     char temp[93],*pch;
//
//     int count = 0 ;
//     strcpy(temp, buffer);
//
//     pch = strtok(temp, " .,:;\n");
//     while(pch != NULL){
//         count++;
//         pch = strtok(NULL, " .,:;\n");
//     }
//
//     return count;
// }


long int wordCount(char *buf,  int size){
  long int count = 0;
  int i = 0;
  _Bool flag = 0;
  for(i=0; i<size; i++){
    if(!flag){
      if(buf[i] >= 'a' && buf[i] <= 'z')
        flag = 1;
      if(buf[i] >= 'A' && buf[i] <= 'Z')
        flag = 1;
      if(buf[i] >= '0' && buf[i] <= '9')
        flag = 1;
      if(flag)
        count++;
    } else{
      if(buf[i] == '\'')
        continue;
      if(!((buf[i] >= 'a' && buf[i] <= 'z') || (buf[i] >= 'A' && buf[i] <= 'Z') || (buf[i] >= '0' && buf[i] <= '9') || buf[i] == '\''))
        flag = 0;
    }
    // printf("%d == %c :: %ld\n",i, buf[i] , count);
  }
  return count;
}

int getDecimal(char* buf){
  if(buf[0] == '\0')
    return -1;
  int i=0, decimal=0;
  for(i=0; i<7; i++){
    decimal *= 2;
    if(buf[i] == '1')
      decimal += 1;
  }
  return decimal;
}

/*    THE CLIENT PROCESS */
int main(int argc, char const *argv[]) {
	int			sockfd ;
	struct sockaddr_in	serv_addr;

	int i;
	char buf[100];
  char   buf_length[8], buf_message[93];

	/* Opening a socket is exactly similar to the server process */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Unable to create socket\n");
		exit(0);
	}

	/* Recall that we specified INADDR_ANY when we specified the server
	   address in the server. Since the client can run on a different
	   machine, we must specify the IP address of the server.

	   In this program, we assume that the server is running on the
	   same machine as the client. 127.0.0.1 is a special address
	   for "localhost" (this machine)

	/* IF YOUR SERVER RUNS ON SOME OTHER MACHINE, YOU MUST CHANGE
           THE IP ADDRESS SPECIFIED BELOW TO THE IP ADDRESS OF THE
           MACHINE WHERE YOU ARE RUNNING THE SERVER.
    	*/

	serv_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port	= htons(20000);

	/* With the information specified in serv_addr, the connect()
	   system call establishes a connection with the server process.
	*/
	if ((connect(sockfd, (struct sockaddr *) &serv_addr,
						sizeof(serv_addr))) < 0) {
		perror("Unable to connect to server\n");
		exit(0);
	}

	/* After connection, the client can send or receive messages.
	   However, please note that recv() will block when the
	   server is not sending and vice versa. Similarly send() will
	   block when the server is not receiving and vice versa. For
	   non-blocking modes, refer to the online man pages.
	*/
	for(i=0; i < 100; i++) buf[i] = '\0';
	recv(sockfd, buf, 100, 0);
	printf("%s\n", buf);

  // sending file name to the server
  printf("[CLIENT] : ");
  scanf("%s", buf);
	send(sockfd, buf, strlen(buf), 0);

  int decimal;
  long int total_bytes = 0;
  long int recv_byte;
  long int total_word = 0;
  int fd = open("readed.txt", O_CREAT | O_WRONLY , 0644);
  if(fd == -1){
    perror("opening new file :");
    exit(1);
  }
  _Bool flag = 0;
  while(1){
    for(i=0; i<100; i++) buf[i] = '\0';
    // printf("\n");
    recv_byte = recv(sockfd, buf, 100, 0);
    printf("%s", &buf[7]);
    if(recv_byte == 0){
      printf("FILE NOT FOUND\n");
      exit(1);
    }
    // char *stripping;
    // unsigned long ul;
    // stripping = strtok(&buf[7], " ,.;:");
    // while(stripping != NULL){
    //   ul = strtoul(stripping, NULL, 0);
    //   stripping = strtok(NULL, " ,.;:");
    //   if(ul == 0)
    //     total_word++;
    // }
    char temp[97];
    strcpy(temp, &buf[7]);

    buf[99] = '\0';
    // if(buf[7] != '\0')
      total_word += wordCount(temp, recv_byte-7);
    if (flag)
      if ((temp[96]>='a' && temp[96]<='z') || (temp[96]>='A' && temp[96]<='Z') || (temp[96]>='0' && temp[96]<='9') || (temp[96] == '-') || (temp[96]=='\''))
        if ((buf[7]>='a' && buf[7]<='z') || (buf[7]>='A' && buf[7]<='Z') || (buf[7]>='0' && buf[7]<='9') || (buf[7] == '-') || (buf[7]=='\''))
          total_word--;
    flag = 1;
    // if(flag)
    //   if(is_word_continuing(&buf[7], temp[96]))
    //     total_word--;
    // flag = 1;
    // total_word += word_count(temp);


    // printf("\n%ld\n%s :: word ount = %ld\n",recv_byte, buf, total_word);
    // printf("\n%c\n", buf[98]);
    decimal = getDecimal(buf);
    // if(decimal == 0){
    //   printf("File Not Found\n");
    //   break;
    // }

    if(decimal == 0){
      if ((buf[98]>='a' && buf[98]<='z') || (buf[98]>='A' && buf[98]<='Z') || (buf[98]>='0' && buf[98]<='9'))
        ++total_word;
      printf("[CLIENT] : The file transfer is successful. Size of the file = %ld, no. of words = %ld\n", total_bytes-1, total_word);
      // strcpy(buf, "file transfer is complete\n");
      // send(sockfd, "thisdfosdknfkldsanfl", strlen(buf), 0);
      break;
    }
    if(decimal == -1){
      printf("File is empty\n");
      break;
    }
    total_bytes += decimal;
    // printf("%s", &buf[7]);
    write(fd, &buf[7], 92);
  }

  if(close(sockfd) == -1)
    perror("Closing the socket created : ");

  return 0;
}
