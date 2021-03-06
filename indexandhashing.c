#include "uthash.h"
#include <string.h>
#include <stdlib.h>  /* malloc */
#include <stdio.h>   /* printf */
#define INT_DIGITS 19	

void copystring(char **first, char *second);
void copystringwithoutfree(char **first, char *second);
struct my_struct* createmy_structStruct();
struct my_struct* addValuesTomy_struct(struct my_struct *s, char word[], int noOfOccurences, char fileName[]);
struct my_struct* appendValuesTomy_struct(struct my_struct *s, char word[], int noOfOccurences, char fileName[]);
struct my_struct* createmy_structForEmptyValues();
struct localhashstruct* createLocalHashStruct();
struct node* initializeNode();
struct node* addValueToNode(struct node* n, char c[], int noOfOccurences);
void addToList(struct node* root, char c[], int noOfOccurences);
void printLinkedList(struct node* root);
void deleteLinkedList(struct node* root);
void sortLinkedList(struct node* root);
void sortAllSearchedWords(struct my_struct *arrayOfStructs[],int *arrayOfStructsCounter);
void hashWordIntoLocalHash(char c[]);
void hashWordIntoGlobalHash(char docName[], char wordBeingHashed[], int noOfOccurences);
void LocalHashIterate();
void addToGlobalHash(char docName[]);
void globalHashIterate();
void globalHashCount();
void freeLocalHash();
void freeGlobalHash();
int hashFile(char fileName[]);
int checkIfLinkedListContains(struct node* root, char docNameToCheck[]);
void findWordInHash(char c[],struct my_struct *arrayOfStructs[],int *arrayOfStructsCounter);
//void findMultipleWordsInHash(int numberOfWords);
void findMultipleWordsInHashWithSTRTOK(char string[], char **finalOutput);
void computeDocNameIntersection();
void computeDocNameIntersectionWithCharStar(char **finalOutput, struct my_struct *arrayOfStructs[],int *arrayOfStructsCounter);
void hashWordFromString(char string[]);
void initializeConversionGlobalHashToString();
void convertGlobalHashIntoString(char **);
void initializeConversionLocalHashToString();
//char * convertLocalHashIntoString(char fileName[]);
void convertLocalHashIntoString(char fileName[], char **dest2);
void sortAllSearchedWords(struct my_struct *arrayOfStructs[],int *arrayOfStructsCounter);
// void findMultipleWordsInHashNewRanker(char string[], char **finalOutput);
void computeRank(char **finalOutput, struct my_struct *arrayOfStructs[],int *arrayOfStructsCounter);
void fullPrecisionRanker(char **finalOutput, struct my_struct *arrayOfStructs[],int *arrayOfStructsCounter, int noOfMatches);

struct localhashstruct {
    char* wordBeingHashed;             /* key (string is WITHIN the structure) */
    int noOfHits;
    UT_hash_handle hh;         /* makes this structure hashable */
};

struct localhashstruct *s, *tmp, *varForTransportingLocalHash, *localhashid = NULL;

struct my_struct {
	char* wordBeingHashed;             /* key (string is WITHIN the structure) */
	struct node *root;
	UT_hash_handle hh;         /* makes this structure hashable */
};

struct my_struct *s2, *tmp2, *structtopointtoemptyvalues, *varForTransportingHash, *users = NULL;

struct node {
  int noOfOccurences;
  char* docName;
  struct node *next;
};

int sizeOfWordBeingHashed = 50; //used to set the size of the variable "wordBeingHashed"
int sizeOfDocName = 50;




//functions for itoa1

char *itoa1(i)
     int i;
{
  /* Room for INT_DIGITS digits, - and '\0' */
  static char buf[INT_DIGITS + 2];
  char *p = buf + INT_DIGITS + 1;	/* points to terminating '\0' */
  if (i >= 0) {
    do {
      *--p = '0' + (i % 10);
      i /= 10;
    } while (i != 0);
    return p;
  }
  else {			/* i < 0 */
    do {
      *--p = '0' - (i % 10);
      i /= 10;
    } while (i != 0);
    *--p = '-';
  }
  return p;
}

//function for copying strings with dynamic memory 
void copystring(char **first, char *second)
{
	//here assuming *first is a malloced thing
    char *result = malloc(strlen(*first)+strlen(second)+1);
    strcpy(result, *first);
    strcpy(result+strlen(*first), second);
	free(*first);
	*first = result;
}

