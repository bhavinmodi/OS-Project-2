#include "uthash.h"
#include <string.h>
#include <stdlib.h>  /* malloc */
#include <stdio.h>   /* printf */

#include "filelochash.h"

struct filelochash {
    char* fileName;             /* key (string is WITHIN the structure) */
    int portNo;
	char* ip;
    UT_hash_handle hh;         /* makes this structure hashable */
};

struct filelochash *s3, *tmp, *hashforfileloc = NULL;

int maxFileNameSize = 50;
int ipSize = 50;

struct filelochash* createFileLocHash()
{
	struct filelochash *dest;
	dest = (struct filelochash*)malloc(sizeof(struct filelochash));
	dest->fileName = (char*)malloc(sizeof(char)*maxFileNameSize);
	dest->ip = (char*)malloc(sizeof(char)*ipSize);
	return dest;
}

void hashFileAndPort(char filename[], char ip[], int portNumber)
{
	HASH_FIND_STR(hashforfileloc, filename, s3);
	if(s3!=NULL)
	{
	s3->portNo = portNumber;
	strncpy(s3->ip,ip,ipSize);
	}
	else
	{
	s3 = createFileLocHash();
	strncpy(s3->fileName,filename,maxFileNameSize);
	s3->portNo = portNumber;
	strncpy(s3->ip,ip,ipSize);
	HASH_ADD_STR( hashforfileloc, fileName, s3 );
	}

}

int findFileLocPort(char fileLocToFind[])
{
    HASH_FIND_STR(hashforfileloc, fileLocToFind, s3);
    if (s3)
    {
		return s3->portNo;
    }
    else
    {
    	printf("File Name not found in hash \n");
		return -1;
    }
}

char* findFileLocIP(char fileLocToFind[])
{
    HASH_FIND_STR(hashforfileloc, fileLocToFind, s3);
    if (s3)
    {
		return s3->ip;
    }
    else
    {
    	printf("File Name not found in hash \n");
		return NULL;
    }
}

int checkIfFileExists(char fileLocToFind[])
{
	    HASH_FIND_STR(hashforfileloc, fileLocToFind, s3);
    if (s3)
    {
		return 1;
    }
    else
    {
		return 0;
    }
}

void fileLocHashIterate()
{
    struct filelochash *s;

    for(s=hashforfileloc; s != NULL; s=s->hh.next) {
        printf("File is %s :\n",s->fileName);
		printf("Port is %d :\n",s->portNo);
		printf("IP is %s :\n",s->ip);
    }
}

void freeFileLocHash()
{
	HASH_ITER(hh, hashforfileloc, s3, tmp)
	{
      HASH_DEL(hashforfileloc, s3);
	  free(s3->fileName);
      free(s3);
    }
}
