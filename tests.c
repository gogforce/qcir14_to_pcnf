#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.c"
#include "data_types.h"

#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) do { char *message = test(); tests_run++; if (message) return message; } while (0)

int tests_run = 0;

char * compareTrees(var *t1, var *t2){
	if(strcmp(t1->name, t2->name)){
		return 0; // assume both are the same bool variable
	}
	// assume they are gates
	printTree(t1);
	printTree(t2);
	return 0;
}

static char * test_copy_tree(){
	var *v1 = createVar("v1");
	var *v2 = createVar("v2");
	var *v3 = createVar("v3");
	var *v4 = createVar("v4");
	var *v5 = createVar("v5");
	var *g1 = createVar("g1");
	var *g2 = createVar("g2");
	var *g3 = createVar("g3");
	var *g4 = createVar("g4");
	var *g5 = createVar("g5");
	
	gate *g = defineGateVar(g1, AND);
	addVarLLHead(&(g->literals), 0, g2);
	addVarLLHead(&(g->literals), 1, v1);
	addVarLLHead(&(g->literals), 0, g3);
	g->subtreeSize = 3;
	
	g = defineGateVar(g2, ITE);
	addVarLLHead(&(g->literals), 1, v2);
	addVarLLHead(&(g->literals), 1, v3);
	addVarLLHead(&(g->literals), 0, v5);
	g->subtreeSize = 1;
	
	g = defineGateVar(g3, EXISTS);
	addVarLLHead(&(g->literals), 1, g4);
	g->subtreeSize = 2;
	qBlock *qB = malloc(sizeof(qBlock));
	qB->type = EXISTS;
	qB->next = qB;
	qB->prev = qB;
	addVarVL(&(qB->variables), v1);
	addVarVL(&(qB->variables), v4);
	addVarVL(&(qB->variables), v5);
	g->localPrefix = qB;
	
	g = defineGateVar(g4, OR);
	addVarLLHead(&(g->literals), 1, g5);
	addVarLLHead(&(g->literals), 0, v1);
	g->subtreeSize = 1;
	
	g = defineGateVar(g5, AND);
	g->subtreeSize = 0;
	
	var *g1_copy = copyTree(g1);
	printTree(g1);
	printTree(g1_copy);
	return 0;
}

void huh(qBlock *qB){
	printf("foo");
}

unsigned int LL_length(litList *current){
	unsigned int length = 0;
	while(current){
		++length;
		current = current->next;
	}
	return length;
}

