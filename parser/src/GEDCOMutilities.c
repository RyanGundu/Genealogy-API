#include "GEDCOMutilities.h"

GEDCOMerror writeToFile(char* fileName, const GEDCOMobject* obj) {

	GEDCOMerror errorType;
	errorType.type = OK;
	errorType.line = -1;

	if (obj == NULL || fileName == NULL) {
		errorType.type = WRITE_ERROR;
		errorType.line = -1;
		return errorType;
	}
	FILE *fptr2 = NULL;
	fptr2 = myOpenforReadOrWrite(fileName, WRITE);
	//Load Xrefs
	char **indXrefs = generateIndXrefs(obj);
	char **famXrefs = generateFamXrefs(obj);

	List newIndividuals = initializeList(&printIndivFunction,&dummyDelete, &compareIndRecordTag);
	List newFamilies = initializeList(&printFam,&dummyDelete,&compareFam);

	ListIterator iter = createIterator(obj->individuals);
	void* data = NULL;
	int i = 0;
	while( (data = nextElement(&iter)) != NULL && indXrefs[i] != NULL) {
		Individual *ind = (Individual *)data;
		Person *p = createPerson(indXrefs[i]);
		free(p->tag);
		free(p->person);
		p->person = ind;
		p->tag = indXrefs[i];
		insertFront(&newIndividuals,p);
		++i;
	}
	iter = createIterator(obj->families);
	data = NULL;
	i = 0;
	while( (data = nextElement(&iter)) != NULL && famXrefs[i] != NULL) {
		Family *fam = (Family *)data;
		Fam *f = createFamily(famXrefs[i]);
		free(f->tag);
		free(f->family);
		f->family = fam;
		f->tag = famXrefs[i];
		insertFront(&newFamilies,f);
		++i;
	}

	//Write to file
	writeHeader(fptr2, obj);
	writeSubmitter(fptr2, obj);
	writeIndividuals(fptr2, obj, newIndividuals, newFamilies);
	writeFamilies(fptr2, obj, newFamilies, newIndividuals);
	fprintf(fptr2, "0 TRLR\n");


	freeXrefs(NULL, &newIndividuals, &newFamilies);
	free(indXrefs);
	free(famXrefs);
	clearList(&newIndividuals);
	clearList(&newFamilies);
	fclose(fptr2);
	return errorType;

}

char **generateIndXrefs (const GEDCOMobject* obj) {	

	char idStart[] = "@I0";
	char num[10];
	int numFams = getLength(obj->individuals);
	if (numFams > 0) {
		char **xrefs = malloc(sizeof(char *) * (numFams + 1) );
		sprintf(num, "%d", numFams);
		int i = 0;
		for (i = 0; i < numFams; ++i) {
			xrefs[i] = malloc(sizeof(char) * (strlen(num) + 5));
			strcpy(xrefs[i],idStart);
			strcpy(num, "");
			sprintf(num, "%d", i+1);
			strcat(xrefs[i], num);
			strcat(xrefs[i],"@");
		}
		xrefs[i] = NULL;
		return xrefs;
	}
	return NULL;
}

char **generateFamXrefs (const GEDCOMobject* obj) {	

	char idStart[] = "@F0";
	char num[10];
	int numInd = getLength(obj->families);
	if (numInd > 0) {
		char **xrefs = malloc(sizeof(char *) * (numInd + 1) );
		sprintf(num, "%d", numInd);
		int i = 0;
		for (i = 0; i < numInd; ++i) {
			xrefs[i] = malloc(sizeof(char) * (strlen(num) + 5));
			strcpy(xrefs[i],idStart);
			strcpy(num, "");
			sprintf(num, "%d", i+1);
			strcat(xrefs[i], num);
			strcat(xrefs[i],"@");
		}
		xrefs[i] = NULL;
		return xrefs;
	}
	return NULL;
}



void writeHeader (FILE *fptr, const GEDCOMobject* obj) {

	fprintf(fptr, "0 HEAD\n");
	fprintf(fptr, "1 SOUR %s\n", obj->header->source);
	fprintf(fptr, "1 GEDC\n");
	fprintf(fptr, "2 VERS %.2lf\n", obj->header->gedcVersion);
	fprintf(fptr, "2 FORM LINEAGE-LINKED\n");

	if (obj->header->encoding == UTF8)
		fprintf(fptr, "1 CHAR UTF-8\n");
	else if (obj->header->encoding == ANSEL)
		fprintf(fptr, "1 CHAR ANSEL\n");
	else if (obj->header->encoding == UNICODE)
		fprintf(fptr, "1 CHAR UNICODE\n");
	else fprintf(fptr, "1 CHAR ASCII\n");

	fprintf(fptr, "1 SUBM @SUB1@\n");

}

void writeSubmitter (FILE *fptr, const GEDCOMobject* obj) {

	fprintf(fptr, "0 @SUB1@ SUBM\n");
	fprintf(fptr, "1 NAME %s\n", obj->submitter->submitterName);
	if(strcmp(obj->submitter->address,"") != 0)
		fprintf(fptr, "1 ADDR %s\n", obj->submitter->address);

}

void writeIndividuals (FILE *fptr, const GEDCOMobject* obj, List individuals, List families) {

	ListIterator iter = createIterator(individuals);
	void* data = NULL;
	while( (data = nextElement(&iter)) != NULL) {
		Person *p = (Person *)data;
		fprintf(fptr, "0 %s INDI\n", p->tag); //Ex. 0 @I012@ INDI

		fprintf(fptr, "1 NAME %s /%s/\n", p->person->givenName, p->person->surname);

		if (strcmp(p->person->givenName, "") != 0)
			fprintf(fptr, "2 GIVN %s\n", p->person->givenName);
		if (strcmp(p->person->surname, "") != 0)
			fprintf(fptr, "2 SURN %s\n", p->person->surname);

		/*********************** Print OtherFields & Events **************************/
		ListIterator iter2 = createIterator(p->person->events);
		void *data2 = NULL;
		while( (data2 = nextElement(&iter2)) != NULL) {
			Event *e = (Event *)data2;
			fprintf(fptr, "1 %s\n", e->type);
			if(strcmp(e->date, "") != 0)
				fprintf(fptr, "2 DATE %s\n", e->date);
			if (strcmp(e->place, "") != 0)
				fprintf(fptr, "2 PLAC %s\n", e->place);

		}
		iter2 = createIterator(p->person->otherFields);
		data2 = NULL;
		while( (data2 = nextElement(&iter2)) != NULL) {
			Field *f = (Field *)data2;
			if (strcmp(f->tag, "GIVN") != 0 && strcmp(f->tag, "SURN") != 0)
				fprintf(fptr, "1 %s %s\n", f->tag, f->value);
		}
		/****************************************************************************/
		/*********************** Print Related Families *****************************/
		iter2 = createIterator(families);
		data2 = NULL;
		while( (data2 = nextElement(&iter2)) != NULL) {
			Fam *f = (Fam *)data2;
			void *fam = findElement(p->person->families,&compareNewFamily,f->family);	
			if (fam != NULL) {
				if (isParent(f->family, p->person) == 1) {
					fprintf(fptr, "1 FAMS %s\n", f->tag);
				} else if (isChild(f->family, p->person) == 1){
					fprintf(fptr, "1 FAMC %s\n", f->tag);
				}
			}
		}
		/****************************************************************************/

	}
}