void copystringwithoutfree(char **first, char *second)
{
	//here assuming *first is a malloced thing
    char *result = malloc(strlen(*first)+strlen(second)+1);
    strcpy(result, *first);
    strcpy(result+strlen(*first), second);
	*first = result;
}

//start of all functions
struct my_struct* createmy_structStruct()
{
	struct my_struct *dest;
	dest = (struct my_struct*)malloc(sizeof(struct my_struct));
	
	dest->wordBeingHashed = (char*)malloc(sizeof(char)*50);
	dest->root = initializeNode();
	return dest;
}

struct my_struct* addValuesTomy_struct(struct my_struct *s, char word[], int noOfOccurences, char fileName[])
{
	strncpy(s->wordBeingHashed,word,sizeOfWordBeingHashed);
	s->root = addValueToNode(s->root, fileName, noOfOccurences);
	return s;	
}

struct my_struct* appendValuesTomy_struct(struct my_struct *s, char word[], int noOfOccurences, char fileName[])
{
	addToList(s->root, fileName, noOfOccurences);
}

struct my_struct* createmy_structForEmptyValues()
{
	struct my_struct* structtopointtoemptyvalues = createmy_structStruct();
	strncpy(structtopointtoemptyvalues->wordBeingHashed,"EMPTY",sizeOfWordBeingHashed);
	structtopointtoemptyvalues->root = initializeNode();
	structtopointtoemptyvalues->root = addValueToNode(structtopointtoemptyvalues->root, "EMPTY", -1);
	return structtopointtoemptyvalues;
	//*finalResult = structtopointtoemptyvalues;
}

struct my_struct* deletemy_struct(struct my_struct* toDel)
{
	free(toDel->wordBeingHashed);
	free(toDel->root);
	free(toDel);
	
	return toDel;
}

struct localhashstruct* createLocalHashStruct()
{
	struct localhashstruct *dest;
	dest = (struct localhashstruct*)malloc(sizeof(struct localhashstruct));
	dest->wordBeingHashed = (char*)malloc(sizeof(char)*sizeOfWordBeingHashed);
	return dest;
}

struct node* initializeNode()
{
	struct node *result;
	result = malloc(sizeof(struct node));
	result->docName = malloc(sizeof(char)*50);
	result->next = 0;
	return result;
}

struct node* addValueToNode(struct node* n, char c[], int noOfOccurences)
{
	//printf("Docname being added is %s \n",c);
	strncpy(n->docName,c,50);
	n->noOfOccurences = noOfOccurences;
	return n;
}

void addToList(struct node* root, char c[], int noOfOccurences)
{
	//printf("c got in addtolist is %s \n",c);
	struct node *conductor;
    conductor = root; 
    if ( conductor != 0 ) {
        while ( conductor->next != 0)
        {
            conductor = conductor->next;
        }
    }
	conductor->next = initializeNode();	
    conductor = conductor->next; 

    if ( conductor == 0 )
    {
        printf( "Out of memory" );
        return ;
    }
	
	conductor->next = 0;
	conductor = addValueToNode(conductor,c,noOfOccurences);
    //conductor->noOfOccurences = noOfOccurences;
	
	//struct node n;
	// n = initializeNode();
	// n = addValueToNode(n,c,noOfOccurences);
	// return n;
}

void printLinkedList(struct node* root)
{
	struct node *conductor;
    conductor = root; 
    if ( conductor != 0 ) {
		printf("%s || %d \n",conductor->docName,conductor->noOfOccurences);
        while ( conductor->next != 0)
        {
            conductor = conductor->next;
			printf("%s || %d \n",conductor->docName,conductor->noOfOccurences);
        }
    }
}

void deleteLinkedList(struct node* root)
{
	//frees all elements of a linked list except the root
	struct node *conductor;
	struct node *nextNode;
    conductor = root; 
    if ( conductor != 0 && conductor->next != 0) {
		conductor = conductor->next;
		//printf("%s || %d \n",conductor->docName,conductor->noOfOccurences);
        while ( conductor->next != 0)
        {
            nextNode = conductor->next;
			free(conductor->docName);
			free(conductor);
			conductor = nextNode;
        }
		//to free last element
		free(conductor->docName);
		free(conductor);
    }
}