static char * test_add_literal_to_gate(){
	initData();
	//char *msg;
	var *v1 = createVar("v1");
	var *v2 = createVar("v2");
	var *v3 = createVar("v3");
	//var *v4 = createVar("v4");
	var *v5 = createVar("v5");
	var *g1 = createVar("g1");
	var *g2 = createVar("g2");
	var *g3 = createVar("g3");
	var *g4 = createVar("g4");
	var *gTRUE = createVar("gTRUE");
	var *gFALSE = createVar("gFALSE");
	defineGateVar(gTRUE, TRUE);
	defineGateVar(gFALSE, FALSE);
	
	gate *g = defineGateVar(g2, ITE);
	addLitToGate(g, 1, v2, 1);
	addLitToGate(g, 1, v3, 1);
	addLitToGate(g, 0, v5, 1);
	mu_assert("Wrong subtree size g2", g->subtreeSize == 1);
	
	g = defineGateVar(g1, AND);
	addLitToGate(g, 1, g2, 0);
	addLitToGate(g, 1, v2, 0);
	mu_assert("Wrong subtree size g1", g->subtreeSize == 2);
	mu_assert("Wrong usage count of g2 after first use in g1", g2->gateDefinition->uses == 1);
	addLitToGate(g, 1, g2, 0);
	mu_assert("Wrong usage count of g2 after second use in g1", g2->gateDefinition->uses == 1);
	mu_assert("Wrong list of literals of g1 after adding g2 twice", (strcmp(g->literals->variable->name,"v2")==0 && LL_length(g->literals)==2));
	
	g = defineGateVar(g4, PASS);
	addLitToGate(g, 0, g2, 0);
	mu_assert("Wrong usage count of g2 after use in PASS gate g4", g2->gateDefinition->uses == 1);
	addLitToGate(g, 0, v1, 0);
	mu_assert("Wrong amount of literals in g4 after trying to add another literal", LL_length(g->literals) == 1);
	
	g = defineGateVar(g3, AND);
	addLitToGate(g, 0, v3, 0);
	addLitToGate(g, 1, g4, 0);
	mu_assert("Wrong subtree size g3", g->subtreeSize == 2);
	mu_assert("Wrong literal sign in g3 after adding PASS to g2", g->literals->sign == 0);
	mu_assert("Wrong literal name in g3 after adding PASS to g2", (strcmp(g->literals->variable->name,"g2")==0));
	mu_assert("Reference points to the same var g2", g->literals->variable != g2);
	
	g = defineGateVar(g1, EXISTS);
	addLitToGate(g, 1, gTRUE, 0);
	mu_assert("EXISTS not converted to TRUE after receiving a TRUE constant", g->type == TRUE);
	g = defineGateVar(g1, FORALL);
	addLitToGate(g, 0, gFALSE, 0);
	mu_assert("FORALL not converted to TRUE after receiving a -FALSE constant", g->type == TRUE);
	g = defineGateVar(g1, EXISTS);
	addLitToGate(g, 1, gFALSE, 0);
	mu_assert("EXISTS not converted to FALSE after receiving a FALSE constant", g->type == FALSE);
	g = defineGateVar(g1, FORALL);
	addLitToGate(g, 0, gTRUE, 0);
	mu_assert("FORALL not converted to FALSE after receiving a TRUE constant", g->type == FALSE);
	
	g = defineGateVar(g1, AND);
	addLitToGate(g, 1, v1, 0);
	addLitToGate(g, 1, v2, 0);
	addLitToGate(g, 0, v1, 0);
	mu_assert("AND not converted to FALSE after x and -x", g->type == FALSE);
	g = defineGateVar(g1, AND);
	addLitToGate(g, 1, v1, 0);
	addLitToGate(g, 1, gFALSE, 0);
	mu_assert("AND not converted to FALSE after receiving a FALSE constant", g->type == FALSE);
	g = defineGateVar(g1, OR);
	addLitToGate(g, 1, v1, 0);
	addLitToGate(g, 1, v2, 0);
	addLitToGate(g, 0, v1, 0);
	mu_assert("OR not converted to TRUE after x and -x", g->type == TRUE);
	g = defineGateVar(g1, OR);
	addLitToGate(g, 1, v1, 0);
	addLitToGate(g, 1, gTRUE, 0);
	mu_assert("OR not converted to TRUE after receiving a TRUE constant", g->type == TRUE);
	
	g = defineGateVar(g1, XOR);
	addLitToGate(g, 1, v1, 0);
	addLitToGate(g, 1, v1, 0);
	mu_assert("XOR not converted to FALSE after x and x", g->type == FALSE);
	g = defineGateVar(g1, XOR);
	addLitToGate(g, 1, v1, 0);
	addLitToGate(g, 0, v1, 0);
	mu_assert("XOR not converted to TRUE after x and -x", g->type == TRUE);
	g = defineGateVar(g1, XOR);
	addLitToGate(g, 1, gTRUE, 0);
	addLitToGate(g, 0, v1, 0);
	mu_assert("XOR not converted to PASS(-x) after TRUE and x", ((g->type == PASS)&&(g->literals->sign==1)));
	g = defineGateVar(g1, XOR);
	addLitToGate(g, 1, gTRUE, 0);
	addLitToGate(g, 0, gFALSE, 0);
	mu_assert("XOR not converted to PASS(-TRUE) after TRUE and -FALSE", ((g->type == PASS)&&(g->literals->sign==0)&&(g->literals->variable->gateDefinition->type==TRUE)));
	g = defineGateVar(g1, XOR);
	addLitToGate(g, 0, v1, 0);
	addLitToGate(g, 1, gTRUE, 0);
	mu_assert("XOR not converted to PASS(-x) after x and TRUE", ((g->type == PASS)&&(g->literals->sign==1)));
	g = defineGateVar(g1, XOR);
	addLitToGate(g, 1, gFALSE, 0);
	mu_assert("XOR not converted to PASS() after FALSE", ((g->type == PASS)&&(g->literals==NULL)));
	g = defineGateVar(g1, XOR);
	addLitToGate(g, 0, v1, 0);
	addLitToGate(g, 1, gFALSE, 0);
	mu_assert("XOR not converted to PASS(x) after x and FALSE", ((g->type == PASS)&&(g->literals->sign==0)&&(g->literals->variable==v1)));
	
	g = defineGateVar(g1, ITE);
	addLitToGate(g, 1, gFALSE, 0);
	addLitToGate(g, 0, v1, 0);
	mu_assert("ITE not converted to PASS() after FALSE and x", ((g->type == PASS)&&(g->literals==NULL)));
	g = defineGateVar(g1, ITE);
	addLitToGate(g, 1, v1, 0);
	addLitToGate(g, 1, v1, 0);
	mu_assert("ITE not converted to OR(x) after x,x", ((g->type == OR)&&(LL_length(g->literals)==1)));
	g = defineGateVar(g1, ITE);
	addLitToGate(g, 1, v1, 0);
	addLitToGate(g, 0, v1, 0);
	mu_assert("ITE not converted to AND(-x) after x,-x", ((g->type == AND)&&(LL_length(g->literals)==1)&&(g->literals->sign==0)));
	g = defineGateVar(g1, ITE);
	addLitToGate(g, 1, v1, 0);
	addLitToGate(g, 1, v2, 0);
	addLitToGate(g, 1, v1, 0);
	mu_assert("ITE not converted to AND(x,y) after x,y,x", ((g->type == AND)&&(LL_length(g->literals)==2)));
	g = defineGateVar(g1, ITE);
	addLitToGate(g, 1, v1, 0);
	addLitToGate(g, 1, v2, 0);
	addLitToGate(g, 0, v1, 0);
	mu_assert("ITE not converted to OR(-x,y) after x,y,-x", ((g->type == OR)&&(LL_length(g->literals)==2)&&(g->literals->sign==0)));
	g = defineGateVar(g1, ITE);
	addLitToGate(g, 1, v1, 0);
	addLitToGate(g, 1, v2, 0);
	addLitToGate(g, 1, v2, 0);
	mu_assert("ITE not converted to PASS(y) after x,y,y", ((g->type == PASS)&&(LL_length(g->literals)==1)&&(g->literals->variable==v2)));
	g = defineGateVar(g1, ITE);
	addLitToGate(g, 1, v1, 0);
	addLitToGate(g, 0, v2, 0);
	addLitToGate(g, 1, v2, 0);
	mu_assert("ITE not converted to XOR(x,-y) after x,y,-y", ((g->type == XOR)&&(LL_length(g->literals)==2)&&(g->literals->variable==v1)&&(g->literals->next->variable==v2)&&(g->literals->next->sign==1)));
	g = defineGateVar(g1, ITE);
	addLitToGate(g, 1, gTRUE, 0);
	mu_assert("ITE not converted to PASS() after TRUE", ((g->type == PASS)&&(LL_length(g->literals)==0)));
	g = defineGateVar(g1, ITE);
	addLitToGate(g, 1, v1, 0);
	addLitToGate(g, 1, gTRUE, 0);
	mu_assert("ITE not converted to OR(x) after x,TRUE", ((g->type == OR)&&(LL_length(g->literals)==1)&&(g->literals->sign==1)));
	g = defineGateVar(g1, ITE);
	addLitToGate(g, 1, v1, 0);
	addLitToGate(g, 1, v2, 0);
	addLitToGate(g, 1, gTRUE, 0);
	mu_assert("ITE not converted to OR(-x,y) after x,y,TRUE", ((g->type == OR)&&(LL_length(g->literals)==2)&&(g->literals->sign==0)));
	g = defineGateVar(g1, ITE);
	addLitToGate(g, 1, v1, 0);
	addLitToGate(g, 1, gFALSE, 0);
	mu_assert("ITE not converted to AND(-x) after x,FALSE", ((g->type == AND)&&(LL_length(g->literals)==1)&&(g->literals->sign==0)));
	g = defineGateVar(g1, ITE);
	addLitToGate(g, 1, v1, 0);
	addLitToGate(g, 1, v2, 0);
	addLitToGate(g, 1, gFALSE, 0);
	mu_assert("ITE not converted to AND(x,y) after x,y,FALSE", ((g->type == AND)&&(LL_length(g->literals)==2)&&(g->literals->sign==1)));
	
	//printf("%d, %d\n", g->type, g->literals->sign);
	//printf("%s\n", g->literals->variable->name);
	//printTree(g->literals->variable);
	freeResources();
	return 0;
}

