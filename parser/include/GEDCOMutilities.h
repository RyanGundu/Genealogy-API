#ifndef UTIL_H
#define UTIL_H

#include "GEDCOMparser.h"
#include "LinkedListAPI.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>

#define SUBMISSION 0
#define INDIVIDUAL 1

//used for myOpenforReadOrWrite
#define READ 0
#define WRITE 1


GEDCOMerror writeToFile(char* fileName, const GEDCOMobject* obj);

void writeHeader (FILE *fptr, const GEDCOMobject* obj);

void writeSubmitter (FILE *fptr, const GEDCOMobject* obj);

void writeIndividuals (FILE *fptr, const GEDCOMobject* obj, List individuals, List families);

void writeFamilies (FILE *fptr, const GEDCOMobject* obj, List families, List individuals);

char **generateIndXrefs (const GEDCOMobject* obj);

char **generateFamXrefs (const GEDCOMobject* obj);

int checkDepth (List children, int *genCount);

int checkHeight (List families, Individual *person, int *genCount);

int alphabeticalCompare (const void* first,const void* second);

void getChildrenListN (List *generations, unsigned int maxGen, List children, int *genCount);
void getAncestors (List *ancestors, unsigned int maxGen, List families, Individual *person, int *genCount, List *loadedIndivs);
void deleteIndividualCopy(void* toBeDeleted);


bool compareNewFamily (const void* first,const void* second);

/* returns 1 if parent otherwise -1 assuming a child*/
int isParent (const void* family,const void* person);
int isChild (const void* family,const void* person);

bool findIndividual (const void* first,const void* second);


Individual *copyIndividual (const Individual *ind);


int fragmentCount (const char *line);
char **parseJSON(const char *line, int numWords);


Individual* splitJSONInd (const char* str);

GEDCOMobject* splitJSONObj (const char* str);



/********************************** A1 ********************************/
//modified Fgets
char *fgetss(char *destination, int max, FILE *fp);

void getChildren(List *descendants, List children);

void freeXrefs(List *subRecord, List *individualRecords, List *famRecords);

void deleteDummy (void *toBeDeleted);


/*************************** checkFile.h *************************/

FILE* myOpenforReadOrWrite (char fileName[], int command);

int tagScan (char *tag);

GEDCOMerror checkHeader (FILE *fptr, char*line, int lineCount);

GEDCOMerror scanFile(FILE *fptr, GEDCOMobject** obj);

GEDCOMerror validityCheck(char *line, int numWords);

/*************************** checkFile.h *************************/

/*************************** load.h ****************************/
typedef struct Submission{
	char *tag;
	Submitter *submitter;
} Submission;


typedef struct Person{
	char *tag;
	Individual* person;
} Person;

typedef struct fam {
	char *tag;
	Family *family;
}Fam;


Fam *createFamily(char *tag);
Person *createPerson (char *tag);
Submission *createSubmission (char *tag); 

List loadOtherFields (List *otherFields, char** fragments, int numWords);

void readSubRecord (FILE *fptr, Submission *submission);
void readIndRecord (FILE *fptr, Person *p);
void getName (FILE *fptr, int level, Person *p);
void readFamRecord (FILE *fptr, Fam *f, List individualRecords);

/****************** SUBMISSION LIST *************************/
char *printSubFunction(void* toBePrinted);

void deleteSubRecordData (void* toBeDeleted);

int compareSubRecordTag (const void* first,const void* second);
/*************************************************************/

/*******************LIST FUNCTIONS (Individual List) ***********************/
char *printIndivFunction(void* toBePrinted);

void deleteIndRecordData (void* toBeDeleted);

int compareIndRecordTag (const void* first,const void* second);
/***************************************************************************/

/*******************LIST FUNCTIONS (Fam List) ***********************/
char *printFam(void* toBePrinted);

void deleteFam (void* toBeDeleted);

int compareFam (const void* first,const void* second);

/*****************************************************************************/


/* To compare family tags to individual tags*/
bool compareTags (const void* first,const void* second);



List loadRecords(FILE *fptr, int recordType);

List loadFamilyRecords(FILE *fptr, List individualRecords);
/*************************** load.h ***************************/

/*************************** parseFile.h ***************************/

void parse(FILE *fptr, GEDCOMobject** obj, List subRecords, List individualRecords, List famRecords);

void createHeaderRecord(FILE *fptr, GEDCOMobject **obj, List subRecords);

void getGedVersion (List otherFields, FILE *fptr, GEDCOMobject **obj, int level);

void connectToFamily (FILE *fptr, List *individuals, List families, char *tagToMatch);
/*************************** parseFile.h ***************************/

/*************************** splitter.h ***************************/
/* Function counts number of words in a string */
int wordCount (char *line);

/* Parses all words and returns 2D array */
char **parseLine(char *line, int numWords);

/* Checks if a word is in a string */
int containsWord (char *word, char *line);

/* prints a 2D array of characters */
void printArrayOfStrings (char *line, char** array);

/* frees memory for 2D aray of characters */
void freeWords(char **array, int numWords);

void makeLowerCase (char *text);

void makeUpperCase (char *text);

/* Checks if string is an int */
int isInt(char *toCheck);

/*creates a substring from a string */
char *createWord(char* string, int start, int end);

/** 
* Function removes a hard return if it exists
*@param line is an array of characters
*/
void removeTrailingHardReturn(char **toClean);
/*************************** splitter.h ***************************/
void dummyDelete(void *toBeDeleted);

#endif