void writeFamilies (FILE *fptr, const GEDCOMobject* obj, List families, List individuals) {

	ListIterator iter = createIterator(families);
	void* data = NULL;
	while( (data = nextElement(&iter)) != NULL) {
		Fam *f = (Fam *)data;
		fprintf(fptr, "0 %s FAM\n", f->tag); //Ex. 0 @F012@ FAM

		void *husb = findElement(obj->individuals, &findIndividual, f->family->husband);
		if (husb != NULL) {
			ListIterator iter2 = createIterator(individuals);
			void *data2 = NULL;
			while( (data2 = nextElement(&iter2)) != NULL) {
				Person *p = (Person *)data2;
				if (compareIndividuals(husb,p->person) == 0)
					fprintf(fptr, "1 HUSB %s\n", p->tag);
			}
		}

		void *wife = findElement(obj->individuals, &findIndividual, f->family->wife);
		if (wife != NULL) {
			ListIterator iter2 = createIterator(individuals);
			void *data2 = NULL;
			while( (data2 = nextElement(&iter2)) != NULL) {
				Person *p = (Person *)data2;
				if (compareIndividuals(wife,p->person) == 0)
					fprintf(fptr, "1 WIFE %s\n", p->tag);
			}
		}
		ListIterator iter2 = createIterator(f->family->events);
		void *data2 = NULL;
		while( (data2 = nextElement(&iter2)) != NULL) {
			Event *e = (Event *)data2;
			fprintf(fptr, "1 %s\n", e->type);
			if(strcmp(e->date, "") != 0)
				fprintf(fptr, "2 DATE %s\n", e->date);
			if (strcmp(e->place, "") != 0)
				fprintf(fptr, "2 PLAC %s\n", e->place);

		}
		//Print children
		iter2 = createIterator(individuals);
		data2 = NULL;
		while( (data2 = nextElement(&iter2)) != NULL) {
			Person *p = (Person *)data2;
			void *child = findElement(f->family->children, &findIndividual, p->person);
			if (child != NULL)
				fprintf(fptr, "1 CHIL %s\n", p->tag);
		}

	}

}

int checkDepth (List children, int *genCount) {

	int max = 0;
	if (max < (*genCount))
		max = (*genCount);

	ListIterator iter = createIterator(children);
	void* elem = NULL;
	while( (elem = nextElement(&iter)) != NULL) {
		if (max < 1)
			max = 1;
		Individual *ind = (Individual *)elem;
		List families = ind->families;
		ListIterator iter2 = createIterator(families);
		void* elem2 = NULL;
		while( (elem2 = nextElement(&iter2)) != NULL) {

			Family *family = (Family *)elem2;

			if (family->husband == elem||family->wife == elem) {
				++(*genCount);
				max = checkDepth(family->children, genCount);
				--(*genCount);
			}
		}
	}

	return max;
}

void getChildrenListN (List *generations, unsigned int maxGen, List children, int *genCount) {


	if (generations == NULL)
		return;
	if (maxGen != 0 && (*genCount) == maxGen)
		return;

	ListIterator iter = createIterator(children);
	void* elem = NULL;
	while( (elem = nextElement(&iter)) != NULL) {
		//make copy
		Individual *ind = (Individual *)elem;
		Individual *indCopy = copyIndividual(ind);

		void *toInsert = findElement(generations[(*genCount)], &findIndividual, ind);
		if (toInsert == NULL) {
			if (strcmp(ind->surname, "") == 0)
				insertBack(&generations[(*genCount)], indCopy);
			else insertSorted(&generations[(*genCount)], indCopy);
		}

		List families = ind->families;
		ListIterator iter2 = createIterator(families);
		void* elem2 = NULL;
		while( (elem2 = nextElement(&iter2)) != NULL) {

			Family *family = (Family *)elem2;

			if (compareIndividuals(family->husband,elem) == 0 ||compareIndividuals(family->wife,elem) == 0) {
				++(*genCount);
				getChildrenListN(generations, maxGen, family->children, genCount);
				--(*genCount);
			}
		}

	}

}

int checkHeight (List families, Individual *person, int *genCount) {

	int max = 0;
	if (max < (*genCount))
		max = (*genCount);

	ListIterator iter = createIterator(families);
	void* elem = NULL;
	while( (elem = nextElement(&iter)) != NULL) {
		Family *fam = (Family *)elem;
		if (compareIndividuals(fam->husband, person) != 0 && compareIndividuals(fam->wife, person) != 0) {
			
			if (max < 1)
				max = 1;

			if (fam->husband != NULL) {
				++(*genCount);
				max = checkHeight(fam->husband->families, fam->husband, genCount);
				--(*genCount);
			}
			if (fam->wife != NULL) {
				++(*genCount);
				 max = checkHeight(fam->wife->families, fam->wife, genCount);
				--(*genCount);
			}
			
		}

	}

	return max;
}

void getAncestors (List *ancestors, unsigned int maxGen, List families, Individual *person, int *genCount, List *loadedIndivs) {

	if (ancestors == NULL)
		return;
	if (maxGen != 0 && (*genCount) == maxGen)
		return;

	ListIterator iter = createIterator(families);
	void* elem = NULL;
	while( (elem = nextElement(&iter)) != NULL) {
		Family *fam = (Family *)elem;
		if (compareIndividuals(fam->husband, person) != 0 && compareIndividuals(fam->wife, person) != 0) {

			if (fam->husband != NULL) {
				Individual *indCopy = copyIndividual(fam->husband);

				void *exist = findElement(*loadedIndivs, &findIndividual, fam->husband);
				void *toInsert = findElement(ancestors[(*genCount)], &findIndividual, fam->husband);
				if (toInsert == NULL && exist == NULL) {
					if (strcmp(indCopy->surname, "") == 0) 
						insertBack(&ancestors[(*genCount)], indCopy);
					else insertSorted(&ancestors[(*genCount)], indCopy);

					insertFront(loadedIndivs, indCopy);
				}

				++(*genCount);
				getAncestors(ancestors, maxGen, fam->husband->families, fam->husband, genCount, loadedIndivs);
				--(*genCount);
			}
			if (fam->wife != NULL) {
				Individual *indCopy = copyIndividual(fam->wife);

				void *exist = findElement(*loadedIndivs, &findIndividual, fam->wife);
				void *toInsert = findElement(ancestors[(*genCount)], &findIndividual, fam->wife);
				if (toInsert == NULL && exist == NULL) {
					if (strcmp(indCopy->surname, "") == 0)
						insertBack(&ancestors[(*genCount)], indCopy);
					else insertSorted(&ancestors[(*genCount)], indCopy);

					insertFront(loadedIndivs, indCopy);
				}

				++(*genCount);
				getAncestors(ancestors, maxGen, fam->wife->families, fam->wife, genCount, loadedIndivs);
				--(*genCount);
			}

		}

	}

}

int alphabeticalCompare (const void* first,const void* second) {

	Individual *ind1 = (Individual *)first;
	Individual *ind2 = (Individual *)second;

	int x = strcmp(ind1->surname, ind2->surname);

	if (x == 0)
		x = strcmp(ind1->givenName, ind2->givenName);

	return x;
}

int fragmentCount (const char *line) {

	if (line == NULL || strlen(line) == 0)
		return 0;

	char *copy = malloc(sizeof(char) * (strlen(line) + 1));
	strcpy(copy, line);
	int count = 0;
	char *words = strtok (copy, "\"");
	while (words != NULL) {
		++count;
		words = strtok (NULL, "\"");
	}
	free(copy);

	return count;
}



char **parseJSON(const char *line, int numWords) {

	char *words;
	int i = 0;
	char *copy = malloc(sizeof(char) * (strlen(line) + 1));
	strcpy(copy, line);
	char **frag = malloc(sizeof(char*) * (numWords + 1));
	words = strtok (copy, "\"");
	while (words != NULL) {

		frag[i] = malloc(sizeof(char) * strlen(words)+1);
		strcpy(frag[i], words);
		++i;
		words = strtok (NULL, "\"");
	}
	frag[i] = NULL;
	free(copy);

	return frag;

}

