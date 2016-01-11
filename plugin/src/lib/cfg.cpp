/**
 * \brief Configuration file reader.
 * \author Jeremy Decker
 * \version 0.1
 * \date 31/09/2009
 */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "cfg.hpp"

#define BLOC_SIZE 256

Cfg::Cfg() {
	size  = 0;
	name  = NULL;
	value = NULL;
}

Cfg::~Cfg() { clear(); }

void Cfg::load(const char * filename) {
	unsigned long sizeReaded = 0;
	char *		  variable;
	char *		  valeur;
	int           Index = 0;

	clear();
	FILE * filePtr = fopen (filename, "rb");
	if (filePtr == NULL) { return; }

	size  = BLOC_SIZE;
	name  = (char * *) malloc(sizeof(char*) * BLOC_SIZE);
	value = (char * *) malloc(sizeof(char*) * BLOC_SIZE);
	memset(name,  0, sizeof(char*) * BLOC_SIZE);
	memset(value, 0, sizeof(char*) * BLOC_SIZE);
	
	variable = (char*) malloc (2048);
	valeur   = (char*) malloc (2048);

	do {
	ReadConfig_Begin:
		memset (variable, 0x00, 2048);
		memset (valeur,   0x00, 2048);

		/* Leture de la variable */
		Index = 0;
		do {
			sizeReaded = fread (variable,  1, 1, filePtr); 
			
			if ( (feof(filePtr)) || (sizeReaded == 0) ) {
				fclose(filePtr);
				goto ReadConfig_End; 
			}
		} while ((variable[0] == '\t') || (variable[0] == ' ') || (variable[0] == '\n') || (variable[0] == '\r'));		
		Index = 1;
	
		do {
			sizeReaded = fread (variable + Index,  1, 1, filePtr);
			if (feof(filePtr)) {
				fclose(filePtr);
				goto ReadConfig_End; 
			}
			
			if (sizeReaded == 0) { 
				variable[Index] = 0x00;
				while ((variable[Index] == '\t') || (variable[Index] == ' ') || (variable[Index] == '\r') || (variable[Index] == '\n'))
                { Index--; }
				
				variable[Index+1] = 0x00;
				setValue (variable, valeur);
				fclose(filePtr);
				goto ReadConfig_End;
			} else if ((variable[Index] == '\r') || (variable[Index] == '\n')) {
				variable[Index] = 0x00;
				while ((variable[Index] == '\t') || (variable[Index] == ' ') || (variable[Index] == '\r') || (variable[Index] == '\n'))
                { Index--; }
				
				variable[Index+1] = 0x00;
				setValue (variable, valeur);
				goto ReadConfig_Begin;
			}
			
			Index++;
		} while (variable[Index-1] != '=');
	  
		Index-=2;
		while ((variable[Index] == '\t') || (variable[Index] == ' ') || (variable[Index] == '\r') || (variable[Index] == '\n'))
        { Index--; }
		variable[Index+1] = 0x00;
	
		
		/* Leture de la valeur */
		Index = 0;
		do {
			sizeReaded = fread (valeur,  1, 1, filePtr);
			
			if ( (feof(filePtr)) || (sizeReaded == 0) ) {
				fclose(filePtr);
				goto ReadConfig_End; 
			}
		} while ((valeur[0] == '\t') || (valeur[0] == ' '));
     
		Index = 1;
		do {
			sizeReaded = fread (valeur + Index,  1, 1, filePtr);
			
			if (sizeReaded == 0) { 
				valeur[Index] = 0x00;
				while ( ((valeur[Index] == '\t') || (valeur[Index] == ' ')) || ((valeur[Index] == '\n') || (valeur[Index] == '\r')))
                { Index--; }
				
				valeur[Index+1] = 0x00;
				setValue (variable, valeur);
				fclose(filePtr);
				goto ReadConfig_End;
			}
			
			Index++;
		} while ((valeur[Index-1] != '\n') && (valeur[Index-1] != '\r') && (!(feof(filePtr))));
		
		while ( ((valeur[Index-1] == '\t') || (valeur[Index-1] == ' ')) || ((valeur[Index-1] == '\n') || (valeur[Index-1] == '\r')))
        { Index--; }
		
		valeur[Index] = 0x00;
		setValue (variable, valeur);
	} while (!(feof(filePtr)));
	
	fclose(filePtr);
ReadConfig_End:
	free(variable);
	free(valeur);
}

void Cfg::setValue (const char * variable, const char * valeur) {
	int boucle;

	for (boucle = 0; boucle < size; boucle++) {
		if (name[boucle] == NULL) { continue; }
		if (strcmp (name[boucle], variable) == 0) { 
			if (value[boucle] != NULL) { free(value[boucle]); }
			value[boucle] = (char*) malloc (strlen(valeur)+1);
			strcpy(value[boucle], valeur);
			return;
		} 
	}

	for (boucle = 0; boucle < size; boucle++) {
		if (name[boucle] == NULL) { 
			name [boucle] = (char*) malloc(strlen(variable)+1);
			value[boucle] = (char*) malloc(strlen(valeur)+1);
			strcpy(name[boucle],  variable);
			strcpy(value[boucle], valeur);
			return;
		} 
	}

	realloc(name,  sizeof(char*) * (size + BLOC_SIZE));
	realloc(value, sizeof(char*) * (size + BLOC_SIZE));
	strcpy(name[size],  variable);
	strcpy(value[size], valeur);
	size += BLOC_SIZE;
}

const char * Cfg::getValue (const char * variable) {
	for (int Boucle = 0; Boucle < size; Boucle++) {
		if (name[Boucle] == NULL) { continue; }
		if (strcmp (name[Boucle], variable) == 0) { return value[Boucle]; } 
	}
	return NULL;
}

void Cfg::clear() {
	if (name) {
		for (int boucle = 0; boucle < size; boucle++) {
			if (name[boucle] != NULL)  { free (name[boucle]); }
		}
		free(name);  
		name = NULL;
	}

	if (value) {
		for (int boucle = 0; boucle < size; boucle++) {
			if (value[boucle] != NULL) { free (value[boucle]); }
		}

		free(value); 
		value = NULL;
	}

	size = 0;
}
