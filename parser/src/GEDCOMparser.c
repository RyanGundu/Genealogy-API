#include "GEDCOMparser.h"
#include "GEDCOMutilities.h"

/************************* A3 INTERFACE ********************/

char *displayIndividuals(char *fileName) {

	GEDCOMobject *obj = NULL;
	GEDCOMerror errorType = createGEDCOM(fileName, &obj);
	if (errorType.type != OK)
		return "ERROR";

	char *display = iListToJSON2(obj->individuals);
	if (display == NULL)
		return "NI";

	// char* newString = malloc(sizeof(char) * (strlen(display) + 5));
	// strcpy(newString, display);
	// deleteGEDCOM(obj);

	return display;
}
 
char* iListToJSON2(List iList) {

	char bounds[] = "[]";
	char *string = malloc(sizeof(char) * strlen(bounds) + 1);

	strcpy(string,"[");

	ListIterator iter2 = createIterator(iList);
	void* data = NULL;
	int flag = 0;
	while( (data = nextElement(&iter2)) != NULL) {
		Individual *ind = (Individual *)data;
		char *newInd = indToJSON2(ind);
		string = realloc(string, sizeof(char) * (strlen(string) + strlen(newInd) + 3));
		strcat(string, newInd);
		strcat(string, ",");
		free(newInd);
		flag = 1;
	}
	if (flag == 1)
		string[strlen(string)-1] = ']';
	else strcpy(string, "[]");

	return string;
}

char* indToJSON2(const Individual* ind) {

	if (ind == NULL) {
		char *string = malloc(sizeof(char) * (2));
		strcpy(string,"");
		return string;
	}	

	char givenName[] = "{\"givenName\":\"";
	char surname[] = "\",\"surname\":\"";
	char sex[] = "\",\"sex\":\"";
	char famSize[] = "\",\"famSize\":\"";
	char end[] = "\"}";

	int size = strlen(givenName) + strlen(surname) + strlen(ind->givenName) + strlen(ind->surname) + strlen(sex) + strlen(famSize) + strlen(end) + 1;

	char gender[10] = "-";
	ListIterator iter = createIterator(ind->otherFields);
	void* data = NULL;
	while( (data = nextElement(&iter)) != NULL) {
		Field *f = (Field *)data;
		if (f->tag != NULL && f->value != NULL) {
			if (strcmp(f->tag,"SEX") == 0 || strcmp(f->tag,"sex") == 0) {
				if (strlen(f->value) < 10)
					strcpy(gender,f->value);
			}
		}
	}

	int numKids = 0;
	int numSpouse = 1;//includes themselves
	ListIterator iter2 = createIterator(ind->families);
	Family *fam = NULL;
	while( (fam = (Family *)nextElement(&iter2)) != NULL) {
		if (compareIndividuals(ind,fam->husband) == 0) {
			if (fam->wife != NULL)
				numSpouse += 1;
			numKids += fam->children.length;
		} 
		if (compareIndividuals(ind,fam->wife) == 0) {
			if (fam->husband != NULL)
				numSpouse += 1;
			numKids += fam->children.length;
		}

	}


	char famCount[20] = "1"; //= malloc(sizeof(char)*20);
	sprintf(famCount,"%d",(numKids + numSpouse));

	size += strlen(gender) + strlen(famCount);

	char *string = malloc(sizeof(char) * (size));


	strcpy(string, givenName);
	strcat(string, ind->givenName);
	strcat(string, surname);
	strcat(string, ind->surname);

	strcat(string, sex);
	strcat(string, gender);

	strcat(string, famSize);
	strcat(string, famCount);

	strcat(string, end);

	return string;

}

char *fileInfo(char *path, char *fileName) {


	GEDCOMobject *obj = NULL;
	char *file = malloc(sizeof(char) * (strlen(path) + strlen(fileName) + 1000));
	strcpy(file,"");
	strcat(file,path);
	strcat(file,fileName);



	GEDCOMerror errorType = createGEDCOM(file, &obj);
	if (errorType.type != OK)
		return "ERROR";



	char *ret = fileToJSON(fileName, obj);

	if (ret == NULL)
		return "ERROR";


	//return "{\"fileName\":\"newFile.ged\",\"source\":\"PAF\",\"gedcVersion\":\"5.50\",\"encoding\":\"ASCII\",\"subName\":\"Submitter\",\"subAddress\":\"\",\"numInd\":\"0\",\"numFam\":\"0\"}";

	//printf("%s\n", ret);

	return ret;
}

