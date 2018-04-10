#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <data_types.h>

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
	char *msg;
	var *v1 = createVar("v1");
	var *v2 = createVar("v2");
	var *v3 = createVar("v3");
	var *v4 = createVar("v4");
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
	return 0;
}

static char * all_tests() {
	mu_run_test(test_add_literal_to_gate);
	return 0;
}

/*int main(int argc, char **argv) {
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
}*/