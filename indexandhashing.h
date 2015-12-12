#ifndef INDEXANDHASHING_H_   /* Include guard */
#define INDEXANDHASHING_H_

struct my_struct* createmy_structStruct();
struct my_struct* addValuesTomy_struct(struct my_struct *s, char word[], int noOfOccurences, char fileName[]);
struct my_struct* appendValuesTomy_struct(struct my_struct *s, char word[], int noOfOccurences, char fileName[]);
struct my_struct* createmy_structForEmptyValues();
struct localhashstruct* createLocalHashStruct();
struct node* initializeNode();
struct node* addValueToNode(struct node* n, char c[], int noOfOccurences);
void addToList(struct node* root, char c[], int noOfOccurences);
void printLinkedList(struct node* root);
void sortLinkedList(struct node* root);
void hashWordIntoLocalHash(char c[]);
void hashWordIntoGlobalHash(char docName[], char wordBeingHashed[], int noOfOccurences);
void LocalHashIterate();
void addToGlobalHash(char docName[]);
void globalHashIterate();
void freeLocalHash();
int hashFile(char fileName[]);
int checkIfLinkedListContains(struct node* root, char docNameToCheck[]);
void findWordInHash(char c[],struct my_struct *arrayOfStructs[],int *arrayOfStructsCounter);
void findMultipleWordsInHash(int numberOfWords);
void computeDocNameIntersection();
void computeDocNameIntersectionWithCharStar(char **finalOutput, struct my_struct *arrayOfStructs[],int *arrayOfStructsCounter);
void hashWordFromString(char string[]);
void initializeConversionGlobalHashToString();
char * convertGlobalHashIntoString();
void initializeConversionLocalHashToString();
//char * convertLocalHashIntoString(char fileName[]);
void convertLocalHashIntoString(char fileName[], char **dest2);
void globalHashCount();
void sortAllSearchedWords(struct my_struct *arrayOfStructs[],int *arrayOfStructsCounter);
void findMultipleWordsInHashWithSTRTOK(char string[], char **finalOutput);

#endif // INDEXANDHASHING_H_