void sortLinkedList(struct node* root)
{
	struct node *conductor;
	struct node *nextConductor;
	
	char temp[sizeOfDocName];
	int tmp;
	
    conductor = root; 
    if ( conductor != 0 )
	{
		if(conductor->next!=0)
		{
			nextConductor = conductor->next;
			if(nextConductor->noOfOccurences>conductor->noOfOccurences)
			{
				tmp = conductor->noOfOccurences;
				strncpy(temp,conductor->docName,sizeOfDocName);
				
				conductor->noOfOccurences = nextConductor->noOfOccurences;
				strncpy(conductor->docName,nextConductor->docName,sizeOfDocName);
				
				nextConductor->noOfOccurences = tmp;
				strncpy(nextConductor->docName,temp,sizeOfDocName);
			}
		}
    }
}

void sortAllSearchedWords(struct my_struct *arrayOfStructs[],int *arrayOfStructsCounter)
{	
	int i=0;
	while(i<*arrayOfStructsCounter)
	{
		sortLinkedList(arrayOfStructs[i]->root);
		i++;
	}
}

void hashWordIntoLocalHash(char c[])
{
 
	int i = 0;
	HASH_FIND_STR( localhashid, c, s);
	if(s!=NULL)
	{
	s->noOfHits++;
	}
	else
	{
	s = createLocalHashStruct();
	strncpy(s->wordBeingHashed,c,sizeOfWordBeingHashed);
	s->noOfHits = 1;
	HASH_ADD_STR( localhashid, wordBeingHashed, s );
	}

}

void hashWordIntoGlobalHash(char docName[], char wordBeingHashed[], int noOfOccurences)
{
 
	int i = 0;
	HASH_FIND_STR( users, wordBeingHashed, s2);
	if(s2!=NULL)
	{
	//s2->noOfHits++;
	//s2 = addValuesTomy_struct(s2, wordBeingHashed, noOfOccurences, docName);
	s2 = appendValuesTomy_struct(s2, wordBeingHashed, noOfOccurences, docName);
	}
	else
	{
	s2 = createmy_structStruct();
	s2 = addValuesTomy_struct(s2, wordBeingHashed, noOfOccurences, docName);
	HASH_ADD_KEYPTR( hh, users, s2->wordBeingHashed, strlen(s2->wordBeingHashed), s2 );
	//HASH_ADD_STR( users, wordBeingHashed, s2 );
	}

}

void LocalHashIterate()
{
    struct localhashstruct *s;

    for(s=localhashid; s != NULL; s=s->hh.next) {
        printf("word : %s \t count : %d \n",s->wordBeingHashed,s->noOfHits);
    }
}

void addToGlobalHash(char docName[])
{
	struct localhashstruct *s;

    for(s=localhashid; s != NULL; s=s->hh.next) {
	
	hashWordIntoGlobalHash(docName, s->wordBeingHashed,s->noOfHits);
	
    }
}

void globalHashIterate()
{
    struct my_struct *s;

    for(s=users; s != NULL; s=s->hh.next) {
        printf("Word is %s :\n",s->wordBeingHashed);
		printLinkedList(s->root);
    }
}

void globalHashCount()
{
	struct my_struct *s;
	int count=0;
    for(s=users; s != NULL; s=s->hh.next) {
        //printf("Word is %s :\n",s->wordBeingHashed);
		//printLinkedList(s->root);
		count ++;
    }
	printf("Total no of distinct words in global hash is %d \n",count);
}


void freeLocalHash()
{
	HASH_ITER(hh, localhashid, s, tmp)
	{
      HASH_DEL(localhashid, s);
      free(s);
    }
}

void freeGlobalHash()
{
	HASH_ITER(hh, users, s2, tmp2)
	{
      HASH_DEL(users, s2);
	  deleteLinkedList(s2->root);
      s2 = deletemy_struct(s2);
    }
}