char *fileToJSON(char *fileName, GEDCOMobject *obj) {


	char fName[] = "{\"fileName\":\"";
	char src[] = "\",\"source\":\"";
	char gedcVersion[] = "\",\"gedcVersion\":\"";
	char encoding[] = "\",\"encoding\":\"";
	char subName[] = "\",\"subName\":\"";
	char subAddress[] = "\",\"subAddress\":\"";
	char numInd[] = "\",\"numInd\":\"";
	char numFam[] = "\",\"numFam\":\"";
	char end[] = "\"}";

	char version[20]; //= malloc(sizeof(char)*20);
	sprintf(version,"%.2lf",obj->header->gedcVersion);

	char indCount[20]; //= malloc(sizeof(char)*20);
	sprintf(indCount,"%d",obj->individuals.length);

	char famCount[20]; //= malloc(sizeof(char)*20);
	sprintf(famCount,"%d",obj->families.length);

	char *ret = malloc(sizeof(char) * 3000);
	strcpy(ret,fName);
	strcat(ret,fileName);
	strcat(ret,src);
	strcat(ret, obj->header->source);
	strcat(ret,gedcVersion);
	strcat(ret,version);
	strcat(ret, encoding);

	if (obj->header->encoding == UTF8)
		strcat(ret,"UTF-8");
	else if (obj->header->encoding == ANSEL)
		strcat(ret,"ANSEL");
	else if (obj->header->encoding == UNICODE)
		strcat(ret,"UNICODE");
	else strcat(ret, "ASCII");
	
	strcat(ret, subName);
	strcat(ret, obj->submitter->submitterName);
	strcat(ret, subAddress);
	strcat(ret, obj->submitter->address);
	strcat(ret, numInd);
	strcat(ret, indCount);
	strcat(ret, numFam);
	strcat(ret, famCount);
	strcat(ret,end);
	
	return ret;
}

//ADD INDIVIDUAL 
char *addIndToFile (char *fileName, char *indJSON) {

	GEDCOMobject *obj = NULL;
	GEDCOMerror errorType = createGEDCOM(fileName, &obj);
	if (errorType.type != OK)
		return "ERROR";

	Individual *ind = JSONtoInd(indJSON);
	addIndividual(obj, ind);

	errorType = writeGEDCOM(fileName, obj);
	//deleteGEDCOM(obj); 

	if (errorType.type != OK)
		return "ERROR";

	return "OK";

}

char *returnDecendantsList (char *fileName, char *indJSON, int maxGen) {

	GEDCOMobject *obj = NULL;
	GEDCOMerror errorType = createGEDCOM(fileName, &obj);
	if (errorType.type != OK)
		return "ERROR";

	Individual *ind = JSONtoInd(indJSON);

	void *exist = findElement(obj->individuals, &findIndividual, ind);
	if (exist == NULL)
		return "DNE"; //individual dosent exist in the file

	List dec = getDescendantListN(obj, ind, maxGen);

	char* string = gListToJSON(dec);
	//printf("%s\n", string);

	//deleteGEDCOM(obj);

	return string;

}

char *returnAncestorsList (char *fileName, char *indJSON, int maxGen) {

	GEDCOMobject *obj = NULL;
	GEDCOMerror errorType = createGEDCOM(fileName, &obj);
	if (errorType.type != OK)
		return "ERROR";

	Individual *ind = JSONtoInd(indJSON);
	
	void *exist = findElement(obj->individuals, &findIndividual, ind);
	if (exist == NULL)
		return "DNE"; //individual dosent exist in the file

	List anc = getAncestorListN(obj, ind, maxGen);

	char* string = gListToJSON(anc);

	//deleteGEDCOM(obj);

	return string;

}


char *createNewGEDCOM (char *fileName, char *fileJSON) {

	GEDCOMobject *obj = JSONtoGEDCOM(fileJSON);
	if (obj == NULL)
		return "ERROR";
	GEDCOMerror errorType = writeGEDCOM(fileName,obj);

	if (errorType.type != OK)
		return "ERROR";

	return "OK";

}




/**************************   A2   ************************/

GEDCOMerror writeGEDCOM(char* fileName, const GEDCOMobject* obj) {

	GEDCOMerror errorType;
	errorType.type = OK;
	errorType.line = -1;

	if (obj == NULL || fileName == NULL) {
		errorType.type = WRITE_ERROR;
		errorType.line = -1;
		return errorType;
	}

	errorType = writeToFile(fileName, obj);

	return errorType;

}