Individual* splitJSONInd (const char* str) {

	Individual *ind = malloc(sizeof(Individual));
	ind->givenName = malloc(sizeof(char)*2);
	ind->surname = malloc(sizeof(char)*2);
	strcpy(ind->givenName, "");
	strcpy(ind->surname, "");
	ind->events = initializeList(&printEvent,&dummyDelete,&compareEvents);
	ind->families = initializeList(&printFamily,&dummyDelete,&compareFamilies);
	ind->otherFields = initializeList(&printField,&dummyDelete,&compareFields);

	int numFragments = fragmentCount(str);

	char **fragments = parseJSON(str, numFragments);

	if (numFragments > 6) {

		int indGivn = 0;
		int indSurn = 0;

		for (int i = 0; i < numFragments - 2; ++i)
		{
			if (strcmp(fragments[i],"givenName") == 0 && indGivn == 0)
				indGivn = i;
			if (strcmp(fragments[i],"surname") == 0 && indSurn == 0)
				indSurn = i;
		}

		if (indGivn == 0 || indSurn == 0) {
			freeWords(fragments, numFragments);
			free(ind->givenName);
			free(ind->surname);
			return NULL;
		}

		if (strcmp(fragments[indGivn + 2],",") != 0 && strcmp(fragments[indGivn + 2],"}") != 0) {
			free(ind->givenName);
			ind->givenName = malloc(sizeof(char)*(strlen(fragments[indGivn + 2]) + 1));
			strcpy(ind->givenName, fragments[indGivn + 2]);
		}

		if (strcmp(fragments[indSurn + 2],",") != 0 && strcmp(fragments[indSurn + 2],"}") != 0) {
			free(ind->surname);
			ind->surname = malloc(sizeof(char)*(strlen(fragments[indSurn + 2]) + 1));
			strcpy(ind->surname, fragments[indSurn + 2]);
		}

		freeWords(fragments, numFragments);
		return ind;


	}

	//Should only reach this point if it did not parse correctly
	freeWords(fragments, numFragments);
	free(ind->givenName);
	free(ind->surname);

	return NULL;

}


GEDCOMobject* splitJSONObj (const char* str) {

	GEDCOMobject *newObj = malloc(sizeof(GEDCOMobject));
	newObj->header = malloc(sizeof(Header));
	newObj->header->otherFields = initializeList(&printField,&dummyDelete,&compareFields);
	newObj->submitter = malloc(sizeof(Submitter)*500);
	newObj->header->submitter = newObj->submitter;
	newObj->submitter->otherFields = initializeList(&printField,&dummyDelete,&compareFields);
	newObj->individuals = initializeList(&printIndividual,&dummyDelete,&compareIndividuals);
	newObj->families = initializeList(&printFamily,&dummyDelete,&compareFamilies);

	int numFragments = fragmentCount(str);

	char **fragments = parseJSON(str, numFragments);

	if (numFragments > 14) {

		int indSource = 0;
		int indVers = 0;
		int indEncoding = 0;
		int indName = 0;
		int indAddress = 0;
		for (int i = 0; i < numFragments - 2; ++i)
		{
			if (strcmp(fragments[i],"source") == 0 && indSource == 0)
				indSource = i;
			if (strcmp(fragments[i],"gedcVersion") == 0 && indVers == 0)
				indVers = i;
			if (strcmp(fragments[i],"encoding") == 0 && indEncoding == 0)
				indEncoding = i;
			if (strcmp(fragments[i],"subName") == 0 && indName == 0)
				indName = i;
			if (strcmp(fragments[i],"subAddress") == 0 && indAddress == 0)
				indAddress = i;
		}

		if (indSource == 0 || indVers == 0 || indEncoding == 0 || indName == 0 || indAddress == 0) {
			freeWords(fragments, numFragments);
			free(newObj->header);
			free(newObj->submitter);
			free(newObj);
			return NULL;
		}

		if (strcmp(fragments[indSource + 2],",") != 0 && strcmp(fragments[indSource + 2],"}") != 0) {
			strcpy(newObj->header->source, fragments[indSource + 2]);
		} else {
			strcpy(newObj->header->source, "");
		}

		if (strcmp(fragments[indVers + 2],",") != 0 && strcmp(fragments[indVers + 2],"}") != 0) {
			//sscanf(fragments[indVers + 2], "%f", (float)newObj->header->gedcVersion);
			newObj->header->gedcVersion = atof(fragments[indVers + 2]);
		} else {
			newObj->header->gedcVersion = 0;
		}

		if (strcmp(fragments[indEncoding + 2],",") != 0 && strcmp(fragments[indEncoding + 2],"}") != 0) {

			makeLowerCase(fragments[indEncoding + 2]);
			if (strcmp(fragments[indEncoding + 2],"ansel") == 0) {
				newObj->header->encoding = ANSEL;
			} else if (strcmp(fragments[indEncoding + 2],"utf8") == 0 || strcmp(fragments[indEncoding + 2],"utf-8") == 0) {
				newObj->header->encoding = UTF8;
			} else if (strcmp(fragments[indEncoding + 2],"unicode") == 0) {
				newObj->header->encoding = UNICODE;
			} else if (strcmp(fragments[indEncoding + 2],"ascii") == 0) {
				newObj->header->encoding = ASCII;
			} else {
				freeWords(fragments, numFragments);
				free(newObj->header);
				free(newObj->submitter);
				free(newObj);
				return NULL;
			}

		} else {
			freeWords(fragments, numFragments);
			free(newObj->header);
			free(newObj->submitter);
			free(newObj);
			return NULL;
		}

		if (strcmp(fragments[indName + 2],",") != 0 && strcmp(fragments[indName + 2],"}") != 0) {
			strcpy(newObj->submitter->submitterName, fragments[indName + 2]);
		} else {
			strcpy(newObj->submitter->submitterName, "");
		}

		if (strcmp(fragments[indAddress + 2],",") != 0 && strcmp(fragments[indAddress + 2],"}") != 0) {
			strcpy(newObj->submitter->address, fragments[indAddress + 2]);
		} else {
			strcpy(newObj->submitter->address, "");
		}

		return newObj;

	}

	freeWords(fragments, numFragments);
	free(newObj->header);
	free(newObj->submitter);
	free(newObj);

	return NULL;

}

void deleteIndividualCopy(void* toBeDeleted) { 

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

	free(ind);
	ind = NULL;

}

/******************************* A1 *************************************/


Individual *copyIndividual (const Individual *ind) {

		if (ind == NULL)
			return NULL;

		Individual *indCopy = malloc(sizeof(Individual));
		indCopy->givenName = malloc(sizeof(char) *(strlen(ind->givenName) +1));
		strcpy(indCopy->givenName, ind->givenName); //Assign the Name
		indCopy->surname = malloc(sizeof(char) *(strlen(ind->surname) +1));
		strcpy(indCopy->surname, ind->surname); //Assign the surname

		ListIterator fieldIter = createIterator(ind->otherFields);
		void* fields = NULL;
		List otherFields = initializeList(&printField,&dummyDelete,&compareFields);
		while( (fields = nextElement(&fieldIter)) != NULL) {
			Field *f = (Field *)fields;
			Field *fCopy = malloc(sizeof(Field));
			
			fCopy->tag = malloc(sizeof(char) * (strlen(f->tag) +1));
			strcpy(fCopy->tag, f->tag);
			
			fCopy->value = malloc(sizeof(char) * (strlen(f->value) +1));
			strcpy(fCopy->value, f->value);
			insertFront(&otherFields, fCopy);

		}
		indCopy->otherFields = otherFields; //Asign the copies of otherFields
		indCopy->families = ind->families; //Asigns the families

		ListIterator eventIter = createIterator(ind->events);
		void* event = NULL;
		List events = initializeList(&printEvent,&dummyDelete,&compareEvents);
		while( (event = nextElement(&eventIter)) != NULL) {
			Event *e = (Event *)event;
			Event *eCopy = malloc(sizeof(Event));

			strcpy(eCopy->type, e->type);
			
			eCopy->date = malloc(sizeof(char) * (strlen(e->date) +1));
			strcpy(eCopy->date, e->date);

			eCopy->place = malloc(sizeof(char) * (strlen(e->place) +1));
			strcpy(eCopy->place, e->place);

			///////////////////////Load OtherFields within an Event/////////////////////
			ListIterator otherIter = createIterator(e->otherFields);
			void* other = NULL;
			List otherFields2 = initializeList(&printField,&dummyDelete,&compareFields);
			while( (other = nextElement(&otherIter)) != NULL) {
				Field *f = (Field *)other;
				Field *fCopy = malloc(sizeof(Field));
				
				fCopy->tag = malloc(sizeof(char) * (strlen(f->tag) +1));
				strcpy(fCopy->tag, f->tag);
				
				fCopy->value = malloc(sizeof(char) * (strlen(f->value) +1));
				strcpy(fCopy->value, f->value);
				insertFront(&otherFields2, fCopy);

			}
			eCopy->otherFields = otherFields2;
			/////////////////////////////////////////////////////////////////////////////
			insertFront(&events, eCopy);
		}
		indCopy->events = events; //Assign events

	return indCopy;
}


