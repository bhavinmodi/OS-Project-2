
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

// Client Directory address
#define directoryIP "127.0.0.1"
#define directoryPort 8001

// For mutex
pthread_mutex_t lock;

typedef struct{
	int sock;
	int ip[4];
}threadArgs;

struct serverNode
{
	int ip[4];
	int port;
	int load;
	struct serverNode *next;
}*head, *curr;

int addToList(int ipArr[], int port){
	int i;

	struct serverNode *ptr = (struct serverNode*)malloc(sizeof(struct serverNode));

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

int scanListServerR(int ipArr[], int port){

	struct serverNode *ptr = head;
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

int scanListServerDR(int ipArr[], int port){

	struct serverNode *ptr = head;
	struct serverNode *follow = head;

	int nodeFound = 0, i, temp = 0;

	while(ptr != NULL){
		// Check Port
		if(ptr->port == port){
			//check IP
			int noMatch = 0;
			for(i = 0; i < 4; i++){
				if(ptr->ip[i] != ipArr[i]){
					noMatch = 1;
				}
			}

			printf("Quick printing ip \n");
			for(i = 0; i < 4; i++){
				printf("ip[%d] = %d\n",i,ptr->ip[i]);
				printf("ipArr[%d] = %d\n",i,ipArr[i]);
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
		puts("Server Deregistered");
		return 1;
	}else{
		puts("Server Not Registered");
		return -1;
	}
}

struct serverNode* scanListClient(){

	int minLoad = -1;
	struct serverNode *ptr = head;
	struct serverNode *result = NULL;

	if(ptr == NULL){
		puts("No Server Found In List");
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

	// Increase load counter for server
	if(result != NULL){
		int temp = result->load;
		temp = temp + 1;
		result->load = temp;
	}

	return result;
}

int locker(int operation, int ip[], int port){

	pthread_mutex_lock(&lock);

	switch(operation){
	case 0:
		// Add to list - Register server
		if(addToList(ip, port) < 0){
			// Release Lock
			pthread_mutex_unlock(&lock);
			return -1;
		}
		break;
	case 1:
		// Remove for list - Deregister server
		if(scanListServerDR(ip, port) < 0){
			// Release Lock
			pthread_mutex_unlock(&lock);
			return -1;
		}
		break;
	case 2:
		// Scan list to check for duplicate servers
		if(scanListServerR(ip, port) < 0){
			// Release Lock
			pthread_mutex_unlock(&lock);
			return -1;
		}
		break;
	default:
		// Invalid op
		puts("Invalid Operation");
		// Release Lock
		pthread_mutex_unlock(&lock);
		return -1;
	}

	// Release Lock
	pthread_mutex_unlock(&lock);
	return 1;
}

struct serverNode* lockerForServerLookup(){
	// Need to check lock before scanning
	pthread_mutex_lock(&lock);

	struct serverNode *ptr = scanListClient();

	// Release Lock
	pthread_mutex_unlock(&lock);

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

int readInt(int sock, int* a){
	int ackValue =1 ;
	while(1){
		if(recv(sock , a , sizeof(int),0) < 0){
			puts("Receive Failed");
			return -1;
		}else{
			break;
		}
	}

	if(send(sock , &ackValue , sizeof(int) , 0) < 0){
		puts("Receive Failed");
		return -1;
	}
	return 1;
}

int registerServer(int ipArr[], int port){
	//Register the server and Check for duplicate

	int i = 0;
	printf("Quick printing ip \n");
	for(i = 0; i < 4; i++){
		//printf("ip[%d] = %d\n",i,ptr->ip[i]);
		printf("Register : ipArr[%d] = %d\n",i,ipArr[i]);
	}

	if(locker(2, ipArr, port) < 0){
		puts("Duplicate: Existing IP and Port Address");
		return -1;
	}else{
		if(locker(0, ipArr, port) < 0){
			puts("Creating Sever Node Failed");
			return -1;
		}
	}

	//Successful
	puts("Server Registered");
	return 1;
}

int deRegisterServer(int ipArr[], int port){
	//Deregister Server
	if(locker(1, ipArr, port) < 0){
		puts("Deregister Failed");
		return -1;
	}else{
		puts("Deregister Success");
		return 1;
	}
}

int serverStatus(char ip_addr[], int port){
	int sock;
	struct sockaddr_in server;

	//Create socket
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1) {
		puts("Could not create socket");
		return -1;
	}
	puts("Socket created");

	//Now setup Server connection
	server.sin_addr.s_addr = inet_addr(ip_addr);
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	//Connect to Remote Server
	puts("Trying to connect to Remote Server");
	int connected = 1;
	while(connected == 1){
		connected = connect(sock , (struct sockaddr *)&server , sizeof(server));
	}

	if(connected == 0){
		//Connected to Remote Server
		puts("Connected");

		// Inform you are the directory and it was a ping request
		if(sendInt(sock, 3) < 0){
			puts("Ping request failed");
			return -1;
		}

		// Connection verified, close it
		close(sock);
		return 1;
	}else{
		// server is not available
		return -1;
	}
}

void runServerPing(){
	struct serverNode *ptr = head;

	while(ptr != NULL){

		// Get IP and Port and Ping the server
		//Convert ip to string
		char address[50];
		sprintf(address, "%d.%d.%d.%d", ptr->ip[0], ptr->ip[1], ptr->ip[2], ptr->ip[3]);

		if(serverStatus(address, ptr->port) < 0) {
			// Deregister the server
			locker(1, ptr->ip, ptr->port);
		}

		ptr = ptr->next;
	}

}

int runClientSetup(int sock){
	//Look for the server with the requested service
	//Find the server and send the IP and Port to the client
	struct serverNode *ptr = lockerForServerLookup();

	int requestResult;
	if(ptr == NULL){
		//Send Server not Found
		requestResult = 0;
		if(send(sock, &requestResult, sizeof(requestResult), 0) < 0){
			puts("Send Query result Failed");
			return -1;
		}

		puts("No Server Found");
		return -1;
	}else{

		//Send Server Found
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
	puts("Client got server");
	return 1;
}

int runServerSetup(int sock, int ipArr[]){
	//Get Server and the Service to Register/Deregister
	int requestType, statusOfRead, statusOfSend, port;

	//Get type of request, register/deregister
	while(1){
		statusOfRead = recv(sock , &requestType , sizeof(requestType) , 0);
		if(statusOfRead > 0){
			break;
		}else{
			puts("Error receiving register/deregister request type");
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
			puts("Waiting for port");

			//Wait for Port
			while(1){
				statusOfRead = recv(sock , &port , sizeof(port) , 0);
				if(statusOfRead > 0){
					break;
				}else{
					puts("Error receiving port number");
					return -1;
				}
			}

			//Send Ack, to be ready fo next message
			if(sendAck(sock) < 0){
				puts("Sending Ack Failed");
				return -1;
			}

			if(deRegisterServer(ipArr, port) < 0){
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
			puts("Waiting for port");

			//Wait for Port
			while(1){
				statusOfRead = recv(sock , &port , sizeof(port) , 0);
				if(statusOfRead > 0){
					break;
				}else{
					puts("Error receiving port number");
					return -1;
				}
			}

			//Send Ack, to be ready fo next message
			if(sendAck(sock) < 0){
				puts("Sending Ack Failed");
				return -1;
			}

			if(registerServer(ipArr, port) < 0){
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
		default:
			puts("Invalid Server Request");
			return -1;
	}

	//Successful
	return 1;
}

/*
 * This will handle connection for each client/server
 */
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
		printf("ipArr[%d] = %d\n",i,ipArr[i]);
	}

	puts("Connection Started");
	if(readInt(sock, &type) < 0){
		puts("Failed to get type");
		//Free the socket pointer
		free(initArgs);
		close(sock);
		return 0;
	}

	printf("Got Type Client | Worker | Server = %d\n", type);

	switch(type){
		case 0:
		{
			puts("Calling Client | Worker");
			// Tell type of request
			int request = -1;
			if(readInt(sock, &request) < 0){
				puts("Client : Failed Receiving Request");
			}

			if(request != -1){
				if(request == 0){
					// Call server present ping
					puts("Client : Server Checker Ping called");
					runServerPing();
				}else{
					if(request == 1){
						// Call server lookup
						if(runClientSetup(sock) < 0){
							puts("Lookup Failure - Connection Terminated");
						}
					}else{
						puts("Client : Invalid Request");
					}
				}
			}

		}
		break;
		case 1:
			puts("Calling Server");
			//Call Server
			if(runServerSetup(sock, ipArr) < 0){
				puts("Register/Unregister Failure - Connection Terminated");
			}
		break;
		default:
			//Invalid Value
			puts("Unknown Request - Connection Terminated");
	}

	//Free the socket pointer
	close(sock);
	free(initArgs);
	return 0;
}

void* pingFunction(void *args){
	// Continue pinging till the directory is running
	while(1){
		runServerPing();

		// Wait for 10 seconds
		time_t startTime = time(NULL); // return current time in seconds
		while (time(NULL) - startTime < 10) {
		   // Wait
		}
	}
}

int main(){

	if (pthread_mutex_init(&lock, NULL) != 0){
		printf("\n mutex init failed\n");
		return 1;
	}

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
    puts("Directory Service Socket created");

    //Prepare the sockaddr_in structure
    directory.sin_family = AF_INET;
    directory.sin_addr.s_addr = inet_addr(directoryIP);
    directory.sin_port = htons(directoryPort);

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

		//Get IP Address of Server/Client
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
        pthread_join( sniffer_thread , NULL);

        puts("Handler assigned");

		puts("Waiting for incoming connections...");
    }

    pthread_mutex_destroy(&lock);

    puts("Exiting");

    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }

    return 0;
}

