#include <stdio.h>
#include <stdlib.h>
#include <data_types.h>

int main(int argc, char **argv) {
	char a = 0;
	char b = 0;
	
	multiplySign(&a, 1);
	printf("%d",a);
	/*var *v1 = createVar("v1");
	var *v2 = createVar("v2");
	var *v3 = createVar("v3");
	var *v4 = createVar("v4");
	var *v5 = createVar("v5");
	varList *vl1 = NULL;
	varList *vl2 = NULL;
	
	addVarVL(&vl1, v3);
	addVarVL(&vl1, v2);
	addVarVL(&vl1, v1);
	addVarVL(&vl2, v5);
	addVarVL(&vl2, v3);
	addVarVL(&vl2, v4);
	addVarVL(&vl2, v1);
	
	varList **fvl = &(vl2);
	varList *f1;
	while(*fvl){
		f1 = vl1;
		while(f1){
			if((*fvl)->variable == f1->variable){
				*fvl = (*fvl)->next;
				break;
			}
			f1 = f1->next;
		}
		fvl = &((*fvl)->next);
	}
	*fvl = vl1; // append global list to end of local list and make it global
	vl1 = vl2;
	
	varList *current = vl1;
	while(current){
		printf("%s ", current->variable->name);
		current = current->next;
	}
	//addNewVarVL(&vl, "c");
	//addNewVarVL(&vl, "b");
	//addNewVarVL(&vl, "a");*/
}