#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <stdarg.h>
#include <math.h>
#include <data_types.h>

FILE *inputStream; // data stream containing input
unsigned long lineCount = 1; // tracks line number on input file
char format[10]; // stores input file format
unsigned long declaredVariableCount = 0; // if it is present and > 0, the formula is in cleansed form
char* word; // stores current string of interest
size_t wordlen; // size of word
char* varFormat; // Format argument for fscanf regarding variable names, dependant on wordlen

/*
 * freeResources frees malloced variables
 */
void freeResources(){
	free(word);
	free(varFormat);
	freeVS(data);
}
/*
 * A simple program interruption function.
 */
void die(const char *fmt, ...){
	va_list args;
	
	if(errno){
		size_t  len;
		char   *message;

		va_start(args, fmt);
		len = vsnprintf(0, 0, fmt, args);
		va_end(args);
		if ((message = malloc(len + 1)) != 0){
			va_start(args, fmt);
			vsnprintf(message, len+1, fmt, args);
			va_end(args);
			perror(message);
			free(message);
		} else {
			perror("Memory allocation failure");
		}
		
		freeResources();
		exit(EXIT_FAILURE);
	}else{
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
		printf("\n");
		
		freeResources();
		exit(EXIT_SUCCESS);
	}
}

/*
 * checkCorrectMatch compares the contents of word with the parameter.
 * Returns 1 on exact match, terminates program when word has 
 * 
 * NEEDED?
 *
int checkCorrectMatch(const char *str)
{
    size_t lenpre = strlen(word),
           lenstr = strlen(str);
	if(lenpre >= lenstr){
		if(strncmp(word, str, lenstr) == 0){
			if(lenpre == lenstr){
				return 1;
			}else{
				die("Parsing error on line %lu: \"%s\" expected, \"%s\" found", lineCount, str, word);
			}
		}
	}
    return 0;
}*/

/*
 * fCheckNewline checks if a newline or EOF follows, terminates program otherwise
 * 
 * returns: -1 if EOF reached
 */
int fCheckNewline(){
	char auxChar;
	fscanf(inputStream, "%*[ \t]");
	if (fscanf(inputStream,"%c", &auxChar) < 1 || auxChar != '\n'){
		if(fscanf(inputStream, "%*c") == EOF){
			return EOF;
		}
		die("Parsing failed at line %lu: newline expected", lineCount);
	}else{
		++lineCount;
		return 1;
	}
	return 0; // should not be reached
}
/*
 * fSkipWhitespaceAndComments takes a data stream and moves the 
 * scanner to first non-whitespace, non-comment position. Increments value at
 * lineCount for every encountered \n.
 * 
 * unsigned long *lineCount: pointer to the variable to be incremented
 * 					   returns: -1 if EOF reached, otherwise 0
 */
void fSkipWhitespaceAndComments(){
	char aux[2] = "";
	errno = 0;
	
	while(1){
		fscanf(inputStream, "%*[ \t]"); // omits spaces and tabs
		if((fscanf(inputStream, "%1[\n]", aux) == 1) && (errno == 0)){ // checks if \n follows
				++lineCount;
		}else if((fscanf(inputStream, "%1[#]", aux) == 1) && (errno == 0)){ // detect a comment line
			if(fscanf(inputStream, "%*[^\n]") || fscanf(inputStream, "%1[\n]", aux)){ // discard all untill a newline
				++lineCount;
			} else { // no more lines
				return; 
			}
		}else{ // something else found
			return;
		}
	}
	return; // should not be reached
}

/* fscanfPlus calls fSkipWhitespaceAndComments and than attempts to fscanf according to provided format.
 * Also checks for a runtime error.
 * 
 * 				  char *format: format to be checked for, no more than wordlen-1 chars should be read
 *					   returns: 1 on success, 0 on failure
 * */
