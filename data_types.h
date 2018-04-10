#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#define EXISTS 1
#define FORALL 2
#define AND 3
#define OR 4
#define XOR 5
#define ITE 6
#define TRUE 7
#define FALSE 8
#define PASS 9 // type of a gate that has been simplified to a link to another variable


// --- TYPEDEFS --- //

struct gate;

typedef struct{
    char* name; // variable name as found in input
	unsigned long alias; // 0 untill checked for inclusion in formula, set to ULONG_MAX afterwards, than replaced with new alias
	struct gate* gateDefinition; // points to gate definition if this is a gate variable, otherwise NULL
}var;

typedef struct varList{
	var* variable;
	struct varList* next;
}varList;

typedef struct litList{
	var* variable;
	struct litList* next;
	char sign; // 0 means negative, positive otherwise
}litList;

typedef struct qBlock{
	char type; // 1:=exists;2:=forall
	varList* variables;
	struct qBlock* next;
	struct qBlock* prev;
}qBlock;

typedef struct gate{
	var* variable; // pointer to name definition
	char type; // 1:=exists;2:=forall;3:=and;4:=or;5:=xor;6:=ite;7:=true;8:=false;
	litList* literals;
	unsigned int subtreeSize; // longest path to a leaf, 0 means a constant
	qBlock* localPrefix; // initialized only if it is a quantifier gate, later referencing the dynamic prefix
	varList* localFreeVars; // list of free variables in the subtree
	varList* localScope; // list of variables for which this subtree is exactly their scope
	unsigned int uses; // initially 0, incremented every time the gate is used inside the definition of another gate
}gate;

/*typedef struct varListList{
	varList *list;
	struct varListList *next;
}varListList; */

typedef struct {
	 varList* freeVars; // list of free variables
	 qBlock* prefix; // quantified prefix
	 varList* gateVars; // List of gate variables //(First List with gates referencing only non-gates, second List with gates containing other gates only from the first list, etc.)
	 varList *lastGateVar; // poiter to end of gateVars list
	 char outputSign; // sign of root gate literal
	 char* outputName; // name of root gate literal
	 var *outputVar; // pointer to actual variable
	 unsigned long tseitinClauseCount; // amount of clauses to be printed
	 unsigned long tseitinVariableCount; // free + quantified + gate 
}varSets;


// --- GLOBAL VARS, EXTERNALLY INITIALIZED --- //

varSets *data;
unsigned long lineCount;
char* word;

// --- REQUIRED FUNCTIONS --- //

void die(const char *fmt, ...);

// --- PROVIDED FUNCTIONALITY --- //

// Search in a list for a variable that has given name, return pointer to it, NULL if no match found.
var *getVarVL(varList *list, char *name);
//var *getVarVLL(varListList *llist, char *name);
var *getVarLL(litList *list, char *name);
var *getVarQB(qBlock *qList, char *name);
var *getVarVS(varSets *set, char *name);

// interrupts program if contents of word correspondent to an already existing var name in data
void checkUniqueness(char *word);

// create a new var with given name
var *createVar(char *name);

// create and add a new variable with given name to beginning of the list,
// updates argument pointer and returns pointer to new head
varList *addNewVarVL(varList **list, char *name);
// additionally checks and terminates if a var with such name already exists in data
varList *addUniqueVarVL(varList **list, char *name);
// add an already existing variable to to beginning of the list,
// updates argument pointer and returns pointer to new head
varList *addVarVL(varList **list, var *toAdd);
// add a var as a new last element, update currentTail pointer
// 'currentTail' is assumed to be an end of an existing list (not NULL)
varList *addVarVLTail(varList **currentTail, var *toAdd);

// add a litList to beginning of the list,
// updates argument pointer and returns pointer to new head
//litListList *addListLLLHead(litListList **list, litList *toAdd);
// add a litList as a new last element, update currentTail pointer
// 'currentTail' is assumed to be an end of an existing list (not NULL)
//litListList *addListLLLTail(litListList **currentTail, litList *toAdd);

// add a new literal to beginning of the list,
// updates argument pointer and returns pointer to new head
litList *addVarLLHead(litList **list, char sign, var *variable);
// add a new literal to end of the list,
// returns new length of the list
litList *addVarLLTail(litList **list, char sign, var *variable);
//litList *addUniqueVarLL(litList **list, char *name); // additionally checks and terminates if a var with such name already exists in data

// create and add a new unique free variable to data set
var *addUniqueFreeVar();

// upgrade a var with a gate definition, returns a pointer to newly created gate definition
// if var already has a gate definition, it is freed and replaced with a new one
gate *defineGateVar(var *target, char type);

// add a new unique gate variable to data set;
void addUniqueGateVar(var *toAdd);
// add a new literal to a gate, assume a free variable if not already defined in data
// returns a pointer to and if a new var was created and added to free()
qBlock *copyQB(qBlock *toCopy);
var *copyTree(var *root);
void addLitToGate(gate *g, char sign, var *atom, char isNewVar);
var *addLitToGateAssumeFree(gate *g, char sign);
// create and add a new variable with given name and quantification to prefix,
// added to the existing innermost block if it has the same quantification, creating a new block otherwise
void addVarQB(qBlock **head, char quantifier);
// additionally checks and terminates if a var with such name already exists in data
void addUniqueVarQB(char quantifier);
// append list of gate variables to innermost quantifier block, existentially quantified
void quantifyGateVars();

// go through all the relevant variables and give them a new integer alias
void indexVars();

// free data structure
void freeVar(var *);
void freeVL(varList *);
void freeLL(litList *);
void freeQB(qBlock *);
void freeVS(varSets *);

// --- TEST FUNCTIONALITY --- //

//print contents of quantifier prefix to stdout
void printFree();
void printPrefix();
void printGates();
void printData();
void printTree(var *root);
#endif