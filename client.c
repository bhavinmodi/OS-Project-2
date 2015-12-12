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
#include<time.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<dirent.h>
#include<libgen.h>

// Server Directory address
#define ServerDirectoryIP "127.0.0.1"
#define ServerDirectoryPort 8001

int sendAck(int sock)
{
	int ackValue = 1;

	if(send(sock, &ackValue, sizeof(int), 0) > 0){
		//puts("Ack Sent");
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
			//puts("Got ACK");
			//Got ACK
			break;
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
				//puts("Got Ack");
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
			puts("Send connector type client failed");
			close(sock);
			return -1;
		}

		// Tell the directory there the reason for connecting
		if(sendInt(sock, 1) < 1){
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

int informDirectoryServerMaybeAbsent(){
	// Initialize variables
	int sock;
	struct sockaddr_in server;

	// Create socket
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
	int connected = 1;
	while(connected == 1){
		connected = connect(sock , (struct sockaddr *)&server , sizeof(server));
	}

	if(connected == 0){
		//Connected to Directory Service
		puts("Connected");
	}else{
		puts("Server Directory Connection failed");
		return -1;
	}

	//Let Directory know you are the client
	if(sendInt(sock, 0) < 0){
		puts("Send connector type client failed");
		close(sock);
		return -1;
	}

	// Tell the directory there is problem connecting to the server
	if(sendInt(sock, 0) < 1){
		puts("Send connector type client failed");
		close(sock);
		return -1;
	}

	//Close connection to Directory Service
	close(sock);
	return 1;
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

		// Inform the server you are the client
		if(sendInt(sock, 1) < 0){
			puts("Sending connector type to server failed");
			return -1;
		}

		return sock;
	}else{
		printf("Connection Failed : Return Value %d\n",connected);

		// Let the directory know that the server is not available
		informDirectoryServerMaybeAbsent();
		return -1;
	}

}

int sendFileToServer(int sock, char path[100], char fileName[100]){

	// Entire file path
	char wholePath[200];

	strcpy(wholePath, path);
	strcat(wholePath, "\\");
	strcat(wholePath, fileName);

	printf("Whole File Path = %s\n",wholePath);

	//Sending 1 KB at a time
	char fileContents[1024];

	// Find out file size
	int size;
	struct stat s;

	int status = stat(wholePath, &s);

	if(status < 0){
		// stat failed
		puts("No Such File or Directory");

		// Send incorrect file details stopper as status -1
		if(sendInt(sock, -1) < 0){
			puts("Incorrect file details sent as status -1 :  Failed to send");
		}
		return -1;
	}else{
		if(sendInt(sock, 1) < 0){
			puts("Incorrect file details sent as status 1 :  Failed to send");
		}
	}

	size = s.st_size;

	printf("File Size = %d\n",size);

	// Open file
	FILE *fp;

	fp = fopen(wholePath, "r");

	if(fp == NULL){
		printf("%s : Error opening file\n",fileName);
		return -1;
	}

	//Send FileName
	if(send(sock , &fileName[0] , sizeof(char)*100 , 0) < 0){
		puts("Send File Name Failed");
		return -1;
	}

	printf("File Name Sent = %s\n",fileName);

	if(waitForAck(sock) < 0){
		return -1;
	}

	// Wait for server response for file already exists
	int fileExists;

	if(readInt(sock, &fileExists) < 0){
		puts("Failed to read file exists from server");
		return -1;
	}

	if(fileExists < 0){
		// File already exists, do not send, but do consider this to be an error
		puts("File already Indexed | Will not be indexed again");
		return 1;
	}

	//Send File size
	if(sendInt(sock, size) < 0){
		puts("Send File Size Failed");
		return -1;
	}

	puts("File Size Sent");

	// Read file
	while(fgets(fileContents, 1024, (FILE*)fp) != NULL){
		//Sending maximum 1 KB of the file (Can be less depending of bytes in the line)
		if(send(sock , &fileContents , sizeof(char)*1024 , 0) < 0){
			puts("Send Failed");
			return -1;
		}

		// Wait for Ack
		if(waitForAck(sock) < 0){
			puts("Failed receiving Ack when sending the file");
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

int waitForSearchResult(int sock){

	// Get result of search from server
	int sizeOfResult;
	if(readInt(sock, &sizeOfResult) < 0){
		puts("Failed to receive size of result");
		return -1;
	}

	// Get Result and Display
	char result[sizeOfResult];
	if(readString(sock, sizeOfResult, &result[0]) < 0){
		puts("Failed to receive result array.");
		return -1;
	}

	printf("Result is:\n %s\n",result);

	// Select which file you want from the result
	char c;

	//flushing input stream
	while((c = getchar()) != '\n' && c != EOF);

	char answer[100];
	printf("Which File do you want to retrieve: ");
	fgets(answer, sizeof(answer), stdin);

	int index;
	for(index = 0; index < 100; index++){
		if(answer[index] == '\n'){
			answer[index] = '\0';
			break;
		}
	}

	// Send the file to retrieve
	if(sendString(sock, 100, &answer[0]) < 0){
		puts("Sending Retrieve request failed.");
		return -1;
	}

	// Receive the file size
	int fileSize = 0;
	// Get the file size
	if(readInt(sock, &fileSize) < 0){
		puts("Receiving file size from server failed.");
		return -1;
	}

	printf("File Size = %d\n",fileSize);

	// Initialize file
	FILE *fp;
	fp = fopen(answer, "w+");

	if(fp == NULL){
		printf("%s : Error opening file",answer);
		return -1;
	}

	// Get File
	int bytesRead = 0;
	char buffer[1024];

	while(bytesRead < fileSize){

		// Read the first 1 KB
		if(readString(sock, 1024, &buffer[0]) < 0){
			puts("Failed receiving file contents.");
			return -1;
		}else{
			//Write to file
			fputs(buffer, fp);
		}

		// Add bytes read from length of buffer
		bytesRead = bytesRead + strlen(buffer);

		//printf("Length of buffer read for file = %d\n",bytesRead);
	}

	if(sendAck(sock) < 0){
		return -1;
	}

	puts("Got the file from server\n");

	// Close the file
	fclose(fp);

	return 1;
}

int clientInput(int sock){

	int choice = -1;
	char path[100], fileName[100];
	char keywords[100];
	int c;

	// Print Menu for selecting Indexing or Search Query
	printf("Select Job:\n1. Indexing\n2. Search\n3. Exit\nChoice: ");

	char line[2];
	if (fgets(line, sizeof(line), stdin)) {
	    sscanf(line, "%d", &choice);
	}

	switch(choice){
	case 1:
	{
		// Indexing

		// Re-initialize choice
		choice = 0;

		//Step 1 : Inform the server of the type of request
		if(sendInt(sock, 1) < 0){
			puts("Indexing Request Failed : Send request type failed");
			return -1;
		}

		//Step 2 : Ask user for input directory with the files to index
		//flushing input stream
		while((c = getchar()) != '\n' && c != EOF);

		puts("Enter the directory path with the files you want to index : ");
		fgets(path, 100, stdin);

		struct stat s;

		int index;
		for(index = 0; index < 100; index++){
			if(path[index] == '\n'){
				path[index] = '\0';
				break;
			}
		}

		printf("Path is = %s\n",path);

		if (stat(path, &s) == 0){
			if(S_ISDIR(s.st_mode)){
				puts("Directory");

				// Check directory for files, we only send .txt files
				DIR *dp;
				struct dirent *ep;
				dp = opendir (path);

				if (dp != NULL){
					while ((ep = readdir (dp)) != NULL){
						strcpy(fileName, ep->d_name);

						// If it is a .txt file, then send it over
						if(strstr(fileName, ".txt")){
							// Inform the server if there is a file to send
							if(sendInt(sock, 1) < 0){
								puts("Sending File present failed");
								break;
							}

							if(sendFileToServer(sock, path, fileName) < 0){
								printf("%s : Send Failed\n",fileName);
								break;
							}else{
								printf("%s : Sent\n",fileName);
							}

							// Wait for status of server after file sent
							if(readInt(sock, &choice) < 0){
								puts("Waiting for status of file response DIR failed");
							}

							if(choice < 0){
								puts("Indexing Failure");
								break;
							}
						}
					}

					if(choice >= 0){
						// Inform the server there are no more files to send
						if(sendInt(sock, -1) < 0){
							puts("Sending File not present failed");
						}
					}

					(void) closedir (dp);
				}else{
					perror ("Couldn't open the directory");
					// Inform the server there was error in opening directory
					if(sendInt(sock, -2) < 0){
						puts("Sending error in opening directory failed");
					}
				}

			}else{
				if(S_ISREG(s.st_mode)){
					puts("Regular File");

					// Inform the server if there is a file to send
					if(sendInt(sock, 1) < 0){
						puts("Sending File present failed");
						break;
					}

					// Extracting file and directory from path
					char *bname = basename(path);
					char *dname = dirname(path);

					strcpy(fileName, bname);
					strcpy(path, dname);

					printf("FileName = %s, DirName = %s\n",fileName, path);

					if(sendFileToServer(sock, path, fileName) < 0){
						printf("%s : Send Failed\n",fileName);
						break;
					}else{
						printf("%s : Sent\n",fileName);
					}

					// Wait for status of server after file sent
					if(readInt(sock, &choice) < 0){
						puts("Waiting for status of file response REG failed");
					}

					if(choice  < 0){
						puts("Indexing Failure | Try Again");
					}else{
						puts("Indexing Success");
						// Inform the server there are no more files to send
						if(sendInt(sock, -1) < 0){
							puts("Sending File not present failed");
						}
					}
				}else{
					puts("Invalid Path");

					// Inform the server that the REG file path was invalid
					if(sendInt(sock, -2) < 0){
						puts("Sending Invalid Path :  failed");
					}
				}
			}
		}else{
			puts("Failed to determine directory or file");
			// Inform the server there was a error in the file path
			if(sendInt(sock, -2) < 0){
				puts("Sending Failed to determine directory or file :  failed");
			}
		}
		break;
	}
	case 2:
	{
		//Step 1 : Inform the server of the type of request
		if(sendInt(sock, 2) < 0){
			puts("Search Request Failed : Send request type failed");
			return -1;
		}

		// Search
		puts("Enter the keywords for searching:");

		//flushing the input stream
		while((c = getchar()) != '\n' && c != EOF);

		fgets(keywords, 100, stdin);

		//Remove the \n char for the keyword string
		int index;
		for(index = 0; index < 100; index++){
			if(keywords[index] == '\n'){
				keywords[index] = '\0';
				break;
			}
		}

		// Send the string across
		if(sendString(sock, 100, &keywords[0]) < 0){
			puts("Sending keywords failed");
		}

		// Wait for result
		if(waitForSearchResult(sock) < 0){
			puts("Search Result from server failed.");
		}

		break;
	}
	case 3:
		//Exit
		//Let the server know this is an exit request
		if(sendInt(sock, 3) < 0){
			puts("Exit Request Failed");
		}
		return -1;
	default:
		// Invalid Option, wait for a valid response
		puts("Invalid Option");

		//flushing input stream
		while((c = getchar()) != '\n' && c != EOF);

		return 0;
	}

	// Operation Complete, go back to menu
	return 1;
}

int startRetryMode(){
	// Connection to the server failed, most likely it is down

	//Send Lookup Request
	int ip[4];
	int port;

	// Get a new server
	while(1){
		if(getServerFromDirectory(&ip[0], &port) < 0){
			printf("No Server Found");
			// Try again after 1 minute
			time_t startTime = time(NULL); // return current time in seconds
			while (time(NULL) - startTime < 30) {
			   // Wait
			}
		}else{
			break;
		}
	}

	return connectToServer(ip, port);
}

int AskUserToContinue(){
	// Character used for continue message
	char response;
	char c;

	puts("Failed to connect to server | Exit Request");

	int inputAccepted = 0;
	while(inputAccepted == 0){
		printf("Continue to keep searching or exit (Y/N)?");
		scanf(" %c",&response);

		if(response == 'y' || response == 'Y'){
			//flushing input stream
			while((c = getchar()) != '\n' && c != EOF);

			return startRetryMode();
		}else{
			if(response == 'n' || response == 'N'){
				return -1;
			}else{
				puts("Invalid Choice");
			}
		}
	}

	// If it reaches here, bad input exit
	return -1;
}

int main(){

	int sock = startRetryMode();

	if(sock < 0){
		sock = AskUserToContinue();
		if(sock < 0){
			puts("Exiting");
			return 0;
		}
	}

	while(1){
		if(clientInput(sock) < 0){
			sock = AskUserToContinue();
			if(sock < 0){
				puts("Exiting | Connection terminated");
				close(sock);
				return 0;
			}
		}
	}

}



