/*
 * client.c
 *
 *  Created on: Nov 24, 2015
 *      Author: Bhavin
 */

#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

int sendInt(int sock, int a){
	int ackValue;

	if(send(sock , &a , sizeof(int) , 0) < 0){
		puts("Send Failed");
		return -1;
	}
	while(1){
		if(recv(sock , &ackValue , sizeof(int),0) < 0){
			puts("Send Failed");
			return -1;
		}else{
			if(ackValue == 1){
				return 1;
			}else{
				puts("Send Failed");
				return -1;
			}
		}
	}
}

int getServerFromDirectory(int *ip, int *port){

		//Initialize variables
		int sock, result;
		struct sockaddr_in server;

		//Create socket
	    sock = socket(AF_INET , SOCK_STREAM , 0);
	    if (sock == -1)
	    {
	        puts("Could not create socket");
			return 0;
	    }
	    puts("Socket created");

		//Connect to Directory Register and get ip and port for service
		server.sin_addr.s_addr = inet_addr("127.0.0.1");
	    server.sin_family = AF_INET;
	    server.sin_port = htons( 8000 );

		//Connect to Directory Service
		puts("Trying to connect to Directory Service");
		int connected = -1;
		while(connected == -1){
	    	connected = connect(sock , (struct sockaddr *)&server , sizeof(server));
		}

		//Connected to Directory Service
	    puts("Connected");

		//Let Directory know you are the client
		if(sendInt(sock, 1) < 0){
			puts("Send programID failed");
			close(sock);
			return 0;
		}

		//Get Status of Server Found or Not
		if(recv(sock , &result , sizeof(result),0) < 0){
			puts("Receive Query Result failed");
			close(sock);
			return 0;
		}else{
			if(result == 0){
				puts("Server Not Found");
				close(sock);
				return 0;
			}
		}

		//Send the ACK
		if(sendInt(sock, 1) < 0){
			puts("Send ACK failed");
			close(sock);
			return 0;
		}

		//Get IP
		if(recv(sock , ip , sizeof(int)*4,0) < 0){
			puts("Receive IP failed");
			close(sock);
			return 0;
		}

		//Send the ACK
		if(sendInt(sock, 1) < 0){
			puts("Send ACK failed");
			close(sock);
			return 0;
		}

		//Get Port
		if(recv(sock , port , sizeof(port),0) < 0){
			puts("Receive Port failed");
			close(sock);
			return 0;
		}

		//Successful
		//Close connection to Directory Service
		close(sock);
		return 1;
}

int clientInput(){

	return 1;
}

int main(){

	int sock;
    struct sockaddr_in server;

	//Send Lookup Request
	int ip[4];
	int port;

	if(getServerFromDirectory(&ip[0], &port) < 0){
		printf("Server Not Found");
		return 1;
	}

	//Convert ip to string
	char address[50];
	sprintf(address, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

	//Trim the char array
	printf("The address we have is %s \n",address);

	//Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        puts("Could not create socket");
		return 1;
    }
    puts("Socket created");

	//Now setup Server connection
	server.sin_addr.s_addr = inet_addr(address);
	//server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( port );
    printf("Port it is trying to connect to is %d \n",port);

	//Connect to Remote Server
	puts("Trying to connect to Remote Server");
	int connected = -1;
	while(connected == -1){
    	connected = connect(sock , (struct sockaddr *)&server , sizeof(server));
	}

	//Connected to Remote Server
    puts("Connected");

	while(1){
		if(clientInput() < 0){
			break;
		}
	}

	puts("Connection terminated.");
	close(sock);
	return 0;
}