ErrorCode validateGEDCOM(const GEDCOMobject* obj) { // add check for 255

	if (obj == NULL || obj->submitter == NULL)
		return INV_GEDCOM;

	if (obj->header == NULL)
		return INV_GEDCOM;

	if (obj->header->submitter == NULL || strlen(obj->header->source) == 0)
		return INV_HEADER;

	if (strlen(obj->submitter->submitterName) == 0 || strlen(obj->submitter->submitterName) > 61)
		return INV_RECORD;

	if (strlen(obj->header->source) > 249)
		return INV_HEADER;

	//check for NULL values in List
	ListIterator iter = createIterator(obj->individuals);
	void* data = NULL;
	int counter = 0;
	while( (data = nextElement(&iter)) != NULL) {

		Individual *ind = (Individual *)data;
		ListIterator iter2 = createIterator(ind->events);
		void* data2 = NULL;
		int subCounter = 0;
		while( (data2 = nextElement(&iter2)) != NULL) {
			Event *e = (Event *)data2;
			if (strlen(e->type) > 5)
				return INV_RECORD;

			++subCounter;
		}
		if (ind->events.length != subCounter)
			return INV_RECORD;
		
		iter2 = createIterator(ind->families);
		data2 = NULL;
		subCounter = 0;
		while( (data2 = nextElement(&iter2)) != NULL) {
			Family *f = (Family *)data2;
			ListIterator iter3 = createIterator(f->events);
			void* data3 = NULL;
			int subCounter2 = 0;
			while( (data3 = nextElement(&iter3)) != NULL) {
				Event *e = (Event *)data3;
				if (strlen(e->type) > 5)
					return INV_RECORD;
				++subCounter2;
			}
			if (f->events.length != subCounter2)
				return INV_RECORD;

			++subCounter;
		}
		if (ind->families.length != subCounter)
			return INV_RECORD;

		++counter;
	}
	if (obj->individuals.length != counter) 
		return INV_RECORD;

	iter = createIterator(obj->families);
	data = NULL;
	counter = 0;
	while( (data = nextElement(&iter)) != NULL) {

		Family *fam = (Family *)data;
		ListIterator iter2 = createIterator(fam->events);
		void *data2 = NULL;
		int subCounter = 0;
		while( (data2 = nextElement(&iter2)) != NULL) {
			Event *e = (Event *)data2;
			if (strlen(e->type) > 5)
				return INV_RECORD;
			++subCounter;
		}
		if (fam->events.length != subCounter)
			return INV_RECORD;

		iter2 = createIterator(fam->children);
		data2 = NULL;
		subCounter = 0;
		while( (data2 = nextElement(&iter2)) != NULL)
			++subCounter;
		if (fam->children.length != subCounter)
			return INV_RECORD;

		++counter;
	}
	if (obj->families.length != counter) 
		return INV_RECORD;

	return OK;
}

List getDescendantListN(const GEDCOMobject* familyRecord, const Individual* person, unsigned int maxGen) {

	List generations = initializeList(&printGeneration, &deleteGeneration, &compareGenerations);

	if (familyRecord == NULL || person == NULL)
		return generations;

	List individuals = familyRecord->individuals;
	//First check if the individual exists in the family record
	ListIterator iter = createIterator(individuals);
	void* elem = NULL;
	while( (elem = nextElement(&iter)) != NULL) {
		if (compareIndividuals(elem, person) == 0)
			break;
	}
	if (elem != NULL) { //If the individual exists, find the family in which they are a parent

		List *gens;
		int count = 0;
		int *max = &count;

		Individual *ind = (Individual *)elem;
		List families = ind->families;
		ListIterator iter2 = createIterator(families);
		void* data = NULL;
		int flag = 0;

		while( (data = nextElement(&iter2)) != NULL) {

			Family *family = (Family *)data;
			if (compareIndividuals(family->husband,elem) == 0 || compareIndividuals(family->wife,elem) == 0) {
				if (flag == 0) {

					if (maxGen == 0)
						maxGen = 30;

					gens = malloc(sizeof(List)*(maxGen));
					for (int i = 0; i < maxGen; ++i)
						gens[i] = initializeList(&printIndividual,&deleteIndividualCopy,&alphabeticalCompare);

					flag = 1;

				}
				(*max) = 0;
				getChildrenListN(gens, maxGen, family->children, max);
			}

		}

		if (flag == 1) {
			for (int i = 0; i < maxGen; ++i) {
				if (gens[i].length > 0) {
					List *newL = malloc(sizeof(List));
					*newL = gens[i];
					insertBack(&generations, newL);
				} else {
					break;
				}
			}
			free(gens);
		}

	}

	return generations;
}

List getAncestorListN(const GEDCOMobject* familyRecord, const Individual* person, int maxGen) {

	List ancestors = initializeList(&printGeneration, &deleteGeneration, &compareGenerations);

	if (familyRecord == NULL || person == NULL)
		return ancestors;

	List individuals = familyRecord->individuals;
	//First check if the individual exists in the family record
	ListIterator iter = createIterator(individuals);
	void* elem = NULL;
	while( (elem = nextElement(&iter)) != NULL) {
		if (compareIndividuals(elem, person) == 0)
			break;
	}
	if (elem != NULL) { //If the individual exists, find the family in which they are a child

		List *ances;
		int count = 0;
		int *max = &count;
		Individual *ind = (Individual *)elem;

		if (maxGen == 0)
			maxGen = 30;

		ances = malloc(sizeof(List)*(maxGen));
			for (int i = 0; i < maxGen; ++i)
				ances[i] = initializeList(&printIndividual,&deleteIndividualCopy,&alphabeticalCompare);

		List loadedInivs = initializeList(&printIndividual,&dummyDelete,&alphabeticalCompare);
		
		getAncestors(ances, maxGen, ind->families, ind, max ,&loadedInivs);

		clearList(&loadedInivs);

		//No generations should contain the original individual
		// If this is the case, a loop occurred and it will eliminate uneccesary lists
		int newGenMax = maxGen;
		int loopFlag = 0;
		for (int i = 0; i < maxGen; ++i) {
			void *found = NULL;
			if (loopFlag == 0)
				found = findElement(ances[i], &findIndividual, ind);
			if (found != NULL)
				loopFlag = 1;
			if (loopFlag == 1) {
				clearList(&ances[i]);
				--newGenMax;
			}
		}

		for (int i = 0; i < newGenMax; ++i) {
			if (ances[i].length != 0) {
				List *newL = malloc(sizeof(List));
				*newL = ances[i];
				insertBack(&ancestors, newL);
			} else {
				break;
			}
		}
		free(ances);

	}

	return ancestors;
}