void getChildren (List *descendants, List children) {

	if (descendants == NULL)
		return;

	ListIterator iter = createIterator(children);
	void* elem = NULL;
	while( (elem = nextElement(&iter)) != NULL) {
		
		Individual *ind = (Individual *)elem;
		Individual *indCopy = copyIndividual(ind);
		insertFront(descendants, indCopy);
		List families = ind->families;

		ListIterator iter2 = createIterator(families);
		void* elem2 = NULL;
		while( (elem2 = nextElement(&iter2)) != NULL) {

			Family *family = (Family *)elem2;

			if (family->husband == elem||family->wife == elem) {
				getChildren(descendants, family->children);
			}
		}

	}

}

void freeXrefs(List *subRecord, List *individualRecords, List *famRecords) {

	ListIterator iter;
	int flag = 0;
	for (int i = 0; i < 3; ++i) {
		if (i == 0 && subRecord != NULL) {
			iter = createIterator(*subRecord);
			flag = 1;
		} else if (i == 1 && individualRecords != NULL) {
			iter = createIterator(*individualRecords);
			flag = 1;
		} else if (i == 2 && famRecords != NULL) {
			iter = createIterator(*famRecords);
			flag = 1;
		}
		if (flag == 1) {
			void* elem = NULL;
			while( (elem = nextElement(&iter)) != NULL) {

				if (i == 0) {
					Submission *s = (Submission *)elem;
					free(s->tag);
					s->tag = NULL;
					free(s);
					s = NULL;
				} else if (i == 1) {
					Person *p = (Person *)elem;
					free(p->tag);
					p->tag = NULL;
					free(p);
					p = NULL;
				} else if (i == 2) {
					Fam *f = (Fam *)elem;
					free(f->tag);
					f->tag = NULL;
					free(f);
					f = NULL;
				}
			}
			flag = 0;
		}

	}

}

/********************************* checkFile.c **********************/


FILE* myOpenforReadOrWrite (char fileName[], int command) {

	if (fileName == NULL)
		return NULL;
	removeTrailingHardReturn(&fileName);
	if (fileName[strlen(fileName) -1] == 'd' && fileName[strlen(fileName) - 2] == 'e' && fileName[strlen(fileName) -3] == 'g' && fileName[strlen(fileName) - 4] == '.') {
		if (command == READ) {
			FILE *fileptr = fopen(fileName, "r");
			return fileptr;
		} else if (command == WRITE) {
			FILE *fileptr = fopen(fileName, "w");
			return fileptr;
		}
	}
   return NULL;
}

GEDCOMerror checkLevel(char *line) {

	static int levelCheck = 0; 
	static int lineNumber = 1;

	GEDCOMerror errorType;
	errorType.type = OK;
	errorType.line = -1; 

	//check for things like 01
	if (strlen(line) > 2 || (strlen(line) == 2 && line[0] == '0')) {
		errorType.type = INV_HEADER;
		errorType.line = lineNumber;
		++lineNumber;
		return errorType;
	}

	int level = isInt(line);

	//check line level for validity
	if (level == -1) {
		errorType.type = INV_HEADER;
		errorType.line = lineNumber;
	} else if (level == 0) {
		levelCheck = 0;
	} else if (level == 1) {
		levelCheck = 1;
	} else if (level == levelCheck + 1) {
		++levelCheck;
	} else if ( level > 0 && level <= levelCheck) {
		levelCheck = level;
	} else {
		errorType.type = INV_RECORD;
		errorType.line = lineNumber;
	}
	/************************************************/

	++lineNumber;
	return errorType;
}

int tagScan (char *tag) {

	if (tag == NULL)
		return -1;
	makeLowerCase(tag);
	char tags[3][5];
	strcpy(tags[0], "subm");
	strcpy(tags[1], "indi");
	strcpy(tags[2], "fam");

	for (int i = 0; i < 3; ++i) {
		if (strcmp(tag,tags[i]) == 0) {
			return 0;
		}
	}

	return -1;
}

GEDCOMerror validityCheck(char *line, int numWords) {

	static int lineNum = 1;
	GEDCOMerror errorType;
	errorType.type = OK;
	errorType.line = -1;

	//split line into fragments to scan
	char **frag = parseLine(line, numWords);

	//check line # validity
	if ( numWords > 0) {
		
		if (lineNum == 1) {
			if (numWords == 2) {
				makeLowerCase(frag[1]);
				if (strcmp(frag[0],"0") == 0 && strcmp(frag[1],"head") == 0) {
					errorType.type = OK;
					errorType.line = -1;
				} else {
					errorType.type = INV_HEADER;
					errorType.line = -1;
				}

			} else {
				//if found but more then just "0 head" inv_header
				if (strstr(line,"HEAD") != NULL || strstr(line,"head") != NULL)
					errorType.type = INV_HEADER;
				else errorType.type = INV_GEDCOM;
				errorType.line = -1;
			}
			++lineNum;
			freeWords(frag, numWords);
			return errorType;
		}
		errorType = checkLevel(frag[0]);
		if (errorType.type != OK) {
			freeWords(frag, numWords);
			return errorType;
		}

	}

	//free words
	freeWords(frag, numWords);

	++lineNum;
	return errorType;
}

GEDCOMerror checkHeader (FILE *fptr, char*line, int lineCount) {

	GEDCOMerror errorType;
	errorType.type = OK;
	errorType.line = -1;
	if (wordCount(line) == 2 && (strstr(line,"HEAD") != NULL || strstr(line,"head") != NULL )) {

		int sourFlag = 0;
		int versFlag = 0;
		int charFlag = 0;
		int subFlag = 0;

		char *text = malloc(sizeof(char) * 400);

		while ( fgetss ( text, 400, fptr ) ) {
			removeTrailingHardReturn( &text);
			if (strlen(text) > 255) {
				errorType.type = INV_RECORD;
				errorType.line = lineCount;
				free(text);
				return errorType;
			}

			int numWords = wordCount(text); 
			if (numWords != 0) {
				char **fragments = parseLine(text, numWords); 
				int level = isInt(fragments[0]);
				if (level == 0) {
					freeWords(fragments, numWords);
					free(text);
					rewind(fptr);
				if (sourFlag == 0 || versFlag == 0 || charFlag == 0 || subFlag == 0)
					errorType.type = INV_HEADER;
				return errorType;
				}
				if (numWords > 2) {
					makeLowerCase(fragments[1]);
					if (strcmp(fragments[1],"sour") == 0) {
						sourFlag = 1;
					} else if (strcmp(fragments[1],"vers") == 0) {
						versFlag = 1;
					} else if (strcmp(fragments[1],"char") == 0) {
						charFlag = 1;
					} else if (strcmp(fragments[1],"subm") == 0) {
						subFlag = 1;
					}
				}
				freeWords(fragments, numWords);
			}
			++lineCount;
		}
		free(text);
	} else {
		errorType.type = INV_GEDCOM;
	}
	return errorType;
}

