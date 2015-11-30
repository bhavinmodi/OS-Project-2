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
#include<sys/stat.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>

// Client Directory address
#define ServerDirectoryIP "127.0.0.1"
#define ServerDirectoryPort 8000

// Server Directory address
#define ServerIP "127.0.0.1"
#define ServerPort 8888

// MUTEX LOCK
int mutex = 0;

//the thread function
int deRegistered=0;

int waitForAck(int sock){
	//Wait for Ack
	int ack;

	while(1){
		if(recv(sock , &ack , sizeof(int),0) < 0){
			//Failure break
			return -1;
		}else{
			puts("Got ACK");
			//Got ACK
			return 1;
		}
	}

	//Check Ack
	if(ack == 0){
		//Failure break
		return -1;
	}

	return 1;
}

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
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(ServerDirectoryIP);
	server.sin_port = htons(ServerDirectoryPort);

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

	//now we send the port
	//here 8888 is the port that we are listening to for connections from the dir service
	if(sendInt(sock, ServerPort) < 0)
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
	/// Deregister the server with the directory service
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
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(ServerDirectoryIP);
	server.sin_port = htons(ServerDirectoryPort);

	//Connect to Directory
	puts("Trying to connect\n");
	int connected = -1;
	while(connected == -1){
		//Connect to Directory Service
		connected = connect(sock , (struct sockaddr *)&server , sizeof(server));
	}

	//Connected to Directory
	puts("Connected \n");

	//we first send '1' to the Dir Service which shows it that we are a server
	if(sendInt(sock, 1) < 0)
	{
		printf("Sending '1' to indicate Server to Dir Service failed \n");
		return -1;
	}

	puts("Directory Knows we are the server");

	//we then send '0' to indicate that we want to deregister
	if(sendInt(sock, 0) < 0)
	{
		printf("Sending '0' to indicate Server to Dir Service failed \n");
		return -1;
	}

	puts("0 for deregister, successful");

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

	return 0;
}

int sendFileToWorker(int sock, char fileName[100]){

	// Run setup at worker
	// Indexing request to worker
	if(sendInt(sock, 1) < 0){
		puts("Indexing request to worker failed");
		return -1;
	}

	//Sending 1KB at a time
	char fileContents[1024];

	// Find out file size
	int size;
	struct stat s;

	if(stat(fileName, &s) == 0){
		size = s.st_size;
	}

	printf("File Size = %d\n",size);

	// Open file
	FILE *fp;

	fp = fopen(fileName, "r");

	if(fp == NULL){
		printf("%s : Error opening file\n",fileName);
		return -1;
	}

	//Send FileName
	if(send(sock , &fileName[0] , sizeof(char)*100 , 0) < 0){
		puts("Send File Name Failed");
		return -1;
	}

	puts("File Name Sent");

	if(waitForAck(sock) < 0){
		return -1;
	}

	//Send File size
	if(sendInt(sock, size) < 0){
		puts("Send File Size Failed");
		return -1;
	}

	puts("File Size Sent");

	// Read file
	while(fgets(fileContents, 1024, (FILE*)fp) != NULL){
		//Sending 1 KB of the file
		if(send(sock , &fileContents , sizeof(char)*1024 , 0) < 0){
			puts("Send Failed");
			return -1;
		}
	}

	puts("File Sent");

	if(waitForAck(sock) < 0){
		return -1;
	}

	// Close file
	fclose(fp);

	return 1;
}

int sendKeywordsToWorker(int sock, char keywords[100]){

	// Run setup at worker
	// Searching request to worker
	if(sendInt(sock, 2) < 0){
		puts("Search request to worker failed");
		return -1;
	}

	//Send Keywords
	if(send(sock , &keywords[0] , sizeof(char)*100 , 0) < 0){
		puts("Send Keywords Failed");
		return -1;
	}

	puts("Keywords Sent");

	if(waitForAck(sock) < 0){
		return -1;
	}

	return 1;
}