int fscanfPlus(char *format){
	int result;
	
	fSkipWhitespaceAndComments();
	result = fscanf(inputStream, format, word);
	if(result == EOF){
		die("Parsing failed: EOF reached");
	}else if(result < 0){
		die("Runtime error occured while parsing line %lu", lineCount);
	}
	return result;
}
/*
 * fscanExpectedChar calls fSkipWhitespaceAndComments and than attempts to 
 * fscanf given char. Error message is printed and program exits if no match is found.
 * 
 * 				  char expected: char to be checked for
 * */
void fscanExpectedChar(char expected){
	int result;
	char aux;
	fSkipWhitespaceAndComments();
	result = fscanf(inputStream, "%c", &aux);
	if(result == 1){
		if(aux == expected){
			return;
		}else{
			die("Parsing error at line %lu: '%c' expected, '%c' found", lineCount, expected, aux);
		}
	}else if(result < 0) {
		die("Runtime error occured while parsing line %lu", lineCount);
	}else{
		die("Parsing error at line %lu: '%c' expected", lineCount, expected);
	}
}
/*
 * fscanfAndCheck calls fSkipWhitespaceAndComments and than attempts to 
 * fscanf according to provided format. Error message is printed and program exits if no match is found.
 * 
 * 				  char *format: format to be checked for, no more than 32 chars should be read
 * 					(char *word: to store result, must be big enough!)
 *					   returns: 1 on success, 0 on failure
 * 
 * NEEDED ?
 * 
int fscanfAndCheck(char *format){
	//long positionInStream;
	int result = 0;
	
	return result;
	positionInStream = ftell(inputStream) - positionInStream;
	printf("exact read %lu chars\n", positionInStream);
	if(positionInStream == ftell(inputStream)){ // compare positions before and after read, if equal => couldn't read the provided string
		return 1;
	}else{ // string found
		return 0;
	}
}*/
 
 /*
  * parseVariable reads a valid variable name, storing it in 'word' variable.
  * Doubles the size of 'word' until it is big enough to fit if neccessary.
  * 
  *    returns: 1 if a variable was read, EOF if EOF reached, 0 otherwise
  */
int parseImmediateVariable(){
	int result;
	char SingleCharString[2] = "";
	
	result = fscanf(inputStream, varFormat, word); // ascii letters, digits and underscores
	if(result < 0){
		if(fscanf(inputStream,"%*c") == EOF){
			return EOF;
		}
		die("Runtime error occured while parsing line %lu (parseImmediateVariable)", lineCount);
	}
	while(fscanf(inputStream,"%1[abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_]", SingleCharString) == 1){
		// variable name longer than current wordlen-1
		char *auxWord;
		char *auxFormat;
		int newFormatLength; // length of new format string
		
		newFormatLength = (69 + floor(log10(abs(wordlen)))); // 66 basic symbols (ascii letters, digits and underscores), 1 more for the digit counting function, 1 more for \0, and 1 more to cover for unknown behaviour
		auxWord = malloc(sizeof(char)*wordlen*2);
		auxFormat = malloc(sizeof(char)*newFormatLength);
		if(auxWord == NULL || auxFormat == NULL){
			die("Could not allocate memory");
		}
		// fill the new buffer with the content of the previous smaller one and the rest
		strcpy(auxWord, word);
		free(word);
		auxWord[wordlen - 1] = SingleCharString[0]; // the char used to check if there is more
		if(fscanf(inputStream, varFormat, (char *)(auxWord + wordlen)) < 0){
			if(fscanf(inputStream,"%*c") == EOF){
				return EOF;
			}
			die("Runtime error occured while parsing line %lu (parseImmediateVariable)", lineCount);
		}
		free(varFormat);
		wordlen *= 2; // new length double the previous
		snprintf(auxFormat, newFormatLength, "%c%zu%s", '%', (wordlen-1), "[abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_]");
		varFormat = auxFormat;
		word = auxWord;
		//printf("wordlen: %zu; word: %s; foramt: %s\n", wordlen, word, varFormat);
	}
	return result;
}
/*
 * parseVariableNoDuplicate calls parseVariable and than checks if parsed word is already defined.
 * If yes, program is interrupted.
 * 
 * returns: 1 if a variable was read, 0 otherwise
 */