GEDCOMerror scanFile(FILE *fptr, GEDCOMobject** obj) { //CALLED BY createGEDCOM

	GEDCOMerror errorType;
	errorType.type = OK;
	errorType.line = -1;
	*obj = NULL;
	char *text = malloc(sizeof(char) * 400); //MAX SIZE (400)
	int lineCount = 1;
	int numWords = 0;
	int subRecFound = 0;
	int headCheck = 0;
	char temp[255];

	while ( fgetss ( text, 400, fptr ) ) {
		
		removeTrailingHardReturn( &text);
		if (strlen(text) > 0) {
			if (strlen(text) > 255) {
				errorType.type = INV_RECORD;
				errorType.line = lineCount;
				free(text);
				fclose(fptr);
				return errorType;
			}

			numWords = wordCount(text);

			if (headCheck == 0 && numWords > 0) {
				errorType = checkHeader(fptr,text,lineCount+1);
				if (errorType.type != OK) {
					free(text);
					fclose(fptr);
					return errorType;
				}
				headCheck = 1;
				lineCount = 0;
			} else {

				if (strlen(text) > 0 && strstr("\n",text) == NULL && strstr("\r",text) == NULL)
					strcpy(temp,text);

				//checks for Actual Submitter Record
				if (subRecFound == 0) {
					if (numWords == 3 && (strstr(text,"subm") != NULL || strstr(text,"SUBM") != NULL) && (strstr(text,"0 ") != NULL || strstr(text,"0\t") != NULL) && strstr(text,"@") != NULL) {
						subRecFound = 1;
					}
				}
				if (numWords > 0) {
					errorType = validityCheck(text, numWords);
				}
				if (errorType.type != OK) {
					if (errorType.line != -1) {
						errorType.line = lineCount;
					}
					free(text);
					fclose(fptr);
					return errorType;
				}

			}
		}

		++lineCount;
	}

	makeLowerCase(temp);
	makeLowerCase(text);
	if ((strstr(temp,"0") == NULL || strstr(temp,"trlr") == NULL) && (strstr(text,"0") == NULL || strstr(text,"trlr") == NULL) )
		subRecFound = 0;
	
	if (subRecFound == 0) {
		errorType.type = INV_GEDCOM;
		errorType.line = -1;
	} else {
		// After checking for valid file, read through again 
		rewind(fptr);
		List subRecords = loadRecords(fptr, SUBMISSION); //Load the submission records
		rewind(fptr);
		List individualRecords = loadRecords(fptr, INDIVIDUAL); //load the individual records
		rewind(fptr);
		List famRecords = loadFamilyRecords(fptr,individualRecords); // load the family records
		rewind(fptr);
		parse(fptr, obj, subRecords, individualRecords, famRecords); //creates object and connects individuals to families
		freeXrefs(&subRecords,&individualRecords, &famRecords); //free all xrefs and unused data
		clearList(&subRecords);
		clearList(&individualRecords);
		clearList(&famRecords);
		errorType.type = OK;
	}
	free(text);
	fclose(fptr);

	return errorType;

}
/********************************* checkFile.c **********************/


/********************************* load.c **********************/

Submission *createSubmission (char *tag) {

	Submission *sub = malloc(sizeof(Submission));
	sub->tag = malloc(sizeof(char) * (strlen(tag) +1));
	sub->submitter = malloc(sizeof(Submitter) + (sizeof(char) * 40));
	return sub;
}

Person *createPerson (char *tag) {

	Person *p = malloc(sizeof(Submission));
	p->tag = malloc(sizeof(char) * (strlen(tag) +1));
	p->person = malloc(sizeof(Individual));
	return p;
}

Fam *createFamily(char *tag) {

	Fam *f = malloc(sizeof(Fam));
	f->tag = malloc(sizeof(char) * (strlen(tag) +1));
	f->family = malloc(sizeof(Family));

	return f;

}

Event *loadEvent (FILE *fptr, int level, char type[]) {
	
	List otherFields = initializeList(&printField,&dummyDelete,&compareFields);
	Event *e = malloc(sizeof(Event));
	makeUpperCase(type);
	if (strlen(type) <= 5)
		strcpy(e->type, type);
	else strcpy(e->type, "");
	e->date = malloc(sizeof(char) *2);
	e->place = malloc(sizeof(char) *2);
	strcpy(e->date,"");
	strcpy(e->place,"");
	e->otherFields = otherFields;
	
	char *text = malloc(sizeof(char) * 300);

	while ( fgetss ( text, 300, fptr ) ) {

		removeTrailingHardReturn(&text);
		int numWords = wordCount(text); 
		if (numWords != 0) {
			int lineLength = strlen(text) + 2; //used to move back in file
			char **fragments = parseLine(text, numWords); 
			int curLevel = isInt(fragments[0]);
			if (curLevel == level || curLevel == 0) {
				freeWords(fragments, numWords);
				free(text);
				fseek(fptr,-lineLength, SEEK_CUR);
				return e;
			}
			if (numWords > 2) {
				if (fragments[1][0] != '@' && fragments[1][strlen(fragments[1]) -1] != '@') {
					makeLowerCase(fragments[1]);
					if (strcmp(fragments[1] , "plac") == 0) {
						char temp[300];
						strcpy(temp,fragments[2]);
						if (numWords > 3) {
							for(int i = 3; i < numWords; ++ i) {
								strcat(temp, " ");
								strcat(temp, fragments[i]);
							}

						}
						free(e->place);
						e->place = malloc(sizeof(char) * (strlen(temp)+1));
						strcpy(e->place, temp);

					} else if ( strcmp(fragments[1] , "date") == 0) {

						char temp[300];
						strcpy(temp,fragments[2]);
						if (numWords > 3) {						
							for(int i = 3; i < numWords; ++ i) {
								strcat(temp, " ");
								strcat(temp, fragments[i]);
							}
						}
						free(e->date);
						e->date = malloc(sizeof(char) * (strlen(temp)+1));
						strcpy(e->date, temp);

					} else {
						e->otherFields = loadOtherFields(&otherFields, fragments, numWords);;
					}

				} 

			} else {
				e->otherFields = loadOtherFields(&otherFields, fragments, numWords);
			}

			freeWords(fragments, numWords);
		}
	}
	free(text);

	return e;
}


void readSubRecord (FILE *fptr, Submission *submission) {

	char *text = malloc(sizeof(char) * 300);
	List otherFields = initializeList(&printField,&dummyDelete,&compareFields);
	submission->submitter->otherFields = otherFields;
	strcpy(submission->submitter->address, "");

	while ( fgetss ( text, 300, fptr ) ) {
		removeTrailingHardReturn(&text);
		int numWords = wordCount(text); 
		if (numWords != 0) {
			int lineLength = strlen(text) + 2; //used to move back in file
			char **fragments = parseLine(text, numWords); 
			int level = isInt(fragments[0]);

			if (level == 0) {
				freeWords(fragments, numWords);
				free(text);
				fseek(fptr,-lineLength, SEEK_CUR);
				return;
			}
			if (numWords > 2 && level == 1) {
				if (fragments[1][0] != '@' && fragments[1][strlen(fragments[1]) -1] != '@') {

					makeLowerCase(fragments[1]);
					if (strcmp(fragments[1] , "name") == 0) {
						char temp[300];
						strcpy(temp,fragments[2]);
						if (numWords > 3) {
							for(int i = 3; i < numWords; ++ i) {
								strcat(temp, " ");
								strcat(temp, fragments[i]);
							}

						}

						strcpy(submission->submitter->submitterName, temp);

					} else if ( strcmp(fragments[1] , "addr") == 0) {
						char temp[300];
						strcpy(temp,fragments[2]);
						if (numWords > 3) {						
							for(int i = 3; i < numWords; ++ i) {
								strcat(temp, " ");
								strcat(temp, fragments[i]);
							}
						}

						strcpy(submission->submitter->address, temp);

					} else {
						submission->submitter->otherFields = loadOtherFields(&otherFields, fragments, numWords);
					}

				} 

			} else if (numWords > 2 && level != 1) {
				submission->submitter->otherFields = loadOtherFields(&otherFields, fragments, numWords);

			}

			freeWords(fragments, numWords);
		}
	}
	free(text);

}