int getWorkerFromDirectory(char fileName[100], int op){

		//Initialize variables
		int sock, result;
		struct sockaddr_in server;

		//Send Lookup Request
		int ip[4];
		int port;

		//Create socket
	    sock = socket(AF_INET , SOCK_STREAM , 0);
	    if (sock == -1)
	    {
	        puts("Could not create socket");
			return -1;
	    }
	    puts("Socket created");

		//Connect to Directory Register and get ip and port for service
		server.sin_addr.s_addr = inet_addr("127.0.0.2");
	    server.sin_family = AF_INET;
	    server.sin_port = htons( 8010 );

		//Connect to Directory Service
		puts("Trying to connect to Directory Service");
		int connected = -1;
		while(connected == -1){
	    	connected = connect(sock , (struct sockaddr *)&server , sizeof(server));
		}

		//Connected to Directory Service
	    puts("Connected");

		//Let Directory know you are the client
		if(sendInt(sock, 0) < 0){
			puts("Send connector type client failed");
			close(sock);
			return -1;
		}

		//Get Status of Server Found or Not
		if(readInt(sock, &result) < 0){
			puts("Receive Query Result failed");
			close(sock);
			return -1;
		}else{
			if(result == 0){
				puts("Server Not Found");
				close(sock);
				return -1;
			}
		}

		printf("Got Result = %d\n",result);

		//Get IP
		if(recv(sock , &ip , sizeof(int)*4,0) < 0){
			puts("Receive IP failed");
			close(sock);
			return -1;
		}

		//Send the ACK
		if(sendAck(sock) < 0){
			puts("Send ACK failed");
			close(sock);
			return -1;
		}

		//Get Port
		if(readInt(sock , &port) < 0){
			puts("Receive Port failed");
			close(sock);
			return -1;
		}

		//Successful
		//Close connection to Directory Service
		close(sock);

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
			return -1;
		}
		puts("Socket created");

		//Now setup Worker connection
		server.sin_addr.s_addr = inet_addr(address);
		server.sin_family = AF_INET;
		server.sin_port = htons( port );
		printf("Port it is trying to connect to is %d \n",port);

		//Connect to Remote Worker
		puts("Trying to connect to Remote Worker");
		connected = -1;
		while(connected == -1){
			connected = connect(sock , (struct sockaddr *)&server , sizeof(server));
		}

		//Connected to Remote Worker
		puts("Connected");

		// call function depending on Op
		switch(op){
		case 1:
			// Send the file to a worker node for indexing
			if(sendFileToWorker(sock, fileName) < 0){
				printf("%s : Send To Worker Failed\n",fileName);
				return -1;
			}else{
				printf("%s : Sent to Worker\n",fileName);
			}
			break;
		case 2:
			// Send the file to a worker node for searching
			// variable is called fileName, but it has keywords
			if(sendKeywordsToWorker(sock, fileName) < 0){
				printf("%s : Send To Worker Failed\n",fileName);
				return -1;
			}else{
				printf("%s : Sent to Worker\n",fileName);
			}
			break;
		default:
			puts("Invalid Operation for worker");
			return -1;
		}

		return 1;
}

int startIndexing(int sock){

	//Accept incoming files and store them
	char fileName[100];
	int fileSize, bytesRead;
	char buffer[1024];
	int filePresent;

	while(1){

		//Check if there are files
		if(readInt(sock, &filePresent) < 0){
			puts("Reading File Present Checker failed");
			break;
		}

		if(filePresent < 0){
			// No more files
			break;
		}

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

			bytesRead = bytesRead + 1024;
		}

		if(sendAck(sock) < 0){
			return -1;
		}

		// Close file
		fclose(fp);

		// Ask workerDirectory for worker and send it the file
		if(getWorkerFromDirectory(fileName, 1) < 0){
			puts("Server Not Found");
			return -1;
		}

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
	getWorkerFromDirectory(keywords, 2);

	return 1;
}

/***
 * MUTEX
 */
int startOp(int sock, int op){

	if(mutex == 0){
		// Perform Op
		switch(op){
		case 1:
			// Indexing Op
			mutex = 1;
			if(startIndexing(sock)< 0){
				mutex = 0;
				return -1;
			}else{
				mutex = 0;
				return 1;
			}
		case 2:
			// Searching Op
			mutex = 2;
			if(startSearch(sock) < 0){
				mutex = 0;
				return -1;
			}else{
				mutex = 0;
				return 1;
			}
			break;
		default:
			// Invalid Op
			puts("Invalid Operation");
			return -1;
		}
	}

	if(mutex == 2){
		// Searching Going On : Search Allowed, Indexing Not Allowed
		if(op == 2){
			// Conitnue
		}else{
			if(op == 1){
				while(mutex == 2){
					// Wait
				}
			}else{
				// Invalid op, exit
				puts("Invlaid Op");
				return -1;
			}
		}
	}else{
		if(mutex == 1){
			// Indexing going on, so wait
			while(mutex == 1){
				// Wait
			}
		}
	}

	switch(op){
	case 1:
		// Indexing Op
		mutex = 1;
		if(startIndexing(sock)< 0){
			mutex = 0;
			return -1;
		}else{
			mutex = 0;
			return 1;
		}
	case 2:
		// Searching Op
		mutex = 2;
		if(startSearch(sock) < 0){
			mutex = 0;
			return -1;
		}else{
			mutex = 0;
			return 1;
		}
		break;
	default:
		// Invalid Op
		puts("Invalid Operation");
		return -1;
	}

}

void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;

	// Choice
	int choice = 0;

	while(deRegistered == 0){
		puts("Waiting For User Choice...");

		while(1){
			//initializing new while to look for choice value
			if(readInt(sock, &choice) < 0){
				puts("Receive Choice Failed | Maybe Client disconnected");
				return 0;
			}else{
				printf("Choice is = %d\n",choice);
				break; //breaks from inner while loop
			}
		}

		switch(choice){
			case 1:
				// This is an indexing request
				if(startOp(sock, 1) < 0){
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
				if(startOp(sock, 2) < 0){
					puts("Search Failed | Closing Connection");

					//Free the socket pointer
					free(socket_desc);
					return 0;
				}else{
					puts("Search Successful");
				}
				break;
			default:
				printf("Invalid Choice %d",choice);
				break;
		}

	}

	//If we reach here, we have de-registered
	//Free the socket pointer
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
    server.sin_addr.s_addr = inet_addr(ServerIP);
    server.sin_port = htons(ServerPort);

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



