#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum token_type;
struct token_struct;
char* symbols[] = {"+=", "-="};

enum node_type;
struct node_struct;

enum token_type {
  WORD = 128 + sizeof(symbols)/sizeof(symbols[0]), // 128 because that's where ASCII ends
  INT,
  FLOAT
};

enum node_type {
  PROGRAM = 128 + sizeof(symbols)/sizeof(symbols[0]),
};

typedef struct token_struct {
  enum token_type type;
  char* string_argument;
  struct token_struct* token_argument;
} token;

typedef struct node_struct {
  enum node_type type;
  token* token_argument;
  struct node_struct* back;
  struct node_struct* left;
  struct node_struct* right;
} node;

void append_token(token** code_lex, size_t* code_lex_size, size_t* code_lex_index, enum token_type type, char* string_argument, token* token_argument){ 
  while ((*code_lex_size) < ((*code_lex_index) + 1) * sizeof(token)){
    (*code_lex_size) *= 2;
    *code_lex = realloc(*code_lex, (*code_lex_size));
  }

  (*code_lex)[(*code_lex_index)].type = type;
  (*code_lex)[(*code_lex_index)].string_argument = string_argument;
  (*code_lex)[(*code_lex_index)].token_argument = token_argument;

  (*code_lex_index)++;
}

int main(int argc, char** argv){
	size_t strlen_argv_1 = strlen(argv[1]); // "argv[1]" because I don't want to have to deal with file management until I need to
	char* raw_code = argv[1];

	size_t code_lex_size = 1;
	size_t code_lex_index = 0;
	token* code_lex = malloc(code_lex_size);

	int quote_mode = 0;
	size_t quote_buf_start;

  int comment_mode = 0;
  int multi_comment_mode = 0;

	for (int i = 0; i < strlen_argv_1; i++){
    if (raw_code[i] == '\\'){ // for escaping
      i++;
      continue;
    }

    if (comment_mode && raw_code[i] != '\n') continue;
    else if (comment_mode) comment_mode--;

    if (multi_comment_mode && strncmp(&raw_code[i], "*/", 2) != 0) continue;
    else if (multi_comment_mode) multi_comment_mode--;

		if (quote_mode && raw_code[i] == '"'){
			quote_mode--;

			int buf_size = i - quote_buf_start;
			char* quote_arg = malloc(1 + buf_size); // free every string argument in quotes
			strncpy(quote_arg, &raw_code[quote_buf_start], buf_size);
      quote_arg[buf_size] = '\0';
      append_token(&code_lex, &code_lex_size, &code_lex_index, '"', quote_arg, NULL);

			continue;
		} else if (quote_mode) continue;

		if (raw_code[i] == '"'){
			quote_mode++;
			quote_buf_start = i + 1;

			continue;
		}

    if (strncmp(&raw_code[i], "//", 2) == 0){
      comment_mode++;
      continue;
    }
    
    if (strncmp(&raw_code[i], "/*", 2) == 0){
      multi_comment_mode++;
      continue;
    }
	}
  

  printf("%s\n", code_lex[0].string_argument); // for debug purposes only

	node code_tree;
	code_tree.type = PROGRAM;
	code_tree.back = NULL;
	code_tree.left = malloc(sizeof(node));
	code_tree.right = malloc(sizeof(node));

	return 0;
}