void readIndRecord (FILE *fptr, Person *p) {

	char *text = malloc(sizeof(char) * 300);
	List events = initializeList(&printEvent,&dummyDelete,&compareEvents);
	List families = initializeList(&printFam,&deleteFam,&compareFam);
	List otherFields = initializeList(&printField,&dummyDelete,&compareFields);
	p->person->events = events;
	p->person->families = families;
	p->person->otherFields = otherFields;
	p->person->givenName = malloc(sizeof(char) * 2);
	strcpy(p->person->givenName,"");
	p->person->surname = malloc(sizeof(char) * 2);
	strcpy(p->person->surname,"");

	while ( fgetss ( text, 300, fptr ) ) {
		removeTrailingHardReturn(&text);
		int numWords = wordCount(text);
		if (numWords != 0) {
			int lineLength = strlen(text) + 2; //used to move back in file
			char **fragments = parseLine(text, numWords); 
			int level = isInt(fragments[0]);

			if (level == 0) {
				p->person->families = families;
				freeWords(fragments, numWords);
				free(text);
				fseek(fptr,-lineLength, SEEK_CUR);
				return;
			}

			int nameFlag = 0;
			int surnameFlag = 0;
			if (numWords > 2 && level == 1) {
				if (fragments[1][0] != '@' && fragments[1][strlen(fragments[1]) -1] != '@') {
					makeLowerCase(fragments[1]);
					if (strcmp(fragments[1] , "name") == 0) { 
						char name[200];
						strcpy(name,fragments[2]);
						if (numWords > 3) {
							int limit = numWords;

							for (int k = 3; k < numWords; ++k) {
								if (fragments[k][0] == '/' && fragments[k][strlen(fragments[k]) -1] == '/' && strlen(fragments[k]) != 2) {
									free(p->person->surname);
									char *surname = createWord(fragments[k],1,strlen(fragments[k]) - 2);
									p->person->surname = surname;
									limit = k;
									surnameFlag = 1;
									break;
								}
							}
							for(int i = 3; i < limit; ++ i) {
								if (strcmp(fragments[i],"//") != 0) {
									strcat(name, " ");
									strcat(name, fragments[i]);
								}
							}
							
						}
						if (name[0] != '/' && name[strlen(name) -1] != '/') {
							free(p->person->givenName);
							p->person->givenName = malloc(sizeof(char) *(strlen(name)+1));
							strcpy(p->person->givenName, name);
							nameFlag = 1;
						} else if (name[0] == '/' && name[strlen(name) -1] == '/') {
							free(p->person->surname);
							char *surname = createWord(name,1,strlen(name) - 2);
							p->person->surname = surname;
						}

					} else {
						if (strcmp(fragments[1] , "fams") != 0 && strcmp(fragments[1] , "famc") != 0)
							p->person->otherFields = loadOtherFields(&otherFields, fragments, numWords);
					}

				} 

			} else if (numWords > 2 && level == 2) {

				if (fragments[1][0] != '@' && fragments[1][strlen(fragments[1]) -1] != '@') {
					makeLowerCase(fragments[1]);
					if (strcmp(fragments[1] , "givn") == 0 && nameFlag == 0) { 
						char name[100];
						strcpy(name,fragments[2]);
						if (numWords > 3) {
							for(int i = 3; i < numWords; ++ i) {
									strcat(name, " ");
									strcat(name, fragments[i]);
							}
							
						}
						free(p->person->givenName);
						p->person->givenName = malloc(sizeof(char) *(strlen(name)+1));
						strcpy(p->person->givenName, name);

					} else if (strcmp(fragments[1] , "surn") == 0 && surnameFlag == 0) {
						char surname[100];
						strcpy(surname,fragments[2]);
						if (numWords > 3) {
							for(int i = 3; i < numWords; ++ i) {
									strcat(surname, " ");
									strcat(surname, fragments[i]);
							}
							
						}
						free(p->person->surname);
						p->person->surname = malloc(sizeof(char) *(strlen(surname)+1));
						strcpy(p->person->surname, surname);

					} else {
						p->person->otherFields = loadOtherFields(&otherFields, fragments, numWords);
					}

				} 
			

			} else if (numWords == 2) {
				makeLowerCase(fragments[1]);
				if (strcmp(fragments[1] , "name") != 0) {
					Event *e = loadEvent(fptr,level, fragments[1]);
					insertFront(&events, e);
					p->person->events = events;
				}

			} else {
				p->person->otherFields = loadOtherFields(&otherFields, fragments, numWords);
			}

			freeWords(fragments, numWords);
		}
	}
	free(text);

}



List loadRecords(FILE *fptr, int recordType) {

	char *text = malloc(sizeof(char) * 300); //MAX SIZE (300)
	List subRecords;
	List individualRecords;

	if(recordType == SUBMISSION)
		subRecords = initializeList(&printSubFunction,&deleteSubRecordData, &compareSubRecordTag);
	if (recordType == INDIVIDUAL)
		individualRecords = initializeList(&printIndivFunction,&deleteIndRecordData, &compareIndRecordTag);

	while ( fgetss ( text, 300, fptr ) ) {
		
		removeTrailingHardReturn( &text);
		int numWords = wordCount(text);
		if (numWords != 0) {
			char **words = parseLine(text, numWords);
			if (numWords > 2 && recordType == SUBMISSION) {

				int level = isInt(words[0]);
				if (level == 0) {
					makeLowerCase(words[2]);
					if ((words[1][0] == '@' && words[1][strlen(words[1]) -1] == '@') && (strcmp(words[2],"subm") == 0)) {
						Submission *submission = createSubmission(words[1]);
						readSubRecord(fptr, submission);
						strcpy(submission->tag,words[1]);
						insertFront(&subRecords, submission);

					}
				}

			} else if (numWords > 2 && recordType == INDIVIDUAL) {

				int level = isInt(words[0]);
				if (level == 0) {
					makeLowerCase(words[2]);
					if ((words[1][0] == '@' && words[1][strlen(words[1]) -1] == '@') && (strcmp(words[2],"indi") == 0)) {
						Person *p = createPerson(words[1]);
						readIndRecord(fptr, p);
						strcpy(p->tag,words[1]);
						insertFront(&individualRecords, p);
					}
				}

			}
			freeWords(words, numWords);
		}
	}
	free(text);
	if (recordType == SUBMISSION)
		return subRecords;
	
	return individualRecords;

}

void readFamRecord (FILE *fptr, Fam *f, List individualRecords) {

	char *text = malloc(sizeof(char) * 300);
	List children = initializeList(&printIndivFunction,&deleteIndRecordData,&compareIndRecordTag);
	List otherFields = initializeList(&printField,&dummyDelete,&compareFields);
	List events = initializeList(&printEvent,&dummyDelete,&compareEvents);
	f->family->children = children;
	f->family->otherFields = otherFields;
	f->family->events = events;
	f->family->husband = NULL;
	f->family->wife = NULL;
	
	while ( fgetss ( text, 300, fptr ) ) {
		removeTrailingHardReturn(&text);
		int numWords = wordCount(text);
		if (numWords != 0) { 
			int lineLength = strlen(text) + 2; //used to move back in file
			char **fragments = parseLine(text, numWords); 
			int level = isInt(fragments[0]);

			if (level == 0) {
				freeWords(fragments, numWords);
				free(text);
				fseek(fptr,-lineLength, SEEK_CUR);
				return;
			}
			if (numWords > 2 && level == 1) {
				if (fragments[1][0] != '@' && fragments[1][strlen(fragments[1]) -1] != '@') {
					makeLowerCase(fragments[1]);
					if (strcmp(fragments[1] , "husb") == 0) { 
						void* data = findElement(individualRecords, &compareTags, fragments[2]);
						if (data != NULL) {
							Person *p = (Person *)data;
							f->family->husband = p->person;
						}

					} else if (strcmp(fragments[1] , "wife") == 0) {
						void* data = findElement(individualRecords, &compareTags, fragments[2]);
						if (data != NULL) {
							Person *p = (Person *)data;
							f->family->wife = p->person;
						}

					} else if (strcmp(fragments[1] , "chil") == 0) {
						void* data = findElement(individualRecords, &compareTags, fragments[2]);
						if (data != NULL) {
							Person *p = (Person *)data;
							insertFront(&children,p->person);
							f->family->children = children;
						}

					} else {
						f->family->otherFields = loadOtherFields(&otherFields, fragments, numWords);
					}

				} 

			} else if (numWords > 2){
				f->family->otherFields = loadOtherFields(&otherFields, fragments, numWords);
			} else if (numWords == 2) {
				Event *e = loadEvent(fptr,level, fragments[1]);
				insertFront(&events, e);
				f->family->events = events;
			}

			freeWords(fragments, numWords);
		}
	}
	free(text);

}