int hashFile(char fileName[])
{
	freeLocalHash();
	
	char fileContents[1024];
    const char delimiters[] = " $%*#@()[]&^_/|\"<>.,;:!-\n";
    
    // Open file
    FILE *fp;
    char *token;
    
    fp = fopen(fileName, "r");
    if(fp == NULL)
	{
		printf("Error opening file \n");
		return 0;
	}
	
 //New function to read file
    char x[1024];
	char *rEndPtr;
    /* assumes no word exceeds length of 1023 */
    while (fscanf(fp, " %1023s", x) == 1) {
        //puts(x);
		token = strtok_r(x, delimiters,&rEndPtr);
        while(token!=NULL)
        {
            //printf("Token : %s \n",token);
            hashWordIntoLocalHash(token);
            token = strtok_r(NULL, delimiters,&rEndPtr);
        }
    }

	
	
    // Read file
	/*
    while(fgets(fileContents, 1024, (FILE*)fp) != NULL){
        //Sending 1 KB of the file
        printf("Line : \n");
        printf("%s\n",fileContents);
        printf("Tokens : \n");
        token = strtok (fileContents, delimiters);
        while(token!=NULL)
        {
            //printf("Token : %s \n",token);
            hashWordIntoLocalHash(token);
            token = strtok (NULL, delimiters);
        }
    }
	*/
    
    //LocalHashIterate();
	//TODO : Add function to send relevant data to server
	addToGlobalHash(fileName);
	initializeConversionLocalHashToString();
	//convertLocalHashIntoString(fileName);
 
    fclose(fp);
	return 1;
}

int checkIfLinkedListContains(struct node* root, char docNameToCheck[])
{
	//this function is used to check if the linked list contains a particular docname and if yes returns occurence number
	if(root==0)
		return 0;
	
	if(strcmp(root->docName,docNameToCheck)==0)
		return root->noOfOccurences;
	
	while(root->next!=0)
	{
		root = root->next;
		if(strcmp(root->docName,docNameToCheck)==0)
			return root->noOfOccurences;
	}
	
	return 0;
}

void findWordInHash(char c[],struct my_struct *arrayOfStructs[],int *arrayOfStructsCounter)
{
    HASH_FIND_STR( users, c, s2);
    if (s2)
    {
    	// printf("Details in hash table for word: %s \n",c);
		// printLinkedList(s2->root);
		
		//int randomTest = checkIfLinkedListContains(s2->root, "bigfile.txt");
		//printf("Value of randomTest is %d \n",randomTest);
		arrayOfStructs[*arrayOfStructsCounter] = s2;
		*arrayOfStructsCounter = *arrayOfStructsCounter + 1;
		//arrayOfStructs[arrayOfStructsCounter] = createCopyStruct(s,arrayOfStructs[arrayOfStructsCounter]);
		//arrayOfStructsCounter++;
    }
    else
    {
    	//TODO : Add part for when word is not found
    	printf("Word not found in hash \n");
    	
    	// struct my_struct* structtopointtoemptyvalues;
		//structtopointtoemptyvalues = createmy_structStruct();
		//strncpy(structtopointtoemptyvalues->wordBeingHashed,"EMPTY",sizeOfWordBeingHashed);
		// int randomTest = checkIfLinkedListContains(structtopointtoemptyvalues->root, "bigfile.txt");
		// printf("Value of randomTest is %d \n",randomTest);
		
		arrayOfStructs[*arrayOfStructsCounter] = structtopointtoemptyvalues;
		*arrayOfStructsCounter = *arrayOfStructsCounter + 1;
		
    	
    }
}

/*
void findMultipleWordsInHash(int numberOfWords)
{
	arrayOfStructsCounter=0;
	char words[numberOfWords][50];
	int i=0;
	while(i<numberOfWords)
	{
		printf("Please enter word number %d \n",(i+1));
		scanf("%s",words[i]);
		findWordInHash(words[i]);
		i++;
	}
	
	char *test;
	computeDocNameIntersectionWithCharStar(&test);
	printf("Test value is : \n %s \n",test);
		
}
*/

//TODO
/*
void findMultipleWordsInHashWithSTRTOK(char string[], char **finalOutput)
{
	structtopointtoemptyvalues = createmy_structForEmptyValues();
	
	struct my_struct *arrayOfStructs[50];
	int arrayOfStructsCounter=0;
	
	char *token=NULL;
	char *endPtr;
	char delimiters[] = " \n";
	token = strtok_r(string, delimiters,&endPtr);
	// printf("Word being searched is : %s \n",token);
        while(token!=NULL)
        {
        	findWordInHash(token,arrayOfStructs,&arrayOfStructsCounter);
        	token = strtok_r(NULL,delimiters,&endPtr);
        }
    // printf("Going to print test \n");
	char *test;
	computeDocNameIntersectionWithCharStar(&test,arrayOfStructs,&arrayOfStructsCounter);
	
	*finalOutput = test;
	//printf("Test value is : \n %s \n",test);
}
*/

