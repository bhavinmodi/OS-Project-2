
/*
 * directory.c
 *
 *  Created on: Nov 24, 2015
 *      Author: Bhavin
 */

#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>

// Worker Directory address
#define WorkerDirectoryIP "127.0.0.1"
#define WorkerDirectoryPort 8002

// For mutex
int mutex = 0;

typedef struct{
	int sock;
	int ip[4];
}threadArgs;

struct workerNode
{
	int ip[4];
	int port;
	int load;
	struct workerNode *next;
}*head, *curr;

int addToList(int ipArr[], int port){
	int i;

	struct workerNode *ptr = (struct workerNode*)malloc(sizeof(struct workerNode));

	if(NULL == ptr){
		puts("Node Creation Failed");
		return -1;
	}

	for(i = 0; i < 4; i++){
		ptr->ip[i] = ipArr[i];
	}

	ptr->port = port;
	ptr->load = 0;
	ptr->next = NULL;

	if(head == NULL){
		head = curr = ptr;
	}else{
		curr->next = ptr;
		curr = ptr;
	}

	return 1;
}

int scanListworkerR(int ipArr[], int port){

	struct workerNode *ptr = head;
	int dupFound = 0, i;

	while(ptr != NULL){

		//Check match for port
		if(ptr->port == port){
			//check IP
			int noMatch = 0;
			for(i = 0; i < 4; i++){
				if(ptr->ip[i] != ipArr[i]){
					noMatch = 1;
				}
			}
			if(noMatch == 0){
				//Dup Found
				dupFound = 1;
				break;
			}
		}

		ptr = ptr->next;
	}

	//If duplicate found return -1
	if(dupFound == 0){
		return 1;
	}else{
		return -1;
	}
}

int scanListWorkerDR(int ipArr[], int port){

	struct workerNode *ptr = head;
	struct workerNode *follow = head;

	int nodeFound = 0, i, temp = 0;

	while(ptr != NULL){
		//Check match for port
		if(ptr->port == port){
			//check IP
			int noMatch = 0;
			for(i = 0; i < 4; i++){
				if(ptr->ip[i] != ipArr[i]){
					noMatch = 1;
				}
			}
			if(noMatch == 0){
				//Match Found
				nodeFound = 1;

				//Delete Node
				if(ptr == head){
					head = NULL;
					free(ptr);
				}else{
					follow->next = ptr->next;
					free(ptr);
				}
				break;
			}
		}

		if(temp == 0){
			ptr = ptr->next;
			temp++;
		}else{
			ptr = ptr->next;
			follow = follow->next;
		}
	}

	if(nodeFound == 1){
		puts("Worker Deregistered");
		return 1;
	}else{
		puts("Worker Not Registered");
		return -1;
	}
}

struct workerNode* scanListWorker(){

	int minLoad = -1;
	struct workerNode *ptr = head;
	struct workerNode *result = NULL;

	if(ptr == NULL){
		puts("No Worker Found In List");
		return NULL;
	}


	while(ptr != NULL){
		if(minLoad == -1){
			minLoad = ptr->load;
			result = ptr;
		}else{
			if(ptr->load < minLoad){
				minLoad = ptr->load;
				result = ptr;
			}
		}

		ptr = ptr->next;
	}

	// Increase load counter for worker
	if(result != NULL){
		int temp = result->load;
		temp = temp + 1;
		result->load = temp;
	}

	return result;
}

int locker(int operation, int ip[], int port){

	if(mutex == 1){
		while(mutex == 1){
			// wait
		}

		// Lock for use
		mutex = 1;
	}

	switch(operation){
	case 0:
		// Add to list - Register worker
		if(addToList(ip, port) < 0){
			// Release Lock
			mutex = 0;
			return -1;
		}
		break;
	case 1:
		// Remove for list - Deregister worker
		if(scanListWorkerDR(ip, port) < 0){
			// Release Lock
			mutex = 0;
			return -1;
		}
		break;
	case 2:
		// Scan list to check for duplicate workers
		if(scanListworkerR(ip, port) < 0){
			// Release Lock
			mutex = 0;
			return -1;
		}
		break;
	default:
		// Invalid op
		puts("Invalid Operation");
		// Release Lock
		mutex = 0;
		return -1;
	}

	// Release Lock
	mutex = 0;
	return 1;
}

struct workerNode* lockerForWorkerLookup(){
	// Need to check lock before scanning
	if(mutex == 1){
		while(mutex == 0){
			// Wait
		}

		// Lock it for use
		mutex = 1;
	}

	struct workerNode *ptr = scanListWorker();

	// Release Lock
	mutex = 0;

	return ptr;
}

int sendAck(int sock)
{
	int ackValue = 1;

	if(send(sock, &ackValue, sizeof(int), 0) > 0){
		puts("Ack Sent");
		return 1;
	}else{
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
				puts("Got Ack");
				return 1;
			}else{
				puts("Send Failed");
				return -1;
			}
		}
	}
}