/*int parseVariableNoDuplicate(){ //parseGateVariable?
	int result;
	var *check;
	
	result = parseVariable(word);
	if(result == 0){ // no variable name could be read
		return result;
	}
	check = getVarVS(data, word); // check if a variable with the same name is already defined
	if(check != NULL){
		die("Duplicate variable definition at line %lu", lineCount);
	}
	return result; // 1
}*/

/*
 * parseVariable calls fSkipWhitespaceAndComments and than parseImmediateVariable
 */
int parseVariable(){
	fSkipWhitespaceAndComments();
	return parseImmediateVariable();
}
/*
 * parseLiteral calls fSkipWhitespaceAndComments and attempts to read a valid literal.
 * 
 * char *sign: address for the sign to be stored, 0 negative, otherwise positive
 *    returns: 1 if a literal was read, 0 otherwise
 */
int parseLiteral(char *sign){
	int result;
	
	fSkipWhitespaceAndComments();
	if(fscanfPlus("%1[-]")){ // check for a negation
		*sign = 0;
	}else{
		*sign = 1;
	}
	result = parseImmediateVariable();
	return result;
}
/*
 * parseQCIRAndEcho takes a string, checks if it is a valid QCIR-(G)14 formula
 * 
 * char *input: input string
 *  long fsize: input string size
 */
