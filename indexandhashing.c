#include <string.h>  /* strcpy */
#include <stdlib.h>  /* malloc */
#include <stdio.h>   /* printf */
#include "uthash.h"
#include <stdio.h>
#include <string.h>

struct my_struct {
    char wordBeingHashed[50];             /* key (string is WITHIN the structure) */
    char docName[50][50];
    int noOfTimes[50];
    int noOfHits;
    UT_hash_handle hh;         /* makes this structure hashable */
};

struct my_struct *s, *tmp, *users = NULL;
char currDocName[] = "DOC3";
char currDocNo=-1;

struct my_struct *arrayOfStructs[50];
int arrayOfStructsCounter=0;

/*
struct my_struct* copyStruct(struct my_struct *source, struct my_struct *dest)
{
	printf("Inside copy function \n");
	dest = source;	
	return dest;
}
*/

int checkForSymbols(char c)
{
    if(c>31&&c<48)
    {
        return 1;
    }
    
    if(c>57&&c<65)
    {
        return 1;
    }
    if(c>90&&c<97)
    {
        return 1;
    }
    if(c>124&&c<127)
    {
        return 1;
    }
    if(c=='\n')
    {
        return 1;
    }
    return 0;
}

void hashWord(char c[], int noOfOccurences)
{
 
	int i = 0;
	//char test[10];
	//strncpy(test, *n,10);
	//HASH_FIND_STR( users, test, s);
	HASH_FIND_STR( users, c, s);
	//printf("Checkpoint 1 \n");
	if(s!=NULL)
	{
	//printf("Duplicate found for word %s \n",c);
	s->noOfHits++;
	s->noOfTimes[s->noOfHits] = noOfOccurences;
	strncpy(s->docName[s->noOfHits],currDocName,50);
	//s->count++;
	}
	else
	{
	//printf("Single id found and name is %s \n",*n);
	//printf("Checkpoint 2 \n");
	s = (struct my_struct*)malloc(sizeof(struct my_struct));
	strncpy(s->wordBeingHashed,c,50);
	s->noOfHits = 0;
	s->noOfTimes[s->noOfHits] = noOfOccurences;
	strncpy(s->docName[s->noOfHits],currDocName,50);
	//s->docName[s->noOfHits] = currDocName;
	HASH_ADD_STR( users, wordBeingHashed, s );
	}

}

void computeDocIntersection()
{
	if(arrayOfStructsCounter==0)
	{
		printf("No docs to compute \n");
		return;
	}
	
	int k;
	int l;
	int i;
	int j;
	for(k=0;k<arrayOfStructsCounter;k++)
	{
		
		for(l=0; l<arrayOfStructsCounter; l++)
		{
			//used to set a variable first struct aka spin the first struct iterator

			for(i=0; i<=arrayOfStructs[k]->noOfHits; i++)
			{
				int HitMe = 0;
				for(j=0; j<=arrayOfStructs[l]->noOfHits; j++)
				{
					int valueOfStrCmp = strcmp(arrayOfStructs[k]->docName[i],arrayOfStructs[l]->docName[j]);
					if(valueOfStrCmp==0)
					{
						HitMe++;
					}

				}
				if(HitMe==0)
				{
					strncpy(arrayOfStructs[k]->docName[i],"DEAD",50);
				}
			}
		}
	}

	
	//displaying final result
	printf("Final Result \n");
	k=0;
	for(k=0;k<arrayOfStructsCounter;k++)
	{
		printf("Details in hash table for word: %s \n",arrayOfStructs[k]->wordBeingHashed);
    	//printf("%s's count is %d\n",c,s->count);
    	int i=0;
    	for(i=0; i<=(arrayOfStructs[k]->noOfHits); i++)
    	{
    		if(strcmp(arrayOfStructs[k]->docName[i],"DEAD")!=0)
	    		printf("Docname: %s || Count: %d \n",arrayOfStructs[k]->docName[i],arrayOfStructs[k]->noOfTimes[i]);    		
    	}
    	
	}
	
}

void findWordInHash(char c[])
{
    HASH_FIND_STR( users, c, s);
    if (s)
    {
    	printf("Details in hash table for word: %s \n",c);
    	//printf("%s's count is %d\n",c,s->count);
    	int i=0;
    	for(i=0; i<=(s->noOfHits); i++)
    	{
    		printf("Docname: %s || Count: %d \n",s->docName[i],s->noOfTimes[i]);    		
    	}
    	
		arrayOfStructs[arrayOfStructsCounter] = s;
		arrayOfStructsCounter++;
    	
    }
    else
    {
    	
    	printf("Word not found in hash \n");
    	
    	//adding code here to enter dummy struct pointer to keep results consistent
    	//TODO FIX THE UNDERLYING COMMENTED CODE
    	
    	/*
    	strncpy(tmp->wordBeingHashed,"DUMMY",50);
    	printf("Test1 \n");
    	strncpy(tmp->docName[0],"DEAD",50);
    	tmp->noOfTimes[0]=0;
    	tmp->noOfHits=0;
    	
    	arrayOfStructs[arrayOfStructsCounter] = tmp;
		arrayOfStructsCounter++;
		
		printf("End of adding dummy struct \n");
		*/
    	
    }
}