List loadFamilyRecords(FILE *fptr, List individualRecords) {

	char *text = malloc(sizeof(char) * 300); //MAX SIZE (300)
	List familyRecords = initializeList(&printFam,&deleteFam, &compareFam);

	while ( fgetss ( text, 300, fptr ) ) {
		removeTrailingHardReturn( &text);
		int numWords = wordCount(text);
		if (numWords != 0) {
			char **words = parseLine(text, numWords);
			if (numWords > 2) {

				int level = isInt(words[0]);
				if (level == 0) {
					makeLowerCase(words[2]);
					if ((words[1][0] == '@' && words[1][strlen(words[1]) -1] == '@') && (strcmp(words[2],"fam") == 0)) {
						Fam *family = createFamily(words[1]);
						readFamRecord(fptr, family, individualRecords);
						strcpy(family->tag,words[1]);
						insertFront(&familyRecords, family);
					}
				}
			}
			
			freeWords(words, numWords);
		}
	}
	free(text);

	return familyRecords;

}


/*******************LIST FUNCTIONS (Submission List) ***********************/
char *printSubFunction(void* toBePrinted) {
	return NULL;
}

void deleteSubRecordData (void* toBeDeleted) {
}

int compareSubRecordTag (const void* first,const void* second) {
	Submission *a = (Submission *)first;
	Submission *b = (Submission *)second;

	return strcmp(a->tag,b->tag);
}
/***************************************************************************/
/*******************LIST FUNCTIONS (Individual List) ***********************/
char *printIndivFunction(void* toBePrinted) { //dummy
	return NULL;
}

void deleteIndRecordData (void* toBeDeleted) { //dummy
}

/* For Person struct */
int compareIndRecordTag (const void* first,const void* second) {
	Person *a = (Person *)first;
	Person *b = (Person *)second;

	return strcmp(a->tag,b->tag);
}
/***************************************************************************/

/*******************LIST FUNCTIONS (Family List) ***********************/
char *printFam(void* toBePrinted) {
	Fam *f = (Fam *)toBePrinted;
	return f->tag;
}

void deleteFam (void* toBeDeleted) { //dummy
}

int compareFam (const void* first,const void* second) {
	Fam *a = (Fam *)first;
	Fam *b = (Fam *)second;

	return strcmp(a->tag,b->tag);
}

int isParent (const void* family,const void* person) { //made in A2

	Family *f = (Family *)family;
	Individual *ind = (Individual *)person;

	if (f->husband == ind || f->wife == ind) 
		return 1; //is a parent
	if (f->husband != NULL)
		if (strcmp(f->husband->givenName, ind->givenName) == 0 && strcmp(f->husband->surname,ind->surname) == 0)
			return 1; //is a parent
	if (f->wife != NULL)
		if (strcmp(f->wife->givenName, ind->givenName) == 0 && strcmp(f->wife->surname,ind->surname) == 0)
			return 1; //is a parent

	return -1; //is a child in the family
}

int isChild (const void* family,const void* person) { //made in A3

	Family *f = (Family *)family;
	Individual *ind = (Individual *)person;

	if (f->husband == ind || f->wife == ind) 
		return -1; //is a parent

	void *found = findElement(f->children, &findIndividual, ind);
	if (found != NULL)
		return 1;


	return -1; //is NOT a child in the family
}

bool compareNewFamily (const void* first,const void* second) { //made in A2

	if (compareFamilies(first, second) != 0)
		return false;

	Family *a = (Family *)first;
	Family *b = (Family *)second;

	if (a == b) 
		return true; 
	if (a->husband != NULL && a->wife != NULL && b->husband != NULL && b->wife != NULL)
		if (strcmp(a->husband->givenName, b->husband->givenName) == 0 && strcmp(a->husband->surname, b->husband->surname) == 0)
			if (strcmp(a->wife->givenName, b->wife->givenName) == 0 && strcmp(a->wife->surname,b->wife->surname) == 0)
				return true; 
	if (a->husband != NULL && b->husband != NULL)
		if (strcmp(a->husband->givenName, b->husband->givenName) == 0 && strcmp(a->husband->surname, b->husband->surname) == 0)
			return true; 
	if (a->wife != NULL && b->wife != NULL)
		if (strcmp(a->wife->givenName, b->wife->givenName) == 0 && strcmp(a->wife->surname,b->wife->surname) == 0)
			return true; 

	return false; 
}

/*****************************************************************************/

/***************** Compare Family Tags to Individual Tags (find element)********************/
bool compareTags (const void* first,const void* second) {

	Person *a = (Person*)first;
	char *b = (char *)second;

	if (strcmp(a->tag,b) == 0)
		return true;

	return false;

}
/*****************************************************************************/

bool findIndividual (const void* first,const void* second) {

	if (compareIndividuals(first,second) == 0)
		return true;

	return false;
}

/********************************* load.c **********************/


/********************************* parseFile.c ************************/

void parse(FILE *fptr, GEDCOMobject** obj, List subRecords, List individualRecords, List famRecords) {

	char *text = malloc(sizeof(char) * 300); //TEMP SIZE (300)

	*obj = malloc(sizeof(GEDCOMobject));
	(*obj)->header = malloc(sizeof(Header));
	(*obj)->individuals = initializeList(&printIndividual,&dummyDelete,&compareIndividuals);
	(*obj)->families = initializeList(&printFamily,&dummyDelete,&compareFamilies);

	while ( fgetss ( text, 300, fptr ) ) {
	 	removeTrailingHardReturn(&text);
		int numWords = wordCount(text); 
		if (numWords != 0) {
			char **fragments = parseLine(text, numWords); 
			int level = isInt(fragments[0]);
			if (level == 0) {

				if (numWords == 2) {
					makeLowerCase(fragments[1]);
					if (strcmp(fragments[1] , "head") == 0) {
						createHeaderRecord(fptr, obj, subRecords);
					}		
				}
				if (numWords > 2) {
					makeLowerCase(fragments[2]);
					if (strcmp(fragments[2] , "fam") == 0) {
						void* data = findElement(famRecords, &compareTags, fragments[1]);
						if (data != NULL) {
							Fam *f = (Fam *)data;
							Family *family = f->family;
							insertFront(&(*obj)->families, family);

						}
					} else if (strcmp(fragments[2] , "indi") == 0) {
						connectToFamily(fptr,&individualRecords,famRecords, fragments[1]);
						void* data = findElement(individualRecords, &compareTags, fragments[1]);
						if (data != NULL) {
							Person *p = (Person *)data;
							Individual *ind = p->person;
							insertFront(&(*obj)->individuals, ind);

						}

					} else if (strcmp(fragments[2] , "subm") == 0) {
						void* data = findElement(subRecords, &compareTags, fragments[1]);
						if (data != NULL) {
							Submission *s = (Submission *)data;
							(*obj)->submitter = s->submitter;
						}

					} 

				}
			}

			freeWords(fragments, numWords);
		}
	}
	free(text);
}