void parseQCIR(char *inputString, long fsize)
{
	char SingleCharString[2] = "";
	
	inputStream = fmemopen(inputString, fsize, "r");
	if(inputStream == NULL){
		if(inputStream == NULL){
			die("Failure to open a data stream");
		}
	}
	
	while(1){ // omit whitespace and comments at beginning untill a valid format declaration is found
		if(fscanf(inputStream, "%1[#]", SingleCharString)){ // detect a comment line
			fscanf(inputStream, "%8[QCIR-G14]", format); // read in possible format declaration
			if (strcmp(format,"QCIR-14") == 0){
				fscanf(inputStream, "%*[ \t]");
				if(fscanf(inputStream, "%1[\n]", SingleCharString)){
					// #QCIR-14 format ID fine
					++lineCount;
					break;
				}
			} else if (strcmp(format,"QCIR-G14") == 0){
				int auxInt;
				fscanf(inputStream,"%*[ \t]");
				auxInt = fscanf(inputStream,"%21[0123456789]", word); // check if an integer follows
				if (auxInt < 0 || auxInt > 1){
						die("Runtime error while parsing QCIR-G14 argument");
				}else if (auxInt == 0){ // no integer found, proceed with parcing
					if(fscanf(inputStream,"%1[\n]", SingleCharString) == 1){
					// #QCIR-G14 format ID fine
					++lineCount;
					break;
					}
				}else {
					char auxWord[32] = "";
					fscanf(inputStream,"%*[0123456789]"); // omit any digits left
					declaredVariableCount = strtoul(word, NULL, 10);
					snprintf(auxWord, 31, "%lu", declaredVariableCount);
					if(strcmp(word,auxWord)){ // if parsed value is different from input one, than input is too big to store, replace with ULONG_MAX
						declaredVariableCount = ULONG_MAX;
					}
					fscanf(inputStream,"%*[ \t]");
					if(fscanf(inputStream,"%1[\n]", SingleCharString) == 1){
					// #QCIR-G14 format ID fine
					++lineCount;
					break;
					}
				}
			}
			if(fscanf(inputStream, "%*[^\n]") || fscanf(inputStream, "%1[\n]", SingleCharString)){ // discard all untill a newline
				++lineCount;
			} else { // no more lines
				die("Parsing failed: no valid format declaration found\n(\"#QCIR-14 <newline>\" or \"#QCIR-G14 [integer] <newline>\")\n");
			}
		}else if(fscanf(inputStream, "%*[ \t]") || fscanf(inputStream, "%1[\n]", SingleCharString)){ // first term always 0, omits spaces and tabs, second checks if \n follows
			++lineCount;
		}else if(fscanf(inputStream, "%1[#]", SingleCharString)){ // detect a comment line
			if(fscanf(inputStream, "%*[^\n]") || fscanf(inputStream, "%1[\n]", SingleCharString)){ // discard all untill a newline
				++lineCount;
			}
		}else{// something else found
			die("Parsing failed: no valid format declaration found\n(\"#QCIR-14 <newline>\" or \"#QCIR-G14 [integer] <newline>\")\n");
		}
	}

	if(fscanfPlus("%7[frexistoalup]") == 0){ // read first keyword
		die("Parsing failed at line %lu: 'free', 'exists', 'forall' or 'output' expected", lineCount);
	}
	
	if(strcmp(word, "free") == 0){ // list of free variables is present
		fscanExpectedChar('(');
		if(parseVariable() == 0){
			die("Parsing failed at line %lu: variable name expected in free() block", lineCount);
		}
		addUniqueFreeVar();
		while(fscanfPlus("%1[,]")){ // while a comma is following, a variable must also!
			if(parseVariable() == 0){
				die("Parsing failed at line %lu: variable name expected after ','", lineCount);
			}
			addUniqueFreeVar();
		}
		fscanExpectedChar(')');
		//fCheckNewline();
		
		if(fscanfPlus("%7[frexistoalup]") == 0){ // scan next keyword
			die("Parsing failed at line %lu: 'exists', 'forall' or 'output' expected", lineCount);
		}
	}
	
	while(1){ // quantifier prefix

		if(strcmp(word, "exists") == 0){ // exists block parsed
			fscanExpectedChar('(');
			if(parseVariable() == 0){
				die("Parsing failed at line %lu: variable name expected in exists() block", lineCount);
			}
			addUniqueVarQB(EXISTS);
			while(fscanfPlus("%1[,]")){ // while a comma is following, a variable must also!
				if(parseVariable() == 0){
					die("Parsing failed at line %lu: variable name expected after ','", lineCount);
				}
				addUniqueVarQB(EXISTS);
			}
			fscanExpectedChar(')');
			//fCheckNewline();
		}else if(strcmp(word, "forall") == 0){ // forall block parsed
			fscanExpectedChar('(');
			if(parseVariable() == 0){
				die("Parsing failed at line %lu: variable name expected in forall() block", lineCount);
			}
			addUniqueVarQB(FORALL);
			while(fscanfPlus("%1[,]")){ // while a comma is following, a variable must also!
				if(parseVariable() == 0){
					die("Parsing failed at line %lu: variable name expected after ','", lineCount);
				}
				addUniqueVarQB(FORALL);
			}
			fscanExpectedChar(')');
			//fCheckNewline();
		}else{ // other keyword
			break;
		}
		
		if(fscanfPlus("%7[frexistoalup]") == 0){ // scan next keyword
			die("Parsing failed at line %lu: 'exists', 'forall' or 'output' expected", lineCount);
		}
	}
	
	if(strcmp(word, "output") == 0){ // begin output statement 
		fscanExpectedChar('(');
		if(parseLiteral(&(data->outputSign)) == 0){
			die("Parsing failed at line %lu: variable name expected in output() statement", lineCount);
		}
		data->outputName = malloc(strlen(word)+1);
		strcpy(data->outputName, word);
		fscanExpectedChar(')');
		//fCheckNewline();
	}else{
		die("Parsing failed at line %lu: 'output' expected, '%s' found", lineCount, word);
	}
	
	while(1){ // gate statements
		var *v;
		gate *g;
		char sign;
		
		if(parseVariable() < 1){ // parse next gate name
			if(parseVariable() == -1){
				break; // EOF reached
			}else{
				// TODO something else found, die ?
				break;
			}
		}
		// Gate name must be unique, create a new var but first we must parse literals to see at which level it belongs
		checkUniqueness(word);
		v = createVar(word);
		
		fscanExpectedChar('=');
		if(fscanfPlus("%7[frexistoalndFREXISTOALND]") == 0){ // scan next keyword
			die("Parsing failed at line %lu: 'exists', 'forall', 'and', 'or', 'xor' or 'ite' expected", lineCount);
		}
		
		if(strcmp(word, "and") == 0 || strcmp(word, "AND") == 0){ // 'and' gate
			g = defineGateVar(v, AND); // add definition
			fscanExpectedChar('(');
			if(parseLiteral(&sign)){ // check if at least 1 literal
				addLitToGateAssumeFree(g, sign); // add to gate definition
				while(fscanfPlus("%1[,]")){ // while a comma follows, a literal must also!
					if(parseLiteral(&sign)){
						addLitToGateAssumeFree(g, sign); // add to gate definition
					}else{
						die("Parsing failed at line %lu: variable name expected after ','", lineCount);
					}
				}
				if(g->literals){
					if(g->literals->next){
						// at least two literals, no need to change
					}else{ // only one literal
						if(g->type == AND){
							g->type = PASS;
						}
					}
				}else{
					if(g->type == AND){
						g->type = TRUE;
					}
				}
			}
			if(g->literals == NULL && g->type == AND){
				g->type = TRUE;
			}
		}else if(strcmp(word, "or") == 0 || strcmp(word, "OR") == 0){ // 'or' gate
			g = defineGateVar(v, OR); // add definition
			fscanExpectedChar('(');
			if(parseLiteral(&sign)){ // check if at least 1 literal
				addLitToGateAssumeFree(g, sign); // add to gate definition
				while(fscanfPlus("%1[,]")){ // while a comma follows, a literal must also!
					if(parseLiteral(&sign)){
						addLitToGateAssumeFree(g, sign); // add to gate definition
					}else{
						die("Parsing failed at line %lu: variable name expected after ','", lineCount);
					}
				}
				if(g->literals){
					if(g->literals->next){
						// at least two literals, no need to change
					}else{ // only one literal
						if(g->type == OR){
							g->type = PASS;
						}
					}
				}
			}
			if(g->literals == NULL && g->type == OR){
				g->type = FALSE;
			}
		}else if(strcmp(word, "xor" ) == 0 || strcmp(word, "XOR") == 0){ // 'xor' gate
			char error = 1;
			g = defineGateVar(v, XOR); // add definition
			fscanExpectedChar('(');
			if(parseLiteral(&sign)){ // check first literal
				addLitToGateAssumeFree(g, sign); // add to gate definition
				if(fscanfPlus("%1[,]")){ // a comma follows
					if(parseLiteral(&sign)){ // check second literal
						addLitToGateAssumeFree(g, sign); // add to gate definition
						error = 0;
					}
				}
			}
			if(error){
				die("Parsing failed at line %lu: bad 'xor' syntax", lineCount);
			}
		}else if(strcmp(word, "ite") == 0 || strcmp(word, "ITE") == 0){ // 'ite' gate
			char error = 1;
			g = defineGateVar(v, ITE); // add definition
			fscanExpectedChar('(');
			if(parseLiteral(&sign)){ // check first literal
				addLitToGateAssumeFree(g, sign); // add to gate definition
				if(fscanfPlus("%1[,]")){ // a comma follows
					if(parseLiteral(&sign)){ // check second literal
						addLitToGateAssumeFree(g, sign);
						if(fscanfPlus("%1[,]")){ // a comma follows
							if(parseLiteral(&sign)){ // check third literal
								addLitToGateAssumeFree(g, sign);
								error = 0;
							}
						}
					}
				}
			}
			if(error){
				die("Parsing failed at line %lu: bad 'ite' syntax", lineCount);
			}
			if(g->type == AND || g->type == OR){
				if(g->literals){
					if(g->literals->next == NULL){ // only one literal
						g->type = PASS;
					}
				}
			}
		}else{
			die("Parsing failed at line %lu: 'exists', 'forall', 'and', 'or', 'xor' or 'ite' expected", lineCount);
		}
		
		fscanExpectedChar(')'); // all gate statements end with ')'
		addUniqueGateVar(v);
		//fCheckNewline();
	}
	
	// !!! Adds gate vars to innermost quantifier block, existentially quantified !!!
	quantifyGateVars();
}

