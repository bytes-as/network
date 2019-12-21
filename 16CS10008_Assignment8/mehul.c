#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<linux/ip.h>/*foripv4header*/
#include<linux/udp.h>/*forupdheader*/
#include<linux/icmp.h>/*foricmpheader*/
#include<unistd.h>
#include<string.h>
#include<signal.h>
#include<sys/wait.h> //Provides declarations for ip header
#include <netdb.h> // gethostbyname

#define source_ip "127.0.0.1"
#define SRC_PORT 54321
#define DEST_PORT 32164
#define BUFF_SIZE 4096

/*
    Generic checksum calculation function
*/
unsigned short csum(unsigned short *ptr,int nbytes)
{
    register long sum;
    unsigned short oddbyte;
    register short answer;

    sum=0;
    while(nbytes>1) {
        sum+=*ptr++;
        nbytes-=2;
    }
    if(nbytes==1) {
        oddbyte=0;
        *((u_char*)&oddbyte)=*(u_char*)ptr;
        sum+=oddbyte;
    }

    sum = (sum>>16)+(sum & 0xffff);
    sum = sum + (sum>>16);
    answer=(short)~sum;

    return(answer);
}



//packet to represent the packet
char message[] = "This is a 52 bytes string for testing of traceroute.";
//IP header
struct iphdr iph;
//UDP header
struct udphdr udph;
// ICMP header



struct sockaddr_in daddr, saddr;

int read_from(int S2, char* ip){

    struct sockaddr_in raddr;
    int raddr_len = sizeof(raddr);

    char buffer[BUFF_SIZE];
    memset(buffer,'\0', BUFF_SIZE);
    int bufferlen = recvfrom(S2, buffer, BUFF_SIZE, 0, (struct sockaddr*) &raddr, &raddr_len);

    if (bufferlen <= 0) return -1;
    struct iphdr *iph = (struct iphdr*) buffer;
    struct icmphdr *icmph = (struct icmphdr*)(buffer + sizeof(struct iphdr));
    buffer[bufferlen] = 0;
    // printf("RAW socket: ");
    // printf("hl: %d, version: %d, ttl: %d, protocol: %d", iph->ihl, iph->version, iph->ttl, iph->protocol);
    // printf(", src: %s", inet_ntoa( * ((struct in_addr * ) & iph->saddr)));
    // printf(", dst: %s", inet_ntoa( * ((struct in_addr * ) & iph->daddr)));
    // printf("\nRAW socket: \ttype: %d", icmph->type);
    // printf("\nRAW socket: \tfrom: %s:%d", inet_ntoa(raddr.sin_addr), ntohs(raddr.sin_port));
    // printf("\nRaw Socket: \tUDP payload: %s", buffer + sizeof(struct iphdr) + sizeof(struct udphdr));
    // printf("\n");
    if(iph->protocol != 1) return -1;
    strcpy(ip, inet_ntoa( * ((struct in_addr * ) & iph->saddr)));
    return icmph->type;
}

void create_packet(char* packet, int* len, int ttl){

    *len = sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(message);

    //Fill in the IP Header
    iph.ihl = 5;  // no idea
    iph.version = 4;
    iph.tos = 0; // no idea
    iph.tot_len = *len;
    iph.id = htonl (SRC_PORT);  //Id of this packet
    iph.frag_off = 0;  // no idea
    iph.ttl = ttl;
    iph.protocol = IPPROTO_UDP;
    iph.check = 0;
    iph.saddr = INADDR_ANY; //the source ip address
    iph.daddr = daddr.sin_addr.s_addr;

    //Ip checksum
    iph.check = csum ((unsigned short *) message, iph.tot_len);

    //UDP header
    udph.source = htons (SRC_PORT);
    udph.dest = htons (DEST_PORT);
    udph.len = htons(8 + strlen(message));  //tcp header size
    udph.check = 0;


    memset (packet, '\0', BUFF_SIZE);
    memcpy(packet, (char*) &iph, sizeof (struct iphdr));
    memcpy(packet + sizeof(struct iphdr), (char*) &udph, sizeof(struct udphdr));
    memcpy(packet + sizeof(struct iphdr) + sizeof(struct udphdr), message, strlen(message));

    udph.check = csum( (unsigned short*) packet , *len);
}

char* DNS(char* address){
	struct hostent *tmp = 0;
	struct in_addr **addr_list;
	tmp = gethostbyname(address);
	if (!tmp ){
	    perror("gethostbyname failed!! ");
	    return NULL;
	}
	strcpy(address, inet_ntoa( (struct in_addr) *((struct in_addr *) tmp->h_addr_list[0])));
	return address;
}

int main (int argc, char** argv)
{
    //Create a raw socket of type IPPROTO
    int S1 = socket (AF_INET, SOCK_RAW, IPPROTO_RAW);
    int S2 = socket (AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(S1 < 0 || S2 < 0){
        //socket creation failed, may be because of non-root privileges
        perror("Failed to create raw socket");
        exit(__LINE__);
    }
    int enable = 1;
    if (setsockopt(S1, IPPROTO_IP, IP_HDRINCL, &enable, sizeof(int)) < 0){
        perror("setsockopt failed");
        exit(__LINE__);
    }
    printf("traceroute for %s", argv[1]);
    printf(" (%s), %ld byte packets\n", DNS(argv[1]), strlen(message));

    daddr.sin_family = AF_INET;
    daddr.sin_port = htons(DEST_PORT);
    daddr.sin_addr.s_addr = inet_addr(argv[1]);

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(SRC_PORT);
    saddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(S2, (const struct sockaddr *)&saddr, sizeof(saddr)) < 0 ) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Hop_Count(TTL_Value) \t IP address \t Response_Time\n");
    for(int ttl = 1;;ttl++){
        int psize;
        char packet[BUFF_SIZE];
        create_packet(packet, &psize, ttl);
        int done = 0;
        while(done<3){
            if (sendto (S1, packet, psize , 0, (struct sockaddr *) &daddr, sizeof (daddr)) < 0){
                perror("sendto failed");
                done++;
            }
            else{
                // printf ("Packet Send. Length : %d \n" , psize);
                fd_set rfds;
                struct timeval timeout;
                timeout.tv_sec = 1;
                while(done < 3)
                {
                    FD_ZERO(&rfds);
                    FD_SET(S2, &rfds);

                    int r = select(S2+1, &rfds, NULL, NULL, &timeout);
                    // printf("select exit  ttl: %d\n",ttl);
                    if(r < 0){
                        perror("Select Failed\n");
                    }
                    if(!FD_ISSET(S2, &rfds)) {//came out by timeout
                        timeout.tv_sec = 1;
                        done++;
                    }
                    else if(FD_ISSET(S2, &rfds)){ //came out when received a message
                        char ip[16];
                        memset(ip,'\0', 16);
                        int type = read_from(S2, ip);
                        printf("%d \t\t\t %s \t %f ms\n", ttl, ip, 1000 - timeout.tv_usec*1.0/1000);
                        if(type == 3){
                            return 0;
                        }
                        if(type == 11){
                        	done = 4;
                        	break;
                        }
                    }
                }

            }
        }
        if (done == 3){
        	printf("%d \t\t\t * \t\t * \n", ttl);
        }
    }
    close(S1);
    close(S2);
    return 0;
}
