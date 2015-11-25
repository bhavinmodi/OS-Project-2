/*
 * server.c
 *
 *  Created on: Nov 24, 2015
 *      Author: Bhavin
 */

#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread

#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>

//the thread function
int deRegistered=0;

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

int readInt(int sock, int a){
	int statusOfRead, ackValue=1, statusOfAck;
	while(1){
		statusOfRead = recv(sock , &a , sizeof(int),0);
		if(statusOfRead < 0){
			puts("Receive Failed");
			return -1;
		}else{
			break;
		}
	}
	statusOfAck = send(sock , &ackValue , sizeof(int) , 0);
	if(statusOfAck < 0){
		puts("Receive Failed");
		return -1;
	}
	return 1;
}

int deregisterWithDirService()
{
	/// Register the server with the directory service
	int sock;
	struct sockaddr_in server;

	//Create socket
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1)
	{
		puts("Could not create socket");
		return -1;
	}
	puts("Socket created");

	//below should contain the ip address of the Directory Service
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons( 8000 );

	//Connect to Server
	puts("Trying to connect\n");
	int connected = -1;
	while(connected == -1){
		//Connect to remote server
		connected = connect(sock , (struct sockaddr *)&server , sizeof(server));
	}

	//Connected to server
	puts("Connected \n");

	//we first send '1' to the Dir Service which shows it that we are a server
	if(sendInt(sock, 1) < 0)
	{
		printf("Sending '1' to indicate Server to Dir Service failed \n");
		return -1;
	}

	//we then send '0' to indicate that we want to deregister
	if(sendInt(sock, 0) < 0)
	{
		printf("Sending '0' to indicate Server to Dir Service failed \n");
		return -1;
	}

	int deregistrationStatus=0;
	if(readInt(sock, deregistrationStatus) < 0){
		puts("Deregistration failed");
		return -1;
	}

	if(deregistrationStatus == 1)
	{
		puts("Service successfully deregistered");
		close(sock);
		return 1;
	}

	close(sock);
	return -1;
}


void setDeregisteredTrue(void)
{
	deRegistered = 1;

	int statusOfDeregistration = deregisterWithDirService();
	if(statusOfDeregistration==1)
	{
		printf("Deregistration complete \n");
	}
	else
	{
		printf("Deregistration failed \n");
	}

	//write invocation to deregister function
	//TODO: Write deregister function
}

void* deRegisterMenu(void *args)
{
	printf("Please enter 1 to deregister \n");
	scanf("%d",&deRegistered);
	if(deRegistered!=1)
	{
		printf("Incorrect option \n");
		deRegisterMenu(&deRegistered);
	}
	setDeregisteredTrue();
}

void successOrFailedSend(int a)
{
	if(a==1)
	{
		printf("Value sent successfully \n");
	}
	else
	{
		printf("Value NOT sent successfully \n");
	}
}

void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char *message , client_message[2000];
	//int testvar=0;
	int **testarray;

	//Counter is a depricated variable that is being kept for future use
	int counter=0;

    //Receive a message from client

	while(deRegistered==0){
		//printf("Value of deregistered is %d \n",deRegistered);
		//TODO: Add function to send client some message when the connection is terminated due to deRegistered=0

		while(1){
			//initializing new while to look for counter value
			if(readInt(sock, counter) > 0)
			{
				printf("Counter value successfully received and is %d \n",counter);
				break; //breaks from outer while loop
			}
		}


		switch(counter){
			case 1:
				break;
			case 2:
				break;
			case 3:
				break;
			case 4:
				break;
			case 5:
				break;
		}

}

	//Free the socket pointer
	free(socket_desc);

	return 0;
}

int registerWithDirService()
{
	// Register the server with the directory service
	int sock;
	struct sockaddr_in server;

	//Create socket
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1)
	{
		puts("Could not create socket");
		return -1;
	}
	puts("Socket created");

	//below should contain the ip address of the Directory Service
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons( 8000 );

	//Connect to Server
	puts("Trying to connect\n");
	int connected = -1;
	while(connected == -1){
		//Connect to remote server
		connected = connect(sock , (struct sockaddr *)&server , sizeof(server));
	}

	//Connected to server
	puts("Connected \n");

	//we first send '1' to the Dir Service which shows it that we are a server
	if(sendInt(sock,1) < 0)
	{
		printf("Sending '1' to indicate Server to Dir Service failed \n");
		return -1;
	}

	printf("Going to send value to indicate I want to register \n");
	//we then send '1' to indicate that we want to Register
	if(sendInt(sock, 1) < 0)
	{
		printf("Sending '1' to indicate Server to Dir Service failed \n");
		return -1;
	}

	//now we send the port
	//here 8888 is the port that we are listening to for connections from the dir service
	if(sendInt(sock, 8888) < 0)
	{
		printf("Sending Port Number failed \n");
		return -1;
	}

	int registrationStatus = 0;

	if(readInt(sock, registrationStatus) < 0){
		printf("Registration failed \n");
		return -1;
	}

	if(registrationStatus == 1){
		printf("Service successfully registered \n");
		return 1;
	}

	close(sock);
	return -1;
}

int main(int argc , char *argv[])
{
	// Register with directory service
	if(registerWithDirService() < 0)
	{
		puts("Directory Registry Failed");
		return 1;
	}

    int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    //server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons( 8888 );

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

    //Listen
    listen(socket_desc , 3);

	//adding code for thread that is supposed to send deregister info to directory

	pthread_t deregisterThread;
	int statusOfDeregisterThread = pthread_create(&deregisterThread, NULL, deRegisterMenu, &deRegistered);
	if(statusOfDeregisterThread==0)
	{
		 printf("Deregister Thread created successfully \n");
	}
	else
	{
		printf("Deregister Thread creation failed \n");
	}



	//*************************************************************************


    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");

        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;

        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }

        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( sniffer_thread , NULL);
        puts("Handler assigned");
    }

    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }

    return 0;
}