void findMultipleWordsInHashWithSTRTOK(char string[], char **finalOutput)
{
	structtopointtoemptyvalues = createmy_structForEmptyValues();
	
	struct my_struct *arrayOfStructs[50];
	int arrayOfStructsCounter=0;
	
	char *token=NULL;
	char *endPtr;
	char delimiters[] = " \n";
	token = strtok_r(string, delimiters,&endPtr);
	// printf("Word being searched is : %s \n",token);
        while(token!=NULL)
        {
        	findWordInHash(token,arrayOfStructs,&arrayOfStructsCounter);
        	token = strtok_r(NULL,delimiters,&endPtr);
        }
    // printf("Going to print test \n");
	char *test;
	//computeDocNameIntersectionWithCharStar(&test,arrayOfStructs,&arrayOfStructsCounter);
	computeRank(&test,arrayOfStructs,&arrayOfStructsCounter);
	
	*finalOutput = test;
	//printf("Test value is : \n %s \n",test);
}

/*
void computeDocNameIntersection()
{
	//this array gives the number of occurences related to a docName in arrayOfStructs[i]
	int arrayForOccurencesEachWord[arrayOfStructsCounter+1];
	
	if(arrayOfStructsCounter==0)
	{
		printf("Error. System has not searched for any words \n");
	}
	
	struct node* primary = arrayOfStructs[0]->root;
	
	//printf("Random docname test : %s \n",arrayOfStructs[0]->root->docName);
	
	int varToCheckAtLeastOneMatch = 0;
	
	//start some form of loop
	while(primary!=0)
	{
		char docNameBeingChecked[50];
		strncpy(docNameBeingChecked,primary->docName,50);
		
		arrayForOccurencesEachWord[0] = checkIfLinkedListContains(arrayOfStructs[0]->root, docNameBeingChecked);
		
		int i=1;
		int flag=0;
		while(i<arrayOfStructsCounter)
		{
			printf("i value is %d \n",i);
			arrayForOccurencesEachWord[i] = checkIfLinkedListContains(arrayOfStructs[i]->root, docNameBeingChecked);
			if(arrayForOccurencesEachWord[i]==0)
				flag = 1;
			
			i++;
		}
		
		if(flag==0)
		{
			//this should show that array of two words have one common docName
			varToCheckAtLeastOneMatch++;
			printf("%s matches all words being searched \n",docNameBeingChecked);
			int j=0;
			while(j<arrayOfStructsCounter)
			{
				printf("%s || %d \n",arrayOfStructs[j]->wordBeingHashed,arrayForOccurencesEachWord[j]);
				j++;
			}
		}
		
		if(primary->next==0)
			break;
		
		primary=primary->next;
	}
	//end the loop
	
	if(varToCheckAtLeastOneMatch==0)
	{
		printf("No document matches the given set of words \n");
	}
	//put some check to see if at least one doc has all words
}
*/
void computeDocNameIntersectionWithCharStar(char **finalOutput, struct my_struct *arrayOfStructs[],int *arrayOfStructsCounter)
{
	
	char *output = malloc(sizeof(char)*100);
	output = " ";
	char tempout[100];
	char *nullTerminator = "\0";
	
	int rank=1;
	
	int arrayForOccurencesEachWord[*arrayOfStructsCounter+1];
	
	if(*arrayOfStructsCounter==0)
	{
		printf("Error. System has not searched for any words \n");
	}
	
	sortAllSearchedWords(arrayOfStructs,arrayOfStructsCounter);
	
	struct node* primary = arrayOfStructs[0]->root;
	
	
	int varToCheckAtLeastOneMatch = 0;
	
	//start some form of loop
	while(primary!=0 && rank<11)
	{
		char docNameBeingChecked[50];
		strncpy(docNameBeingChecked,primary->docName,50);
		
		
		arrayForOccurencesEachWord[0] = checkIfLinkedListContains(arrayOfStructs[0]->root, docNameBeingChecked);
		
		int i=1;
		int flag=0;
		while(i<*arrayOfStructsCounter)
		{
			arrayForOccurencesEachWord[i] = checkIfLinkedListContains(arrayOfStructs[i]->root, docNameBeingChecked);
			if(arrayForOccurencesEachWord[i]==0)
				flag = 1;
			
			i++;
		}
		
		if(flag==0)
		{
			varToCheckAtLeastOneMatch++;
			sprintf(tempout,"Rank %d :\n",rank);
			copystringwithoutfree(&output,tempout);
			sprintf(tempout,"%s matches all words being searched \n",docNameBeingChecked);
			copystring(&output,tempout);
			int j=0;
			while(j<*arrayOfStructsCounter)
			{
				sprintf(tempout,"%s || %d \n",arrayOfStructs[j]->wordBeingHashed,arrayForOccurencesEachWord[j]);
				copystring(&output,tempout);
				j++;
			}
			rank++;
		}
		
		if(primary->next==0)
			break;
		
		primary=primary->next;
	}
}
	
