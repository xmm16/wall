#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum token_type;
char** symbols;
typedef struct token_struct token;

enum node_type;
typedef struct node_struct node;

int main(int argc, char** argv){
	char* symbols[] = {"+=", "-="};

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

	struct node_struct {
		enum node_type type;
		token* token_argument;
		node* back;
		node* left;
		node* right;
	};

	token code_lex[strlen(argv[1])]; // "argv[1]" because I don't want to have to deal with file management until I need to
	for (int i = 0; i < strlen(argv[1]); i++){
		
	}
	
	node* code_tree;
	code_tree->type = PROGRAM;
	code_tree->back = NULL;
	code_tree->left = malloc(sizeof(node));
	code_tree->right = malloc(sizeof(node));
}
