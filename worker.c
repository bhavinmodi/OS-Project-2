/*
 * server.c
 *
 *  Created on: Nov 24, 2015
 *      Author: Bhavin
 */

#include "indexandhashing.h"
#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread
#include<sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include<dirent.h>

// Worker Directory address
#define WorkerDirectoryIP "127.0.0.1"
#define WorkerDirectoryPort 8002

// Server Directory address
#define ServerDirectoryIP "127.0.0.1"
#define ServerDirectoryPort 8001

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
		//puts("Ack Sent");
		return 1;
	}else{
		puts("Ack Send Failed");
		return -1;
	}
}

int waitForAck(int sock){
	//Wait for Ack
	int ack;

	while(1){
		if(recv(sock , &ack , sizeof(int),0) < 0){
			//Failure break
			return -1;
		}else{
			//puts("Got ACK");
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

int sendString(int sock, int a, char *b){
	int statusOfSend, ackValue=1, statusOfAck;
	statusOfSend = send(sock , b , sizeof(char)*a , 0);
	if(statusOfSend < 0){
		puts("Send Failed");
		return -1;
	}
	while(1){
		statusOfAck = recv(sock , &ackValue , sizeof(int),0);
		if(statusOfAck < 0){
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

int readString(int sock, int a, char *b){
	int statusOfRead, ackValue=1, statusOfAck;
	while(1){
		statusOfRead = recv(sock , b , sizeof(char)*a,0);
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
	char c;
	puts("Please enter 1 to De-register");

	char line[2];
	if (fgets(line, sizeof(line), stdin)) {
		sscanf(line, "%d", &deRegistered);
	}

	if(deRegistered != 1)
	{
		printf("Incorrect option \n");
		deRegisterMenu(&deRegistered);
		return 0;
	}
	setDeregisteredTrue();

	return 0;
}

int connectToServer(int ip[], int port){
	int sock;
	struct sockaddr_in server;

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

	//Now setup Server connection
	server.sin_addr.s_addr = inet_addr(address);
	//server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	printf("Port it is trying to connect to is %d \n",port);

	//Connect to Remote Server
	puts("Trying to connect to Remote Server");
	int connected = 1;
	while(connected == 1){
		connected = connect(sock , (struct sockaddr *)&server , sizeof(server));
	}

	if(connected == 0){
		//Connected to Remote Server
		puts("Connected");
		return sock;
	}else{
		printf("Connection Failed : Return Value %d\n",connected);
		return -1;
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
			return -1;
	    }
	    puts("Socket created");

		//Connect to Directory Register and get ip and port for service
		server.sin_addr.s_addr = inet_addr(ServerDirectoryIP);
	    server.sin_family = AF_INET;
	    server.sin_port = htons(ServerDirectoryPort);

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
			puts("Send connector type worker failed");
			close(sock);
			return -1;
		}

		// Tell the directory there the reason for connecting
		if(sendInt(sock, 1) < 1){
			puts("Send connector reason worker failed");
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
		if(recv(sock , ip , sizeof(int)*4,0) < 0){
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
		if(readInt(sock , port) < 0){
			puts("Receive Port failed");
			close(sock);
			return -1;
		}

		//Successful
		//Close connection to Directory Service
		close(sock);
		return 1;
}

int sendIndexToServer(char fileName[]){
	//Send Lookup Request
	int ip[4];
	int port;
	int sock;
	int sendFlag;

	// Get a new server
	if(getServerFromDirectory(&ip[0], &port) < 0){
		puts("No Server Found | Will Not send index");
		sendFlag = 0;
	}else{
		puts("Server Found");
		sendFlag = 1;
	}

	if(sendFlag == 1){
		sock = connectToServer(ip, port);
		if(sock < 0){
			puts("Worker failed to connect to the server");
			sendFlag = 0;
		}
	}

	if(sendFlag == 1){
		// Inform server that you are the worker
		if(sendInt(sock, 2) < 0){
			puts("Send Connector type to server failed");
			return -1;
		}

		// Inform server the purpose : Indexing
		if(sendInt(sock, 1) < 0){
			puts("Send request type to server failed");
			return -1;
		}
	}

	while(mutex == 1){
		// Wait
	}

	mutex = 1;

	char *word2=NULL;

	hashFile(fileName);
	//globalHashIterate();
	printf("Hashing file done \n");
	initializeConversionLocalHashToString();
	convertLocalHashIntoString(fileName,&word2);
	while(strcmp(word2,"EMPTY")!=0)
	{

		if(sendFlag == 1){
			// Send a 1 indicating we still want to send words
			if(sendInt(sock, 1) < 0){
				puts("Sending '1' indicator that we still have words to send failed");
				mutex = 0;
				return -1;
			}else{
				//puts("1 sent");
			}

			if(sendString(sock, 1024, word2) < 0){
				puts("Sending Index Back To Server Failed");
				mutex = 0;
				return -1;
			}else{
				//puts("Word sent");
			}
		}

		//printf("Word is : %s \n",word2);
		convertLocalHashIntoString(fileName,&word2);
	}

	if(sendFlag == 1){
		// Send 0 to indicate completion
		if(sendInt(sock, 0) < 0){
			puts("Sending '0' for completion of send index to server failed");
			mutex = 0;
			return -1;
		}
	}

	puts("0 for completion sent");

	// Close the connection established with the server
	close(sock);

	mutex = 0;
	return 1;
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

		// Send Ack
		if(sendAck(sock) < 0){
			puts("Sending Ack for File receive from server failed");
			return -1;
		}

		bytesRead = bytesRead + strlen(buffer);
		//printf("Length of buffer read for file = %d\n",bytesRead);
	}

	if(sendAck(sock) < 0){
		return -1;
	}

	// Close file
	fclose(fp);

	// Close connection to server
	close(sock);

	// Perform indexing
	if(sendIndexToServer(fileName) < 0){
		puts("Index Generation or Sending Failed");
		return -1;
	}

	return 1;
}

int sendFileToServer(int sock, char fileName[100]){
	// Searching request to worker

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

		// Wait for Ack
		if(waitForAck(sock) < 0){
			puts("Waiting for Ack from server while sending file failed");
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

int startSearch(int sock){

	char fileName[100];

	// Receive FileName
	if(readString(sock, 100, fileName) < 0){
		puts("Receiving fileName from server for searching failed.");
		return -1;
	}

	while(mutex == 1){
		// Wait
	}

	// Accquire Lock
	mutex = 1;

	if(sendFileToServer(sock, fileName) < 0){
		puts("Sending file to server failed.");
		mutex = 0;
		return -1;
	}

	mutex = 0;

	return 1;
}

int rebuild(int sock){
	// Scan all files in the worker directory and send them across
	char currentDirPath[1024];
	char fileName[100];

	if(getcwd(&currentDirPath[0], 1024) == NULL){
		puts("Failed to get current directory");
		return -1;
	}

	printf("Current Directory Path is = %s\n", currentDirPath);

	struct stat s;

	if (stat(currentDirPath, &s) == 0){
		if(S_ISDIR(s.st_mode)){
			puts("Directory");

			// Check directory for files, we only consider .txt files
			DIR *dp;
			struct dirent *ep;
			dp = opendir (currentDirPath);

			if (dp != NULL){
				while ((ep = readdir (dp)) != NULL){
					strcpy(fileName, ep->d_name);

					// If it is a .txt file, then send it over
					if(strstr(fileName, ".txt")){
						// Inform the server if there is a file name to send
						if(sendInt(sock, 1) < 0){
							puts("Sending File present failed");
							break;
						}

						// Send file name
						if(sendString(sock, 100, &fileName[0]) < 0){
							puts("Sending file name failed");
							return -1;
						}

					}
				}

				// No more file names left to send
				if(sendInt(sock, -1) < 0){
					puts("Failed to send no more file names left indicator");
					return -1;
				}

				(void) closedir (dp);
			}
		}
	}else{
		// Something went wrong with reading the directory, let the server know
		if(sendInt(sock, -2) < 0){
			puts("Failed to send wrong path error to server");
			return -1;
		}
	}

	// Send Index to server
	while(mutex == 1){
		// Wait
	}

	mutex = 1;

	initializeConversionGlobalHashToString();
	char *word=NULL;

	convertGlobalHashIntoString(&word);
	while(strcmp(word,"EMPTY")!=0){
		convertGlobalHashIntoString(&word);

		// Send a 1 indicating we still want to send words
		if(sendInt(sock, 1) < 0){
			puts("Sending '1' indicator that we still have words to send failed");
			mutex = 0;
			return -1;
		}

		if(sendString(sock, 1024, word) < 0){
			puts("Sending Index To Server for rebuilding Failed");
			mutex = 0;
			return -1;
		}
	}

	mutex = 0;
	return 1;
}

void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;

	// Choice
	int choice = 0;

	//puts("Waiting For Server | Worker Directory Command...");

	//initializing new while to look for choice value
	if(readInt(sock, &choice) < 0){
		puts("Receive Choice Failed | Maybe Server disconnected");
		return 0;
	}else{
		if(choice != 3){
			printf("Choice is = %d\n",choice);
		}
	}

	switch(choice){
		case 1:
			// This is an indexing request
			if(startIndexing(sock) < 0){
				puts("Indexing Failed | Closing connection");
			}else{
				puts("Indexing Successful");
			}
			break;
		case 2:
			// This is a searching request
			if(startSearch(sock) < 0){
				puts("Search Failed | Closing Connection");
			}else{
				puts("Search Successful");
			}
			break;
		case 3:
			//puts("Ping Request from the Worker Directory");
			close(sock);
			free(socket_desc);
			return 0;
		case 4:
			// This is a rebuild request from the server
			if(rebuild(sock) < 0){
				puts("Server rebuild request failed");
			}else{
				puts("Rebuild successful");
			}
			break;
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

        //puts("Connection accepted");

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
        //puts("Handler assigned");
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