void computeRank(char **finalOutput, struct my_struct *arrayOfStructs[],int *arrayOfStructsCounter)
{
	
	char *output = malloc(sizeof(char)*100);
	output = " ";
	char tempout[100];
	char *nullTerminator = "\0";
	
	int rank=1;
	
	char *rankHeader = "----------SEARCH RESULTS--------\n";
	copystringwithoutfree(&output,rankHeader);
	
	//this array gives the number of occurences related to a docName in arrayOfStructs[i]
	int arrayForOccurencesEachWord[*arrayOfStructsCounter+1];
	
	if(*arrayOfStructsCounter==0)
	{
		printf("Error. System has not searched for any words \n");
	}
	sortAllSearchedWords(arrayOfStructs,arrayOfStructsCounter);
	
	int initialValue=0;
	/*
	while((strcmp((arrayOfStructs[initialValue]->wordBeingHashed),"EMPTY")==0)&&(initialValue<*arrayOfStructsCounter))
	{
		initialValue++;
	}
	*/
	
	while(initialValue<*arrayOfStructsCounter)
	{
		if(strcmp(arrayOfStructs[initialValue]->wordBeingHashed,"EMPTY")==0)
		{
			initialValue++;
		}
		else
		{
			break;
		}
	}
	
	struct node* primary;
	
	if(initialValue>=*arrayOfStructsCounter)
	{
		 primary = 0;
		 sprintf(tempout,"Search query only contains words not found in hash\n");
		 copystring(&output,tempout);
	}
	else
	{
		 primary = arrayOfStructs[0]->root;
	}
	
	//struct node* primary = arrayOfStructs[0]->root;
	int varToCheckAtLeastOneMatch = 0;
	//start some form of loop
	while(primary!=0 && rank<11)
	{
		char docNameBeingChecked[50];
		strncpy(docNameBeingChecked,primary->docName,50);
		arrayForOccurencesEachWord[0] = checkIfLinkedListContains(arrayOfStructs[0]->root, docNameBeingChecked);
		int i=1;
		int flag=0;
		while(i<*arrayOfStructsCounter)
		{
			arrayForOccurencesEachWord[i] = checkIfLinkedListContains(arrayOfStructs[i]->root, docNameBeingChecked);
			if(arrayForOccurencesEachWord[i]==0)
				flag = 1;
			i++;
		}
		
		if(flag==0)
		{
			//this should show that array of N words have one common docName
			varToCheckAtLeastOneMatch++;
			// printf("Rank %d :\n",rank);
			sprintf(tempout,"Rank %d :\n",rank);
			copystring(&output,tempout);
			sprintf(tempout,"%s matches the following words being searched \n",docNameBeingChecked);
			copystring(&output,tempout);
			int j=0;
			while(j<*arrayOfStructsCounter)
			{
				sprintf(tempout,"%s || %d \n",arrayOfStructs[j]->wordBeingHashed,arrayForOccurencesEachWord[j]);
				copystring(&output,tempout);
				j++;
			}
			rank++;
		}
		
		if(primary->next==0)
			break;
		
		primary=primary->next;
	}
	//end the loop
	if(varToCheckAtLeastOneMatch==0)
	{
		if(*arrayOfStructsCounter-1<=2)
		{
			sprintf(tempout,"No document matches for all these words\n");
			//new code to show best in rest of the words
			copystring(&output,tempout);
			sprintf(tempout,"Printing documents containing maximum occurences for each word\n");
			copystring(&output,tempout);
			int j=0;
			while(j<*arrayOfStructsCounter)
			{
				if(strcmp(arrayOfStructs[j]->wordBeingHashed,"EMPTY")!=0)
				{
				sprintf(tempout,"Word : %s\n Doc Name : %s\n Count : %d\n\n",arrayOfStructs[j]->wordBeingHashed,arrayOfStructs[j]->root->docName,arrayOfStructs[j]->root->noOfOccurences);
				copystring(&output,tempout);
				}
				j++;
			}
			//copystring(&output,nullTerminator);
		}
		else
		{
			//triggered when the number of words to match are more than 2
			fullPrecisionRanker(&output, arrayOfStructs, arrayOfStructsCounter, *arrayOfStructsCounter-2);
			
		}
	}
	copystring(&output,nullTerminator);
	*finalOutput = output;
}

