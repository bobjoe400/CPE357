#include <stdio.h>
#include <ctype.h>

/*
Function: wc
Inputs: file to count words, pointers to global char, word, and line count
Outputs: None
Description: Character by character counts the number of chars, words, and lines in a file. 
*/

void wc(FILE *file, int* char_count, int* word_count, int* line_count){
	//initializing variables local to function
	int cc = 0;
	int wc = 0;
	int lc = 0;
	int curr_word = 0;
	int in;
	//computing word count
	while((in = fgetc(file))!= EOF){
		char curr = (char) in;
		cc = cc+1;
		if(curr == '\n'){
			lc = lc+1;
			curr_word = 0;
		}if(!isspace(curr)){
			if(curr_word == 0){
				wc = wc+1;
			}
			curr_word = curr_word+1;
		}else{
			curr_word = 0;
		}
	}
	//sending values from computation to those in main
	*char_count = cc;
	*word_count = wc;
	*line_count = lc;
}

int main(int argc, char *argv[]){
	FILE *ptr_f;
	if(argc !=2 ){
		ptr_f = stdin;
	}else if(!(ptr_f = fopen(argv[1], "r"))){
		fprintf(stderr,"File cannot be opened");
		exit(1);
	}
	int char_count, word_count, line_count;

	wc(ptr_f, &char_count, &word_count, &line_count);
	fclose(ptr_f);
	printf("The number of characters read is: %d\nThe number of words read is: %d\nThe number of lines is: %d\n",char_count, word_count, line_count);
	return 0;
}