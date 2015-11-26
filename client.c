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
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<dirent.h>

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

int sendFileToServer(int sock, char path[100], char fileName[100]){

	// Entire file path
	char wholePath[200];
	strcpy(wholePath, path);
	strcat(wholePath, "\\");
	strcat(wholePath, fileName);

	printf("Whole File Path = %s\n",wholePath);

	//Sending 1KB at a time
	char fileContents[1024];

	// Find out file size
	int size;
	struct stat s;

	if (stat(wholePath, &s) == 0){
		size = s.st_size;
	}

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

int clientInput(int sock){

	int choice;
	char path[100], fileName[100];

	// Print Menu for selecting Indexing or Search Query
	printf("Select Job:\n1. Indexing\n2. Search\n3. Exit\nChoice: ");
	scanf(" %d", &choice);

	switch(choice){
	case 1:
	{
		// Indexing

		//Step 1 : Inform the server of the type of request
		if(sendInt(sock, 1) < 0){
			puts("Indexing Request Failed : Send request type failed");
			return -1;
		}

		//Step 2 : Ask user for input directory with the files to index
		//flushing input stream
		int c;
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
						}
					}

					// Inform the server there are no more files to send
					if(sendInt(sock, -1) < 0){
						puts("Sending File not present failed");
					}

					(void) closedir (dp);
				}else{
					perror ("Couldn't open the directory");
				}

			}else{
				if(S_ISREG(s.st_mode)){
					puts("Regular File");

					// Send file

				}else{
					puts("Invalid Path");
				}
			}
		}else{
			puts("Failed to determine directory or file");
		}

		break;
	}
	case 2:
		// Search
		break;
	case 3:
		//Exit
		return -1;
	default:
		// Invalid Option, wait for a valid response
		puts("Invalid Option");
		return 0;
	}

	// Operation Complete, go back to menu
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
		getchar();
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
		if(clientInput(sock) < 0){
			break;
		}
	}

	puts("Connection terminated.");
	close(sock);
	return 0;
}



