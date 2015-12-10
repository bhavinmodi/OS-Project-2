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



// Worker Directory address

#define WorkerDirectoryIP "127.0.0.1"

#define WorkerDirectoryPort 8002



// Worker Port

#define WorkerIP "127.0.0.1"

#define WorkerPort 8021



//the thread function

int deRegistered=0;



// MUTEX ON INDEX

int mutex = 0;



int sendAck(int sock)

{

	int ackValue = 1;



	if(send(sock, &ackValue, sizeof(int), 0) > 0){

		puts("Ack Sent");

		return 1;

	}else{

		puts("Ack Send Failed");

		return -1;

	}

}



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



int readInt(int sock, int* a){

	int statusOfRead, ackValue=1, statusOfAck;

	while(1){

		statusOfRead = recv(sock , a , sizeof(int),0);

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

	server.sin_addr.s_addr = inet_addr(WorkerDirectoryIP);

	server.sin_family = AF_INET;

	server.sin_port = htons( WorkerDirectoryPort );



	//Connect to Server

	puts("Trying to connect");

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

		puts("Sending '1' to indicate Server to Dir Service failed");

		return -1;

	}



	//we then send '1' to indicate that we want to Register

	if(sendInt(sock, 1) < 0)

	{

		puts("Sending '1' to indicate Server to Dir Service failed");

		return -1;

	}



	// Send the port

	if(sendInt(sock, WorkerPort) < 0)

	{

		printf("Sending Port Number failed \n");

		return -1;

	}



	int registrationStatus = 0;



	if(readInt(sock, &registrationStatus) < 0){

		puts("Registration failed");

		return -1;

	}



	if(registrationStatus == 1){

		printf("Service successfully registered \n");

		return 1;

	}



	close(sock);

	return -1;

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

	server.sin_addr.s_addr = inet_addr(WorkerDirectoryIP);

	server.sin_family = AF_INET;

	server.sin_port = htons(WorkerDirectoryPort);



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



	// Send the port

	if(sendInt(sock, WorkerPort) < 0)

	{

		printf("Sending Port Number failed \n");

		return -1;

	}



	int deregistrationStatus = 0;

	if(readInt(sock, &deregistrationStatus) < 0){

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



	return 0;

}



int startIndexing(int sock){



	//Accept incoming files and store them

	char fileName[100];

	int fileSize, bytesRead;

	char buffer[1024];



	// Get Filename

	while(1){

		if(recv(sock , &fileName , sizeof(char)*100,0) < 0){

			puts("Receive File Name Failed");

			return -1;

		}else{

			break;

		}

	}



	printf("File Name Received = %s\n",fileName);



	// Initialize file

	FILE *fp;

	fp = fopen(fileName, "w+");



	if(fp == NULL){

		printf("%s : Error opening file",fileName);

		return -1;

	}



	if(sendAck(sock) < 0){

		return -1;

	}



	// Get File Size

	if(readInt(sock, &fileSize) < 0){

		return -1;

	}



	printf("File Size Received = %d\n",fileSize);



	// Get file contents and store them

	bytesRead = 0;

	while(bytesRead < fileSize){



		// Read the first 1 KB

		while(1){

			if(recv(sock , &buffer , sizeof(char)*1024,0) < 0){

				puts("Receive File Contents Failed");

				return -1;

			}else{

				//Write to file

				fputs(buffer, fp);

				break;

			}

		}



		bytesRead = bytesRead + strlen(buffer);

	}



	if(sendAck(sock) < 0){

		return -1;

	}



	// Close file

	fclose(fp);



	//TODO: Perform indexing

	if(mutex  == 0){

		mutex = 1;

		char *word2=NULL;

		hashFile(fileName);
		//globalHashIterate();
		initializeConversionLocalHashToString();
		convertLocalHashIntoString(fileName,&word2);
		while(strcmp(word2,"EMPTY")!=0)
		{
			//TODO: Bhavin add code here to push this word to server
			printf("Word is : %s \n",word2);
			convertLocalHashIntoString(fileName,&word2);
		}

		mutex = 0;

	}else{

		while(mutex == 1){

			// Wait

		}



		mutex = 1;

		char *word2=NULL;

		hashFile(fileName);
		//globalHashIterate();
		initializeConversionLocalHashToString();
		convertLocalHashIntoString(fileName,&word2);
		while(strcmp(word2,"EMPTY")!=0)
		{
			printf("Word is : %s \n",word2);
			//TODO: Bhavin add code here to push this word to server
			convertLocalHashIntoString(fileName,&word2);
		}

		mutex = 0;

	}



	return 1;

}



int startSearch(int sock){



	char keywords[100];



	// Receive keywords

	if(recv(sock, &keywords, sizeof(char)*100, 0) < 0){

		return -1;

	}



	// Send Ack

	if(sendAck(sock) < 0){

		return -1;

	}



	// TODO: Perform search

	if(mutex  == 0){

		mutex = 1;

		// TODO: Perform search

		mutex = 0;

	}else{

		while(mutex == 1){

			// Wait

		}



		mutex = 1;

		// TODO: Perform search

		mutex = 0;

	}



	return 1;

}



void *connection_handler(void *socket_desc)

{

    //Get the socket descriptor

    int sock = *(int*)socket_desc;



	// Choice

	int choice = 0;



	puts("Waiting For Server | Worker Directory Command...");



	while(1){

		//initializing new while to look for choice value

		if(readInt(sock, &choice) < 0){

			puts("Receive Choice Failed | Maybe Server disconnected");

			return 0;

		}else{

			printf("Choice is = %d\n",choice);

			break; //breaks from inner while loop

		}

	}



	switch(choice){

		case 1:

			// This is an indexing request

			if(startIndexing(sock) < 0){

				puts("Indexing Failed | Closing connection");



				//Free the socket pointer

				free(socket_desc);

				return 0;

			}else{

				puts("Indexing Successful");

			}

			break;

		case 2:

			// This is a searching request

			if(startSearch(sock) < 0){

				puts("Search Failed | Closing Connection");



				//Free the socket pointer

				close(sock);

				free(socket_desc);

				return 0;

			}else{

				puts("Search Successful");

			}

			break;

		case 3:

			puts("Ping Request from the Worker Directory");

			close(sock);

			free(socket_desc);

			return 0;

		default:

			printf("Invalid Choice %d",choice);

			break;

	}





	//If we reach here, we have de-registered

	//Free the socket pointer

	close(sock);

	free(socket_desc);



	return 0;

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

    //server.sin_addr.s_addr = INADDR_ANY;

    server.sin_addr.s_addr = inet_addr(WorkerIP);

    server.sin_port = htons(WorkerPort);



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

    	if(deRegistered == 1){

    		break;

    	}



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

    }else{

    	close(client_sock);

    }



    return 0;

}