char* indToJSON(const Individual* ind) {

	if (ind == NULL) {
		char *string = malloc(sizeof(char) * (2));
		strcpy(string,"");
		return string;
	}	

	char givenName[] = "{\"givenName\":\"";
	char surname[] = "\",\"surname\":\"";
	char end[] = "\"}";

	int size = strlen(givenName) + strlen(surname) + strlen(ind->givenName) + strlen(ind->surname) + strlen(end) + 1;
	char *string = malloc(sizeof(char) * (size));

	strcpy(string, givenName);
	strcat(string, ind->givenName);
	strcat(string, surname);
	strcat(string, ind->surname);
	strcat(string, end);

	return string;

}

Individual* JSONtoInd(const char* str) {

	char givenName[] = "{\"givenName\":\"";
	char surname[] = "\",\"surname\":\"";
	char end[] = "\"}";
	int minLength = strlen(givenName) + strlen(surname) + strlen(end);

	if (str == NULL || strlen(str) < minLength)
		return NULL;

	int count = 0;
	for (int i = 0; i < strlen(str); ++i)
		if (str[i] == '"')
			++count;
		
	if (count < 8 || str[0] != '{' || str[strlen(str)-1] != '}')
		return NULL;

	if ((strstr(str,"{\"givenName\":\"") == NULL && strstr(str,"{\"surname\":\"") == NULL) || strstr(str,"\",\"") == NULL || (strstr(str,"\"surname\":\"") == NULL && strstr(str,"\"givenName\":\"") == NULL) || strstr(str,"\"}") == NULL)
		return NULL;


	Individual *ind = splitJSONInd(str);

	return ind;

}

GEDCOMobject* JSONtoGEDCOM(const char* str) {

	char source[] = "{\"source\":\"";
	char gedcVersion[] = "\",\"gedcVersion\":\"";
	char encoding[] = "\",\"encoding\":\"";
	char subName[] = "\",\"subName\":\"";
	char subAddress[] = "\",\"subAddress\":\"";
	char end[] = "\"}";
	int minLength = strlen(source) + strlen(gedcVersion) + strlen(encoding) + strlen(subName) + strlen(subAddress) + strlen(end);

	if (str == NULL || strlen(str) < minLength)
		return NULL;

	int count = 0;
	for (int i = 0; i < strlen(str); ++i)
		if (str[i] == '"')
			++count;
		
	if (count < 20 || str[0] != '{' || str[strlen(str)-1] != '}')
		return NULL;

	if (strstr(str,"\"source\":\"") == NULL || strstr(str,"\",\"") == NULL || strstr(str,"\"gedcVersion\":\"") == NULL || strstr(str,"\"encoding\":\"") == NULL || strstr(str,"\"subName\":\"") == NULL || strstr(str,"\"subAddress\":\"") == NULL || strstr(str,"\"}") == NULL)
		return NULL;

	GEDCOMobject *newObj = splitJSONObj(str);

	return newObj;
}

void addIndividual(GEDCOMobject* obj, const Individual* toBeAdded) {

	if (obj == NULL || toBeAdded == NULL)
		return;

	insertBack(&obj->individuals, (void *)toBeAdded);
}

char* iListToJSON(List iList) {

	char bounds[] = "[]";
	char *string = malloc(sizeof(char) * strlen(bounds) + 1);

	strcpy(string,"[");

	ListIterator iter2 = createIterator(iList);
	void* data = NULL;
	int flag = 0;
	while( (data = nextElement(&iter2)) != NULL) {
		Individual *ind = (Individual *)data;
		char *newInd = indToJSON(ind);
		string = realloc(string, sizeof(char) * (strlen(string) + strlen(newInd) + 3));
		strcat(string, newInd);
		strcat(string, ",");
		free(newInd);
		flag = 1;
	}
	if (flag == 1)
		string[strlen(string)-1] = ']';
	else strcpy(string, "[]");

	return string;

}


