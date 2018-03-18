#include <data_types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Search in a list for a variable that has given name, return pointer to it, NULL if no match found.
 var *getVarVL( varList *list, char *name){
	//if(&list.variable == NULL)
	 varList *current = list;
	while(current != NULL){
		if(strcmp(current->variable->name, name)){
			current = current->next;
		}else{
			return current->variable;
		}
	}
	return NULL; // no match found
}

 var *getVarLL( litList *list, char *name){
	 litList *current = list;
	while(current != NULL){
		if(strcmp(current->variable->name, name)){
			current = current->next;
		}else{
			return current->variable;
		}
	}
	return NULL; // no match found
}

 var *getVarQB( qBlock *qList, char *name){
	qBlock *currentQ = qList;
	while(currentQ != NULL){
		var *aux = getVarVL(currentQ->variables, name);
		if(aux){
			return aux;
		}
		currentQ = currentQ->next;
		if(currentQ == qList){ // reached head again
			break;
		}
	}
	return NULL; // no match found
}

 var *getVarVS( varSets *set, char *name){
	var *result;
	result = getVarVL(set->freeVars, name);
	if(result == NULL){
		result = getVarQB(set->prefix, name);
		if(result == NULL){
			result = getVarVL(set->gateVars, name);
		}
	}
	return result;
}

// interrupts program if contents of word correspondent to an already existing var name in data
void checkUniqueness(char *word){
	if(getVarVS(data, word)){
		die("Duplicate variable definition at line %lu: %s", lineCount, word);
	}
}

// create a new var with given name, returns NULL on failure
var *createVar(char *name){
	var *result;
	if((result = malloc(sizeof(var)))){
		if((result->name = malloc(strlen(name)+1))){
			strcpy(result->name, name);
			result->alias = 0; // initially unreached
			result->gateDefinition = NULL;
			return result;
		}
		free(result);
	}
	//die("Runtime error occured at memory initialization (createVar)");
	return NULL;
}

// create and add a new variable with given name to beginning of the list,
// updates argument pointer and returns pointer to new head
// interrupts program on failure
varList *addNewVarVL(varList **list, char *name){
	varList *head = malloc(sizeof(varList));
	if(head != NULL){
		if((head->variable = createVar(name))){
			head->next = *list;
			*list = head;
			return head;
		}
		free(head);
	}
	die("Runtime error occured at memory initialization (addNewVarVL)");
	return NULL;
}
// additionally checks and terminates if a var with such name already exists in data
varList *addUniqueVarVL(varList **list, char *name){
	checkUniqueness(name);
	return addNewVarVL(list, name);
}
// add an already existing variable to to beginning of the list,
// updates argument pointer and returns pointer to new head
varList *addVarVL(varList **list, var *toAdd){
	varList *head;
	if(toAdd == NULL){
		return NULL;
	}
	head = malloc(sizeof(varList));
	if(head){
		head->variable = toAdd;
		head->next = *list;
		*list = head;
		return head;
	}else{
		die("Runtime error occured at memory initialization (addVarVL)");
	}
	return NULL;
}

// add a new literal to beginning of the list, sign = 0 means negative, otherwise positive
// updates argument pointer and returns pointer to new head
litList *addVarLLHead(litList **list, char sign, var *variable){
	litList *head = malloc(sizeof(litList));
	if(head != NULL){
		head->sign = sign;
		head->variable = variable;
		head->next = *list;
		*list = head;
		return head;
	}
	die("Runtime error occured at memory initialization (addVarLLHead)");
	return NULL;
}
// add a new literal to end of the list
// returns pointer to new element
litList *addVarLLTail(litList **list, char sign, var *variable){ // TODO: check **
	litList *newElem;
	newElem = malloc(sizeof(litList));
	if(newElem != NULL){
		newElem->sign = sign;
		newElem->variable = variable;
		newElem->next = NULL;
		while(*list){ // set list to point to the 'next' pointer in the last element of the list
			list = &((*list)->next);
		}
		*list = newElem;
		return newElem;
	}
	die("Runtime error occured at memory initialization (addVarLLTail)");
	return NULL;
}

// add a var as a new last element, update currentTail pointer
// 'currentTail' is assumed to be an end of an existing list (not NULL)
varList *addVarVLTail(varList **currentTail, var *toAdd){
	varList *tail;
	if(toAdd == NULL){
		return NULL;
	}
	tail = malloc(sizeof(varList));
	if(tail){
		tail->variable = toAdd;
		tail->next = NULL;
		(*currentTail)->next = tail;
		*currentTail = tail;
		return tail;
	}else{
		die("Runtime error occured at memory initialization (addVarVLTail)");
	}
	return NULL;
}

