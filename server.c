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

#include <sys/types.h>
#include<sys/stat.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>

// Server Directory address
#define ServerDirectoryIP "127.0.0.1"
#define ServerDirectoryPort 8001

// Worker Directory address
#define WorkerDirectoryIP "127.0.0.1"
#define WorkerDirectoryPort 8002

// server address
#define ServerIP "127.0.0.1"
#define ServerPort 8011

// MUTEX LOCK ON INDEX
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

	//Create socket
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1)
	{
		puts("Could not create socket");
		return -1;
	}
	puts("Socket created");

	// Bind the server to its own socket before connecting,
	//Bind to a specific network interface (and optionally a specific local port)
	struct sockaddr_in localaddr;
	localaddr.sin_family = AF_INET;
	localaddr.sin_addr.s_addr = inet_addr(ServerIP);
	localaddr.sin_port = ServerPort;
	bind(sock, (struct sockaddr *)&localaddr, sizeof(localaddr));

	//below should contain the ip address of the Directory Service
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(ServerDirectoryIP);
	server.sin_port = htons(ServerDirectoryPort);

	//Connect to Directory
	puts("Trying to connect");
	int connected = -1;
	while(connected == -1){
		//Connect to client Directory
		connected = connect(sock , (struct sockaddr *)&server , sizeof(server));
	}

	//Connected to Directory
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

	//Create socket
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1)
	{
		puts("Could not create socket");
		return -1;
	}
	puts("Socket created");

	// Bind the server to its own socket before connecting,
	//Bind to a specific network interface (and optionally a specific local port)
	struct sockaddr_in localaddr;
	localaddr.sin_family = AF_INET;
	localaddr.sin_addr.s_addr = inet_addr(ServerIP);
	localaddr.sin_port = ServerPort;
	bind(sock, (struct sockaddr *)&localaddr, sizeof(localaddr));

	// Below should contain the ip address of the Directory Service
	struct sockaddr_in server;
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

	// Send port
	if(sendInt(sock, ServerPort) < 0){
		puts("Sending port for deregistration failed");
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
		puts("Deregistration complete");
	}
	else
	{
		puts("Deregistration failed");
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
		puts("Incorrect option");

		//flushing input stream
		while((c = getchar()) != '\n' && c != EOF);

		deRegisterMenu(&deRegistered);
		return 0;
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

		// Wait for Ack
		if(waitForAck(sock) < 0){
			puts("Waiting for Ack from worker while sending file failed");
			return -1;
		}
	}

	puts("File Sent");

	if(waitForAck(sock) < 0){
		return -1;
	}

	// Close file
	fclose(fp);

	int ret = remove(fileName);

    if(ret == 0) {
	  puts("File deleted successfully");
    }else {
	  puts("Error: unable to delete the file");
	  return -1;
    }

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

		// Connect to Worker Directory Register and get ip and port for service
		server.sin_addr.s_addr = inet_addr(WorkerDirectoryIP);
	    server.sin_family = AF_INET;
	    server.sin_port = htons(WorkerDirectoryPort);

		// Connect to Worker Directory Service
		puts("Trying to connect to Worker Directory Service");
		int connected = 1;
		while(connected == 1){
	    	connected = connect(sock , (struct sockaddr *)&server , sizeof(server));
		}

		if(connected < 0){
			puts("Worker Directory Not Present");
			return -1;
		}

		// Connected to Worker Directory Service
	    puts("Connected to Worker Directory");

		//Let Worker Directory know you are the server
		if(sendInt(sock, 0) < 0){
			puts("Send connector type Server failed");
			close(sock);
			return -1;
		}

		//Get Status of Worker Found or Not
		if(readInt(sock, &result) < 0){
			puts("Receive Query Result failed");
			close(sock);
			return -1;
		}else{
			if(result == 0){
				puts("Worker Not Found");
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
		//Close connection to Worker Directory Service
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
		connected = 1;
		while(connected == 1){
			connected = connect(sock , (struct sockaddr *)&server , sizeof(server));
		}

		if(connected < 0){
			puts("Connection to worker failed");
			return -1;
		}

		//Connected to Remote Worker
		puts("Remote Worker Connected");

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

		// Close connection to worker
		close(sock);

		return 1;
}

int updateIndex(int sock){

	while(mutex == 1 || mutex == 2){
		//wait
	}

	mutex = 1;

	// Wait for index from worker
	int completionIndicator;
	char word[1024];
	while(1){
		if(readInt(sock, &completionIndicator) < 0){
			puts("Reading completion indicator from worker failed");
			mutex = 0;
			return -1;
		}

		if(completionIndicator == 0){
			// Finished receiving index
			break;
		}else{
			if(readString(sock, 1024, &word[0]) < 0){
				puts("Failed to read index word from worker");
				mutex = 0;
				return -1;
			}

			//printf("Word Received is : %s \n",word);
			//printf("Test \n");
			int sizeOfWord = strlen(word);
			char stringToHash[sizeOfWord];
			strncpy(stringToHash,word,sizeOfWord);
			hashWordFromString(stringToHash);

			//globalHashIterate();
		}
	}

	puts("Received Index from worker");

	mutex = 0;

	return 1;
}

int startIndexing(int sock){

	//Accept incoming files and store them
	char fileName[100];
	int fileSize, bytesRead;
	char buffer[1024];
	int filePresent;
	int fileDetailStatus;

	while(1){

		//Check if there are files
		if(readInt(sock, &filePresent) < 0){
			puts("Reading File Present Checker failed");
			break;
		}

		if(filePresent < 0){
			// No more files or Invalid Path on client
			if(filePresent == -2){
				// invalid path
				return -1;
			}
			break;
		}

		// Get status if file details on client is correct
		if(readInt(sock, &fileDetailStatus) < 0){
			return -1;
		}else{
			if(fileDetailStatus == -1){
				puts("Client got incorrect file details | Closing Indexing");
				return -1;
			}
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

			if(sendAck(sock) < 0){
				puts("Sending Ack on File receive failed");
				return -1;
			}

			// Add bytes read from length of buffer
			bytesRead = bytesRead + strlen(buffer);

			//printf("Length of buffer read for file = %d\n",bytesRead);
		}

		if(sendAck(sock) < 0){
			return -1;
		}

		// Close file
		fclose(fp);

		// Ask workerDirectory for worker and send it the file and do the indexing
		puts("Asking for worker from Worker Directory");
		if(getWorkerFromDirectory(fileName, 1) < 0){
			puts("Worker Directory | Worker Not Found");

			// Let the client know of indexing failure
			if(sendInt(sock, -1) < 0){
				puts("Letting the client know that the server could not find the directory : failure");
			}
			return -1;
		}else{
			// Let the client know that the worker got the file
			if(sendInt(sock, 1) < 0){
				puts("Letting the client know that the worker got the file : failure");
			}
		}

	}

	return 1;
}

int searchIndex(){
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
	if(mutex == 0 || mutex == 2){
		mutex = 2;
		if(searchIndex() < 0){
			puts("Searching Index failed");
			mutex = 0;
			return -1;
		}
		mutex = 0;
	}else{
		while(mutex == 1){
			// Wait
		}

		// Acquire lock
		mutex = 2;
		if(searchIndex() < 0){
			puts("Searching Index failed");
			mutex = 0;
			return -1;
		}
		mutex = 0;
	}

	// TODO: Get Files from worker depending on result of search
	//getWorkerFromDirectory(keywords, 2);

	return 1;
}

void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;

    // Connector
    int connector = -1;

	// Choice
	int choice = 0;

	if(readInt(sock, &connector) < 0){
		puts("Connector Type Receive Failed");
		//Free the socket pointer
		close(sock);
		free(socket_desc);
		return 0;
	}

	switch(connector){
	case 1:
		// Client
		while(deRegistered == 0){
			puts("Waiting For Client Choice...");

			while(1){
				//initializing new while to look for choice value
				if(readInt(sock, &choice) < 0){
					if(choice == 3){
						// Ping request Closure
						puts("Connection closed by directory");
					}else{
						puts("Receive Choice Failed | Maybe Client disconnected");
					}
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
						puts("Indexing Failed");
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
				default:
					printf("Invalid Choice %d\n",choice);
					break;
			}

		}
		break;
	case 2:
		// Worker
		puts("Waiting for worker choice..");

		if(readInt(sock, &choice) < 0){
			puts("Read Worker choice failed");
			//Free the socket pointer
			close(sock);
			free(socket_desc);
			return 0;
		}

		switch(choice){
			case 1:
				// Indexing
				puts("Update index called");
				if(updateIndex(sock) < 0){
					puts("Update Index by worker failed");
				}
				break;
			case 2:
				// Searching
				puts("Search index called");
				if(searchIndex()){
					puts("Search File return failed");
				}
				break;
			default:
				//Free the socket pointer
				close(sock);
				free(socket_desc);
				return 0;
		}
		break;
	case 3:
		//puts("Ping Request From Directory");
		//Free the socket pointer
		close(sock);
		free(socket_desc);
		return 0;
		break;
	default:
		puts("Invalid Connector Type");
	}

	//If we reach here, we have de-registered or completed
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
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) ) {
    	// If server has deregistered exit
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

    if (client_sock < 0) {
        perror("accept failed");
        return 1;
    }else{
    	close(client_sock);
    }

    return 0;
}