char* gListToJSON(List gList) {

	char bounds[] = "[]";
	char *string = malloc(sizeof(char) * strlen(bounds) + 1);

	strcpy(string,"[");

	ListIterator iter2 = createIterator(gList);
	void* data = NULL;
	int flag = 0;
	while( (data = nextElement(&iter2)) != NULL) {
		List *iList = (List *)data;
		char *newGen = iListToJSON(*iList);
		string = realloc(string, sizeof(char) * (strlen(string) + strlen(newGen) + 3));
		strcat(string, newGen);
		strcat(string, ",");
		free(newGen);
		flag = 1;
	}
	if (flag == 1)
		string[strlen(string)-1] = ']';
	else strcpy(string, "[]");

	return string;

}



// HELPER FUNCTIONS
char *printGeneration(void* toBePrinted) {

	if (toBePrinted == NULL)
		return NULL;

	List *generation = (List *)toBePrinted;

	char *string = malloc(sizeof(char) * 2);
	strcpy(string, "");

	if (generation->length == 0)
		return string;

	ListIterator iter = createIterator(*generation);
	Individual* ind = NULL;
	while( (ind = (Individual *)nextElement(&iter)) != NULL) {

		string = realloc(string, sizeof(char) * (strlen(string) + strlen(ind->givenName) + strlen(ind->surname) + 3));
		strcat(string, ind->givenName);
		strcat(string, " ");
		strcat(string, ind->surname);
		strcat(string, "\n");

	}

	return string;
}

void deleteGeneration (void* toBeDeleted) {

	List *generation = (List *)toBeDeleted;
	clearList(generation);
	free(generation);
}

int compareGenerations (const void* first,const void* second) {

	if (first == NULL || second == NULL)
		return -1;

	List *generation1 = (List *)first;
	List *generation2 = (List *)second;

	if (generation1->length != generation2->length)
		return -1;

	ListIterator iter = createIterator(*generation1);
	Individual* ind = NULL;
	while( (ind = (Individual *)nextElement(&iter)) != NULL) {

		void *found = findElement(*generation2, &findIndividual, ind);
		if (found == NULL)
			return -1;

	}

	return 0;

}

/******************************* A1 *************************************/
GEDCOMerror createGEDCOM(char* fileName, GEDCOMobject** obj) {

	GEDCOMerror errorType;
	errorType.type = OK;
	errorType.line = -1;

	FILE *fptr = NULL;
	fptr = myOpenforReadOrWrite(fileName, READ);

	if (fptr == NULL) {
		errorType.type = INV_FILE;
		errorType.line = -1;
		return errorType;
	} else {
		errorType = scanFile(fptr, obj);
	}

	return errorType;
}

char* printGEDCOM(const GEDCOMobject* obj) {

	if (obj == NULL)
		return NULL;

	char famHead[21] = "\nFAMILIES\n########\n";
	char indHead[27] = "\nINDIVIDUALS\n############\n";
	char subRec[35] = "SUBMISSION RECORD\n###############\n";
	char others[23] = "Other Info\n~~~~~~~~~~\n";
	char end[28] = "\n-------------------------\n";

	char *object = malloc(sizeof(char) * (strlen(subRec) + strlen(end) + strlen(obj->submitter->submitterName) + strlen(obj->submitter->address) + strlen(obj->header->source) + 95));
	char version[10];
	strcpy(object, "HEADER\n#########\n");
	strcat(object, "Source: ");
	strcat(object, obj->header->source);
	if (obj->header->encoding == UTF8)
		strcat(object, "\nEncoding: UTF-8\n");
	else if (obj->header->encoding == ANSEL)
		strcat(object, "\nEncoding: ANSEL\n");
	else if (obj->header->encoding == UNICODE)
		strcat(object, "\nEncoding: UNICODE\n");
	else strcat(object, "\nEncoding: ASCII\n");
	sprintf(version,"%.2lf",obj->header->gedcVersion);
	strcat(object, "Version: ");
	strcat(object, version);
	strcat(object, "\n\n");
	strcat(object, subRec);
	strcat(object, "Submitter Name: ");
	strcat(object, obj->submitter->submitterName);
	strcat(object, "\n");
	strcat(object, "Address: ");
	strcat(object, obj->submitter->address);
	strcat(object, "\n");

	ListIterator iter = createIterator(obj->submitter->otherFields);
	void* data = NULL;
	int i = 0;
	while( (data = nextElement(&iter)) != NULL) {
		if (i == 0) {
			object = realloc(object, strlen(others) + strlen(object) + 22);
			strcat(object, others);
		}
		char *temp = printField(data);
		object = realloc(object, strlen(temp) + strlen(object) + 22);
		strcat(object,"\n");
		strcat(object, temp);
		free(temp);
		i = 1;
	}
	strcat(object, end);

	iter = createIterator(obj->individuals);
	data = NULL;
	i = 0;
	while( (data = nextElement(&iter)) != NULL) {
		if (i == 0) {
			object = realloc(object, strlen(indHead) + strlen(object) + 22);
			strcat(object,indHead);
		} 
		char *temp = printIndividual(data);
		object = realloc(object, strlen(temp) + strlen(object) + 22);
		strcat(object,"\n");
		strcat(object, temp);
		free(temp);
		i = 1;
	}

	iter = createIterator(obj->families);
	data = NULL;
	i = 0;
	while( (data = nextElement(&iter)) != NULL) {
		if (i == 0) {
			object = realloc(object, strlen(famHead) + strlen(object) + 22);
			strcat(object,famHead);
		} 
		char *temp = printFamily(data);
		object = realloc(object, strlen(temp) + strlen(object) + 22);
		strcat(object,"\n");
		strcat(object, temp);
		free(temp);
		i = 1;
	}
	return object;

}