void fullPrecisionRanker(char **finalOutput, struct my_struct *arrayOfStructs[],int *arrayOfStructsCounter, int noOfMatches)
{
	//add stuff to store output
	char *output = malloc(sizeof(char)*100);
	output = " ";
	char tempout[100];
	char *nullTerminator = "\0";
	
	int rank=1;
	int varToCheckAtLeastOneMatch = 0;
	int arrayForOccurencesEachWord[*arrayOfStructsCounter+1];

	int emptyWordCount=0;
	
	char *rankHeader = "----------SEARCH RESULTS--------\n";
	copystringwithoutfree(&output,rankHeader);
	
	if(*arrayOfStructsCounter==0)
	{
		printf("Error. System has not searched for any words \n");
	}
	sortAllSearchedWords(arrayOfStructs,arrayOfStructsCounter);
	
	int loopCounter=0;
	//this loop is used to consecutively check docs between different words (not limited to first word)
	for(loopCounter=0;loopCounter<*arrayOfStructsCounter;loopCounter++)
	{
	struct node* primary = arrayOfStructs[loopCounter]->root;
	
	if(strcmp(arrayOfStructs[loopCounter]->wordBeingHashed,"EMPTY")==0)
		continue;

	//start some form of loop
		while(primary!=0 && rank<11)
		{
			char docNameBeingChecked[50];
			strncpy(docNameBeingChecked,primary->docName,50);
			if(strcmp(docNameBeingChecked,"EMPTY")==0)
			{
				emptyWordCount++;
				
				if(primary->next==0)
					break;
			
				primary=primary->next;
			}
			arrayForOccurencesEachWord[loopCounter] = checkIfLinkedListContains(arrayOfStructs[loopCounter]->root, docNameBeingChecked);
			int i=loopCounter+1;
			int points=0;
			int indicatorOfMatch[*arrayOfStructsCounter];
			memset(indicatorOfMatch, 0,*arrayOfStructsCounter);
			
			while(i<*arrayOfStructsCounter)
			{
				if(strcmp(arrayOfStructs[i]->wordBeingHashed,"EMPTY")==0)
				{
					i++;
					continue;
				}
				arrayForOccurencesEachWord[i] = checkIfLinkedListContains(arrayOfStructs[i]->root, docNameBeingChecked);
				if(arrayForOccurencesEachWord[i]!=0)
				{
					points++;
					indicatorOfMatch[i]=1;
				}
				i++;
			}
			
			if(points>=noOfMatches)
			{
				//this should show that array of N words have one common docName
				varToCheckAtLeastOneMatch++;
				sprintf(tempout,"Rank %d :\n",rank);
				copystring(&output,tempout);
				sprintf(tempout,"%s matches the following words being searched \n",docNameBeingChecked);
				copystring(&output,tempout);
				sprintf(tempout,"%s || %d \n",arrayOfStructs[loopCounter]->wordBeingHashed,arrayForOccurencesEachWord[loopCounter]);
				copystring(&output,tempout);
				int j=loopCounter;
				
				while(j<*arrayOfStructsCounter)
				{
					if(indicatorOfMatch[j]!=1)
					{
						j++;
						continue;
					}
					sprintf(tempout,"%s || %d \n",arrayOfStructs[j]->wordBeingHashed,arrayForOccurencesEachWord[j]);
					copystring(&output,tempout);
					j++;
				}
				rank++;
			}		
			
			if(primary->next==0)
				break;
			
			primary=primary->next;
		}
	}
	//end of for loop
	if(varToCheckAtLeastOneMatch==0)
	{
		//what to do if none of the docs matches enough for the points
		if(emptyWordCount==*arrayOfStructsCounter-1)
		{
			sprintf(tempout,"No document matches for any of these words\n");
			copystring(&output,tempout);
			copystring(&output,nullTerminator);
			*finalOutput = output;
		}
		else if(noOfMatches<=2)
		{
			sprintf(tempout,"No document matches for all these words\n");
			//new code to show best in rest of the words
			copystring(&output,tempout);
			sprintf(tempout,"Printing documents containing maximum occurences for each word\n");
			copystring(&output,tempout);
			int j=0;
			while(j<*arrayOfStructsCounter)
			{
				if(strcmp(arrayOfStructs[j]->wordBeingHashed,"EMPTY")!=0)
				{
				sprintf(tempout,"Word : %s\n Doc Name : %s\n Count : %d\n\n",arrayOfStructs[j]->wordBeingHashed,arrayOfStructs[j]->root->docName,arrayOfStructs[j]->root->noOfOccurences);
				copystring(&output,tempout);
				}
				j++;
			}
		}
		fullPrecisionRanker(finalOutput, arrayOfStructs, arrayOfStructsCounter, noOfMatches-1);
	}
	else
	{
		copystring(&output,nullTerminator);
		*finalOutput = output;
	}
}