/*
 * traverseTree starts from the gate with the same name as defined in output(),
 * marking all reached variables (alias = ULONG_MAX) and counting the amount of
 * resulting clauses (according to Tseitin)
 */
void traverseTree(){
	varList *vlHead = NULL;
	varList *vlTail = NULL;
	varList *currentVL = NULL;
	var *currentVar;
	gate *g;
	litList *ll; // litList of currently observed gate
	unsigned long litCount = 0; // number of literals in above list
	
	if((currentVar = getVarVS(data, data->outputName)) == NULL){ // check if root variable defined
		die("Variable specified in output statement not defined in the file");
	}
	/*lllHead = malloc(sizeof(litListList));
	currentLit = malloc(sizeof(litList));
	if((lllHead == NULL) || (currentLit == NULL)){
		die("Runtime error occured at memory initialization (traverseTree)");
	}
	currentLit->variable = currentVar; // initialize litListList with one element containing root gate var
	currentLit->next = NULL;
	lllHead->list = currentLit;
	lllHead->next = NULL;
	lllTail = lllHead;
	currentlll = lllHead;*/
	addVarVL(&vlHead, currentVar);
	vlTail = vlHead;
	currentVL = vlHead;
	data->outputVar = currentVar; // update root variable
	data->tseitinClauseCount = 1; // at least the root as a clause
	
	do{ // traverse list as it expands
		currentVar = currentVL->variable;
		currentVar->alias = ULONG_MAX; // mark visited variable
		g = currentVar->gateDefinition;
		if(g){ // check if it is a gate variable
			ll = g->literals;
			litCount = 0; // number of literals
			while(ll){ // add all variables used in this gate to list to be traversed
				addVarVLTail(&vlTail, ll->variable);
				++litCount;
				ll = ll->next;
			}
			if(g->type == AND){
				if(litCount){
					data->tseitinClauseCount += litCount + 1; // disjunction between every literal and negated gate, plus one containing every literal negated
				}else{
					die("Empty AND gate should have been optimized away");
				}
			}else if(g->type == OR){
				if(litCount){
					data->tseitinClauseCount += litCount + 1; // disjunction between every literal negated and gate, plus one containing every literal and gate negated
				}else{
					die("Empty OR gate should have been optimized away");
				}
			}else if(g->type == XOR){
				data->tseitinClauseCount += 4;
			}else if(g->type == ITE){
				data->tseitinClauseCount += 4;
			}
		}
		currentVL = currentVL->next;
		// TODO: maybe free?
	}while(currentVL);
}