void deleteGEDCOM(GEDCOMobject* obj) {

	if (obj == NULL)
		return;

	//Delete submitter other fields
	ListIterator iter = createIterator(obj->submitter->otherFields);
	void* data = NULL;
	while( (data = nextElement(&iter)) != NULL) {
		deleteField(data);
	}
	//freeSubmitter
	clearList(&obj->submitter->otherFields);
	free(obj->submitter);

	//free individuals
	iter = createIterator(obj->individuals);
	data = NULL;
	while( (data = nextElement(&iter)) != NULL) {
		deleteIndividual(data);
	}
	clearList(&obj->individuals);

	//free Families
	iter = createIterator(obj->families);
	data = NULL;
	while( (data = nextElement(&iter)) != NULL) {
		deleteFamily(data);
	}
	clearList(&obj->families);
	//free Header->otherFields
	iter = createIterator(obj->header->otherFields);
	data = NULL;
	while( (data = nextElement(&iter)) != NULL) {
		deleteField(data);
	}
	clearList(&obj->header->otherFields);
	free(obj->header);

	free(obj);

}

char* printError(GEDCOMerror err) {

	char line[10];
	char *error = malloc(sizeof(char) * 40);
	if (err.type == OK) {
		strcpy(error, "OK\nLine: -1\n");
	} else if (err.type == INV_FILE) {
		strcpy(error, "Invalid File\nLine: -1\n");

	} else if (err.type == INV_GEDCOM) {
		strcpy(error, "Invalid Gedcom\n");
		sprintf(line, "%d", err.line);
		strcat(error, "Line: ");
		strcat(error, line);
	} else if (err.type == INV_HEADER) {
		strcpy(error, "Invalid Header\n");
		sprintf(line, "%d", err.line);
		strcat(error, "Line: ");
		strcat(error, line);
		strcat(error, "\n");
	} else if (err.type == INV_RECORD) {
		strcpy(error, "Invalid Record\n");
		sprintf(line, "%d", err.line);
		strcat(error, "Line: ");
		strcat(error, line);
	} else {
		strcpy(error, "Unknown Error\n");
		strcat(error, "Line: -1\n");
	}

	return error;

}


Individual* findPerson(const GEDCOMobject* familyRecord, bool (*compare)(const void* first, const void* second), const void* person) {

	if (familyRecord == NULL || compare == NULL || person == NULL)
		return NULL;

	ListIterator iter = createIterator(familyRecord->individuals);
	void* elem = NULL;
	while( (elem = nextElement(&iter)) != NULL) {
		if (compare(elem,person))
			return elem;
	}

	return NULL;
}


List getDescendants(const GEDCOMobject* familyRecord, const Individual* person) {

	List descendants = initializeList(&printIndividual,&deleteIndividual,&compareIndividuals);
	if (familyRecord == NULL || person == NULL)
		return descendants;

	List individuals = familyRecord->individuals;
	//First check if the individual exists in the family record
	ListIterator iter = createIterator(individuals);
	void* elem = NULL;
	while( (elem = nextElement(&iter)) != NULL) {
		if (compareIndividuals(elem, person) == 0)
			break;
	}
	if (elem != NULL) { //If the individual exists, find the family in which they are a parent
		Individual *ind = (Individual *)elem;
		List families = ind->families;
		ListIterator iter2 = createIterator(families);
		void* data = NULL;
		while( (data = nextElement(&iter2)) != NULL) {
			Family *family = (Family *)data;
			if (compareIndividuals(family->husband,elem) == 0 || compareIndividuals(family->wife,elem) == 0) {
				getChildren(&descendants, family->children);
			}
		}
	}

	return descendants;

}

/************************************************************************************************************/

/*******************LIST FUNCTIONS (Event List) ***********************/
char *printEvent(void* toBePrinted) {
	Event *e = (Event *)toBePrinted;
	char *string = malloc(sizeof(char) * (strlen(e->type) + strlen(e->date) + strlen(e->place) + 6));
	strcpy(string, "");
	strcat(string, e->type);
	strcat(string, " ");
	strcat(string, e->place);
	strcat(string, ", ");
	strcat(string, e->date);
	
	return string;
}

