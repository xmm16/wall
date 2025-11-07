#include <stdio.h>
#include <stdlib.h>

enum token_type;
char** symbols;
typedef struct token_struct token;

enum node_type;
typedef struct node_struct node;

int main(){
	char* symbols[] = {"=", "+=", "-="};

	enum token_type {
		WORD = 128 + sizeof(symbols)/sizeof(symbols[0]), // 128 because that's where ASCII ends
		INT,
		FLOAT
	};

	enum node_type {
		PROGRAM = 128 + sizeof(symbols)/sizeof(symbols[0]),
	};

	struct token_struct {
		enum token_type type;
		char* string_argument;
		token* token_argument;
	};

}