void hashWordFromString(char string[])
{
	char *token=NULL;
	char *endPtr;
	char delimiters[] = "$\n";
	token = strtok_r(string, delimiters,&endPtr);
	char wordBeingHashed[50];
	strncpy(wordBeingHashed,token,50);
	int i=0;
        while(token!=NULL)
        {

        	char *smallToken;
        	char *smallEnd;
        	if(i!=0)
        	{
		    	smallToken = strtok_r(token,":",&smallEnd);
				char docName[50];
				strncpy(docName,smallToken,50);
		    	smallToken = strtok_r(NULL,":",&smallEnd);
				int wordCount = atoi(smallToken);
				hashWordIntoGlobalHash(docName,wordBeingHashed,wordCount);
        	}
        	token = strtok_r(NULL, delimiters,&endPtr);
        	i++;
        }
}

void initializeConversionGlobalHashToString()
{
	varForTransportingHash = users;
}

void convertGlobalHashIntoString(char **dest2)
{
	if(varForTransportingHash!=NULL)
	{
		char *dest="\0";
		
		struct node *conductor;
		
		char *delim1 = "$";
		char *delim2 = ":";
		char *nullTerminator = "\0";
		copystringwithoutfree(&dest,varForTransportingHash->wordBeingHashed);
		conductor = varForTransportingHash->root; 
		if ( conductor != 0 )
		{
			
			copystring(&dest,delim1);
			copystring(&dest,conductor->docName);
			copystring(&dest,delim2);
			char *wordCount=NULL;
			wordCount = itoa1(conductor->noOfOccurences);
			copystring(&dest,wordCount);
			while ( conductor->next != 0)
			{
				conductor = conductor->next;
				copystring(&dest,delim1);
				copystring(&dest,conductor->docName);
				copystring(&dest,delim2);
				char *wordCount=NULL;
				wordCount = itoa1(conductor->noOfOccurences);
				copystring(&dest,wordCount);
			}
		}
		
		varForTransportingHash = varForTransportingHash->hh.next;
		copystring(&dest,nullTerminator);
		// strcat(dest,nullTerminator);
		//return strdup(dest);
		*dest2 = dest;
	}
	else
	{
		//if varForTransportingHash is null
		char *emptyList = "EMPTY";
		//return "EMPTY";
		*dest2 = emptyList;
	}
	
}

void initializeConversionLocalHashToString()
{
	varForTransportingLocalHash = localhashid;
}

void convertLocalHashIntoString(char fileName[], char **dest2)
{
	if(varForTransportingLocalHash!=NULL)
	{

		char *dest;
		dest="\0";
		
		char *delim1 = "$";
		char *delim2 = ":";
		char *nullTerminator = "\0";

		copystringwithoutfree(&dest,varForTransportingLocalHash->wordBeingHashed);
		
		copystring(&dest,delim1);		
		copystring(&dest,fileName);
		copystring(&dest,delim2);
		char *wordCount=NULL;
		wordCount = itoa1(varForTransportingLocalHash->noOfHits);
		copystring(&dest,wordCount);
		
		copystring(&dest,nullTerminator);
		varForTransportingLocalHash = varForTransportingLocalHash->hh.next;
		*dest2 = dest;
	}
	else
	{
		char *empty = "EMPTY";
		//return "EMPTY";
		*dest2 = empty;
	}
}



	