void deleteEvent (void* toBeDeleted) {

	if (toBeDeleted == NULL)
		return;

	Event *e = (Event *)toBeDeleted;

	free(e->date);
	free(e->place);
	e->date = NULL;
	e->place = NULL;

	ListIterator iter = createIterator(e->otherFields);
	void* elem = NULL;
	while( (elem = nextElement(&iter)) != NULL) {
		deleteField(elem);
	}
	clearList(&e->otherFields);

	free(e);
	e = NULL;

}

int compareEvents (const void* first,const void* second) {
	Event *a = (Event *)first;
	Event *b = (Event *)second;

	int x = strcmp(a->type,b->type);

	return x;
}
/*****************************************************************************/

/*******************LIST FUNCTIONS (Individuals List) ***********************/
void deleteIndividual(void* toBeDeleted) { 

	if (toBeDeleted == NULL) 
		return;
	Individual *ind = (Individual *)toBeDeleted;

	free(ind->givenName);
	free(ind->surname);

	ind->givenName = NULL;
	ind->surname = NULL;

	//Delete events list
	ListIterator iter = createIterator(ind->events);
	void* elem = NULL;
	while( (elem = nextElement(&iter)) != NULL) {
		deleteEvent(elem);
	}
	clearList(&ind->events);
	//Delete otherFields List
	iter = createIterator(ind->otherFields);
	elem = NULL;
	while( (elem = nextElement(&iter)) != NULL) {
		deleteField(elem);
	}
	clearList(&ind->otherFields);
	clearList(&ind->families);

	free(ind);
	ind = NULL;

}

int compareIndividuals(const void* first,const void* second) {

	if (first == NULL || second == NULL)
		return -1;

	Individual *a = (Individual *)first;
	Individual *b = (Individual *)second;

	char *c = malloc(sizeof(char) * (strlen(a->givenName) + strlen(a->surname) + 5));
	char *d = malloc(sizeof(char) * (strlen(b->givenName) + strlen(b->surname) + 5));

	strcpy(c,"");
	strcpy(d,"");

	strcat(c,a->givenName);
	strcat(c, ",");
	strcat(c, a->surname);

	strcat(d, b->givenName);
	strcat(d, ",");
	strcat(d, b->surname);

	int x = strcmp(c,d);

	strcpy(c,"");
	strcpy(d,"");

	free(c);
	c = NULL;
	free(d);
	d = NULL;

	//For API search 
	if (x == 0 && getLength(b->otherFields) == 0 && getLength(b->events) == 0 && getLength(b->families) == 0)
		return 0;

	if (x == 0) {
		if ( (getLength(a->otherFields) != getLength(b->otherFields)) || (getLength(a->events) != getLength(b->events)) || (getLength(a->families) != getLength(b->families)))
			return -1;
		if (getLength(a->events) != 0) {
			Event *e = (Event *)getFromFront(a->events);
			Event *e2 = (Event *)getFromFront(b->events);
			if (strcmp(e->date,e2->date) != 0)
				return -1;
		}
	}
	
	return x;
}

char* printIndividual(void* toBePrinted) {

	if (toBePrinted == NULL)
		return NULL;

	Individual *ind = (Individual *)toBePrinted;

	char eventHeading[] = "Events\n~~~~~~\n";
	char others[] = "Other Info\n~~~~~~~~~~\n";
	char end[] = "-------------------------\n";

	char *string = malloc(sizeof(char) * (strlen(ind->givenName) + strlen(ind->surname) + 25));

	strcpy(string, "First Name: ");
	strcat(string, ind->givenName);
	strcat(string, "\n");
	strcat(string, "Surname: ");
	strcat(string, ind->surname);
	strcat(string, "\n");

	ListIterator iter = createIterator(ind->events);
	void* elem = NULL;
	int i = 0;
	while( (elem = nextElement(&iter)) != NULL) {
		char *s2 = printEvent(elem);
		if (i == 0) {
			string = realloc(string, strlen(string)+ strlen(eventHeading) + 1);
			strcat(string, eventHeading);
		}
		string = realloc(string, strlen(string)+strlen(s2)+10);
		strcat(string, s2);
		strcat(string,"\n");
		free(s2);
		i = 1;
	}

	iter = createIterator(ind->otherFields);
	elem = NULL;
	i = 0;
	while( (elem = nextElement(&iter)) != NULL) {
		char *s2 = printField(elem);
		if (i == 0) {
			string = realloc(string, strlen(string)+ strlen(others) + 1);
			strcat(string, others);
		}
		string = realloc(string, strlen(string)+strlen(s2)+10);
		strcat(string, s2);
		strcat(string,"\n");
		free(s2);
		i = 1;
	}
	string = realloc(string, strlen(string) + strlen(end) +2);
	strcat(string, end);

	return string;
}
/*****************************************************************************/

/******************* LIST FUNCTIONS (OtherFields List) ***********************/
char *printField(void* toBePrinted) {
	if (toBePrinted == NULL)
		return NULL;

	Field *a = (Field *)toBePrinted;

	char *string = malloc(sizeof(char) * (strlen(a->tag) + strlen(a->value) + 6));
	strcpy(string,a->tag);
	strcat(string, ": ");
	strcat(string, a->value);

	return string;
}