// add a litList, if it is not NULL, to to beginning of the list,
// updates argument pointer and returns pointer to new head
/*litListList *addListLLLHead(litListList **list, litList *toAdd){
	litListList *head;
	if(toAdd == NULL){
		return NULL;
	}
	head = malloc(sizeof(litListList));
	if(head){
		head->list = toAdd;
		head->next = *list;
		*list = head;
		return head;
	}else{
		die("Runtime error occured at memory initialization (addListLLLHead)");
	}
	return NULL;
}
// add a litList as a new last element, update currentTail pointer
// 'currentTail' is assumed to be an end of an existing list (not NULL)
litListList *addListLLLTail(litListList **currentTail, litList *toAdd){
	litListList *tail;
	if(toAdd == NULL){
		return NULL;
	}
	tail = malloc(sizeof(litListList));
	if(tail){
		tail->list = toAdd;
		tail->next = NULL;
		(*currentTail)->next = tail;
		*currentTail = tail;
		return tail;
	}else{
		die("Runtime error occured at memory initialization (addListLLLTail)");
	}
	return NULL;
}*/


// create and add a new unique free variable to data set
var *addUniqueFreeVar(){
	varList *elem = addUniqueVarVL(&(data->freeVars), word);
	return elem->variable;
}

// upgrade a var with a gate definition; type = 0 means negative, otherwise positive
// returns a pointer to newly created gate definition
gate *defineGateVar(var *target, char type){
	gate *result = malloc(sizeof(gate));
	if(result){
		result->literals = NULL;
		result->type = type;
		result->variable = target;
		target->gateDefinition = result;
		return result;
	}
	die("Runtime error occured at memory initialization (defineGateVar)");
	return NULL;
}

// create and add a new unique gate variable to data set
// returns a pointer to newly created variable
var *addUniqueGateVar(){
	varList *elem = addUniqueVarVL(&(data->gateVars), word);
	return elem->variable;
}

// add a new literal to a gate, assume a free variable if not already defined in data
// returns a pointer to and if a new var was created and added to free()
var *addLitToGateAssumeFree(gate *g, char sign){
	var *atom,*result = NULL;
	atom = getVarVS(data, word); // get if already defined
	if(atom == NULL){ // assume free if not already defined
		varList *elem = addNewVarVL(&(data->freeVars), word);
		atom = elem->variable;
		result = atom;
	}
	if(g->type == 3 || g->type == 4){ // and,or gate
		addVarLLHead(&(g->literals), sign, atom);
	}else if(g->type == 5 || g->type == 6){ // xor,ite gate: ordering of literals matters
		addVarLLTail(&(g->literals), sign, atom);
	}
	return result;
}

// create and add a new variable with given name and quantification to prefix,
// added to the existing innermost block if it has the same quantification, creating a new block otherwise
void addVarQB(qBlock **head, char quantifier){
	if(*head){ // QBlock alredy exists
		qBlock *tail = (*head)->prev;
		if(tail){ // innermost block
			if(tail->type == quantifier){ // innermost block has the same quantification
				addNewVarVL(&(tail->variables), word);
			}else{ // new innermost block must be initialized
				qBlock *newBlock = malloc(sizeof(qBlock));
				if(newBlock){
					newBlock->type = quantifier;
					newBlock->variables = NULL;
					addNewVarVL(&(newBlock->variables), word);
					newBlock->next = *head;
					newBlock->prev = tail;
					(*head)->prev = newBlock;
					tail->next = newBlock;
				}else{
					die("Runtime error occured at memory initialization (addVarQB)");
				}
			}
		}else{ // tail pointer is NULL, should not be reached
			
		}
	}else{ // create first QBlock
		*head = malloc(sizeof(qBlock));
		if(*head){
			(*head)->type = quantifier;
			(*head)->variables = NULL;
			addNewVarVL(&((*head)->variables), word);
			(*head)->next = *head;
			(*head)->prev = *head;
		}else{
			die("Runtime error occured at memory initialization (addVarQB)");
		}
	}
}
// additionally checks and terminates if a var with such name already exists in data
void addUniqueVarQB(char quantifier){
	checkUniqueness(word);
	addVarQB(&(data->prefix), quantifier);
}
// append list of gate variables to innermost quantifier block, existentially quantified
void quantifyGateVars(){
	qBlock *head = data->prefix;
	if(head == NULL){ // create first qBlock
		head = malloc(sizeof(qBlock));
		if(head){
			head->type = EXISTS;
			head->variables = data->gateVars;
			head->next = head;
			head->prev = head;
		}else{
			die("Runtime error occured at memory initialization (quantifyGateVars)");
		}
	}else if(head->prev->type == FORALL){ // current innermost block is a FORALL one, new one needed
		qBlock *newBlock = malloc(sizeof(qBlock));
		if(newBlock){
			newBlock->type = EXISTS;
			newBlock->variables = data->gateVars;
			newBlock->next = head;
			newBlock->prev = head->prev;
			head->prev->next = newBlock;
			head->prev = newBlock;
		}else{
			die("Runtime error occured at memory initialization (quantifyGateVars)");
		}
	}else{ // current innermost block is already an EXISTS one
		varList *aux = head->prev->variables;
		while(aux->next){
			aux = aux->next;
		}
		aux->next = data->gateVars;
	}
}