int sendFileToHash(char filename[])
{
	currDocNo++;
	sprintf(&currDocName[0], "DOC%d", currDocNo);
    FILE *fp;
    int charVal;
    
    fp = fopen(filename,"r");
    if (fp==NULL)
    {
    	perror ("Error opening file");
    	return 0;
    }
    
    
    char str[10000];
    int pos=0;
    
    while(1)
    {
        charVal = fgetc(fp);
        if( feof(fp) )
        {
            break ;
        }
        str[pos++] = charVal;
    }
    fclose(fp);
    
    //printf("Final array got is %s \n",str);
    
    
    int count = 0, c = 0, i, j = 0, k, space = 0;
    
    char p[500][100], ptr1[500][100];
    //gonna put last character as something else
    
    //resetting ptr1 and p
    int reset;
    for(reset = 0; reset<500; reset++)
    {
    	strncpy(ptr1[reset],"",2);
    	strncpy(p[reset],"",2);
    }
    
    /*
    printf("Preprinting ptr1 \n");
    int ppos1;
    for(ppos1=0; ppos1<100; ppos1++)
    {
    	printf("%s \n",ptr1[ppos1]);
    }
    
        printf("Preprinting p \n");
   // int ppos1;
    for(ppos1=0; ppos1<100; ppos1++)
    {
    	printf("%s \n",p[ppos1]);
    }
    */
    
    //used to find number of symbols
    for (i = 0;i<strlen(str);i++)
    {
        if(checkForSymbols(str[i]))
        {	
            space++;
        }
    }
    
    //printf("Space value is %d \n",space);
    
    //used to break text into discrete words
    for (i = 0, j = 0, k = 0;j < strlen(str);j++)
    {
        if(checkForSymbols(str[j]))
        {
            p[i][k] = '\0';
            i++;
            k = 0;
        }
        else
        {
            p[i][k++] = str[j];
        }
    }
    //printf("Value of i is %d \n",i);
    //used to assign each discrete word into p array
    k = 0;
    for (i = 0;i <= space;i++)
    {
        for (j = 0;j <= space;j++)
        {
            if (i == j)
            {
                strcpy(ptr1[k], p[i]);
               // printf("Word being formed is %s \n",ptr1[k]);
                k++;
                count++;
                break;
            }
            else
            {
                if (strcmp(ptr1[j], p[i]) != 0)
                {
                	//printf("Value of i is %d and j is %d \n",i,j);
                	//printf("Value of ptr1 is %s \n",ptr1[j]);
                    continue;
                }
                else
                    break;
            }
        }
    }
    
    /*
    printf("Postprinting ptr \n");
    int ppos;
    for(ppos=0; ppos<100; ppos++)
    {
    	printf("%s \n",ptr1[ppos]);
    }
    */
    
    //printf("Value of i after word being formed is %d \n",i);
    //printf("Valur of count is %d \n",count);
    //printf("Printing all values of ptr1 \n");
   
   /*
    int ppos;
    for(ppos=0; ppos<500; ppos++)
    {
    	printf("%s \n",ptr1[ppos]);
    }
    */
    
    //used to check if word1==word2
    for (i = 0;i < count;i++)
    {
        for (j = 0;j <= space;j++)
        {
            if (strcmp(ptr1[i], p[j]) == 0)
                c++;
        }
        //printf("%s -> %d times\n", ptr1[i], c);
        hashWord(ptr1[i],c);
        c = 0;
    }
    
    
    return 1;
}

int main()
{
	//TODO : Remember to make function that asks how many words to search and initializes arrayOfStructsCounter=0 before "full search"
	
	int statusOfSend;
	statusOfSend = sendFileToHash("file.txt");
	statusOfSend = sendFileToHash("1.txt");
	statusOfSend = sendFileToHash("2.txt");
	statusOfSend = sendFileToHash("3.txt");
	
	int rcount=0;
	while(1)
	{
	printf("Please enter the word to find\n");
	char inputWord[30];
	scanf("%s",inputWord);
	
	findWordInHash(inputWord);
	rcount++;
	
	if(rcount==3)
	break;
	}
	
	computeDocIntersection();
	
	return 0;
}