void deleteField (void* toBeDeleted) {
	if (toBeDeleted == NULL)
		return;
	Field *f = (Field *)toBeDeleted;
	free(f->tag);
	if (f->value != NULL)
		free(f->value);
	f->tag = NULL;
	f->value = NULL;

	free(f);
	f = NULL;
}

int compareFields (const void* first,const void* second) {
	Field *a = (Field *)first;
	Field *b = (Field *)second;

	char *c = malloc(sizeof(char) * (strlen(a->tag) + strlen(a->value) + 5));
	char *d = malloc(sizeof(char) * (strlen(b->tag) + strlen(b->value) + 5));

	strcpy(c,"");
	strcpy(d,"");

	strcat(c,a->tag);
	strcat(c, " ");
	strcat(c, a->value);

	strcat(d, b->tag);
	strcat(d, " ");
	strcat(d, b->value);

	int x = strcmp(c,d);

	strcpy(c,"");
	strcpy(d,"");

	free(c);
	c = NULL;
	free(d);
	d = NULL;

	return x;
}
/*****************************************************************************/

/*******************LIST FUNCTIONS (Families List) ***********************/
void deleteFamily(void* toBeDeleted) {

	if (toBeDeleted == NULL)
		return;

	Family *f = (Family *)toBeDeleted;

	//Delete events list
	ListIterator iter = createIterator(f->events);
	void* elem = NULL;
	while( (elem = nextElement(&iter)) != NULL) {
		deleteEvent(elem);
	}
	clearList(&f->events);
	//Delete otherFields List
	iter = createIterator(f->otherFields);
	elem = NULL;
	while( (elem = nextElement(&iter)) != NULL) {
		deleteField(elem);
	}
	clearList(&f->otherFields);
	clearList(&f->children);


	free(f);
	f = NULL;
}

int compareFamilies(const void* first,const void* second) {
	if (first == NULL || second == NULL)
		return -1;

	Family *f1 = (Family *)first;
	Family *f2 = (Family *)second;

	int fam1 = getLength(f1->children);
	int fam2 = getLength(f1->children);

	if (f1->wife != NULL)
		++f1;
	if (f1->husband != NULL) 
		++f1;
	if (f2->wife != NULL)
		++f2;
	if (f2->husband != NULL) 
		++f2;

	if (fam1 > fam2)
		return 1;
	else if (fam1 < fam2)
		return -1;

	return 0;

}

char* printFamily(void* toBePrinted) {

	if (toBePrinted == NULL)
		return NULL;

	Family *f = (Family *)toBePrinted;
	char *string;

	char husb[9] = "Father: ";
	char wife[9] = "Mother: "; 
	char child[8] = "Child: ";
	char end[28] = "-------------------------\n";

	int flag = 0;


	if (f->wife != NULL && f->husband != NULL) {
		string = malloc(sizeof(char) * (strlen(f->husband->givenName) + strlen(f->husband->surname) + strlen(f->wife->givenName) + strlen(f->wife->surname) + 25));
		strcpy(string, "");
		strcat(string, husb);
		strcat(string, f->husband->givenName);
		strcat(string, " ");
		strcat(string, f->husband->surname);
		strcat(string, "\n");
		strcat(string, wife);
		strcat(string, f->wife->givenName);
		strcat(string, " ");
		strcat(string, f->wife->surname);
		strcat(string, "\n");
		flag = 1;
	} else if (f->wife == NULL && f->husband != NULL) {
		string = malloc(sizeof(char) * (strlen(f->husband->givenName) + strlen(f->husband->surname)+ 15));
		strcpy(string, "");
		strcat(string, husb);
		strcat(string, f->husband->givenName);
		strcat(string, " ");
		strcat(string, f->husband->surname);
		strcat(string, "\n");
		flag = 1;
	} else if (f->wife != NULL && f->husband == NULL) {
		string = malloc(sizeof(char) * (strlen(f->wife->givenName) + strlen(f->wife->surname) + 15));
		strcpy(string, "");
		strcat(string, wife);
		strcat(string, f->wife->givenName);
		strcat(string, " ");
		strcat(string, f->wife->surname);
		strcat(string, "\n");
		flag = 1;
	} else {
		return "";
	}

	if (flag == 1) {
		ListIterator iter = createIterator(f->children);
		void* elem = NULL;
		while( (elem = nextElement(&iter)) != NULL) {
			Individual* ind = (Individual *)elem;
			string = realloc(string, strlen(string) + strlen(ind->givenName) + strlen(ind->surname) + strlen(child) + 4);
			strcat(string, child);
			strcat(string, ind->givenName);
			strcat(string, " ");
			strcat(string, ind->surname);
			strcat(string, "\n");
		}

		string = realloc(string, strlen(string) + strlen(end) + 1);
		strcat(string, end);
	}


	return string;
}
/*****************************************************************************/


