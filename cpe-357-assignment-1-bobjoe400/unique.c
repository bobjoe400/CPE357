#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void unique(FILE *file){
	char *prev = NULL;
	char *curr = NULL;
	size_t len = 0;
	ssize_t size;

	while((size = getline(&curr, &len, file)) != -1){
		//Null check
		if(!prev){
			printf("%s",curr);
		}else{
			//cleaning up strings to handle EOF edgecase
			prev[strcspn(prev, "\n")] = '\0';
			curr[strcspn(curr, "\n")] = '\0';
			if(strcmp(prev,curr) != 0){
				printf("%s\n",curr);
			}
		}
		//free prev now that we have checked
		free(prev);
		prev = curr;
		len = 0;
	}
	//final free of prev and we can now free curr
	free(prev);
	free(curr);
}

int main(int argc, char *argv[]){
	FILE *ptr_f;
	//check if we have argument or if file even exists
	if(argc !=2){
		ptr_f = stdin;
	}else if(!(ptr_f = fopen(argv[1], "r"))){
		fprintf(stderr,"File cannot be opened");
		exit(1);
	}
	unique(ptr_f);	
	fclose(ptr_f);
	return 0;
}