int registerWorker(int ipArr[], int port){
	//Register the server and Check for duplicate
	if(locker(2, ipArr, port) < 0){
		puts("Duplicate: Existing IP and Port Address");
		return -1;
	}else{
		if(locker(0, ipArr, port) < 0){
			puts("Creating Worker Node Failed");
			return -1;
		}
	}

	//Successful
	puts("Worker Registered");
	return 1;
}

int deRegisterWorker(int ipArr[], int port){
	//Deregister Worker
	if(locker(1, ipArr, port) < 0){
		puts("Deregister Failed");
		return -1;
	}else{
		puts("Deregister Success");
		return 1;
	}
}

int workerStatus(char ip_addr[], int port){
	int sock;
	struct sockaddr_in server;

	//Create socket
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1) {
		puts("Could not create socket");
		return -1;
	}
	puts("Socket created");

	//Now setup Worker connection
	server.sin_addr.s_addr = inet_addr(ip_addr);
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	//Connect to Worker
	puts("Trying to connect to Worker");
	int connected = 1;
	while(connected == 1){
		connected = connect(sock , (struct sockaddr *)&server , sizeof(server));
	}

	if(connected == 0){
		//Connected to Worker
		puts("Connected");

		// Inform the Worker it was a ping request
		if(sendInt(sock, 3) < 0){
			puts("Ping request failed");
			return -1;
		}

		// Connection verified, close it
		close(sock);
		return 1;
	}else{
		// Worker is not available
		return -1;
	}
}

int runWorkerLookup(int sock){
	//Look for the worker with least load
	//Find the worker and send the IP and Port to the tiny google server
	struct workerNode *ptr = lockerForWorkerLookup();

	int requestResult;
	if(ptr == NULL){
		//Send Worker not Found
		requestResult = 0;
		if(send(sock, &requestResult, sizeof(requestResult), 0) < 0){
			puts("Send Query result Failed");
			return -1;
		}

		puts("No Worker Found");
		return -1;
	}else{

		//Convert ip to string
		char address[50];
		sprintf(address, "%d.%d.%d.%d", ptr->ip[0], ptr->ip[1], ptr->ip[2], ptr->ip[3]);

		// Worker found, make sure it is still alive by pinging it
		if(workerStatus(address, ptr->port) < 0){
			// Worker not present, deregister it and look for another one
			locker(1, ptr->ip, ptr->port);
			runWorkerLookup(sock);
		}

		//Send Worker Found
		requestResult = 1;
		if(send(sock, &requestResult, sizeof(requestResult), 0) < 0){
			puts("Send Query result Failed");
			return -1;
		}

		printf("Result Sent = %d\n",requestResult);

		//Wait for ACK
		if(waitForAck(sock) < 0){
			puts("ACK not received");
			return -1;
		}

		//Send the IP and Port
		if(send(sock, ptr->ip, sizeof(int)*4, 0) < 0){
			puts("Send IP Failed");
			return -1;
		}

		puts("IP sent");

		//Wait for ACK
		if(waitForAck(sock) < 0){
			puts("ACK not received");
			return -1;
		}

		//Send Port number
		if(sendInt(sock, ptr->port) < 0){
			puts("Send Port Failed");
			return -1;
		}

		puts("Port Sent");
	}

	//Successful
	puts("Server got Worker");
	return 1;
}

int runWorkerSetup(int sock, int ipArr[]){
	//Get Server and the Service to Register/Deregister
	int requestType, statusOfRead, statusOfSend, port;

	//Get type of request, register/deregister
	while(1){
		statusOfRead = recv(sock , &requestType , sizeof(requestType) , 0);
		if(statusOfRead > 0){
			break;
		}else{
			puts("Error receiving register/deregister worker request type");
			return -1;
		}
	}

	printf("Got Request %d",requestType);

	//Send Ack, to be ready fo next message
	if(sendAck(sock) < 0){
		puts("Sending Ack Failed");
		return -1;
	}

	puts("Switching on Request");
	int processStatus;

	//Now switch based on Request Type
	switch(requestType){
		case 0:
			puts("Deregister : Waiting for port");

			//Wait for Port
			while(1){
				statusOfRead = recv(sock , &port , sizeof(port) , 0);
				if(statusOfRead > 0){
					break;
				}else{
					puts("Deregister : Error receiving port number");
					return -1;
				}
			}

			//Send Ack, to be ready fo next message
			if(sendAck(sock) < 0){
				puts("Sending Ack Failed");
				return -1;
			}

			if(deRegisterWorker(ipArr, port) < 0){
				processStatus = 0;
				statusOfSend = send(sock, &processStatus, sizeof(processStatus), 0);
				if(statusOfSend < 0){
					puts("Send processStatus Failed");
					return -1;
				}
				puts("Deregister Server failed");
				return -1;
			}else{
				processStatus = 1;
				statusOfSend = send(sock, &processStatus, sizeof(processStatus), 0);
				if(statusOfSend < 0){
					puts("Send processStatus Failed");
				}
			}
		break;
		case 1:
			puts("Register : Waiting for port");

			//Wait for Port
			while(1){
				statusOfRead = recv(sock , &port , sizeof(port) , 0);
				if(statusOfRead > 0){
					break;
				}else{
					puts("Register : Error receiving port number");
					return -1;
				}
			}

			//Send Ack, to be ready fo next message
			if(sendAck(sock) < 0){
				puts("Sending Ack Failed");
				return -1;
			}

			if(registerWorker(ipArr, port) < 0){
				processStatus = 0;
				statusOfSend = send(sock, &processStatus, sizeof(processStatus), 0);
				if(statusOfSend < 0){
					puts("Send processStatus Failed");
					return -1;
				}
				puts("Register Server failed");
				return -1;
			}else{
				processStatus = 1;
				statusOfSend = send(sock, &processStatus, sizeof(processStatus), 0);
				if(statusOfSend < 0){
					puts("Send processStatus Failed");
					return -1;
				}
			}
		break;
		case 2:
			// Unload the server after the operation
			break;
		default:
			puts("Invalid Worker Directory Request");
			return -1;
	}

	//Successful
	return 1;
}