// go through all the relevant variables in the list and give them a new integer alias
void indexVarVL(varList *list){
	while(list != NULL){
		if(list->variable->alias > 0){
			//++(data->tseitinVariableCount);
			list->variable->alias = ++(data->tseitinVariableCount);
			//printf("%lu %s \n", list->variable->alias, list->variable->name);
		}
		list = list->next;
	}
}
// !!! if no relevant variable is encountered in a qBlock, it is freed and its predecessor and successor are merged !!!
void indexVarQB(qBlock *qList){
	qBlock *currentQ = qList, *emptyQB = NULL;
	unsigned long aux;
	
	while(currentQ != NULL){

		aux = data->tseitinVariableCount;
		indexVarVL(currentQ->variables);
		if(aux == (data->tseitinVariableCount)){ // no relevant variable
			if(emptyQB == NULL){ // first irrelevant block
				emptyQB = currentQ; 
			}
		}else{
			if(emptyQB){ // end of irrelevant blocks
				qBlock *tail;
				if(emptyQB->prev->type == currentQ->type){ // bordering blocks same type, must merge
					varList *aux = emptyQB->prev->variables;
					while(aux->next){
						aux = aux->next;
					}
					aux->next = currentQ->variables;
					currentQ->variables = NULL;
					tail = currentQ;
					currentQ = currentQ->next;
					
					tail->next = emptyQB;
					emptyQB->prev->next = currentQ;
					currentQ->prev = emptyQB->prev;
					emptyQB->prev = tail;
					free(emptyQB);
					
					currentQ = currentQ->prev; // currentQ not yet indexed
				}else{
					tail = currentQ->prev;
					
					tail->next = emptyQB;
					emptyQB->prev->next = currentQ;
					currentQ->prev = emptyQB->prev;
					emptyQB->prev = tail;
					free(emptyQB);
				}
				
				emptyQB = NULL;
			}
		}
		
		currentQ = currentQ->next;
		if(currentQ == qList){ // reached head again
			break;
		}
	}
	if(emptyQB){ // list ends with an irrelevant block
		qBlock *tail = data->prefix->prev;
		tail->next = emptyQB;
		emptyQB->prev->next = data->prefix;
		data->prefix->prev = emptyQB->prev;
		emptyQB->prev = tail;
		free(emptyQB);
	}
}
// go through all the relevant variables and give them a new integer alias
void indexVars(){
	data->tseitinVariableCount = 0;
	indexVarVL(data->freeVars);
	indexVarQB(data->prefix);
	//indexVarVL(data->gateVars); // already indexed, because they are part of the prefix
}

// free data structure
void freeVar(var *v){
	free(v->name);
	if(v->gateDefinition){
		freeLL(v->gateDefinition->literals);
		free(v->gateDefinition);
	}
	free(v);
}
void freeVL(varList *current){
	varList *aux;
	while(current){
		if(current->variable){
			freeVar(current->variable);
		}else{
			//die("Empty varList element encountered!");
		}
		aux = current;
		current = current->next;
		free(aux);
	}
}
void freeLL(litList *current){
	litList *aux;
	while(current){
		aux = current;
		current = current->next;
		free(aux);
	}
}
void freeQB(qBlock *current){
	qBlock *aux;
	if(current){
		if(current->prev){
			current->prev->next = NULL; // break the cycle between first and last
		}
	}
	while(current){
		if(current->variables){
			freeVL(current->variables);
		}else{
			//die("Empty qBlock element encountered!");
		}
		aux = current;
		current = current->next;
		free(aux);
	}
}
void freeVS(varSets *data){
	freeVL(data->freeVars);
	freeQB(data->prefix);
	//freeVL(data->gateVars); // freed at freeQB, because list is part of innermost EXISTS block
	free(data->outputName);
	free(data);
}

//print contets of a varList to stdout
void printVL(varList *current){
	while(current){
		printf("%s ", current->variable->name);
		current = current->next;
	}
}
void printFree(){
	if(data->freeVars == NULL){
		return;
	}
	printf("Free:\n");
	printVL(data->freeVars);
	printf("\n");
}
//print contents of quantifier prefix to stdout
void printPrefix(){
	qBlock *head, *current;
	head = data->prefix;
	if(head == NULL){
		return;
	}
	current = head;
	printf("Prefix:\n");
	do{
		printf("%d: ", current->type);
		printVL(current->variables);
		printf("\n");
		current = current->next;
	}while(current && current != head);
}

void printGates(){
	varList *currElem;
	gate* currGate;
	litList *currLit;
	currElem = data->gateVars;
	if(currElem == NULL){
		return;
	}
	printf("Gates:\n");
	do{
		currGate = currElem->variable->gateDefinition;
		currLit = currGate->literals;
		printf("%s = %d(", currElem->variable->name, currGate->type);
		while(currLit){
			printf("%d-%s ", currLit->sign, currLit->variable->name);
			currLit = currLit->next;
		}
		printf(")\n");
		currElem = currElem->next;
	}while(currElem);
}

void printData(){
	printFree();
	printPrefix();
	printf("Output sign:%d name:%s\n", data->outputSign, data->outputName);
	printGates();
	printf("Line counter: %lu\n", lineCount);
}