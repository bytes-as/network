#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

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

int recieveFile(int *fd, int port){
  int    sockfd, newsockfd;
  int    clilen;
  struct sockaddr_in cli_addr, serv_addr;
  long int read_byte, send_byte, recv_byte, writ_byte;

  int i, error_code, success_code;
  char *buffer = (char *)malloc(MAXBUFFER * sizeof(char));
  // creating a new socket
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)  {
    perror("cannot create socket\n");
    return -1;
    exit(0);
  }

  serv_addr.sin_family          = AF_INET;
  serv_addr.sin_addr.s_addr     = INADDR_ANY;
  serv_addr.sin_port            = htons(port);

  // set the details of the server address
  if(bind(sockfd, (const struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    perror("Unable to bind the data transfer address :: ");
    return -1;
  }

  listen(sockfd, 1);

  clilen = sizeof(cli_addr);
  newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
  if(newsockfd < 0) {
    perror("error in accepting connection ");
    return -1;
    exit(1);
  }
  // start sending file in newsockfd
  do {
    bzero(buffer, sizeof(buffer));
    recv_byte = recv(newsockfd, buffer, sizeof(buffer), 0);
    printf("started recieving... :: %ld :: ", recv_byte);
    printf("%s\n", buffer);
    writ_byte = write(*fd, buffer, sizeof(buffer));
    if(recv_byte <= 0){
      close(newsockfd);
      close(sockfd);
      return 0;
    }
    wait(NULL);
  } while(recv_byte > 0);
  //finally)
  close(newsockfd);
  close(sockfd);
  return 1;
}

// file descripter can be used instead of FILE
int sendFile(int *fd, int port) {
  int    sockfd, newsockfd;
  int    clilen;
  struct sockaddr_in cli_addr, serv_addr;
  long int read_byte, send_byte, recv_byte;

  int i, error_code, success_code;
  char *buffer = (char *)malloc(MAXBUFFER * sizeof(char));
  char *message = (char *)malloc(PSEUDO_BUFFER * sizeof(char));
  // creating a new socket
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)  {
    perror("cannot create socket\n");
    return -1;
    exit(0);
  }

  serv_addr.sin_family          = AF_INET;
  serv_addr.sin_addr.s_addr     = INADDR_ANY;
  serv_addr.sin_port            = htons(port);

  // set the details of the server address
  if(bind(sockfd, (const struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    perror("Unable to bind the data transfer address");
    return -1;
  }

  listen(sockfd, 1);
  clilen = sizeof(cli_addr);
  newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
  if(newsockfd < 0) {
    perror("error in accepting connection ");
    return -1;
    exit(1);
  }
  // start sending file in newsockfd
  do {
    read_byte = read(*fd, buffer, sizeof(buffer));
    send_byte = send(newsockfd, buffer, sizeof(buffer), 0);
    if(send_byte < 0){
      close(newsockfd);
      close(sockfd);
      return 0;
    }
  } while(read_byte > 0);
  //finally
  close(newsockfd);
  close(sockfd);
  return 1;
}

int main(int argc, char const *argv[]) {
  int    sockfd, newsockfd;
  int    clilen;
  struct sockaddr_in cli_addr, serv_addr;
  long int read_byte, send_byte, recv_byte;

  int  i, command_token_count, error_code, success_code;;
  char *buffer = (char *)malloc(MAXCOMMAND * sizeof(char));

  // creating a TCP-Socket
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)  {
    perror("cannot create socket\n");
    exit(0);
  }

  // set the details of the server address
  serv_addr.sin_family          = AF_INET;
  serv_addr.sin_addr.s_addr     = INADDR_ANY;
  serv_addr.sin_port            = htons(50000);

  // binding the socket and the server address
  if(bind(sockfd, (const struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      perror("Unable to bind the local address");
      exit(0);
    }
  // setting up the port to be a listening port
  listen(sockfd, 5);
  while (1) {
    // accept() call accept a connection
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if(newsockfd < 0) {
      perror("Accept error");
      exit(0);
    }
    // after creating the connection first command have to
    // be the one to determine the port of the client that
    // is port Y
    recv(newsockfd, buffer, MAXCOMMAND, 0);
    printf("[CLIENT] : %s\n", buffer);
    // puts(buffer);
    char **token = (char **)malloc(20 * sizeof(char *));
    command_token_count = wordParsed(buffer, token);

    // for(i=0; i<command_token_count; i++)
    //   printf("%s\n", token[i]);
    // printf("first token is : %s\n", token[0]);

    // just fot testing purpose
    if(!strcmp(token[0], "quit")){
      close(newsockfd);
      int success_code = htonl(CLOSE_CONNECTION_WITH_CLIENT);
      send(newsockfd, &success_code, sizeof(success_code), 0);
      printf("[SERVER] : %d | Closing the connection with the client\n", CLOSE_CONNECTION_WITH_CLIENT);
      exit(1);
      break;
    }
    // just for testing purpose ^^^^
    long int client_file_transfer_port;
    // printf("waiting for the first command\n");
    if(strcmp(token[0], "port"))  {
        // printf("%d :: Command doesn't contain two arguments or the first one is not \'port\'\n", command_token_count);
        error_code = htonl(NOT_FIRST_COMMAND);
        write(newsockfd, (const int *) &error_code, sizeof(error_code));
        // send(newsockfd, &error_code, sizeof(error_code), 0);
        printf("[SERVER] : error \'%d\' | NOT THE FIRST COMMAND\n", NOT_FIRST_COMMAND);
        close(newsockfd);
        continue;
    } else  {
      if(command_token_count != 2)  {
        // printf("%d :: Command doesn't contain two arguments or the first one is not \'port\'\n", command_token_count);
        error_code = htonl(INVALID_ARGUMENTS);
        write(newsockfd, (const int *) &error_code, sizeof(error_code));
        printf("[SERVER] : error \'%d\' | INVALID ARGUMENTS\n", INVALID_ARGUMENTS);
        close(newsockfd);
        continue;
      }
    }
    client_file_transfer_port = strtol(token[1], NULL, 0);
    if(client_file_transfer_port <= 1024 ||
        client_file_transfer_port >= 65535)  {
      // printf("the port number is : %ld | is not valid\n", client_file_transfer_port);
      error_code = htonl(INVALID_PORT);
      // write(newsockfd, (const int *)error_code, sizeof(error_code));
      send(newsockfd, &error_code, sizeof(error_code), 0);
      printf("[SERVER] : error \'%d\' | INVALID PORT\n", INVALID_PORT);
      close(newsockfd);
      continue;
    }
    success_code = htonl(VALID_PORT);
    // write(newsockfd, (const int *)success_code, sizeof(success_code));
    send(newsockfd, &success_code, sizeof(success_code), 0);
    printf("[SERVER] : \'%d\' | PORT NUMBER RECIEVED SUCCESFULLY\n", VALID_PORT);
    while (1) {
      // printf("entering while\n");
      printf("[SERVER] : Enter the command\n");
      bzero(buffer, MAXCOMMAND);
      recv(newsockfd, buffer, MAXCOMMAND, 0);
      printf("[CLIENT] : %s\n", buffer);
      wait(NULL);
      command_token_count = wordParsed(buffer, token);
      // printf("The nmber of rguments in the command recieved : %d\n", command_token_count);
      if(!strcmp(token[0], "quit")) {
        if(command_token_count != 1)  {
          error_code = ntohl(INVALID_ARGUMENTS);
          send(newsockfd, &error_code, sizeof(error_code), 0);
          printf("[SERVER] : \'%d\'\n", INVALID_ARGUMENTS);
          continue;
        }
        int exit_code = ntohl(CLOSE_CONNECTION_WITH_CLIENT);
        send(newsockfd, &exit_code, sizeof(exit_code), 0);
        printf("[SERVER] : \'%d\' | CLOSE CONNECTION\n", CLOSE_CONNECTION_WITH_CLIENT);
        break;
      }
      if(!strcmp(token[0], "put"))  {
        if(command_token_count != 2){
          error_code = htonl(INVALID_ARGUMENTS);
          send(newsockfd, &error_code, sizeof(error_code), 0);
          printf("[SERVER] : error \'%d\' | Number of arguments aren't matching\n", INVALID_ARGUMENTS);
          continue;
        }
        int filefd = open(token[1], O_WRONLY | O_CREAT, 0666);
        if(filefd < 0)  {
          error_code = htonl(FILE_CANNOT_SEND);
          send(newsockfd, &error_code, sizeof(error_code), 0);
          printf("[SERVER] : error \'%d\' |", error_code);
          perror(" ");
          continue;
        }
        // if there is no error, start the transmission process
        int complete = 0;
        if(fork() == 0) {// newsockfd is to be replaced witht the new socket to be designed here
          // while(until EOF reached){send the chunk}
          // function ```sendFile``` is supposed to be the one to send file to the client
          complete = recieveFile(&filefd, client_file_transfer_port);  // function call
          // at the end of the child process
          close(filefd);  //close the file
          // complete = 1;
        } else  {
          wait(NULL);
          if(complete)  {
            success_code = htonl(FILE_SEND_COMPLETE);
            send(newsockfd, &success_code, sizeof(success_code), 0);
            printf("[SERVER] : \'%d\' | FILE SEND COMPLETE\n", FILE_SEND_COMPLETE);
            continue;
          } else  {
            error_code = htonl(FILE_CANNOT_SEND);
            send(newsockfd, &error_code, sizeof(error_code), 0);
            printf("[SERVER] : error \'%d\' | FILE SENDING FAILED\n", FILE_CANNOT_SEND);
            continue;
          }
        }
      }
      if(!strcmp(token[0], "get"))  {
        if(command_token_count != 2){
          error_code = htonl(INVALID_ARGUMENTS);
          send(newsockfd, &error_code, sizeof(error_code), 0);
          printf("[SERVER] : error \'%d\' | Number of arguments aren't matching\n", INVALID_ARGUMENTS);
          continue;
        }
        // checking if requested file is present or not
        int filefd = open(token[1], O_RDONLY, 0666);
        if(filefd < 0)  {
          error_code = htonl(FILE_CANNOT_SEND);
          send(newsockfd, &error_code, sizeof(error_code), 0);
          printf("[SERVER] : error \'%d\' |", error_code);
          perror(" ");
          continue;
        }
        // if there is no error, start the transmission process

        // int p[2];
        // if(pipe(p) < 0) {
        //   error_code = htonl(FILE_CANNOT_SEND);
        //   send(newsockfd, &error_code, sizeof(error_code), 0);
        //   perror("[SERVER] : Error Creating Pipes | ");
        //   continue;
        // }

        int complete = 0;

        if(fork() == 0) {// newsockfd is to be replaced witht the new socket to be designed here
          // while(until EOF reached){send the chunk}
          // function ```sendFile``` is supposed to be the one to send file to the client
          complete = sendFile(&filefd, client_file_transfer_port);  // function call
          // at the end of the child process
          close(filefd);  //close the file
          // complete = 1;
        } else  {

          // if(sendFile(&filefd) < 0) {
          //   error_code = htonl(FILE_CANNOT_SEND);
          //   send(newsockfd, &error_code, sizeof(error_code), 0);
          //   printf("[SERVER] : \'%d\' | FILE SENDING FAILED\n", FILE_CANNOT_SEND);
          //   continue;
          // } else  {
          //   success_code = htonl(FILE_SEND_COMPLETE);
          //   send(newsockfd, &success_code, sizeof(success_code), 0);
          //   printf("[SERVER] : \'%d\' | FILE SENT Successfully\n", FILE_SEND_COMPLETE);
          //   continue;
          // }
          if(complete)  {
            success_code = htonl(FILE_SEND_COMPLETE);
            send(newsockfd, &success_code, sizeof(success_code), 0);
            printf("[SERVER] : \'%d\' | FILE SEND COMPLETE\n", FILE_SEND_COMPLETE);
            continue;
          } else  {
            error_code = htonl(FILE_CANNOT_SEND);
            send(newsockfd, &error_code, sizeof(error_code), 0);
            printf("[SERVER] : error \'%d\' | FILE SENDING FAILED\n", FILE_CANNOT_SEND);
            continue;
          }
        }
      }
      if(!strcmp(token[0], "cd")) {
        if(command_token_count != 2){
          error_code = htonl(INVALID_ARGUMENTS);
          send(newsockfd, &error_code, sizeof(error_code), 0);
          printf("[SERVER] : error \'%d\' | Number of arguments aren't matching\n", INVALID_ARGUMENTS);
          continue;
        }
        if(chdir(token[1]) < 0) {
          perror("Error changing directory");
          error_code = htonl(CANT_CHANGE_DIRECTORY);
          send(newsockfd, &error_code, sizeof(error_code), 0);
          printf("[SERVER] : error \'%d\' | can't change directory\n", CANT_CHANGE_DIRECTORY);
          continue;
        }
        success_code = htonl(DIR_CHANGED);
        send(newsockfd, &success_code, sizeof(success_code), 0);
        printf("[SERVER] : \'%d\' | Directory has changed successfully\n", DIR_CHANGED);
        continue;
      }
      // check for cd command
      if(!strcmp(token[0], "pwd")){
        if(command_token_count != 1)  {
          error_code = htonl(INVALID_ARGUMENTS);
          send(newsockfd, &error_code, sizeof(error_code), 0);
          printf("[SERVER] : error \'%d\' | Number of arguments aren't matching\n", INVALID_ARGUMENTS);
          continue;
        }
        if(fork() == 0)
          execvp(token[0], token);
        wait(NULL);
        success_code = htonl(0);
        send(newsockfd, &success_code, sizeof(success_code), 0);
        printf("[SERVER] : pwd ^^^^\n");
        continue;
      }
      // if no valid command is given as input then send a error code
      error_code = htonl(INVALID_COMMAND);
      send(newsockfd, &error_code, sizeof(error_code), 0);
      printf("[SERVER]: error \'%d\' | INVALID COMMAND\n", INVALID_COMMAND);
      wait(NULL);
    }
    close(newsockfd);
  }
  return 0;
}
