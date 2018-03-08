#include <stdio.h>	
#include <stdlib.h>	//malloc(1)
#include <string.h>	//memset(3) memcpy(3)
#include <sys/types.h>	//u_int8_t

typedef struct array_node{
	u_int8_t data[6];
	struct array_node *next;
}node_t;

int main(int c, const char *args[]){

	int i;
	node_t start;
	node_t *node = &start;
	u_int8_t data[6];
	for(i = 1;i < c;i++){
		memset(data, 0, sizeof(data));
		int d = sscanf(args[i], "%x:%x:%x:%x:%x:%x",
			&data[0],
			&data[1],
			&data[2],
			&data[3],
			&data[4],
			&data[5]
		);
		if(d == EOF)continue;
		
		node->next = malloc(sizeof(node_t));
		memcpy(&node->next->data, &data, sizeof(data));
		node = node->next;
	}

	node = &start;
	while(node->next != NULL){
		printf("%p\t", &node->next);
		for(i = 0;i < 6;i++){
			if(i != 0)printf(":");
			printf("%02X", node->next->data[i]);
		}
		printf("\n");
		node = node->next;
	}

}