static char * test_replace_var(){
	var *v1 = createVar("v1");
	var *v2 = createVar("v2");
	//var *v3 = createVar("v3");
	var *v4 = createVar("v4");
	var *v5 = createVar("v5");
	var *g1 = createVar("g1");
	var *g2 = createVar("g2");
	var *g3 = createVar("g3");
	var *g4 = createVar("g4");
	var *g5 = createVar("g5");
	
	gate *g = defineGateVar(g1, AND);
	addVarLLHead(&(g->literals), 0, g2);
	addVarLLHead(&(g->literals), 1, v1);
	addVarLLHead(&(g->literals), 0, g3);
	g->subtreeSize = 3;
	
	g = defineGateVar(g2, ITE);
	addVarLLHead(&(g->literals), 1, v2);
	addVarLLHead(&(g->literals), 1, v1);
	addVarLLHead(&(g->literals), 0, v5);
	g->subtreeSize = 1;
	
	g = defineGateVar(g3, EXISTS);
	addVarLLHead(&(g->literals), 1, g4);
	g->subtreeSize = 2;
	qBlock *qB = malloc(sizeof(qBlock));
	qB->type = EXISTS;
	qB->next = qB;
	qB->prev = qB;
	addVarVL(&(qB->variables), v1);
	addVarVL(&(qB->variables), v4);
	addVarVL(&(qB->variables), v5);
	g->localPrefix = qB;
	
	g = defineGateVar(g4, OR);
	addVarLLHead(&(g->literals), 1, g5);
	addVarLLHead(&(g->literals), 0, v1);
	g->subtreeSize = 1;
	
	g = defineGateVar(g5, TRUE);
	g->subtreeSize = 0;
	
	varList *vl = malloc(sizeof(varList));
	vl->variable = v1;
	vl->next = NULL;
	replaceVar(g1->gateDefinition,vl,NULL);
	printTree(g1);
	return 0;
}

