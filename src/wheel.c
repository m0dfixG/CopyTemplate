#include <stdio.h>
#include <signal.h> //signal(2)
#include <unistd.h> //sleep(1);

int term = 1;
void finish(int i){term = 0;}

int main(int c, const char *args[]){
	
	signal(SIGTERM, finish);signal(SIGKILL, finish);signal(SIGINT, finish);
	while(term){
		sleep(1);
	}
	return 0;
}
