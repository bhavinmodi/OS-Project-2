#ifndef FILELOCHASH_H_   /* Include guard */
#define FILELOCHASH_H_

struct filelochash* createFileLocHash();
void hashFileAndPort(char filename[], int portNumber);
int findFileLoc(char fileLocToFind[]);
int checkIfFileExists(char fileLocToFind[]);

#endif // FILELOCHASH_H_