static char * test_parse_EXISTS_FORALL_QCIR(){
	initData();
	char* input = "#QCIR-14\noutput(g1)\n"	
	"g1=AND(f1,f2,f3)\n"
	"e1=exists(f1,f2;g1)\n" // e1 is regular exists gate
	"e2=exists(f1,f3;f1)\n" // e2 becomes TRUE constant
	"e3=exists(f2,f3;f1)\n" // e3 becomes a PASS gate to f1
	"a11=forall(f1,f2;g1)\n"
	"e4=exists(f1,n1,f2;n1)\n" // e4 becomes TRUE constant
	"e5=exists(f1,f2; n1)\n" // e5 becomes a PASS to new free var n1
	
	"a1=forall(f1,f2;g1)\n" // a1 is regular forall gate
	"a2=forall(f1,f3;f1)\n" // a2 becomes FALSE constant
	"a3=forall(f2,f3;f1)\n" // a3 becomes a PASS gate to f1
	"a4=forall(f1,n2,f2;n2)\n" // a4 becomes FALSE constant
	"a5=forall(f1,f2; n2)"; // a5 becomes a PASS to new free var n1
	
	parseQCIR(input, strlen(input));
	printFree();
	printGates();
	freeResources();
	return 0;
}

static char * test_QB_iterator(){
	initData();
	char* input = "#QCIR-14\n"
	"exists(e1, e2)\n"
	"forall(a1,a2,a3)\n"
	"exists(e3)\n"
	"forall(a4)\n"
	"output(g1)\n"
	"g1=AND(e1,a2,e3)\n";
	
	parseQCIR(input, strlen(input));
	iteratorQB *it = newQBiterator(data->prefix);
	var *v = nextVarQB(it);
	while(v){
		printf("%s, ", v->name);
		v = nextVarQB(it);
	}
	freeResources();
	return 0;
}

static char * test_remove_var_QB(){
	var *v1 = createVar("v1");
	var *v2 = createVar("v2");
	var *v3 = createVar("v3");
	var *v4 = createVar("v4");
	var *v5 = createVar("v5");
	qBlock *qb = NULL;
	
	addVarQB(&qb, FORALL, v1);
	addVarQB(&qb, EXISTS, v2);
	addVarQB(&qb, EXISTS, v3);
	addVarQB(&qb, FORALL, v4);
	addVarQB(&qb, EXISTS, v5);
	
	removeVarQB(&qb, v3);
	removeVarQB(&qb, v2);
	mu_assert("Error when middle block removed!", (qb->variables->variable == v1) && (qb->variables->next->variable == v4) && (qb->next->variables->variable == v5) && (qb->next->next == qb));
	
	removeVarQB(&qb, v5);
	mu_assert("Error when 2 blocks and tail removed!", (qb->variables->variable == v1) && (qb->next == qb));
	
	addVarQB(&qb, EXISTS, v5);
	removeVarQB(&qb, v1);
	removeVarQB(&qb, v4);
	mu_assert("Error when 2 blocks and head removed!", (qb->variables->variable == v5) && (qb->next == qb));
	
	removeVarQB(&qb, v5);
	mu_assert("QB not empty!", qb == NULL);
	
	return 0;
}