void runWorkerPing(){
	struct workerNode *ptr = head;

	while(ptr != NULL){

		// Get IP and Port and Ping the worker
		//Convert ip to string
		char address[50];
		sprintf(address, "%d.%d.%d.%d", ptr->ip[0], ptr->ip[1], ptr->ip[2], ptr->ip[3]);

		if(workerStatus(address, ptr->port) < 0) {
			// Deregister the worker
			locker(1, ptr->ip, ptr->port);
		}

		ptr = ptr->next;
	}

}

/*
 * This will handle connection for each client/server
 * */
void *connection_handler(void *args)
{
	//check whether client or server
	int type, sock, i;
	int ipArr[4];
	threadArgs *initArgs = args;

	//Assign values
	printf("Quick printing ip \n");
	sock = initArgs->sock;
	for(i = 0; i < 4; i++){
		ipArr[i] = initArgs->ip[i];
	}

	puts("Connection Started");
	while(1){
		if(recv(sock , &type , sizeof(type) , 0) > 0){
			break;
		}else{
			puts("Received Failed - Connection Terminated");
			return 0;
		}
	}

	printf("Got Type Worker/Server = %d\n", type);

	//Send Ack, to be ready for next message
	if(sendAck(sock) < 0){
		puts("Sending Ack Failed");
		//Free the socket pointer
		free(initArgs);
		return 0;
	}

	switch(type){
		case 0:
			puts("Calling Worker Lookup By Mini Google Server");
			//Call Worker Lookup
			if(runWorkerLookup(sock) < 0){
				puts("Lookup Failure - Connection Terminated");
			}

			//Free the socket pointer
			free(initArgs);
			return 0;
		break;
		case 1:
			puts("Calling Worker Setup");
			//Call Worker setup
			if(runWorkerSetup(sock, ipArr) < 0){
				puts("Register/Unregister Failure - Connection Terminated");
			}
			//Free the socket pointer
			free(initArgs);
			return 0;
		break;
		default:
			//Invalid Value
			puts("Unknown Request - Connection Terminated");
			//Free the socket pointer
			free(initArgs);
			return 0;
	}
}

void* pingFunction(void *args){
	// Continue pinging till the directory is running
	while(1){
		runWorkerPing();

		// Wait for 5 seconds
		time_t startTime = time(NULL); // return current time in seconds
		while (time(NULL) - startTime < 10) {
		   // Wait
		}
	}
}

int main(){

	int i;
	const char delim[1] = ".";
	char address[90];
	char *token;
	int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in directory , client;

	//Initialize the serverList
	head = NULL;

	// Start Thread to ping for presence of servers
	pthread_t pingThread;
	int statusOfPingThread = pthread_create(&pingThread, NULL, pingFunction, NULL);
	if(statusOfPingThread == 0)
	{
		 printf("Ping Thread created successfully \n");
	}
	else
	{
		printf("Ping Thread creation failed \n");
	}

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        puts("Could not create socket");
    }
    puts("Worker Directory Service Socket created");

    //Prepare the sockaddr_in structure
    directory.sin_family = AF_INET;
    directory.sin_addr.s_addr = inet_addr(WorkerDirectoryIP);
    directory.sin_port = htons(WorkerDirectoryPort);

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&directory , sizeof(directory)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

    //Listen
    listen(socket_desc , 10);

    //Accept incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");

		//Initialize argument structure
		threadArgs *args = malloc(sizeof *args);

		//Get IP Address of Worker / Tiny Google Server
	  	sprintf(address, "%s", inet_ntoa(client.sin_addr));
		token = strtok(address, delim);
		i = 0;
		while(token != NULL){
			sscanf(token, "%d", &args->ip[i]);
			i = i + 1;
			token = strtok(NULL, delim);
		}

        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;

		args->sock = *new_sock;

        if(pthread_create( &sniffer_thread , NULL ,  connection_handler , args) < 0)
        {
			free(args);
            perror("could not create thread");
            return 1;
        }

        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( sniffer_thread , NULL);
        puts("Handler assigned");

		puts("Waiting for incoming connections...");
    }

    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }

    return 0;
}