// prenexTree assumes that gateVars are ordered so that no gate requires another defined later in the list
// in other words: having the same condition as QCIR
void prenexTree(){
	if(data->outputVar->gateDefinition == NULL){
		return;
	}
	varList *current = data->gateVars, *previous = NULL;
	gate * g;
	while(current){
		if(current->variable->alias == 0){ // gate irrelevant
			if(previous){
				previous->next = previous->next->next;
			}
			freeVar(current->variable);
			current = current->next;
			free(current);
		}else{
			g = current->variable->gateDefinition;
			//if(g->type != TRUE)
			//if(g->)
		}
	}
}

/*
 * printPrefixQDIMACS prints the prefix of the parsed data in QDIMACS format
 */
void printPrefixQDIMACS(){
	qBlock *currentQB = data->prefix;
	varList *currentVL;
	do{
		if(currentQB->type == EXISTS){
			printf("e ");
		}else{
			printf("a ");
		}
		currentVL = currentQB->variables;
		while(currentVL){
			if(currentVL->variable->alias > 0){
				printf("%lu ", currentVL->variable->alias);
			}
			currentVL = currentVL->next;
		}
		printf("0\n");
		currentQB = currentQB->next;
	}while(currentQB != data->prefix);
}
/*
 * printQDIMACS prints the parsed data in QDIMACS format
 */
