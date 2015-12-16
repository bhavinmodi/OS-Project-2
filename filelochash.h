#ifndef FILELOCHASH_H_   /* Include guard */
#define FILELOCHASH_H_

struct filelochash* createFileLocHash();
void hashFileAndPort(char filename[], char ip[], int portNumber);
int findFileLocPort(char fileLocToFind[]);
char* findFileLocIP(char fileLocToFind[]);
int checkIfFileExists(char fileLocToFind[]);
void fileLocHashIterate();
void freeFileLocHash();

#endif // FILELOCHASH_H_
