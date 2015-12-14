#ifndef INDEXANDHASHING_H_   /* Include guard */
#define INDEXANDHASHING_H_

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

#endif // INDEXANDHASHING_H_
