#include "uthash.h"
#include <string.h>
#include <stdlib.h>  /* malloc */
#include <stdio.h>   /* printf */

#include "filelochash.h"

struct filelochash {
    char* fileName;             /* key (string is WITHIN the structure) */
    int portNo;
    UT_hash_handle hh;         /* makes this structure hashable */
};

struct filelochash *s3, *tmp, *hashforfileloc = NULL;

int maxFileNameSize = 50;

struct filelochash* createFileLocHash()
{
	struct filelochash *dest;
	dest = (struct filelochash*)malloc(sizeof(struct filelochash));
	dest->fileName = (char*)malloc(sizeof(char)*maxFileNameSize);
	return dest;
}

void hashFileAndPort(char filename[], int portNumber)
{
	HASH_FIND_STR(hashforfileloc, filename, s3);
	if(s3!=NULL)
	{
	s3->portNo = portNumber;
	}
	else
	{
	s3 = createFileLocHash();
	strncpy(s3->fileName,filename,maxFileNameSize);
	s3->portNo = portNumber;
	HASH_ADD_STR( hashforfileloc, fileName, s3 );
	}

}

int findFileLoc(char fileLocToFind[])
{
    HASH_FIND_STR(hashforfileloc, fileLocToFind, s3);
    if (s3)
    {
		return s3->portNo;
    }
    else
    {
    	printf("Word not found in hash \n");
		return 0;
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