void printQDIMACS(){
	varList *currentElem = data->gateVars;
	gate *currentGate;
	litList *currentLit;
	long currentGateAlias;
	
	printf("c formula converted from %s\n", format);
	printf("c Thank you for your mercy and grace, Jesus!\n");
	printf("p cnf %lu %lu\n", data->tseitinVariableCount, data->tseitinClauseCount); // problem line
	printPrefixQDIMACS(); // prefix
	if(data->outputSign){ // root literal positive
		printf("%lu 0\n", data->outputVar->alias);
	}else{ // root literal negative
		printf("-%lu 0\n", data->outputVar->alias);
	}
	
	while(currentElem){
		currentGate = currentElem->variable->gateDefinition;
		currentGateAlias = currentElem->variable->alias;
		currentElem = currentElem->next;
		if(currentGateAlias < 1){ // gate not relevant
			continue;
		}
		currentLit = currentGate->literals;
		if(currentGate->type == AND){
			while(currentLit){
				if(currentLit->sign){ // current literal positive
					printf("-%lu %lu 0\n", currentGateAlias, currentLit->variable->alias);
				}else{ // current literal negative
					printf("-%lu -%lu 0\n", currentGateAlias, currentLit->variable->alias);
				}
				currentLit = currentLit->next;
			}
			printf("%lu ", currentGateAlias);
			// reset pointer over literals
			currentLit = currentGate->literals;
			while(currentLit){
				if(currentLit->sign){ // current literal positive
					printf("-%lu ", currentLit->variable->alias);
				}else{ // current literal negative
					printf("%lu ", currentLit->variable->alias);
				}
				currentLit = currentLit->next;
			}
			printf("0\n");
		}else if(currentGate->type == OR){
			while(currentLit){
				if(currentLit->sign){ // current literal positive
					printf("%lu -%lu 0\n", currentGateAlias, currentLit->variable->alias);
				}else{ // current literal negative
					printf("%lu %lu 0\n", currentGateAlias, currentLit->variable->alias);
				}
				currentLit = currentLit->next;
			}
			printf("-%lu ", currentGateAlias);
			// reset pointer over literals
			currentLit = currentGate->literals;
			while(currentLit){
				if(currentLit->sign){ // current literal positive
					printf("%lu ", currentLit->variable->alias);
				}else{ // current literal negative
					printf("-%lu ", currentLit->variable->alias);
				}
				currentLit = currentLit->next;
			}
			printf("0\n");
		}else if(currentGate->type == XOR){
			long x = currentLit->variable->alias;
			long y = currentLit->next->variable->alias;
			if(currentLit->sign){ // x
				if(currentLit->next->sign){ // x y
					printf("-%lu -%lu -%lu 0\n", x, y, currentGateAlias);
					printf("%lu %lu -%lu 0\n", x, y, currentGateAlias);
					printf("-%lu %lu %lu 0\n", x, y, currentGateAlias);
					printf("%lu -%lu %lu 0\n", x, y, currentGateAlias);
				}else{ // x -y
					printf("-%lu %lu -%lu 0\n", x, y, currentGateAlias);
					printf("%lu -%lu -%lu 0\n", x, y, currentGateAlias);
					printf("-%lu -%lu %lu 0\n", x, y, currentGateAlias);
					printf("%lu %lu %lu 0\n", x, y, currentGateAlias);
				}
			}else{ // -x
				if(currentLit->next->sign){ // -x y
					printf("%lu -%lu -%lu 0\n", x, y, currentGateAlias);
					printf("-%lu %lu -%lu 0\n", x, y, currentGateAlias);
					printf("%lu %lu %lu 0\n", x, y, currentGateAlias);
					printf("-%lu -%lu %lu 0\n", x, y, currentGateAlias);
				}else{ // -x -y
					printf("%lu %lu -%lu 0\n", x, y, currentGateAlias);
					printf("-%lu -%lu -%lu 0\n", x, y, currentGateAlias);
					printf("%lu -%lu %lu 0\n", x, y, currentGateAlias);
					printf("-%lu %lu %lu 0\n", x, y, currentGateAlias);
				}
			}
		}else if(currentGate->type == ITE){
			long x = currentLit->variable->alias;
			long y = currentLit->next->variable->alias;
			long z = currentLit->next->next->variable->alias;
			if(currentLit->sign){ // x
				if(currentLit->next->sign){ // x y
					printf("-%lu -%lu %lu 0\n", x, y, currentGateAlias);
					printf("-%lu %lu -%lu 0\n", x, y, currentGateAlias);
				}else{ // x -y
					printf("-%lu %lu %lu 0\n", x, y, currentGateAlias);
					printf("-%lu -%lu -%lu 0\n", x, y, currentGateAlias);
				}
				if(currentLit->next->next->sign){ // x z
					printf("%lu %lu -%lu 0\n", x, z, currentGateAlias);
					printf("%lu -%lu %lu 0\n", x, z, currentGateAlias);
				}else{ // x -z
					printf("%lu -%lu -%lu 0\n", x, z, currentGateAlias);
					printf("%lu %lu %lu 0\n", x, z, currentGateAlias);
				}
			}else{ // -x
				if(currentLit->next->sign){ // x y
					printf("%lu -%lu %lu 0\n", x, y, currentGateAlias);
					printf("%lu %lu -%lu 0\n", x, y, currentGateAlias);
				}else{ // x -y
					printf("%lu %lu %lu 0\n", x, y, currentGateAlias);
					printf("%lu -%lu -%lu 0\n", x, y, currentGateAlias);
				}
				if(currentLit->next->next->sign){ // x z
					printf("-%lu %lu -%lu 0\n", x, z, currentGateAlias);
					printf("-%lu -%lu %lu 0\n", x, z, currentGateAlias);
				}else{ // x -z
					printf("-%lu -%lu -%lu 0\n", x, z, currentGateAlias);
					printf("-%lu %lu %lu 0\n", x, z, currentGateAlias);
				}
			}
		}
	}
}

