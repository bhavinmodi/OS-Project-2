/*
 * server.c
 *
 *  Created on: Nov 24, 2015
 *      Author: Bhavin
 */

#include "indexandhashing.h"
#include "filelochash.h"
#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread
#include<errno.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/ioctl.h>
#include<netinet/in.h>
#include<net/if.h>

// Server Directory address
#define ServerDirectoryIP "127.0.0.1"
#define ServerDirectoryPort 8001

// Worker Directory address
#define WorkerDirectoryIP "127.0.0.1"
#define WorkerDirectoryPort 8002

// server address for client
#define ServerIP "127.0.0.1"
#define ServerPort 8011

// MUTEX LOCK ON INDEX
pthread_mutex_t lock;

//the thread function
int deRegistered=0;

// Prototype Declaration
int connectToWorkerDirectory();
int askWorkerDirectoryForWorkerDetails(int, int*, int*);
int getWorkerFromDirectory(char []);
int connectToWorker(int [], int);

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

	//Create socket
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1){
		puts("Could not create socket");
		return -1;
	}
	puts("Socket created");

	//below should contain the ip address of the Directory Service
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(ServerDirectoryIP);
	server.sin_port = htons(ServerDirectoryPort);

	//Connect to Directory
	puts("Trying to connect");
	while (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
		// Here is my error
		printf("Error when connecting! %s\n",strerror(errno));

		// Try Again
		sock = socket(AF_INET , SOCK_STREAM , 0);
		if(sock == -1){
			puts("Could not create socket");
			return -1;
		}
		puts("Socket created");
	}

	//Connected to Directory
	puts("Connected \n");

	//we first send '1' to the Dir Service which shows it that we are a server
	if(sendInt(sock, 1) < 0){
		puts("Sending '1' to indicate Server to Dir Service failed");
		close(sock);
		return -1;
	}

	//we then send '1' to indicate that we want to Register
	if(sendInt(sock, 1) < 0) {
		puts("Sending '1' to indicate Server to Dir Service failed");
		close(sock);
		return -1;
	}

	//now we send the port
	if(sendInt(sock, ServerPort) < 0){
		printf("Sending Port Number failed \n");
		close(sock);
		return -1;
	}

	int registrationStatus = 0;

	if(readInt(sock, &registrationStatus) < 0){
		puts("Registration failed");
		close(sock);
		return -1;
	}

	if(registrationStatus == 1){
		printf("Service successfully registered \n");
		close(sock);
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
	if(sendInt(sock, 1) < 0){
		printf("Sending '1' to indicate Server to Dir Service failed \n");
		close(sock);
		return -1;
	}

	puts("Directory Knows we are the server");

	//we then send '0' to indicate that we want to deregister
	if(sendInt(sock, 0) < 0){
		printf("Sending '0' to indicate Server to Dir Service failed \n");
		close(sock);
		return -1;
	}

	// Send port
	if(sendInt(sock, ServerPort) < 0){
		puts("Sending port for deregistration failed");
		close(sock);
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

char * HashIndexLocker(int op, char *word, char **search, char keywords[], int *port){

	pthread_mutex_lock(&lock);

	switch(op){
	case 1:
	{
		int sizeOfWord = strlen(word);
		char stringToHash[sizeOfWord];
		strncpy(stringToHash,word,sizeOfWord);
		hashWordFromString(stringToHash);

		//globalHashIterate();
		break;
	}
	case 2:
		findMultipleWordsInHashWithSTRTOK(keywords,&word);
		break;
	case 3:
		*port = checkIfFileExists(word);
		break;
	case 4:
		hashFileAndPort(word,*port);
		break;
	case 5:
		freeFileLocHash();
		freeGlobalHash();
		break;
	}

	pthread_mutex_unlock(&lock);

	return word;
}

int updateIndex(int sock){

	// Wait for index from worker
	int completionIndicator;
	int sizeOfWord;
	char word[1024];
	while(1){
		if(readInt(sock, &completionIndicator) < 0){
			puts("Reading completion indicator from worker failed");
			return -1;
		}

		if(completionIndicator == 0){
			// Finished receiving index
			break;
		}else{
			if(readInt(sock, &sizeOfWord) < 0){
				puts("Receive size of word failed");
				return -1;
			}

			if(readString(sock, sizeOfWord, &word[0]) < 0){
				puts("Failed to read index word from worker");
				return -1;
			}

			HashIndexLocker(1, &word[0], NULL, NULL, NULL);
		}
	}

	puts("Received Index from worker");

	return 1;
}

int rebuild(int sock, int port){
	// Receive all file names from workers
	char fileName[100];
	int indicator;

	while(1){
		// Receive indicator if there are more file names
		if(readInt(sock, &indicator) < 0){
			puts("Failed to receive indicator if more file names are to be sent");
			return -1;
		}

		if(indicator < 0){
			if(indicator == -2){
				// Worker failed
				puts("Worker had a path reading error");
			}else{
				// Done receiving file names
				puts("Done receiving file names");
			}
			break;
		}

		// Receive file name
		if(readString(sock, 100, &fileName[0]) < 0){
			puts("Receive file name from worker failed");
			return -1;
		}

		// Add the file to a has table table to know which worker got it
		HashIndexLocker(4, &fileName[0], NULL, NULL, &port);
	}

	// Receive index from worker
	if(updateIndex(sock) < 0){
		puts("Failed to receive index from worker");
		return -1;
	}

	return 1;
}

int rebuildIndex(){

	// Connect to Worker Directory
	int WDsock = connectToWorkerDirectory();

	if(WDsock < 1){
		puts("Failed to connect to worker directory");
		return -1;
	}

	//Let Worker Directory know you are the server and doing a rebuild index request
	if(sendInt(WDsock, 2) < 0){
		puts("Send connector type Server failed");
		close(WDsock);
		return -1;
	}

	int ip[4];
	int port, result;
	int Wsock;

	// Getting workers one at a time
	while(1){
		// Get details of Worker
		result = askWorkerDirectoryForWorkerDetails(WDsock, &ip[0], &port);
		if(result < 0){
			if(result == -2){
				puts("No more workers");
				// Close sockets
				close(WDsock);
				break;
			}

			puts("Failed to get worker details");
			// Close sockets
			close(WDsock);
			return -1;
		}

		// Connect to worker
		Wsock = connectToWorker(ip,port);
		if(Wsock < 0){
			puts("Connecting to worker failed for rebuilding Index");

			// Try the next worker
			continue;
		}

		//Let Worker know you are the server and doing a rebuild index request
		if(sendInt(Wsock, 4) < 0){
			puts("Send connector type from Server to worker rebuild failed");
			close(Wsock);
			return -1;
		}

		// Ask Worker for Index and all files
		result = rebuild(Wsock, port);
		if(result < 0){
			puts("Rebuilding Request for Worker failed");
			// Close sockets
			close(WDsock);
			close(Wsock);
			return -1;
		}else{
			// Rebuilding Success, close sockets
			close(Wsock);
		}

	}

	return 1;
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

	return 1;
}

int connectToWorkerDirectory(){
	//Initialize variables
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

	return sock;
}

int connectToWorker(int ip[], int port){
	//Initialize variables
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

	//Now setup Worker connection
	server.sin_addr.s_addr = inet_addr(address);
	server.sin_family = AF_INET;
	server.sin_port = htons( port );
	printf("Port it is trying to connect to is %d \n",port);

	//Connect to Remote Worker
	puts("Trying to connect to Remote Worker");
	int connected = 1;
	while(connected == 1){
		connected = connect(sock , (struct sockaddr *)&server , sizeof(server));
	}

	if(connected < 0){
		puts("Connection to worker failed");
		return -1;
	}

	//Connected to Remote Worker
	puts("Remote Worker Connected");

	return sock;
}

int askWorkerDirectoryForWorkerDetails(int sock, int *ip, int *port){

	int result;

	//Get Status of Worker Found or Not
	if(readInt(sock, &result) < 0){
		puts("Receive Query Result failed");
		close(sock);
		return -1;
	}else{
		if(result == 0){
			puts("Worker Not Found");
			close(sock);
			return -2;
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

	return 1;
}

int getWorkerFromDirectory(char fileName[100]){

		int sock;

		sock = connectToWorkerDirectory();

		if(sock < 0){
			puts("Failed to connect to worker directory");
			return -1;
		}

		//Send Lookup Request
		int ip[4];
		int port;

		//Let Worker Directory know you are the server
		if(sendInt(sock, 0) < 0){
			puts("Send connector type Server failed");
			close(sock);
			return -1;
		}

		int result = askWorkerDirectoryForWorkerDetails(sock, &ip[0], &port);
		if(result < 0){
			puts("Worker Not Found | Failed to get worker ip and port from worker directory");
			return -1;
		}

		//Successful
		//Close connection to Worker Directory Service
		close(sock);

		// Connect to worker
		sock = connectToWorker(ip, port);

		if(sock < 0){
			puts("Connect To Worker failed");
			return -1;
		}

		//fileLocHashIterate();

		// Send the file to a worker node for indexing
		if(sendFileToWorker(sock, fileName) < 0){
			printf("%s : Send To Worker Failed\n",fileName);
			close(sock);
			return -1;
		}else{
			printf("%s : Sent to Worker\n",fileName);
		}

		// Add the file to a has table table to know which worker got it
		HashIndexLocker(4, &fileName[0], NULL, NULL, &port);

		//Successful
		// Close connection to worker
		close(sock);

		return 1;
}

void deleteFile(char fileName[]){
	int ret = remove(fileName);

	if(ret == 0) {
	  puts("File deleted successfully");
	}else {
	  puts("Error: unable to delete the file");
	}
}

int startIndexing(int sock){

	//Accept incoming files and store them
	char fileName[100];
	int fileSize, bytesRead;
	char buffer[1024];
	int filePresent;
	int fileDetailStatus;
	int result;

	while(1){

		//Check if there are files
		if(readInt(sock, &filePresent) < 0){
			puts("Reading File Present Checker failed");
			break;
		}

		printf("File Present = %d\n",filePresent);

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

		printf("fileDetailStatus = %d\n",fileDetailStatus);

		// Get Filename
		while(1){
			if(recv(sock , &fileName , sizeof(char)*100,0) < 0){
				puts("Receive File Name Failed");
				return -1;
			}else{
				break;
			}
		}

		if(sendAck(sock) < 0){
			return -1;
		}

		printf("File Name Received = %s\n",fileName);

		// If fileName already exists, do not index
		HashIndexLocker(3, &fileName[0], NULL, NULL, &result);

		if(result == 1){
			// Already exists
			if(sendInt(sock, -1) < 0){
				puts("Sending will not hash as -1 failed");
				return -1;
			}

			// Send 1 to say indexing was successful
			if(sendInt(sock, 1) < 0){
				puts("Sending 1 after file exists break failed");
				return -1;
			}

			continue;
		}

		// Send 1 for file has not been indexed yet
		if(sendInt(sock, 1) < 0){
			puts("Sending will be indexed as 1 failed");
			return -1;
		}

		// Initialize file
		FILE *fp;
		fp = fopen(fileName, "w+");

		if(fp == NULL){
			printf("%s : Error opening file",fileName);
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
			deleteFile(fileName);
			return -1;
		}

		// Close file
		fclose(fp);

		// Ask workerDirectory for worker and send it the file and do the indexing
		puts("Asking for worker from Worker Directory");
		if(getWorkerFromDirectory(fileName) < 0){
			puts("Worker Directory | Worker Not Found");

			// Delete file
			deleteFile(fileName);

			// Let the client know of indexing failure
			if(sendInt(sock, -1) < 0){
				puts("Letting the client know that the server could not find the directory | Worker | Send the file : failure");
			}
			return -1;
		}else{
			// Delete file
			deleteFile(fileName);

			// Let the client know that the worker got the file
			if(sendInt(sock, 1) < 0){
				puts("Letting the client know that the worker got the file : failure");
			}
		}

	}

	return 1;
}

int requestFileFromWorker(int port, char fileName[]){
	// Connect to the worker
	struct sockaddr_in worker;
	int sock;

	// Create socket
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1) {
		puts("Could not create socket");
		return -1;
	}

	puts("Socket created");

	//Now setup Worker connection
	worker.sin_addr.s_addr = inet_addr("127.0.0.1");
	worker.sin_family = AF_INET;
	worker.sin_port = htons(port);
	printf("Port it is trying to connect to is %d \n",port);

	//Connect to Remote Worker
	puts("Trying to connect to Remote Worker");
	int connected = 1;
	while(connected == 1){
		connected = connect(sock , (struct sockaddr *)&worker , sizeof(worker));
	}

	if(connected < 0){
		puts("Connection to worker failed");
		return -1;
	}

	//Connected to Remote Worker
	puts("Remote Worker Connected");

	// Tell the worker you are the server and connecting for search
	if(sendInt(sock, 2) < 0){
		puts("Sending Request type to worker failed.");

		// Close Socket
		close(sock);
		return -1;
	}

	puts("Sent Server Choice to worker 2");

	// Send FileName you are requesting
	if(sendString(sock, 100, &fileName[0]) < 0){
		puts("Sending file Name for searching to worker failed.");

		// Close Socket
		close(sock);
		return -1;
	}

	printf("Sent Filename to worker = %s\n",fileName);

	int fileSize = 0;
	// Get the file size
	if(readInt(sock, &fileSize) < 0){
		puts("Receiving file size from worker failed.");
		return -1;
	}

	printf("Got filesize from worker = %d\n",fileSize);

	// Initialize file
	FILE *fp;
	fp = fopen(fileName, "w+");

	if(fp == NULL){
		printf("%s : Error opening file",fileName);
		return -1;
	}

	// Get File
	int bytesRead = 0;
	char buffer[1024];

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

	printf("Finished reading file from worker");

	// Close Socket and file
	fclose(fp);
	close(sock);

	return 1;
}

int sendFileToClient(int sock, char fileName[]){
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
		if(sendString(sock, 1024, &fileContents[0]) < 0){
			puts("Failed to send file contents to client.");
			return -1;
		}
	}

	puts("File Sent");

	if(waitForAck(sock) < 0){
		return -1;
	}

	// Close file
	fclose(fp);

	// Delete file on Server
	int ret = remove(fileName);

	if(ret == 0) {
	  puts("File deleted successfully");
	}else {
	  puts("Error: unable to delete the file");
	  return -1;
	}

	return 1;
}

int searchIndex(int sock, char keywords[]){

	// Search Hash table
	char *result;
	result = HashIndexLocker(2, result, NULL, keywords, NULL);

	// Send the client the search result
	int sizeOfResult = strlen(result) + 1;

	printf("Result = %s\n",result);

	// Send result size
	if(sendInt(sock, sizeOfResult) < 0){
		puts("Failed to send result size to client");
		return -1;
	}

	// Send the result
	if(sendString(sock, sizeOfResult, result) < 0){
		puts("Failed to send result to client");
		return -1;
	}

	// Get from client if file is to be retrieved
	int isFileToBeRetrieved;
	if(readInt(sock, &isFileToBeRetrieved) < 0){
		puts("Failed to receive retrieval result");
		return -1;
	}

	if(isFileToBeRetrieved < 0){
		// User does not want to retrieve the file
		puts("User does not want to retrieve a file");
		return 1;
	}

	// Get from client which file he wants
	char requestedFile[100];
	if(readString(sock, 100, &requestedFile[0]) < 0){
		puts("Failed to receive requested fileName from result from client.");
		return -1;
	}
	
	printf("File Name Received = %s\n",requestedFile);

	int portNumber = findFileLoc(requestedFile);
	if(portNumber < 0){
		puts("Get Port Number Failed");
		return -1;
	}

	// Get Files from worker depending on result of search
	if(requestFileFromWorker(portNumber, requestedFile) < 0){
		puts("requestFileFromWorker Failed");
		return -1;
	}

	puts("Starting Send File To Client");

	// Send the requested file to the client
	if(sendFileToClient(sock, requestedFile) < 0){
		puts("sendFileToClient Failed.");
		return -1;
	}

	return 1;
}

int startSearch(int sock){

	char keywords[100];

	// Receive keywords
	if(readString(sock, 100, &keywords[0]) < 0){
		puts("Receive Keywords failed");
		return -1;
	}

	if(searchIndex(sock, keywords) < 0){
		puts("Searching Index failed");
		return -1;
	}

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
			//puts("Waiting For Client Choice...");

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
					puts("Client Called Indexing, choice = 1");
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
					}else{
						puts("Search Successful");
					}
					break;
				case 3:
					// This is an exit request
					//Free the socket pointer
					close(sock);
					free(socket_desc);
					return 0;
					break;
				default:
					printf("Invalid Choice %d\n",choice);
					break;
			}
		}
		break;
	case 2:
		// Worker
		//puts("Waiting for worker choice..");

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
				}else{
					puts("Update Index by worker success");
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
	case 4:
		// Worker Directory
		//Free the socket pointer
		close(sock);
		free(socket_desc);

		// Delete existing Hash structures
		HashIndexLocker(5, NULL, NULL, NULL, 0);

		rebuildIndex();

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
	if (pthread_mutex_init(&lock, NULL) != 0){
		printf("\n mutex init failed\n");
		return 1;
	}

	// Rebuild Index
	if(rebuildIndex() < 0){
		puts("Failed to rebuild Index, closing server");
		return 1;
	}

	// Register with directory service
	if(registerWithDirService() < 0) {
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

    pthread_mutex_destroy(&lock);

    if (client_sock < 0) {
        perror("accept failed");
        return 1;
    }else{
    	close(client_sock);
    }

    return 0;
}