void createHeaderRecord(FILE *fptr, GEDCOMobject **obj, List subRecords) {

	char *text = malloc(sizeof(char) * 300);

	List otherFields = initializeList(&printField,&dummyDelete,&compareFields);

	while ( fgetss ( text, 300, fptr ) ) {

		removeTrailingHardReturn(&text);
		int numWords = wordCount(text);
		if (numWords != 0) {
			int lineLength = strlen(text) + 2; //used to move back in file
			char **fragments = parseLine(text, numWords); 
			int level = isInt(fragments[0]);

			if (level == 0) {
				freeWords(fragments, numWords);
				free(text);
				fseek(fptr,-lineLength, SEEK_CUR);
				return;
			}
			if (numWords > 2 && level == 1) {
				if (fragments[1][0] != '@' && fragments[1][strlen(fragments[1]) -1] != '@') {
					makeLowerCase(fragments[1]);
					if (strcmp(fragments[1] , "sour") == 0) {
						char temp[300];
						strcpy(temp, fragments[2]);
						if (numWords > 3) {
							for(int i = 3; i < numWords; ++ i) {
								strcat(temp, " ");
								strcat(temp, fragments[i]);
							}
						}
						strcpy((*obj)->header->source, temp); //Assign/Store the source 
					} else if ( strcmp(fragments[1] , "vers") == 0) {
						sscanf(fragments[2],"%f", &((*obj)->header->gedcVersion));
					} else if (strcmp(fragments[1] , "char") == 0) {
						makeLowerCase(fragments[2]);
						if (strcmp(fragments[2],"ansel") == 0) {
							(*obj)->header->encoding = ANSEL;
						} else if (strcmp(fragments[2],"utf8") == 0 || strcmp(fragments[2],"utf-8") == 0) {
							(*obj)->header->encoding = UTF8;
						} else if (strcmp(fragments[2],"unicode") == 0) {
							(*obj)->header->encoding = UNICODE;
						} else {
							(*obj)->header->encoding = ASCII;
						}
					} else if (strcmp(fragments[1] , "subm") == 0) {
						void *data = getFromFront(subRecords);
						if (data != NULL) {
							Submission *s = (Submission*)data;
							(*obj)->header->submitter = s->submitter;
							(*obj)->submitter = s->submitter;

						}
					} else {
						(*obj)->header->otherFields = loadOtherFields(&otherFields, fragments, numWords);
					}

				}

			} else if (numWords == 2) {
				makeLowerCase(fragments[1]);
				if (strcmp(fragments[1] , "gedc") == 0 && level == 1) {
					getGedVersion(otherFields ,fptr,obj, level);

				} else {
					(*obj)->header->otherFields = loadOtherFields(&otherFields, fragments, numWords);
				}

			} else {
				(*obj)->header->otherFields = loadOtherFields(&otherFields, fragments, numWords);
			}

			freeWords(fragments, numWords);

		}
	}
	free(text);

}

void getGedVersion (List otherFields, FILE *fptr, GEDCOMobject **obj, int level) {

	char *text = malloc(sizeof(char) * 300);
	while ( fgetss ( text, 300, fptr ) ) {

		removeTrailingHardReturn(&text);
		int numWords = wordCount(text);
		if (numWords != 0) {
			int lineLength = strlen(text) + 2; //used to move back in file
			char **fragments = parseLine(text, numWords); 
			int curLevel = isInt(fragments[0]);
			if (curLevel == level || curLevel == 0) {
				freeWords(fragments, numWords);
				free(text);
				fseek(fptr,-lineLength, SEEK_CUR);
				return;
			}
			if (numWords > 2 ) {
				makeLowerCase(fragments[1]);
				if (strcmp(fragments[1] , "vers") == 0 && curLevel == 2) {
					sscanf(fragments[2],"%f", &((*obj)->header->gedcVersion));
				}

			}
			freeWords(fragments, numWords);
		}
	}
	free(text);

}

void connectToFamily (FILE *fptr, List *individualRecords, List famRecords, char *tagToMatch) {

	char *text = malloc(sizeof(char) * 300);
	while ( fgetss ( text, 300, fptr ) ) {

		removeTrailingHardReturn(&text);
		int numWords = wordCount(text);
		if (numWords != 0) {
			int lineLength = strlen(text) + 2; //used to move back in file
			char **fragments = parseLine(text, numWords); 
			int level = isInt(fragments[0]);
			if (level == 0) {
				freeWords(fragments, numWords);
				free(text);
				fseek(fptr,-lineLength, SEEK_CUR);
				return;
			}
			if (numWords > 2 && level == 1) {
				makeLowerCase(fragments[1]);
				if (strcmp(fragments[1] , "famc") == 0 || strcmp(fragments[1] , "fams") == 0) {
					void* famData = findElement(famRecords, &compareTags, fragments[2]);
					if (famData != NULL) {
						Fam *f = (Fam *)famData;
						Family *family = f->family;
						void* indData = findElement(*individualRecords, &compareTags, tagToMatch);
						if (indData != NULL) {
							Person *p = (Person *)indData;
							Individual *ind = p->person;
							insertFront(&(ind->families), family);
										
						}
					}
				}
			}
			freeWords(fragments, numWords);
		}
	}
	free(text);

}

List loadOtherFields (List *otherFields, char** fragments, int numWords) {
	if (numWords > 0) {
		Field *newField = malloc(sizeof(Field)); //FREE LATER
		newField->tag = malloc(sizeof(char) * (strlen(fragments[1]) +1));//FREE LATER
		makeUpperCase(fragments[1]);
		strcpy(newField->tag, fragments[1]);
		if(numWords > 2) {
			char temp[300];
			strcpy(temp,fragments[2]);
			if (numWords > 3) {						
				for(int i = 3; i < numWords; ++ i) {
					strcat(temp, " ");
					strcat(temp, fragments[i]);
				}
			}

			newField->value = malloc(sizeof(char) * (strlen(temp) +1));//FREE LATER
			strcpy(newField->value, temp);
		} else {
			newField->value = NULL;
		}
		insertFront(otherFields, newField);
	}

	return *otherFields;
}
/********************************* parseFile.c ************************/

/********************************* splitter.c ************************/

int wordCount (char *line) {

	int words = 0;
	int letterCheck = 0;
	for (int i = 0; i < strlen(line); ++i) {
		if (isspace(line[i]) == 0) 
			letterCheck = 1;
		if ((isspace(line[i]) != 0 && letterCheck == 1)) {
			++words;
			letterCheck = 0;
		} else if ( (i == strlen(line)-1) && isspace(line[i]) == 0) {
			++words;
		}
	}

	return words;
}

char **parseLine(char *line, int numWords) {

	char *words;
	int i = 0;
	char **frag = malloc(sizeof(char*) * (numWords + 1));
	words = strtok (line, " \t");
	while (words != NULL) {

		frag[i] = malloc(sizeof(char) * strlen(words)+1);
		strcpy(frag[i], words);
		++i;
		words = strtok (NULL, " \t");
	}
	frag[i] = NULL;

	return frag;

}

void makeLowerCase (char *text) {
	for (int i = 0; i<strlen(text); i++) 
		text[i] = tolower(text[i]);
}

void makeUpperCase (char *text) {
	for (int i = 0; i<strlen(text); i++) 
		text[i] = toupper(text[i]);
}

void freeWords(char **array, int numWords) { //remove num words

	for (int j = 0; array[j] != NULL; ++j)
	{
		free(array[j]);
	}
	free(array);
	array = NULL;

}

char *createWord(char* string, int start, int end) {

	char* word = malloc(sizeof(char) * (end-start+2));
	int j = 0;
	for (int i = start; i<=end; ++i) {
		word[j] = string[i]; 
		++j;
	}
	word[j] = '\0';
	return word;
}

void removeTrailingHardReturn( char **toClean) {

	if(*toClean == NULL)
		return;

	char *clean = (*toClean);
	int i = strlen(clean) -1;
	if ( i <= 0)
		return;

	while (clean[i] == '\n' || clean[i] == '\r') {
		clean[i] = '\0';
		i --;
	}
	return;

}

int isInt(char *toCheck) { 

	int num;

	if (strlen(toCheck) == 0)
		return -1;

	num = atoi(toCheck);

	if (strcmp(toCheck,"0") != 0 && num == 0)
		return -1;
	else if (num == 0)
		return 0;
	else return num;

}

/********************************* splitter.c ************************/
void dummyDelete(void *toBeDeleted) {
}

char *fgetss(char *destination, int max, FILE *fp) {

	int character;
	char *p;
	/* get max bytes or upto a newline or carrage return*/
	for (p = destination, max--; max > 0; max--) {

		if ((character = fgetc (fp)) == EOF) {
			break;
		}

		*p++ = character;

		if (character == '\r' || character == '\n') {
			break;
		}

	}

	*p = 0;
	if (character == EOF || p == destination) {
		return NULL;
	}

	return (p);
}



