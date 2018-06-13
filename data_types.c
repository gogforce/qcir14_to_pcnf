#include <data_types.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// swap target if modifier is negative (0)
void multiplySign(char *target, char modifier){
	if(modifier == 0){
		if(*target){
			*target = 0;
		}else{
			*target = 1;
		}
	}
}
void printLL(litList *current);
void printVL(varList *current);

iteratorQB *newQBiterator(qBlock *qb){
	if(qb == NULL){
		return NULL;
	}
	iteratorQB *result = malloc(sizeof(iteratorQB));
	if(result == NULL){
		die("Runtime error occured at memory initialization (newQBiterator)");
	}
	result->vl = qb->variables;
	result->qb = qb;
	result->qbHead = NULL;
	return result;
}
var *nextVarQB(iteratorQB *it){
	if(it == NULL){
		return NULL;
	}
	if((it->qb == it->qbHead)){ // iteration reached beginning
		free(it);
		return NULL;
	}
	var *result = it->vl->variable;
	if(it->vl->next){
		it->vl = it->vl->next;
	}else{
		if(it->qbHead == NULL){
			it->qbHead = it->qb;
		}
		it->qb = it->qb->next;
		it->vl = it->qb->variables;
	}
	return result;
}
varList *nextVarListQB(iteratorQB *it){
	if(it == NULL){
		return NULL;
	}
	if((it->qb == it->qbHead)){ // iteration reached beginning
		free(it);
		return NULL;
	}
	varList *result = it->vl;
	if(it->vl->next){
		it->vl = it->vl->next;
	}else{
		if(it->qbHead == NULL){
			it->qbHead = it->qb;
		}
		it->qb = it->qb->next;
		it->vl = it->qb->variables;
	}
	return result;
}

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
void removeVarQB(qBlock **head, var *v){
	if(*head == NULL){
		return;
	}
	qBlock **qb = head;
	varList **vl;
	
	do{
		vl = &((*qb)->variables);
		while(*vl){
			if((*vl)->variable == v){
				*vl = (*vl)->next;
				if((*qb)->variables == NULL){ // QB block became empty
					if((*qb)->next == *qb){ // only one block
						*head = NULL;
					/*}else if((*qb)->next->next == *qb) // only 2 blocks
						qBlock *qb2 = (*qb)->next;
						qb2->next = qb2;
						qb2->prev = qb2;
						*head = qb2;*/
					}else if((*qb)->next == *head){
						qBlock *qb2 = (*qb)->prev;
						qb2->next = *head;
						(*head)->prev = qb2;
					}else if((*qb) == *head){
						qBlock *qb2 = (*qb)->next;
						qb2->prev = (*qb)->prev;
						(*qb)->prev->next = qb2;
						*head = qb2;
					}else{ // merge previous and next blocks
						qBlock *qb2 = (*qb)->prev;
						varList *v = qb2->variables;
						while(v->next){
							v = v->next;
						}
						v->next = (*qb)->next->variables;
						/*vl = &(qb2->variables);
						while(*vl){
							vl = &((*vl)->next);
						}
						*vl = (*qb)->next->variables;*/
						qb2->next = qb2->next->next->next;
						qb2->next->prev = qb2;
						//free((*qb)->next); freeing here messes up the other blocks?!
					}
					//free(*qb); freeing here messes up the other blocks?!
				}
				return;
			}
			vl = &((*vl)->next);
		}
		qb = &((*qb)->next);
	}while(*qb != *head);
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

varList * addGateVar(var *toAdd){
	varList *v;
	if(toAdd == NULL){
		return NULL;
	}
	v = malloc(sizeof(varList));
	if(v){
		v->variable = toAdd;
		v->next = NULL;
		if(data->lastGateVar){
			data->lastGateVar->next = v;
		}else{
			data->gateVars = v;
		}
		data->lastGateVar = v;
		return v;
	}else{
		die("Runtime error occured at memory initialization (addGateVar)");
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
		if(type == TRUE || type == FALSE){
			result->subtreeSize = 0;
		}else{
			result->subtreeSize = 1;
		}
		result->localFreeVars = NULL;
		result->localPrefix = NULL;
		//result->localScope = NULL;
		result->uses = 0;
		if(target->gateDefinition){
			freeLL(target->gateDefinition->literals);
			//free(target->gateDefinition->) necessary?
			free(target->gateDefinition);
		}
		target->gateDefinition = result;
		return result;
	}
	die("Runtime error occured at memory initialization (defineGateVar)");
	return NULL;
}

// add a new unique gate variable to data set
void addUniqueGateVar(var *toAdd){
	checkUniqueness(toAdd->name);
	addGateVar(toAdd);
	//addVarVL(&(data->gateVars), toAdd);
}

// copy a varList with references to the same variables
varList *copyVL(varList *toCopy){
	if(toCopy == NULL){
		return NULL;
	}
	varList *current = toCopy;
	varList *newHead, *newCurrent, *newTail;
	newHead = malloc(sizeof(varList));
	if(newHead == NULL){
		die("Runtime error occured at memory initialization (copyVL)");
	}
	newTail = newHead;
	newCurrent = newHead;
	while(1){
		newCurrent->variable = current->variable;
		newTail = newCurrent;
		//printf("Problem copyVL?");
		if(current->next == NULL){
			newCurrent->next = NULL;
			break;
		}
		current = current->next;
		newCurrent = malloc(sizeof(varList));
		if(newCurrent == NULL){
			die("Runtime error occured at memory initialization (copyVL)");
		}
		newTail->next = newCurrent;
	}
	return newHead;
}

// copy a qBlock with references to the same variables
qBlock *copyQB(qBlock *toCopy){
		//printf("Problem copyQB? -1");
	if(toCopy == NULL){
		return NULL;
	}
	qBlock *current = toCopy;
	qBlock *newHead, *newTail, *newBlock;
		//printf("Problem copyQB? 0");
	newHead = malloc(sizeof(qBlock));
	if(newHead == NULL){
		die("Runtime error occured at memory initialization (copyQB)");
	}
	newTail = newHead;
	newBlock = newHead;
		//printf("Problem copyQB? 1");
	while(1){
		newBlock->prev = newTail;
		newBlock->type = current->type;
		//printf("Problem copyQB? 2");
		newBlock->variables = copyVL(current->variables);
		newTail = newBlock;
		current = current->next;
		if(current == toCopy){
			newBlock->next = newHead;
			newHead->prev = newBlock;
			break;
		}
		newBlock = malloc(sizeof(qBlock));
		if(newBlock){
			newTail->next = newBlock;
		}else{
			die("Runtime error occured at memory initialization (addVarQB)");
		}
	}
	return newHead;
}
// copy a tree beginning at root gate, return a pointer to new root,
// references to non-gate vars are kept, returns argument if it is not a gate
var *copyTree(var *root){
	if(root->gateDefinition == NULL){
		return root;
	}
	var *result = createVar(root->name);
	gate *g = defineGateVar(result, root->gateDefinition->type);
	g->uses = 1;
	g->subtreeSize = root->gateDefinition->subtreeSize;
	g->localFreeVars = root->gateDefinition->localFreeVars;
	//g->localScope = root->gateDefinition->localScope;
	g->localPrefix = copyQB(root->gateDefinition->localPrefix);
	// copy literals
	litList *current = root->gateDefinition->literals;
	if(current == NULL){ return result;}
	litList *newHead, *newCurrent, *newTail;
	newHead = malloc(sizeof(litList));
	if(newHead == NULL){
		die("Runtime error occured at memory initialization (copyTree)");
	}
	newTail = newHead;
	newCurrent = newHead;
	while(1){
		newCurrent->sign = current->sign;
		newCurrent->variable = copyTree(current->variable);
		newTail = newCurrent;
		if(current->next == NULL){
			newCurrent->next = NULL;
			break;
		}
		current = current->next;
		newCurrent = malloc(sizeof(litList));
		if(newCurrent == NULL){
			die("Runtime error occured at memory initialization (copyTree)");
		}
		newTail->next = newCurrent;
	}
	g->literals = newHead;
	addGateVar(result);
	return result;
}

void addLitToGate(gate *g, char sign, var *atom, char isNewVar){
	char duplicate = 0; // set to 1 if variable already occurs in this gate
	char duplicateSign; // stores the sign of duplicate variable already present
	int duplicateIndex = 0; // index of duplicate, beginning from 1
	unsigned int litCount = 0; // amount of literals already stored in gate, not fully counted if a duplicate is found
	char type = 0; // type of atom to add if it is a gate
	
	if(isNewVar == 0){ // check if a variable already occurs in this gate
		litList *current = g->literals;
		while(current){
			++litCount;
			if(strcmp(current->variable->name, atom->name) == 0){
				duplicate = 1;
				duplicateSign = current->sign;
				duplicateIndex = litCount;
			}
			if(duplicate && litCount > 3){
				break;
			}
			current = current->next;
		}
		
		if(duplicate == 0){
			while(atom->gateDefinition != NULL && atom->gateDefinition->type == PASS){ // reach through PASS gates
				if(atom->gateDefinition->literals->sign == sign){
					sign = 1;
				}else{
					sign = 0;
				}
				atom = atom->gateDefinition->literals->variable;
			}
			if(atom->gateDefinition != NULL){
				type = atom->gateDefinition->type;
				if(type != TRUE && type != FALSE && g->type != PASS){
					// increase usage counter, copy in case of multiple use
					atom->gateDefinition->uses += 1;
					if(atom->gateDefinition->uses > 1){
						atom = copyTree(atom);
					}
					// update subtree size if necessary
					if(g->subtreeSize <= atom->gateDefinition->subtreeSize){
						g->subtreeSize = atom->gateDefinition->subtreeSize + 1;
					}
				}
			}
		}
	}
	if(g->type == EXISTS || g->type == FORALL){
		if(litCount == 0){
			if((type == TRUE && sign > 0)||(type == FALSE && sign == 0)){
				g->localPrefix = NULL;
				g->type = TRUE;
				g->subtreeSize = 0;
			}else if((type == FALSE && sign > 0)||(type == TRUE && sign == 0)){
				g->localPrefix = NULL;
				g->type = FALSE;
				g->subtreeSize = 0;
			}else{
				addVarLLHead(&(g->literals), sign, atom);
			}
		}
	}else if(g->type == AND){
		if(duplicate){
			if(duplicateSign == sign){
				// AND(_,x,x) -> AND(_,x)
			}else{
				// AND(_,x,-x) -> FALSE()
				g->type = FALSE;
				freeLL(g->literals);
				g->literals = NULL;
				g->subtreeSize = 0;
			}
		}else if((type == TRUE && sign > 0)||(type == FALSE && sign == 0)){
			// AND(_,TRUE) -> AND(_)
		}else if((type == FALSE && sign > 0)||(type == TRUE && sign == 0)){
			// AND(_,FALSE) -> FALSE()
			g->type = FALSE;
			freeLL(g->literals);
			g->literals = NULL;
			g->subtreeSize = 0;
		}else{
			addVarLLHead(&(g->literals), sign, atom);
		}
	}else if(g->type == OR){
		if(duplicate){
			if(duplicateSign == sign){
				// OR(_,x,x) -> OR(_,x)
			}else{
				// OR(_,x,-x) -> TRUE()
				g->type = TRUE;
				freeLL(g->literals);
				g->literals = NULL;
				g->subtreeSize = 0;
			}
		}else if((type == TRUE && sign > 0)||(type == FALSE && sign == 0)){
			// OR(_,TRUE) -> TRUE()
			g->type = TRUE;
			freeLL(g->literals);
			g->literals = NULL;
			g->subtreeSize = 0;
		}else if((type == FALSE && sign > 0)||(type == TRUE && sign == 0)){
			// OR(_,FALSE) -> OR(_)
		}else{
			addVarLLHead(&(g->literals), sign, atom);
		}
	}else if(g->type == XOR){ // xor gate: ordering of literals matters
		if(duplicate){
			if(duplicateSign == sign){
				// FALSE constant
				g->type = FALSE;
				freeLL(g->literals);
				g->literals = NULL;
				g->subtreeSize = 0;
			}else{
				// TRUE constant
				g->type = TRUE;
				freeLL(g->literals);
				g->literals = NULL;
				g->subtreeSize = 0;
			}
		}else if((type == TRUE && sign > 0)||(type == FALSE && sign == 0)){
			if(litCount == 0){ // XOR(TRUE,_) (-> XOR(TRUE,x) -> PASS(-x))
				// add constant and remove it when second lit is parsed
				addVarLLHead(&(g->literals), sign, atom);
			}else{
				// XOR(x, TRUE) -> PASS(-x), also XOR(TRUE,TRUE) -> PASS(-TRUE)
				g->type = PASS;
				if(g->literals->sign){
					g->literals->sign = 0;
				}else{
					g->literals->sign = 1;
				}
			}
		}else if((type == FALSE && sign > 0)||(type == TRUE && sign == 0)){
			// PASS gate; XOR(x,FALSE),XOR(FALSE,x) -> PASS(x)
			g->type = PASS;
		}else{
			if(g->literals && g->literals->variable->gateDefinition){
				char previousType = g->literals->variable->gateDefinition->type;
				if((previousType == TRUE && g->literals->sign > 0)||(previousType == FALSE && g->literals->sign == 0)){
					// XOR(TRUE,x) -> PASS(-x)
					g->type = PASS;
					freeLL(g->literals);
					g->literals = NULL;
					if(sign){
						sign = 0;
					}else{
						sign = 1;
					}
				}
			}
			addVarLLTail(&(g->literals), sign, atom);
		}
	}else if(g->type == ITE){  // ite gate: ordering of literals matters
		if(g->literals && g->literals->variable->gateDefinition && ((g->literals->variable->gateDefinition->type == FALSE && g->literals->sign > 0)||(g->literals->variable->gateDefinition->type == TRUE && g->literals->sign == 0))){
			if(litCount == 1){// ITE(FALSE,x,_) -> PASS()
			g->type = PASS;
			freeLL(g->literals);
			g->literals = NULL;
			}else{
				die("Bad ITE gate content, FALSE constant and another variable (addLitToGateAssumeFree)");
			}
		}else if(duplicate){
			if(duplicateIndex == 1){
				if(litCount == 1){ // first two literals identical
					if(duplicateSign == sign){
						// ITE becomes OR; ITE(x,x,_) -> OR(x,_)
						g->type = OR;
					}else{
						// ITE becomes AND; ITE(x,-x,_) -> AND(-x,_)
						g->type = AND;
						g->literals->sign = sign; // exchange sign
					}
				}else if(litCount == 2){ // first and third literals identical
					if(duplicateSign == sign){
						// ITE becomes AND; ITE(x,y,x) -> AND(x,y)
						g->type = AND;
					}else{
						// ITE becomes OR; ITE(x,y,-x) -> OR(-x,y)
						g->type = OR;
						g->literals->sign = sign; // exchange sign
					}
				}
			}else if(duplicateIndex == 2 && litCount == 2){
				if(duplicateSign == sign){
					// PASS gate; ITE(x,y,y) -> PASS(y)
					g->type = PASS;
					litList *old = g->literals;
					g->literals = g->literals->next;
					old->next = NULL;
					freeLL(old);
				}else{
					// XOR gate; ITE(x,y,-y) -> XOR(x,-y)
					g->type = XOR;
					g->literals->next->sign = sign;
				}
			}else{
				die("Bad ITE gate content (addLitToGateAssumeFree)");
			}
		}else if((type == TRUE && sign > 0)||(type == FALSE && sign == 0)){
			if(litCount == 0){ // ITE(TRUE,x,y) -> PASS(x)
				g->type = PASS;
			}else if(litCount == 1){ // ITE(x,TRUE,_) -> OR(x,_)
				g->type = OR;
			}else if(litCount == 2){ // ITE(x,y,TRUE) -> OR(-x,y)
				g->type = OR;
				if(g->literals->sign){
					g->literals->sign = 0;
				}else{
					g->literals->sign = 1;
				}
			}
		}else if((type == FALSE && sign > 0)||(type == TRUE && sign == 0)){
			if(litCount == 0){ // ITE(FALSE,_,_) ... wait next input
				addVarLLTail(&(g->literals), sign, atom);
			}else if(litCount == 1){ // ITE(x,FALSE,_) -> AND(-x,_)
				g->type = AND;
				if(g->literals->sign){
					g->literals->sign = 0;
				}else{
					g->literals->sign = 1;
				}
			}else if(litCount == 2){ // ITE(x,y,FALSE) -> AND(x,y)
				g->type = AND;
			}
		}else{
			addVarLLTail(&(g->literals), sign, atom);
		}
		printf("ITE %s\n", atom->name);
		printLL(g->literals);
	}else if(g->type == TRUE || g->type == FALSE){
		//die("Adding a literal to a truth constant gate pointless (addLitToGateAssumeFree)");
	}else if(g->type == PASS){
		if(litCount == 0){
			addVarLLHead(&(g->literals), sign, atom);
		}else{
			//die("Adding a literal to a non-empty PASS gate pointless (addLitToGateAssumeFree)");
		}
	}
}
// add a new literal to a gate, assume a free variable if not already defined in data
// returns a pointer to and if a new var was created and added to free()
var *addLitToGateAssumeFree(gate *g, char sign){
	var *atom,*result = NULL;
	char isNewVar = 0;
	atom = getVarVS(data, word); // get if already defined
	if(atom == NULL){ // assume free if not already defined
		varList *elem = addNewVarVL(&(data->freeVars), word);
		atom = elem->variable;
		result = atom;
		isNewVar = 1;
	}
	addLitToGate(g, sign, atom, isNewVar);
	return result;
}

// add a variable with given name and quantification to prefix, create new one if toAdd == NULL
// added to the existing innermost block if it has the same quantification, creating a new block otherwise
void addVarQB(qBlock **head, char quantifier, var *toAdd){
	if(toAdd == NULL){
		toAdd = createVar(word);
		if(toAdd == NULL){
			die("Failed to create a new variable (addVarQB)");
		}
	}
	if(*head){ // QBlock alredy exists
		qBlock *tail = (*head)->prev;
		if(tail){ // innermost block
			if(tail->type == quantifier){ // innermost block has the same quantification
				//addNewVarVL(&(tail->variables), word);
				addVarVL(&(tail->variables), toAdd);
			}else{ // new innermost block must be initialized
				qBlock *newBlock = malloc(sizeof(qBlock));
				if(newBlock){
					newBlock->type = quantifier;
					newBlock->variables = NULL;
					//addNewVarVL(&(newBlock->variables), word);
					addVarVL(&(newBlock->variables), toAdd);
					newBlock->next = *head;
					newBlock->prev = tail;
					(*head)->prev = newBlock;
					tail->next = newBlock;
				}else{
					die("Runtime error occured at memory initialization (addVarQB)");
				}
			}
		}else{ // tail pointer is NULL, should not be reached
			die("VIOLATED INVARIANT: QBlock tail pointer should not be NULL");
		}
	}else{ // create first QBlock
		*head = malloc(sizeof(qBlock));
		if(*head){
			(*head)->type = quantifier;
			(*head)->variables = NULL;
			//addNewVarVL(&((*head)->variables), word);
			addVarVL(&((*head)->variables), toAdd);
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
	addVarQB(&(data->prefix), quantifier, NULL);
}
// checks if a free variable with given name already encountered and add it,
// return pointer to it, NULL otherwise
var *addExistingVarQB(qBlock **qb, char quantifier){
	var *toAdd = getVarVL(data->freeVars, word);
	if(toAdd == NULL){
		toAdd = getVarQB(data->prefix, word);
	}
	if(toAdd){
		addVarQB(qb, quantifier, toAdd);
	}
	return toAdd;
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

// mergeQB unites two QBs into one, the head of the first is the head of the union
qBlock *mergeQB(qBlock *qb1, qBlock *qb2){
	if(qb1 == NULL){
		return qb2;
	}else if(qb2 == NULL){
		return qb1;
	}
	
	qBlock *qb1tail = qb1->prev, *qb2tail = qb2->prev;
	if(qb1tail->type == qb2->type){
		varList *vl = qb1tail->variables;
		while(vl->next){
			vl = vl->next;
		}
		vl->next = qb2->variables;
		if(qb2 == qb2tail){ // qb2 has only one block
			return qb1;
		}
		qb1tail->next = qb2->next;
		qb2->next->prev = qb1tail;
	}else{
		qb1tail->next = qb2;
		qb2->prev = qb1tail;
	}
	qb2tail->next = qb1;
	qb1->prev = qb2tail;
	return qb1;
}

// detectConflict checks if v1 is found in prefix or free vars of g2
// returns 1 on match, returns 0 otherwise 
char detectConflict(var *v1, gate *g2){
	var *v2;
	varList *f2;
	iteratorQB *it = newQBiterator(g2->localPrefix);
	while((v2 = nextVarQB(it))){ // iterate over prefix of g2
		if(v1 == v2){ // Conflict between a quantified var in g1 and a quantified var in g2 -> replace in g1
			// no check for case FORALL+OR / EXISTS+AND
			free(it);
			return 1;
		}
	}
	f2 = g2->localFreeVars;
	while(f2){
		if(v1 == f2->variable){ // Conflict between a quantified var in g1 and a free var in g2 -> replace in g1
			return 1;
		}
		f2 = f2->next;
	}
	return 0;
}
// sortOutConflictsAndMergeFreeVars checks for conflicts between variables of a gate and replaces vars
// also prepares list of all free vars of this gate
void sortOutConflictsAndMergeFreeVars(gate *g){ //, gate *g1, gate *g2
	litList *ll1 = g->literals;
	litList *ll2;
	gate *g1, *g2;
	iteratorQB *it1 = NULL;
	varList *vl = NULL;
	varList *f1;
	varList **fvl;
	char hit = 0;
	
	while(ll1){ // add all free vars from lilist to list of local free vars of this gate
		if(ll1->variable->gateDefinition == NULL){ // free variable
			addVarVL(&(g->localFreeVars), ll1->variable);
		}
		ll1 = ll1->next;
	}
	ll1 = g->literals;
	while(ll1){
		//printf("1e %s\n", g->variable->name);
		g1 = ll1->variable->gateDefinition;
		//printf("%s\n", ll1->variable->name);
		if(g1){ // gate variable
			it1 = newQBiterator(g1->localPrefix);
			while((vl = nextVarListQB(it1))){
				//printf("2e %s\n", g->variable->name);
				f1 = g->localFreeVars;
				while(f1){
					if(vl->variable == f1->variable){
						replaceVar(g1, vl, NULL);
						hit = 1;
						break;
					}
					f1 = f1->next;
				}
				
				if(hit == 0){
					ll2 = ll1->next;
					while(ll2){
						g2 = ll2->variable->gateDefinition;
						if(g2){ // gate variable
							if(detectConflict(vl->variable, g2)){
								replaceVar(g1, vl, NULL);
								hit = 1;
								break;
							}
						}
						ll2 = ll2->next;
					}
				}
				
				if(hit == 0){
					removeVarQB(&(g->localPrefix), vl->variable); // if quantifed variable was not
				}else{
					hit = 0;
				}
				//printf("2o %s\n", g->variable->name);
			}
			
			fvl = &(g1->localFreeVars);
			while(*fvl){
				f1 = g->localFreeVars;
				while(f1){
					if((*fvl)->variable == f1->variable){
						*fvl = (*fvl)->next;
						hit = 1;
						break;
					}
					f1 = f1->next;
				}
				if(hit == 0){
					fvl = &((*fvl)->next);
				}else{
					hit = 0;
				}
			}
			*fvl = g->localFreeVars; // append global list to end of local list and make it global
			g->localFreeVars = g1->localFreeVars;
		}
		//printf("1o %s\n", g->variable->name);
		ll1 = ll1->next;
	}
	hit = 0;
	var *v;
	vl = NULL;
	f1 = NULL;
	it1 = newQBiterator(g->localPrefix); // lastly, check total free against head QB and remove vars that are being quantified/vars that do not appear
	while((v = nextVarQB(it1))){
		fvl = &(g->localFreeVars);
		while(*fvl){
			if((*fvl)->variable == v){
				*fvl = (*fvl)->next;
				hit = 1;
				break;
			}
			fvl = &((*fvl)->next);
		}
		if(hit == 0){
			if(vl == NULL){
				addVarVL(&vl, v);
				f1 = vl;
			}else{
				addVarVLTail(&f1, v);
			}
		}else{
			hit = 0;
		}
	}
	f1 = vl;
	while(f1){
		removeVarQB(&(g->localPrefix), f1->variable);
		f1 = f1->next;
	}
	ll1=g->literals;
	freeVL(vl);
	/*while(ll1){
		if(ll1->variable->gateDefinition){
			printf("while prenexing %s, %s prefix:\n", g->variable->name, ll1->variable->name);
			printQB(ll1->variable->gateDefinition->localPrefix);
		}
		ll1 = ll1->next;
	}*/
	//printf("Free %s:", g->variable->name);
	//printVL(g->localFreeVars);
	//printf("\n");
}
	
	/*
	it2 = newQBiterator(g2->localPrefix);
	while(vl2 = nextVarListQB(it2)){ // iterate over prefix of g2
		v2 = vl2->variable;
		it1 = newQBiterator(g1->localPrefix);
		while(v1 = nextVarQB(it1)){
			if(v1 == v2){ // Conflict between a quantified var in g1 and a quantified var in g2 -> replace in g2
				// no check for case FORALL+OR / EXISTS+AND
				replaceVar(g2, vl2, NULL);
				hit = 1;
				free(it1);
				break;
			}
		}
		if(hit == 0){
			f1 = g1->localFreeVars;
			while(f1){
				if(f1->variable == v2){ // Conflict between a free var in g1 and a quantified var in g2 -> replace in g2
					replaceVar(g2, vl2, NULL);
					break;
				}
				f1 = f1->next;
			}
		}else{
			hit = 0;
		}
	}
	//}
	vl2 = g2->localFreeVars;
	while(vl2){
		v2 = vl2->variable;
		it1 = newQBiterator(g1->localPrefix);
		while(v1 = nextVarQB(it1)){
			if(v1 == v2){ // Conflict between a quantified var in g1 and a free var in g2 -> find relevant gate and replace
				litList *ll = g->literals;
				while(ll){
					if(ll->variable->gateDefinition){
						it1 = newQBiterator(ll->variable->gateDefinition->localPrefix);
						while(f1 = nextVarListQB(it1)){
							if(f1->variable == v2){
								replaceVar(ll->variable->gateDefinition, f1, NULL);
								hit = 1;
								free(it1);
								break;
							}
						}
						if(hit){
							break;
						}
					}
					ll = ll->next;
				}
				//if(hit) break; ??
			}
		}
		if(hit == 0){
			f1 = g1->localFreeVars;
			while(f1){
				if(f1->variable == v2){ // Conflict between a free var in g1 and a free var in g2 -> remove in g2
					varList **ref = &(g2->localFreeVars);
					while(*ref){
						if(*ref->variable == v2){
							*ref = *ref->next;
							break;
						}
						ref = &(*ref->next);
					}
					break;
				}
				f1 = f1->next;
			}
		}else{
			hit = 0;
		}
		vl2 = vl2->next;
	}*/

// sorts out conflicts in g and than unites all prefixes (left outermost, right innermost)
void prenexGateSimple(gate *g){
	//printf("prenexing %s\n", g->variable->name);
	sortOutConflictsAndMergeFreeVars(g);
	//printf("sorted %s\n", g->variable->name);
	litList *ll = g->literals;
	while(ll){
		if(ll->variable->gateDefinition){
			g->localPrefix = mergeQB(g->localPrefix, ll->variable->gateDefinition->localPrefix);
		}
		ll = ll->next;
	}
	printf("%s prenexed:\n", g->variable->name);
	printQB(g->localPrefix);
}

// replaces occurences of toBeReplaced in subtree scope with replaceWith
void replaceVarRecursive(gate *scope, var *toBeReplaced, var *replaceWith){
	if(scope->type == TRUE || scope->type == FALSE){
		return;
	}
	
	litList *current = scope->literals;
	while(current){
		if(current->variable->gateDefinition){
			replaceVarRecursive(current->variable->gateDefinition, toBeReplaced, replaceWith);
		}else{
			if(current->variable == toBeReplaced){
				current->variable = replaceWith;
			}
		}
		current = current->next;
	}
}
// replaces occurences of toBeReplaced in subtree scope with replaceWith
// also updates varList element toBeReplaced
void replaceVar(gate *scope, varList *toBeReplaced, var *replaceWith){
	if(replaceWith == NULL){
		char *newName = malloc(sizeof(char)*(strlen(toBeReplaced->variable->name)+5));
		if(newName){
			sprintf(newName, "new.%s", toBeReplaced->variable->name);
			replaceWith = createVar(newName);
			replaceWith->alias = ULONG_MAX; // mark as visited
		}else{
			die("Runtime error occured at memory initialization (replaceVar)");
		}
	}
	replaceVarRecursive(scope, toBeReplaced->variable, replaceWith);
	toBeReplaced->variable = replaceWith;
}

// prenexTree assumes that gateVars are ordered so that no gate requires another defined later in the list
// in other words: having the same condition as QCIR regarding order of gate definitions
void prenexTree(){
	gate * g = data->outputVar->gateDefinition;
	if(g == NULL || g->type == TRUE || g->type == FALSE){ // no need of prenexing
		return;
	}
	varList **current = &(data->gateVars);// *previous = NULL;
	while(*current){
		if((*current)->variable->alias == 0){ // gate irrelevant
			*current = (*current)->next;
			/*if(previous){
				previous->next = previous->next->next;
			}*/
			//varList *aux = *current;
			//current = &((*current)->next);
			//freeVar(aux->variable);
			//free(aux);
		}else{
			g = (*current)->variable->gateDefinition;
			//if(g->type != TRUE && g->type != FALSE && g->type != EXISTS && g->type != FORALL){
				prenexGateSimple(g);
			//}
			current = &((*current)->next);
		}
	}
	data->prefix = data->outputVar->gateDefinition->localPrefix; // Updating global prefix
	data->freeVars = data->outputVar->gateDefinition->localFreeVars; // Updating global free var list, needed for indexing
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
void indexVarQB(qBlock *qList){
	qBlock *currentQ = qList; //*emptyQB = NULL;
	//unsigned long aux;
	
	while(currentQ != NULL){

		//aux = data->tseitinVariableCount;
		indexVarVL(currentQ->variables);
		/*
		following block has a bug and it is also no longer needed
		
		intended usage: if no relevant variable is encountered in a qBlock,
		it is freed and its predecessor and successor are merged
		
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
		}*/
		
		currentQ = currentQ->next;
		if(currentQ == qList){ // reached head again
			break;
		}
	}
	/*if(emptyQB){ // list ends with an irrelevant block
		qBlock *tail = data->prefix->prev;
		tail->next = emptyQB;
		emptyQB->prev->next = data->prefix;
		data->prefix->prev = emptyQB->prev;
		emptyQB->prev = tail;
		free(emptyQB);
	}*/
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
		//free(v->gateDefinition->) necessary?
		free(v->gateDefinition);
	}
	free(v);
}
void freeVL(varList *current){
	varList *aux;
	while(current){
		aux = current;
		current = current->next;
		free(aux);
	}
}
void deepFreeVL(varList *current){
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
void deepFreeQB(qBlock *current){
	qBlock *aux;
	if(current){
		if(current->prev){
			current->prev->next = NULL; // break the cycle between first and last
		}
	}
	while(current){
		if(current->variables){
			deepFreeVL(current->variables);
		}else{
			//die("Empty qBlock element encountered!");
		}
		aux = current;
		current = current->next;
		free(aux);
	}
}
void freeVS(varSets *data){
	deepFreeVL(data->freeVars);
	deepFreeQB(data->prefix);
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
void printLL(litList *current){
	while(current){
		printf("%s ", current->variable->name);
		current = current->next;
	}
	printf("\n");
}
void printFree(){
	if(data->freeVars == NULL){
		return;
	}
	printf("Free:\n");
	printVL(data->freeVars);
	printf("\n");
}

void printQB(qBlock *head){
	qBlock *current;
	if(head == NULL){
		printf("empty\n");
		return;
	}
	current = head;
	do{
		printf("%d: ", current->type);
		printVL(current->variables);
		printf("\n");
		current = current->next;
	}while(current && current != head);
}
// print contents of quantifier prefix to stdout
void printPrefix(){
	printf("Prefix:\n");
	printQB(data->prefix);
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

void printTree(var *root){
	gate* currGate;
	litList *currLit;
	if(root == NULL || root->gateDefinition == NULL){
		return;
	}
	currGate = root->gateDefinition;
	currLit = currGate->literals;
	printf("%s = %d, length:%d\n", root->name, currGate->type, currGate->subtreeSize);
	printQB(currGate->localPrefix);
	printf("(");
	while(currLit){
		printf("%d-%s ", currLit->sign, currLit->variable->name);
		currLit = currLit->next;
	}
	printf(")\n");
	currLit = currGate->literals;
	while(currLit){
		printTree(currLit->variable);
		currLit = currLit->next;
	}
}

void printData(){
	printFree();
	printPrefix();
	printf("Output sign:%d name:%s\n", data->outputSign, data->outputName);
	printGates();
	printf("Line counter: %lu\n", lineCount);
}