int main(int argc, char **argv){
	FILE *fpIn;
	
	if (argc < 2){
		printf("usage: %s <input_filename>", argv[0]);
		return 0;
	}
	
	fpIn = fopen(argv[1], "r");
	if(fpIn == NULL){
		die("Could not open input file.");
	}
	
	char *input;
	long fsize;
	
	// load file at once to minimize hard disk operations
	fseek(fpIn, 0, SEEK_END);
	fsize = ftell(fpIn);
	rewind(fpIn);

	input = malloc(fsize + 1);
	if(input == NULL){
		die("Could not allocate memory according to file size.");
	}
	if(fread(input, fsize, 1, fpIn) < 1){
		if(fread(input, fsize, 1, fpIn) < 1){
			die("Reading file failed.");
		}
	}
	
	fclose(fpIn);
	input[fsize] = 0;
	
	// initialize data structure
	data = malloc(sizeof(varSets)); // define variable pool
	if(data){
		*data = (varSets){0};
	}else{
		die("Could not allocate memory");
	}
	
	wordlen = 32;
	word = malloc(sizeof(char)*wordlen);
	varFormat = malloc(sizeof(char)*70);
	if(word == NULL || varFormat == NULL){
		die("Could not allocate memory");
	}
	strcpy(varFormat, "%31[abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_]"); // ascii letters, digits and underscores
	
	parseQCIR(input, fsize);
	free(input);
	
	traverseTree();
	indexVars();
	if(declaredVariableCount){
		if(declaredVariableCount != data->tseitinVariableCount){
			die("Declared amount of variables(%lu) differs from actual amount(%lu)!", declaredVariableCount, data->tseitinVariableCount);
		}
	}
	
	 //Test printing
	printData();
	printf("Tseitin clause count: %lu\n", data->tseitinClauseCount);
	printf("Tseitin variable count: %lu\n\n", data->tseitinVariableCount);
	
	printQDIMACS();
	
	freeResources(); // free variable storage
	
	return 0;
}