void printString(char *str){
	int i = 0;
	while(str[i] != 0){
		printf("%d\n", str[i]);
		++i;
	}
}
void printFile(char *filename){
	FILE *fpIn = fopen(filename, "r");
	if(fpIn == NULL){
		perror("Could not open input file. (printFile)");
		exit(EXIT_FAILURE);
	}
	
	char *input;
	long fsize;
	
	// load file at once to minimize hard disk operations
	fseek(fpIn, 0, SEEK_END);
	fsize = ftell(fpIn);
	rewind(fpIn);

	input = malloc(fsize + 1);
	if(input == NULL){
		perror("Could not allocate memory according to file size. (printFile)");
		exit(EXIT_FAILURE);
	}
	if(fread(input, fsize, 1, fpIn) < 1){
		if(fread(input, fsize, 1, fpIn) < 1){
			perror("Reading file failed or file empty. (printFile)");
			exit(EXIT_FAILURE);
		}
	}
	
	fclose(fpIn);
	input[fsize] = 0;
	
	printString(input);
}

static char * test_file() {
	printFile("test_output.txt");
	printf("\n");
	printString("p cnf 1 1\n1 0");
	return 0;
}
static char * test_total() {
	//freopen("test_output.txt", "w", stdout);
	char* input;
	/*input = "#QCIR-14\n"
	"free(f)\n"
	"exists(e1, e2)\n"
	"forall(a1,a2,a3)\n"
	"output(f)\n";
	
	translate(input, strlen(input));
	//fflush(stdout);
	mu_assert("Unexpected output - free var as root", checkFileContent("test_output.txt",
	//"c formula converted from QCIR-14\n"
	//"c Thank you for your mercy and grace, Jesus!\n"
	"p cnf 1 1\n"
	"1 0\n"));
	
	input = "#QCIR-14\n"
	"exists(e1, e2)\n"
	"forall(a1,a2,a3)\n"
	"output(-g1)\n"
	"g1=OR(-f,f)";
	translate(input, strlen(input));
	mu_assert("Unexpected output - -TRUE as root", checkFileContent("test_output.txt",
	"p cnf 1 1\n"
	"a 1 0\n"
	"1 0\n"));
	
	input = "#QCIR-14\n"
	"exists(e1, e2)\n"
	"forall(a1,a2,a3)\n"
	"output(-g1)\n"
	"g2=and(f,f)"
	"g1=exists(x,y,z; g2)";
	translate(input, strlen(input));
	mu_assert("Unexpected output - PASS as root", checkFileContent("test_output.txt",
	"p cnf 1 1\n"
	"-1 0\n"));*/
	
	input = "#QCIR-14\n"
	"exists(e1, e2)\n"
	"forall(a1,a2,f4,a3)\n"
	"output(g1)\n"
	"g15=or(f6,f2,f7)\n"
	"g14=exists(f6;g15)\n"
	"g13=forall(f7;g14)\n"
	"g12=xor(f5,f2)\n"
	"g11=and(f4,f1,f5)\n"
	"g10=exists(f1;g11)\n"
	"g9=forall(f4;g10)\n"
	"g8=ite(g9,g12,g13)\n"
	"g7=forall(x,y;f4)"
	"g6=or(f1,f2,f3)\n"
	"g5=forall(f1;g6)\n"
	"g4=forall(f2;g5)\n"
	"g3=and(g4,g7,g8)\n"
	"g2=forall(f1,f5; g3)\n"
	"g1=exists(x,y; g2)";
	translate(input, strlen(input));
	mu_assert("Unexpected output - regular", checkFileContent("test_output.txt",
	"p cnf 1 1\n"
	"-1 0\n"));
	
	//freopen("/dev/tty", "w", stdout); /*for gcc, ubuntu*/
	return 0;
}

static char * all_tests() {
	//mu_run_test(test_add_literal_to_gate);
	//mu_run_test(test_replace_var);
	//mu_run_test(test_QB_iterator);
	//mu_run_test(test_remove_var_QB);
	mu_run_test(test_total);
	return 0;
}

int main(int argc, char **argv) {
	char *result = all_tests();
	if (result != 0) {
		printf("%s\n", result);
	}
	else {
		printf("ALL TESTS PASSED\n");
	}
	printf("Tests run: %d\n", tests_run);

	return result != 0;
	//test_copy_tree();
	//test_copy_QB();
}