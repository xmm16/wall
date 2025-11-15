#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum token_type;
struct token_struct;
char *symbols[] = { // sort by len of symbol
    "bitcast", "sizeof", "cast", "<<=", ">>=", "..", "+=", "-=", "*=",
    "/=",      "&=",     "^=",   "|=",  "%=",  "||", "&&", "==", "!=",
    ">=",      "<=",     "<<",   ">>",  "++",  "--", "->"};
#define LOCAL_LEN(ARR) (sizeof(ARR) / sizeof(ARR[0]))

enum node_type;
struct node_struct;

enum token_type {
  WORD = 128 + LOCAL_LEN(symbols), // 128 because that's where ASCII ends
  INT,
  FLOAT
};

enum node_type {
  PROGRAM = 128 + LOCAL_LEN(symbols),
  FUNCTION_CALL,
  LITERAL,
  END
};

typedef struct token_struct {
  enum token_type type;
  char *string_argument;

  struct token_struct *token_argument;
  struct token_struct *colon_argument;
  struct token_struct *brace_argument;
  struct token_struct *brack_argument;

  size_t token_length;
  size_t colon_length;
  size_t brace_length;
  size_t brack_length;
} token;

typedef struct node_struct {
  enum node_type type;
  struct node_struct *back;
  struct node_struct *left;
  struct node_struct *right;
  struct token_struct *token_argument;
} node;

void append_token_c(token **code_lex, size_t *code_lex_size,
                    size_t *code_lex_index, enum token_type type,
                    char *string_argument, token *token_argument) {
  while ((*code_lex_size) < ((*code_lex_index) + 1) * sizeof(token)) {
    (*code_lex_size) *= 2;
    *code_lex = realloc(*code_lex, (*code_lex_size));
  }

  (*code_lex)[(*code_lex_index)].type = type;
  (*code_lex)[(*code_lex_index)].string_argument = string_argument;
  (*code_lex)[(*code_lex_index)].colon_argument = token_argument;
  (*code_lex)[(*code_lex_index)].colon_length = 0;
  (*code_lex_index)++;
}

void append_token_b(token **code_lex, size_t *code_lex_size,
                    size_t *code_lex_index, enum token_type type,
                    char *string_argument, token *token_argument) {
  while ((*code_lex_size) < ((*code_lex_index) + 1) * sizeof(token)) {
    (*code_lex_size) *= 2;
    *code_lex = realloc(*code_lex, (*code_lex_size));
  }

  (*code_lex)[(*code_lex_index)].type = type;
  (*code_lex)[(*code_lex_index)].string_argument = string_argument;
  (*code_lex)[(*code_lex_index)].brace_argument = token_argument;
  (*code_lex)[(*code_lex_index)].brace_length = 0;
  (*code_lex_index)++;
}

void append_token_bk(token **code_lex, size_t *code_lex_size,
                     size_t *code_lex_index, enum token_type type,
                     char *string_argument, token *token_argument) {
  while ((*code_lex_size) < ((*code_lex_index) + 1) * sizeof(token)) {
    (*code_lex_size) *= 2;
    *code_lex = realloc(*code_lex, (*code_lex_size));
  }

  (*code_lex)[(*code_lex_index)].type = type;
  (*code_lex)[(*code_lex_index)].string_argument = string_argument;
  (*code_lex)[(*code_lex_index)].brack_argument = token_argument;
  (*code_lex)[(*code_lex_index)].brack_length = 0;
  (*code_lex_index)++;
}

void append_token(token **code_lex, size_t *code_lex_size,
                  size_t *code_lex_index, enum token_type type,
                  char *string_argument, token *token_argument) {
  while ((*code_lex_size) < ((*code_lex_index) + 1) * sizeof(token)) {
    (*code_lex_size) *= 2;
    *code_lex = realloc(*code_lex, (*code_lex_size));
  }

  (*code_lex)[(*code_lex_index)].type = type;
  (*code_lex)[(*code_lex_index)].string_argument = string_argument;
  (*code_lex)[(*code_lex_index)].token_argument = token_argument;
  (*code_lex)[(*code_lex_index)].token_length = 0;

  if (type == WORD) {
    (*code_lex)[(*code_lex_index)].brace_length = 0;
    (*code_lex)[(*code_lex_index)].brack_length = 0;
    (*code_lex)[(*code_lex_index)].colon_length = 0;
  }

  (*code_lex_index)++;
}

int get_symbol(char *symbol) {
  for (int i = 0; i < LOCAL_LEN(symbols); i++) {
    if (strcmp(symbol, symbols[i]) == 0) {
      return 128 + i;
    }
  }
  return -1;
}

int number_or_dot(char symbol) {
  return (symbol >= '0' && symbol <= '9') || (symbol == '.');
}

int letter_number_or_underscore(char symbol) {
  return (symbol >= '0' && symbol <= '9') || (symbol >= 'A' && symbol <= 'Z') ||
         (symbol >= 'a' && symbol <= 'z') || (symbol == '_');
}

int letter_number_underscore_or_space(char symbol) {
  return (symbol >= '0' && symbol <= '9') || (symbol >= 'A' && symbol <= 'Z') ||
         (symbol >= 'a' && symbol <= 'z') || (symbol == '_') || (symbol == ' ');
}

token *lex(char *raw_code, size_t strlen_argv_1, size_t *code_lex_index_ptr) {
  size_t code_lex_size = 1;
  size_t code_lex_index = 0;
  token *code_lex = malloc(code_lex_size * sizeof(token *));

  int quote_mode = 0;
  size_t quote_buf_start;

  int paren_mode = 0;
  int brace_mode = 0;
  int colon_mode = 0;
  int brack_mode = 0;
  size_t paren_buf_start;
  size_t brace_buf_start;
  size_t colon_buf_start;
  size_t brack_buf_start;

  int comment_mode = 0;
  int multi_comment_mode = 0;

  for (int i = 0; i < strlen_argv_1; i++) {
    if (raw_code[i] == '\\') { // for escaping
      i++;
      continue;
    }

    if (comment_mode && raw_code[i] != '\n')
      continue;
    else if (comment_mode)
      comment_mode--;

    if (multi_comment_mode && strncmp(&raw_code[i], "*/", 2) != 0)
      continue;
    else if (multi_comment_mode)
      multi_comment_mode--;

    if (quote_mode && raw_code[i] == '"') {
      quote_mode--;

      int buf_size = i - quote_buf_start;
      char *quote_arg =
          malloc(1 + buf_size); // free every string argument in quotes
      strncpy(quote_arg, &raw_code[quote_buf_start], buf_size);
      quote_arg[buf_size] = '\0';
      append_token(&code_lex, &code_lex_size, &code_lex_index, '"', quote_arg,
                   NULL);

      continue;
    } else if (quote_mode)
      continue;

    if (raw_code[i] == '"') {
      quote_mode++;
      quote_buf_start = i + 1;

      continue;
    }

    if (paren_mode && raw_code[i] == ')') {
      paren_mode--;

      int buf_size = i - paren_buf_start;
      char *paren_arg =
          malloc(1 + buf_size); // free every string argument in parens
      strncpy(paren_arg, &raw_code[paren_buf_start], buf_size);
      paren_arg[buf_size] = '\0';

      size_t lexed_paren_index;
      token *lexed_paren =
          lex(paren_arg, strlen(paren_arg), &lexed_paren_index);
      if (code_lex_index > 0 && (code_lex[code_lex_index - 1].type == WORD ||
                                 code_lex[code_lex_index - 1].type == '(' ||
                                 code_lex[code_lex_index - 1].type == ':' ||
                                 code_lex[code_lex_index - 1].type == '[')) {
        code_lex_index--;
        append_token(&code_lex, &code_lex_size, &code_lex_index, '(',
                     code_lex[code_lex_index].string_argument, lexed_paren);
      } else
        append_token(&code_lex, &code_lex_size, &code_lex_index, '(', NULL,
                     lexed_paren);
      code_lex[code_lex_index - 1].token_length = lexed_paren_index;
      free(paren_arg);

      continue;
    } else if (paren_mode)
      continue;

    if (!paren_mode && raw_code[i] == '(') {
      paren_mode++;
      paren_buf_start = i + 1;

      continue;
    }

    if (brace_mode && raw_code[i] == '}') {
      brace_mode--;

      int buf_size = i - brace_buf_start;
      char *paren_arg =
          malloc(1 + buf_size); // free every string argument in parens
      strncpy(paren_arg, &raw_code[brace_buf_start], buf_size);
      paren_arg[buf_size] = '\0';

      size_t lexed_paren_index;
      token *lexed_paren =
          lex(paren_arg, strlen(paren_arg), &lexed_paren_index);
      if (code_lex_index > 0 && (code_lex[code_lex_index - 1].type == WORD ||
                                 code_lex[code_lex_index - 1].type == '(' ||
                                 code_lex[code_lex_index - 1].type == ':' ||
                                 code_lex[code_lex_index - 1].type == '[')) {
        code_lex_index--;
        append_token_b(&code_lex, &code_lex_size, &code_lex_index, '{',
                       code_lex[code_lex_index].string_argument, lexed_paren);
      } else
        append_token_b(&code_lex, &code_lex_size, &code_lex_index, '{', NULL,
                       lexed_paren);
      code_lex[code_lex_index - 1].brace_length = lexed_paren_index;
      free(paren_arg);

      continue;
    } else if (brace_mode)
      continue;

    if (!brace_mode && raw_code[i] == '{') {
      brace_mode++;
      brace_buf_start = i + 1;

      continue;
    }

    if (brack_mode && raw_code[i] == ']') {
      brack_mode--;

      int buf_size = i - brack_buf_start;
      char *paren_arg =
          malloc(1 + buf_size); // free every string argument in parens
      strncpy(paren_arg, &raw_code[brack_buf_start], buf_size);
      paren_arg[buf_size] = '\0';

      size_t lexed_paren_index;
      token *lexed_paren =
          lex(paren_arg, strlen(paren_arg), &lexed_paren_index);
      if (code_lex_index > 0 && (code_lex[code_lex_index - 1].type == WORD ||
                                 code_lex[code_lex_index - 1].type == '(' ||
                                 code_lex[code_lex_index - 1].type == ':' ||
                                 code_lex[code_lex_index - 1].type == '[')) {
        code_lex_index--;
        append_token_bk(&code_lex, &code_lex_size, &code_lex_index, '[',
                        code_lex[code_lex_index].string_argument, lexed_paren);
      } else
        append_token_bk(&code_lex, &code_lex_size, &code_lex_index, '[', NULL,
                        lexed_paren);
      code_lex[code_lex_index - 1].brack_length = lexed_paren_index;
      free(paren_arg);

      continue;
    } else if (brack_mode)
      continue;

    if (!brack_mode && raw_code[i] == '[') { // give everything different buffer
      brack_mode++;
      brack_buf_start = i + 1;

      continue;
    }

    if (colon_mode && !letter_number_underscore_or_space(raw_code[i])) {
      colon_mode--;

      int buf_size = i - colon_buf_start;
      char *paren_arg =
          malloc(1 + buf_size); // free every string argument in parens
      strncpy(paren_arg, &raw_code[colon_buf_start], buf_size);
      paren_arg[buf_size] = '\0';

      size_t lexed_paren_index;
      token *lexed_paren =
          lex(paren_arg, strlen(paren_arg), &lexed_paren_index);
      if (code_lex_index > 0 && (code_lex[code_lex_index - 1].type == WORD ||
                                 code_lex[code_lex_index - 1].type == '(' ||
                                 code_lex[code_lex_index - 1].type == ':' ||
                                 code_lex[code_lex_index - 1].type == '[')) {
        code_lex_index--;
        append_token_c(&code_lex, &code_lex_size, &code_lex_index, ':',
                       code_lex[code_lex_index].string_argument, lexed_paren);
      } else
        continue;

      code_lex[code_lex_index - 1].colon_length = lexed_paren_index;
      free(paren_arg);

      continue;
    } else if (colon_mode)
      continue;

    if (!colon_mode && raw_code[i] == ':') {
      colon_mode++;
      colon_buf_start = i + 1;

      continue;
    }

    if (strncmp(&raw_code[i], "//", 2) == 0) {
      comment_mode++;
      continue;
    }

    if (strncmp(&raw_code[i], "/*", 2) == 0) {
      multi_comment_mode++;
      continue;
    }

    enum token_type type = INT;
    size_t num_start = i;

    while (number_or_dot(raw_code[i])) {
      if (raw_code[i] == '.')
        type = FLOAT;
      i++;
    }

    if (i - num_start == 1 && type == FLOAT) {
      type = '.';
      num_start = i; // to avoid next conditional
    }

    if (num_start != i) { // if it's a number
      char *string_argument =
          malloc(i - num_start +
                 1); // free every string argument in ints and floats too
      strncpy(string_argument, &raw_code[num_start], i - num_start);
      string_argument[i - num_start] = '\0';

      append_token(&code_lex, &code_lex_size, &code_lex_index, type,
                   string_argument, NULL);
      i--;
      continue;
    }

    type = WORD;
    size_t word_start = i;

    while (letter_number_or_underscore(raw_code[i])) {
      i++;
    }

    if (word_start != i) { // if it's a word
      char *string_argument =
          malloc(i - word_start + 1); // free every string argument in words
      strncpy(string_argument, &raw_code[word_start], i - word_start);
      string_argument[i - word_start] = '\0';

      append_token(&code_lex, &code_lex_size, &code_lex_index, type,
                   string_argument, NULL);
      i--;
      continue;
    }

    if (raw_code[i] == ' ' || raw_code[i] == '\t')
      continue;

    int multi_char_symbol = 0;

    for (int j = 0; j < LOCAL_LEN(symbols); j++) {
      size_t strlen_symbols_j = strlen(symbols[j]);

      if (strncmp(&raw_code[i], symbols[j], strlen_symbols_j) == 0) {
        append_token(&code_lex, &code_lex_size, &code_lex_index, 128 + j, NULL,
                     NULL);
        i += strlen_symbols_j - 1;
        multi_char_symbol++;
        break;
      }
    }

    if (multi_char_symbol)
      continue;
    append_token(&code_lex, &code_lex_size, &code_lex_index, raw_code[i], NULL,
                 NULL);
  }

  (*code_lex_index_ptr) = code_lex_index;
  return code_lex;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
void tree(node *code_tree_ptr, token *code_lex, size_t code_lex_index) {
  if (code_lex_index == 0) {
    code_tree_ptr->type = END;
    return;
  }

  for (int i = 0; i < code_lex_index; i++) {
    int id = code_lex[i].type;

    switch (id) {
    case ',':;
      int restore_i = i;
      i--;
      while (i != -1 && code_lex[i].type != '|') {
        i--;
      }

      i++;
      code_tree_ptr->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->type = PROGRAM;
      code_tree_ptr->left->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->right = malloc(sizeof(node));
      code_tree_ptr->left->left->back = code_tree_ptr->left;
      code_tree_ptr->left->right = malloc(sizeof(node));
      code_tree_ptr->left->right->type = PROGRAM;
      code_tree_ptr->left->right->back = code_tree_ptr->left;
      code_tree_ptr->left->right->right = malloc(sizeof(node));
      code_tree_ptr->left->right->left = malloc(sizeof(node));

      int restore_i_minus_i = restore_i - i;
      token *left_token_argument = malloc(sizeof(token) * restore_i_minus_i);
      memcpy(left_token_argument, &code_lex[i],
             sizeof(token) * restore_i_minus_i);

      i = restore_i;
      i++;
      while (i != code_lex_index && code_lex[i].type != '|') {
        i++;
      }

      int i_minus_restore_i = i - restore_i - 1;
      token *right_token_argument = malloc(sizeof(token) * (i_minus_restore_i));
      memcpy(right_token_argument, &code_lex[restore_i + 1],
             sizeof(token) * (i_minus_restore_i));

      code_tree_ptr->left->type = (enum node_type)code_lex[restore_i].type;
      code_tree_ptr->left->back = code_tree_ptr;

      tree(code_tree_ptr->left->left, left_token_argument, restore_i_minus_i);
      tree(code_tree_ptr->left->right, right_token_argument, i_minus_restore_i);

      code_tree_ptr->right->back = code_tree_ptr;
      code_tree_ptr->right->left = malloc(sizeof(node));
      code_tree_ptr->right->right = malloc(sizeof(node));
      code_tree_ptr->right->type = PROGRAM;
      tree(code_tree_ptr->right, &code_lex[i], code_lex_index - i);
      return;
    default:
      break;
    }
  }

  for (int i = 0; i < code_lex_index; i++) {
    int id;

    if (code_lex[i].type == get_symbol("+="))
      id = '=';
    else if (code_lex[i].type == get_symbol("-="))
      id = '=';
    else if (code_lex[i].type == get_symbol("*="))
      id = '=';
    else if (code_lex[i].type == get_symbol("/="))
      id = '=';
    else if (code_lex[i].type == get_symbol("%="))
      id = '=';
    else if (code_lex[i].type == get_symbol("<<="))
      id = '=';
    else if (code_lex[i].type == get_symbol(">>="))
      id = '=';
    else if (code_lex[i].type == get_symbol("&="))
      id = '=';
    else if (code_lex[i].type == get_symbol("^="))
      id = '=';
    else if (code_lex[i].type == get_symbol("|="))
      id = '=';
    else
      id = code_lex[i].type;

    switch (id) {
    case '=': { // USES NEWLINES FOR ASSIGNMENTS
      int restore_i = i;
      while (i != -1 && code_lex[i].type != '\n') {
        i--;
      }

      i++;
      code_tree_ptr->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->type = PROGRAM;
      code_tree_ptr->left->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->right = malloc(sizeof(node));
      code_tree_ptr->left->left->back = code_tree_ptr->left;
      code_tree_ptr->left->right = malloc(sizeof(node));
      code_tree_ptr->left->right->type = PROGRAM;
      code_tree_ptr->left->right->back = code_tree_ptr->left;
      code_tree_ptr->left->right->right = malloc(sizeof(node));
      code_tree_ptr->left->right->left = malloc(sizeof(node));

      int restore_i_minus_i = restore_i - i;
      token *left_token_argument = malloc(sizeof(token) * restore_i_minus_i);
      memcpy(left_token_argument, &code_lex[i],
             sizeof(token) * restore_i_minus_i);

      i = restore_i;
      while (i != code_lex_index && code_lex[i].type != '\n') {
        i++;
      }

      int i_minus_restore_i = i - restore_i - 1;
      token *right_token_argument = malloc(sizeof(token) * (i_minus_restore_i));
      memcpy(right_token_argument, &code_lex[restore_i + 1],
             sizeof(token) * (i_minus_restore_i));

      code_tree_ptr->left->type = (enum node_type)code_lex[restore_i].type;
      code_tree_ptr->left->back = code_tree_ptr;

      tree(code_tree_ptr->left->left, left_token_argument, restore_i_minus_i);
      tree(code_tree_ptr->left->right, right_token_argument, i_minus_restore_i);

      code_tree_ptr->right->back = code_tree_ptr;
      code_tree_ptr->right->left = malloc(sizeof(node));
      code_tree_ptr->right->right = malloc(sizeof(node));
      code_tree_ptr->right->type = PROGRAM;
      tree(code_tree_ptr->right, &code_lex[i], code_lex_index - i);
      return;
    }
    default:
      break;
    }
  }

  for (int i = 0; i < code_lex_index; i++) {
    int id;
    if (code_lex[i].type ==
        get_symbol("||")) // redo this, just put the entire thing in the if
                          // instead of this process
      id = 'T';
    else
      id = code_lex[i].type;

    switch (id) {
    case 'T': { // PRIORITY THING
      int restore_i = i;
      i--;
      while (i != -1 && code_lex[i].type != get_symbol("||")) {
        i--;
      }

      i++;
      code_tree_ptr->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->type = PROGRAM;
      code_tree_ptr->left->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->right = malloc(sizeof(node));
      code_tree_ptr->left->left->back = code_tree_ptr->left;
      code_tree_ptr->left->right = malloc(sizeof(node));
      code_tree_ptr->left->right->type = PROGRAM;
      code_tree_ptr->left->right->back = code_tree_ptr->left;
      code_tree_ptr->left->right->right = malloc(sizeof(node));
      code_tree_ptr->left->right->left = malloc(sizeof(node));

      int restore_i_minus_i = restore_i - i;
      token *left_token_argument = malloc(sizeof(token) * restore_i_minus_i);
      memcpy(left_token_argument, &code_lex[i],
             sizeof(token) * restore_i_minus_i);

      i = restore_i;
      i++;
      while (i != code_lex_index && code_lex[i].type != get_symbol("||")) {
        i++;
      }

      int i_minus_restore_i = i - restore_i - 1;
      token *right_token_argument = malloc(sizeof(token) * (i_minus_restore_i));
      memcpy(right_token_argument, &code_lex[restore_i + 1],
             sizeof(token) * (i_minus_restore_i));

      code_tree_ptr->left->type = (enum node_type)code_lex[restore_i].type;
      code_tree_ptr->left->back = code_tree_ptr;

      tree(code_tree_ptr->left->left, left_token_argument, restore_i_minus_i);
      tree(code_tree_ptr->left->right, right_token_argument, i_minus_restore_i);

      code_tree_ptr->right->back = code_tree_ptr;
      code_tree_ptr->right->left = malloc(sizeof(node));
      code_tree_ptr->right->right = malloc(sizeof(node));
      code_tree_ptr->right->type = PROGRAM;
      tree(code_tree_ptr->right, &code_lex[i], code_lex_index - i);
      return;
    }
    default:
      break;
    }
  }

  for (int i = 0; i < code_lex_index; i++) {
    int id;
    if (code_lex[i].type ==
        get_symbol("&&")) // redo this, just put the entire thing in the if
                          // instead of this process
      id = 'T';
    else
      id = code_lex[i].type;

    switch (id) {
    case 'T':;
      int restore_i = i;
      i--;
      while (i != -1 && code_lex[i].type != get_symbol("&&")) {
        i--;
      }

      i++;
      code_tree_ptr->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->type = PROGRAM;
      code_tree_ptr->left->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->right = malloc(sizeof(node));
      code_tree_ptr->left->left->back = code_tree_ptr->left;
      code_tree_ptr->left->right = malloc(sizeof(node));
      code_tree_ptr->left->right->type = PROGRAM;
      code_tree_ptr->left->right->back = code_tree_ptr->left;
      code_tree_ptr->left->right->right = malloc(sizeof(node));
      code_tree_ptr->left->right->left = malloc(sizeof(node));

      int restore_i_minus_i = restore_i - i;
      token *left_token_argument = malloc(sizeof(token) * restore_i_minus_i);
      memcpy(left_token_argument, &code_lex[i],
             sizeof(token) * restore_i_minus_i);

      i = restore_i;
      i++;
      while (i != code_lex_index && code_lex[i].type != get_symbol("&&")) {
        i++;
      }

      int i_minus_restore_i = i - restore_i - 1;
      token *right_token_argument = malloc(sizeof(token) * (i_minus_restore_i));
      memcpy(right_token_argument, &code_lex[restore_i + 1],
             sizeof(token) * (i_minus_restore_i));

      code_tree_ptr->left->type = (enum node_type)code_lex[restore_i].type;
      code_tree_ptr->left->back = code_tree_ptr;

      tree(code_tree_ptr->left->left, left_token_argument, restore_i_minus_i);
      tree(code_tree_ptr->left->right, right_token_argument, i_minus_restore_i);

      code_tree_ptr->right->back = code_tree_ptr;
      code_tree_ptr->right->left = malloc(sizeof(node));
      code_tree_ptr->right->right = malloc(sizeof(node));
      code_tree_ptr->right->type = PROGRAM;
      tree(code_tree_ptr->right, &code_lex[i], code_lex_index - i);
      return;
    default:
      break;
    }
  }

  for (int i = 0; i < code_lex_index; i++) {
    int id = code_lex[i].type;

    switch (id) {
    case '|':;
      int restore_i = i;
      i--;
      while (i != -1 && code_lex[i].type != '|') {
        i--;
      }

      i++;
      code_tree_ptr->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->type = PROGRAM;
      code_tree_ptr->left->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->right = malloc(sizeof(node));
      code_tree_ptr->left->left->back = code_tree_ptr->left;
      code_tree_ptr->left->right = malloc(sizeof(node));
      code_tree_ptr->left->right->type = PROGRAM;
      code_tree_ptr->left->right->back = code_tree_ptr->left;
      code_tree_ptr->left->right->right = malloc(sizeof(node));
      code_tree_ptr->left->right->left = malloc(sizeof(node));

      int restore_i_minus_i = restore_i - i;
      token *left_token_argument = malloc(sizeof(token) * restore_i_minus_i);
      memcpy(left_token_argument, &code_lex[i],
             sizeof(token) * restore_i_minus_i);

      i = restore_i;
      i++;
      while (i != code_lex_index && code_lex[i].type != '|') {
        i++;
      }

      int i_minus_restore_i = i - restore_i - 1;
      token *right_token_argument = malloc(sizeof(token) * (i_minus_restore_i));
      memcpy(right_token_argument, &code_lex[restore_i + 1],
             sizeof(token) * (i_minus_restore_i));

      code_tree_ptr->left->type = (enum node_type)code_lex[restore_i].type;
      code_tree_ptr->left->back = code_tree_ptr;

      tree(code_tree_ptr->left->left, left_token_argument, restore_i_minus_i);
      tree(code_tree_ptr->left->right, right_token_argument, i_minus_restore_i);

      code_tree_ptr->right->back = code_tree_ptr;
      code_tree_ptr->right->left = malloc(sizeof(node));
      code_tree_ptr->right->right = malloc(sizeof(node));
      code_tree_ptr->right->type = PROGRAM;
      tree(code_tree_ptr->right, &code_lex[i], code_lex_index - i);
      return;
    default:
      break;
    }
  }

  for (int i = 0; i < code_lex_index; i++) {
    int id = code_lex[i].type;

    switch (id) {
    case '^':;
      int restore_i = i;
      i--;
      while (i != -1 && code_lex[i].type != '^') {
        i--;
      }

      i++;
      code_tree_ptr->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->type = PROGRAM;
      code_tree_ptr->left->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->right = malloc(sizeof(node));
      code_tree_ptr->left->left->back = code_tree_ptr->left;
      code_tree_ptr->left->right = malloc(sizeof(node));
      code_tree_ptr->left->right->type = PROGRAM;
      code_tree_ptr->left->right->back = code_tree_ptr->left;
      code_tree_ptr->left->right->right = malloc(sizeof(node));
      code_tree_ptr->left->right->left = malloc(sizeof(node));

      int restore_i_minus_i = restore_i - i;
      token *left_token_argument = malloc(sizeof(token) * restore_i_minus_i);
      memcpy(left_token_argument, &code_lex[i],
             sizeof(token) * restore_i_minus_i);

      i = restore_i;
      i++;
      while (i != code_lex_index && code_lex[i].type != '^') {
        i++;
      }

      int i_minus_restore_i = i - restore_i - 1;
      token *right_token_argument = malloc(sizeof(token) * (i_minus_restore_i));
      memcpy(right_token_argument, &code_lex[restore_i + 1],
             sizeof(token) * (i_minus_restore_i));

      code_tree_ptr->left->type = (enum node_type)code_lex[restore_i].type;
      code_tree_ptr->left->back = code_tree_ptr;

      tree(code_tree_ptr->left->left, left_token_argument, restore_i_minus_i);
      tree(code_tree_ptr->left->right, right_token_argument, i_minus_restore_i);

      code_tree_ptr->right->back = code_tree_ptr;
      code_tree_ptr->right->left = malloc(sizeof(node));
      code_tree_ptr->right->right = malloc(sizeof(node));
      code_tree_ptr->right->type = PROGRAM;
      tree(code_tree_ptr->right, &code_lex[i], code_lex_index - i);
      return;
    default:
      break;
    }
  }

  for (int i = 0; i < code_lex_index; i++) {
    int id = code_lex[i].type;

    switch (id) {
    case '&':;
      int restore_i = i;
      i--;
      while (i != -1 && code_lex[i].type != '&') {
        i--;
      }

      i++;
      code_tree_ptr->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->type = PROGRAM;
      code_tree_ptr->left->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->right = malloc(sizeof(node));
      code_tree_ptr->left->left->back = code_tree_ptr->left;
      code_tree_ptr->left->right = malloc(sizeof(node));
      code_tree_ptr->left->right->type = PROGRAM;
      code_tree_ptr->left->right->back = code_tree_ptr->left;
      code_tree_ptr->left->right->right = malloc(sizeof(node));
      code_tree_ptr->left->right->left = malloc(sizeof(node));

      int restore_i_minus_i = restore_i - i;
      token *left_token_argument = malloc(sizeof(token) * restore_i_minus_i);
      memcpy(left_token_argument, &code_lex[i],
             sizeof(token) * restore_i_minus_i);

      i = restore_i;
      i++;
      while (i != code_lex_index && code_lex[i].type != '&') {
        i++;
      }

      int i_minus_restore_i = i - restore_i - 1;
      token *right_token_argument = malloc(sizeof(token) * (i_minus_restore_i));
      memcpy(right_token_argument, &code_lex[restore_i + 1],
             sizeof(token) * (i_minus_restore_i));

      code_tree_ptr->left->type = (enum node_type)code_lex[restore_i].type;
      code_tree_ptr->left->back = code_tree_ptr;

      tree(code_tree_ptr->left->left, left_token_argument, restore_i_minus_i);
      tree(code_tree_ptr->left->right, right_token_argument, i_minus_restore_i);

      code_tree_ptr->right->back = code_tree_ptr;
      code_tree_ptr->right->left = malloc(sizeof(node));
      code_tree_ptr->right->right = malloc(sizeof(node));
      code_tree_ptr->right->type = PROGRAM;
      tree(code_tree_ptr->right, &code_lex[i], code_lex_index - i);
      return;
    default:
      break;
    }
  }

  for (int i = 0; i < code_lex_index; i++) {
    int id;
    if (code_lex[i].type == get_symbol("=="))
      id = 'T';
    else if (code_lex[i].type == get_symbol("!="))
      id = 'T';
    else
      id = code_lex[i].type;

    switch (id) {
    case 'T':;
      int restore_i = i;
      i--;
      while (i != -1 && code_lex[i].type != get_symbol("==") &&
             code_lex[i].type != get_symbol("!=")) {
        i--;
      }

      i++;
      code_tree_ptr->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->type = PROGRAM;
      code_tree_ptr->left->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->right = malloc(sizeof(node));
      code_tree_ptr->left->left->back = code_tree_ptr->left;
      code_tree_ptr->left->right = malloc(sizeof(node));
      code_tree_ptr->left->right->type = PROGRAM;
      code_tree_ptr->left->right->back = code_tree_ptr->left;
      code_tree_ptr->left->right->right = malloc(sizeof(node));
      code_tree_ptr->left->right->left = malloc(sizeof(node));

      int restore_i_minus_i = restore_i - i;
      token *left_token_argument = malloc(sizeof(token) * restore_i_minus_i);
      memcpy(left_token_argument, &code_lex[i],
             sizeof(token) * restore_i_minus_i);

      i = restore_i;
      i++;
      while (i != code_lex_index && code_lex[i].type != get_symbol("==") &&
             code_lex[i].type != get_symbol("!=")) {
        i++;
      }

      int i_minus_restore_i = i - restore_i - 1;
      token *right_token_argument = malloc(sizeof(token) * (i_minus_restore_i));
      memcpy(right_token_argument, &code_lex[restore_i + 1],
             sizeof(token) * (i_minus_restore_i));

      code_tree_ptr->left->type = (enum node_type)code_lex[restore_i].type;
      code_tree_ptr->left->back = code_tree_ptr;

      tree(code_tree_ptr->left->left, left_token_argument, restore_i_minus_i);
      tree(code_tree_ptr->left->right, right_token_argument, i_minus_restore_i);

      code_tree_ptr->right->back = code_tree_ptr;
      code_tree_ptr->right->left = malloc(sizeof(node));
      code_tree_ptr->right->right = malloc(sizeof(node));
      code_tree_ptr->right->type = PROGRAM;
      tree(code_tree_ptr->right, &code_lex[i], code_lex_index - i);
      return;
    default:
      break;
    }
  }

  for (int i = 0; i < code_lex_index; i++) {
    int id;
    if (code_lex[i].type == get_symbol(">="))
      id = 'T';
    else if (code_lex[i].type == get_symbol("<="))
      id = 'T';
    else if (code_lex[i].type == get_symbol(".."))
      id = 'T';
    else
      id = code_lex[i].type;

    switch (id) {
    case '>':
    case '<':
    case 'T':;
      int restore_i = i;
      i--;
      while (i != -1 && code_lex[i].type != get_symbol(">=") &&
             code_lex[i].type != get_symbol("<=") && code_lex[i].type != '>' &&
             code_lex[i].type != '<' && code_lex[i].type != get_symbol("..")) {
        i--;
      }

      i++;
      code_tree_ptr->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->type = PROGRAM;
      code_tree_ptr->left->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->right = malloc(sizeof(node));
      code_tree_ptr->left->left->back = code_tree_ptr->left;
      code_tree_ptr->left->right = malloc(sizeof(node));
      code_tree_ptr->left->right->type = PROGRAM;
      code_tree_ptr->left->right->back = code_tree_ptr->left;
      code_tree_ptr->left->right->right = malloc(sizeof(node));
      code_tree_ptr->left->right->left = malloc(sizeof(node));

      int restore_i_minus_i = restore_i - i;
      token *left_token_argument = malloc(sizeof(token) * restore_i_minus_i);
      memcpy(left_token_argument, &code_lex[i],
             sizeof(token) * restore_i_minus_i);

      i = restore_i;
      i++;
      while (i != code_lex_index && code_lex[i].type != get_symbol(">=") &&
             code_lex[i].type != get_symbol("<=") && code_lex[i].type != '>' &&
             code_lex[i].type != '<' && code_lex[i].type != get_symbol("..")) {
        i++;
      }

      int i_minus_restore_i = i - restore_i - 1;
      token *right_token_argument = malloc(sizeof(token) * (i_minus_restore_i));
      memcpy(right_token_argument, &code_lex[restore_i + 1],
             sizeof(token) * (i_minus_restore_i));

      code_tree_ptr->left->type = (enum node_type)code_lex[restore_i].type;
      code_tree_ptr->left->back = code_tree_ptr;

      tree(code_tree_ptr->left->left, left_token_argument, restore_i_minus_i);
      tree(code_tree_ptr->left->right, right_token_argument, i_minus_restore_i);

      code_tree_ptr->right->back = code_tree_ptr;
      code_tree_ptr->right->left = malloc(sizeof(node));
      code_tree_ptr->right->right = malloc(sizeof(node));
      code_tree_ptr->right->type = PROGRAM;
      tree(code_tree_ptr->right, &code_lex[i], code_lex_index - i);
      return;
    default:
      break;
    }
  }

  for (int i = 0; i < code_lex_index; i++) {
    int id;
    if (code_lex[i].type == get_symbol("<<"))
      id = 'T';
    else if (code_lex[i].type == get_symbol(">>"))
      id = 'T';
    else
      id = code_lex[i].type;

    switch (id) {
    case 'T':;
      int restore_i = i;
      i--;
      while (i != -1 && code_lex[i].type != get_symbol("<<") &&
             code_lex[i].type != get_symbol(">>")) {
        i--;
      }

      i++;
      code_tree_ptr->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->type = PROGRAM;
      code_tree_ptr->left->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->right = malloc(sizeof(node));
      code_tree_ptr->left->left->back = code_tree_ptr->left;
      code_tree_ptr->left->right = malloc(sizeof(node));
      code_tree_ptr->left->right->type = PROGRAM;
      code_tree_ptr->left->right->back = code_tree_ptr->left;
      code_tree_ptr->left->right->right = malloc(sizeof(node));
      code_tree_ptr->left->right->left = malloc(sizeof(node));

      int restore_i_minus_i = restore_i - i;
      token *left_token_argument = malloc(sizeof(token) * restore_i_minus_i);
      memcpy(left_token_argument, &code_lex[i],
             sizeof(token) * restore_i_minus_i);

      i = restore_i;
      i++;
      while (i != code_lex_index && code_lex[i].type != get_symbol("<<") &&
             code_lex[i].type != get_symbol(">>")) {
        i++;
      }

      int i_minus_restore_i = i - restore_i - 1;
      token *right_token_argument = malloc(sizeof(token) * (i_minus_restore_i));
      memcpy(right_token_argument, &code_lex[restore_i + 1],
             sizeof(token) * (i_minus_restore_i));

      code_tree_ptr->left->type = (enum node_type)code_lex[restore_i].type;
      code_tree_ptr->left->back = code_tree_ptr;

      tree(code_tree_ptr->left->left, left_token_argument, restore_i_minus_i);
      tree(code_tree_ptr->left->right, right_token_argument, i_minus_restore_i);

      code_tree_ptr->right->back = code_tree_ptr;
      code_tree_ptr->right->left = malloc(sizeof(node));
      code_tree_ptr->right->right = malloc(sizeof(node));
      code_tree_ptr->right->type = PROGRAM;
      tree(code_tree_ptr->right, &code_lex[i], code_lex_index - i);
      return;
    default:
      break;
    }
  }

  for (int i = 0; i < code_lex_index; i++) {
    int id = code_lex[i].type;

    switch (id) {
    case '-':
    case '+':;
      if (i == 0 ||
          (i > 0 && code_lex[i - 1].type != WORD &&
           code_lex[i - 1].type != INT && code_lex[i - 1].type != FLOAT)) {
        break;
      }

      int restore_i = i;
      i--;
      // an interrupting operator would have a word, int, or float immediately
      // behind it
      while (
          i != -1 &&
          !(/*following is defining for interrupting operator*/ (
                code_lex[i].type == '+' || code_lex[i].type == '-') &&
            (i > 0 &&
             (code_lex[i - 1].type == WORD || code_lex[i - 1].type == INT ||
              code_lex[i - 1].type == FLOAT || code_lex[i - 1].type == '(' ||
              code_lex[i - 1].type == '[' || code_lex[i - 1].type == '{')))) {
        i--;
      }

      i++;
      code_tree_ptr->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->type = PROGRAM;
      code_tree_ptr->left->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->right = malloc(sizeof(node));
      code_tree_ptr->left->left->back = code_tree_ptr->left;
      code_tree_ptr->left->right = malloc(sizeof(node));
      code_tree_ptr->left->right->type = PROGRAM;
      code_tree_ptr->left->right->back = code_tree_ptr->left;
      code_tree_ptr->left->right->right = malloc(sizeof(node));
      code_tree_ptr->left->right->left = malloc(sizeof(node));

      int restore_i_minus_i = restore_i - i;
      token *left_token_argument = malloc(sizeof(token) * restore_i_minus_i);
      memcpy(left_token_argument, &code_lex[i],
             sizeof(token) * restore_i_minus_i);

      i = restore_i;
      i++;
      while (
          i != code_lex_index &&
          !(/*following is defining for interrupting operator*/ (
                code_lex[i].type == '+' || code_lex[i].type == '-') &&
            (i > 0 &&
             (code_lex[i - 1].type == WORD || code_lex[i - 1].type == INT ||
              code_lex[i - 1].type == FLOAT || code_lex[i - 1].type == '(' ||
              code_lex[i - 1].type == '[' || code_lex[i - 1].type == '{')))) {
        i++;
      }

      int i_minus_restore_i = i - restore_i - 1;
      token *right_token_argument = malloc(sizeof(token) * (i_minus_restore_i));
      memcpy(right_token_argument, &code_lex[restore_i + 1],
             sizeof(token) * (i_minus_restore_i));

      code_tree_ptr->left->type = (enum node_type)code_lex[restore_i].type;
      code_tree_ptr->left->back = code_tree_ptr;

      tree(code_tree_ptr->left->left, left_token_argument, restore_i_minus_i);
      tree(code_tree_ptr->left->right, right_token_argument, i_minus_restore_i);

      code_tree_ptr->right->back = code_tree_ptr;
      code_tree_ptr->right->left = malloc(sizeof(node));
      code_tree_ptr->right->right = malloc(sizeof(node));
      code_tree_ptr->right->type = PROGRAM;
      tree(code_tree_ptr->right, &code_lex[i], code_lex_index - i);
      return;
    default:
      break;
    }
  }

  for (int i = 0; i < code_lex_index; i++) {
    int id = code_lex[i].type;

    switch (id) {
    case '/':
    case '*':
    case '%':;
      int restore_i = i;
      i--;
      while (i != -1 && code_lex[i].type != '*' && code_lex[i].type != '%' &&
             code_lex[i].type != '/') {
        i--;
      }

      i++;
      code_tree_ptr->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->type = PROGRAM;
      code_tree_ptr->left->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->right = malloc(sizeof(node));
      code_tree_ptr->left->left->back = code_tree_ptr->left;
      code_tree_ptr->left->right = malloc(sizeof(node));
      code_tree_ptr->left->right->type = PROGRAM;
      code_tree_ptr->left->right->back = code_tree_ptr->left;
      code_tree_ptr->left->right->right = malloc(sizeof(node));
      code_tree_ptr->left->right->left = malloc(sizeof(node));

      int restore_i_minus_i = restore_i - i;
      token *left_token_argument = malloc(sizeof(token) * restore_i_minus_i);
      memcpy(left_token_argument, &code_lex[i],
             sizeof(token) * restore_i_minus_i);

      i = restore_i;
      i++;
      while (i != code_lex_index && code_lex[i].type != '*' &&
             code_lex[i].type != '%' && code_lex[i].type != '/') {
        i++;
      }

      int i_minus_restore_i = i - restore_i - 1;
      token *right_token_argument = malloc(sizeof(token) * (i_minus_restore_i));
      memcpy(right_token_argument, &code_lex[restore_i + 1],
             sizeof(token) * (i_minus_restore_i));

      code_tree_ptr->left->type = (enum node_type)code_lex[restore_i].type;
      code_tree_ptr->left->back = code_tree_ptr;

      tree(code_tree_ptr->left->left, left_token_argument, restore_i_minus_i);
      tree(code_tree_ptr->left->right, right_token_argument, i_minus_restore_i);

      code_tree_ptr->right->back = code_tree_ptr;
      code_tree_ptr->right->left = malloc(sizeof(node));
      code_tree_ptr->right->right = malloc(sizeof(node));
      code_tree_ptr->right->type = PROGRAM;
      tree(code_tree_ptr->right, &code_lex[i], code_lex_index - i);
      return;
    default:
      break;
    }
  }

  for (int i = 0; i < code_lex_index; i++) {
    int id;
    if (code_lex[i].type == get_symbol("!!"))
      id = 'U';
    else if (code_lex[i].type ==
             get_symbol("cast")) // syntax will be "var_name cast: i32"
                                 // so type will be inside the cast symbol token
      id = 'T';
    else if (code_lex[i].type == get_symbol("bitcast"))
      id = 'T';
    else if (code_lex[i].type == get_symbol("sizeof"))
      id = 'U';
    else
      id = code_lex[i].type;

    switch (id) {
    case '-':
    case '+':
    case '!':
    case 'U': {
      int restore_i = i + 1;
      i += 2;
      code_tree_ptr->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->type = PROGRAM;
      code_tree_ptr->left->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->right = malloc(sizeof(node));
      code_tree_ptr->left->left->back = code_tree_ptr->left;
      code_tree_ptr->left->right = malloc(sizeof(node));
      code_tree_ptr->left->right->type = END;
      code_tree_ptr->left->right->back = code_tree_ptr->left;

      int restore_i_minus_i = 1;
      token *left_token_argument = malloc(sizeof(token));
      memcpy(left_token_argument, &code_lex[restore_i], sizeof(token));

      code_tree_ptr->left->type = (enum node_type)code_lex[restore_i - 1].type;
      code_tree_ptr->left->back = code_tree_ptr;

      tree(code_tree_ptr->left->left, left_token_argument, restore_i_minus_i);
      code_tree_ptr->right->back = code_tree_ptr;
      code_tree_ptr->right->left = malloc(sizeof(node));
      code_tree_ptr->right->right = malloc(sizeof(node));
      code_tree_ptr->right->type = PROGRAM;
      tree(code_tree_ptr->right, &code_lex[i], code_lex_index - i);
      return;
    }

    case 'T':;
      int restore_i = i; // left
      i--;
      code_tree_ptr->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->type = PROGRAM;
      code_tree_ptr->left->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->right = malloc(sizeof(node));
      code_tree_ptr->left->left->back = code_tree_ptr->left;
      code_tree_ptr->left->right = malloc(sizeof(node));
      code_tree_ptr->left->right->type = END;
      code_tree_ptr->left->right->back = code_tree_ptr->left;

      int restore_i_minus_i = restore_i - i;
      token *left_token_argument = malloc(sizeof(token) * restore_i_minus_i);
      memcpy(left_token_argument, &code_lex[i],
             sizeof(token) * restore_i_minus_i);

      i = restore_i;
      i++;

      code_tree_ptr->left->type = (enum node_type)code_lex[restore_i].type;
      code_tree_ptr->left->back = code_tree_ptr;

      tree(code_tree_ptr->left->left, left_token_argument, restore_i_minus_i);
      code_tree_ptr->right->back = code_tree_ptr;
      code_tree_ptr->right->left = malloc(sizeof(node));
      code_tree_ptr->right->right = malloc(sizeof(node));
      code_tree_ptr->right->type = PROGRAM;
      tree(code_tree_ptr->right, &code_lex[i], code_lex_index - i);
      return;
    default:
      break;
    }
  }

  for (int i = 0; i < code_lex_index; i++) {
    int id;
    if (code_lex[i].type == get_symbol("++"))
      id = 'T';

    else if (code_lex[i].type == get_symbol("--"))
      id = 'T';

    else
      id = code_lex[i].type;

    switch (id) {
    case '(': {
      code_tree_ptr->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->type = PROGRAM;
      code_tree_ptr->left->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->right = malloc(sizeof(node));
      code_tree_ptr->left->left->back = code_tree_ptr->left;
      code_tree_ptr->left->right = malloc(sizeof(node));
      code_tree_ptr->left->right->type = END;
      code_tree_ptr->left->right->back = code_tree_ptr->left;

      code_tree_ptr->left->type = (enum node_type)'(';
      code_tree_ptr->left->back = code_tree_ptr;

      tree(code_tree_ptr->left->left, code_lex[i].token_argument,
           code_lex[i].token_length);
      code_tree_ptr->right->back = code_tree_ptr;
      code_tree_ptr->right->left = malloc(sizeof(node));
      code_tree_ptr->right->right = malloc(sizeof(node));
      code_tree_ptr->right->type = PROGRAM;
      tree(code_tree_ptr->right, &code_lex[i], code_lex_index - i);
      return;
    }

    case '{': {
      code_tree_ptr->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->type = PROGRAM;
      code_tree_ptr->left->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->right = malloc(sizeof(node));
      code_tree_ptr->left->left->back = code_tree_ptr->left;
      code_tree_ptr->left->right = malloc(sizeof(node));
      code_tree_ptr->left->right->type = END;
      code_tree_ptr->left->right->back = code_tree_ptr->left;

      code_tree_ptr->left->type = (enum node_type)'{';
      code_tree_ptr->left->back = code_tree_ptr;

      tree(code_tree_ptr->left->left, code_lex[i].brace_argument,
           code_lex[i].brace_length);
      code_tree_ptr->right->back = code_tree_ptr;
      code_tree_ptr->right->left = malloc(sizeof(node));
      code_tree_ptr->right->right = malloc(sizeof(node));
      code_tree_ptr->right->type = PROGRAM;
      tree(code_tree_ptr->right, &code_lex[i], code_lex_index - i);
      return;
    }

    case '[': {
      code_tree_ptr->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->type = PROGRAM;
      code_tree_ptr->left->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->right = malloc(sizeof(node));
      code_tree_ptr->left->left->back = code_tree_ptr->left;
      code_tree_ptr->left->right = malloc(sizeof(node));
      code_tree_ptr->left->right->type = END;
      code_tree_ptr->left->right->back = code_tree_ptr->left;

      code_tree_ptr->left->type = (enum node_type)'[';
      code_tree_ptr->left->back = code_tree_ptr;

      tree(code_tree_ptr->left->left, code_lex[i].brack_argument,
           code_lex[i].brack_length);
      code_tree_ptr->right->back = code_tree_ptr;
      code_tree_ptr->right->left = malloc(sizeof(node));
      code_tree_ptr->right->right = malloc(sizeof(node));
      code_tree_ptr->right->type = PROGRAM;
      tree(code_tree_ptr->right, &code_lex[i], code_lex_index - i);
      return;
    }

    case WORD:
      // ok so this is gonna handle attached parens, brackets, and braces.
      // evaluator can handle colons itself
    case '.': {
      int restore_i = i;
      i--;
      code_tree_ptr->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->type = PROGRAM;
      code_tree_ptr->left->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->right = malloc(sizeof(node));
      code_tree_ptr->left->left->back = code_tree_ptr->left;
      code_tree_ptr->left->right = malloc(sizeof(node));
      code_tree_ptr->left->right->type = PROGRAM;
      code_tree_ptr->left->right->back = code_tree_ptr->left;
      code_tree_ptr->left->right->right = malloc(sizeof(node));
      code_tree_ptr->left->right->left = malloc(sizeof(node));

      int restore_i_minus_i = restore_i - i;
      token *left_token_argument = malloc(sizeof(token) * restore_i_minus_i);
      memcpy(left_token_argument, &code_lex[i],
             sizeof(token) * restore_i_minus_i);

      i = restore_i;
      i += 2;

      int i_minus_restore_i = i - restore_i - 1;
      token *right_token_argument = malloc(sizeof(token) * (i_minus_restore_i));
      memcpy(right_token_argument, &code_lex[restore_i + 1],
             sizeof(token) * (i_minus_restore_i));

      code_tree_ptr->left->type = (enum node_type)code_lex[restore_i].type;
      code_tree_ptr->left->back = code_tree_ptr;

      tree(code_tree_ptr->left->left, left_token_argument, restore_i_minus_i);
      tree(code_tree_ptr->left->right, right_token_argument, i_minus_restore_i);

      code_tree_ptr->right->back = code_tree_ptr;
      code_tree_ptr->right->left = malloc(sizeof(node));
      code_tree_ptr->right->right = malloc(sizeof(node));
      code_tree_ptr->right->type = PROGRAM;
      tree(code_tree_ptr->right, &code_lex[i], code_lex_index - i);
      return;
    }
    case 'T':;
      int restore_i = i; // left
      i--;
      code_tree_ptr->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->type = PROGRAM;
      code_tree_ptr->left->left->left = malloc(sizeof(node));
      code_tree_ptr->left->left->right = malloc(sizeof(node));
      code_tree_ptr->left->left->back = code_tree_ptr->left;
      code_tree_ptr->left->right = malloc(sizeof(node));
      code_tree_ptr->left->right->type = END;
      code_tree_ptr->left->right->back = code_tree_ptr->left;

      int restore_i_minus_i = restore_i - i;
      token *left_token_argument = malloc(sizeof(token) * restore_i_minus_i);
      memcpy(left_token_argument, &code_lex[i],
             sizeof(token) * restore_i_minus_i);

      i = restore_i;
      i++;

      code_tree_ptr->left->type = (enum node_type)code_lex[restore_i].type;
      code_tree_ptr->left->back = code_tree_ptr;

      tree(code_tree_ptr->left->left, left_token_argument, restore_i_minus_i);
      code_tree_ptr->right->back = code_tree_ptr;
      code_tree_ptr->right->left = malloc(sizeof(node));
      code_tree_ptr->right->right = malloc(sizeof(node));
      code_tree_ptr->right->type = PROGRAM;
      tree(code_tree_ptr->right, &code_lex[i], code_lex_index - i);
      return;
    default:
      break;
    }
  }

  code_tree_ptr->type = LITERAL;
  code_tree_ptr->token_argument = code_lex;
}

void print_tree(node *root, size_t tabs) {
  for (int i = 0; i < tabs; i++)
    printf("   ");
  printf("left type: %d\n", root->left->type);

  if (root->left->type != LITERAL && root->left->type != END)
    print_tree(root->left, tabs + 1);

  for (int i = 0; i < tabs; i++)
    printf("   ");
  printf("right type: %d\n", root->right->type);

  if (root->right->type != LITERAL && root->right->type != END)
    print_tree(root->right, tabs + 1);
}

int main(int argc, char **argv) {
  size_t strlen_argv_1 =
      strlen(argv[1]); // "argv[1]" because I don't want to have to deal with
                       // file management until I need to
  char *raw_code = argv[1];
  size_t code_lex_index;
  token *code_lex = lex(raw_code, strlen_argv_1, &code_lex_index);

  // FOR PRINTING LEX:
  printf("PRINTED LEX:\n");
  for (int i = 0; i < code_lex_index; i++) {
    printf("type: %d\n", code_lex[i].type);
    printf("string argument: %s\n", code_lex[i].string_argument);
  }

  // MOVE THIS WHOLE SETUP TO ITS OWN FUNCTION SO THAT IT CAN RECURSE
  node code_tree;
  node *code_tree_ptr = &code_tree;
  code_tree.type = PROGRAM;
  code_tree.back = NULL;
  code_tree.left = malloc(sizeof(node));
  code_tree.right = malloc(sizeof(node));

  tree(code_tree_ptr, code_lex, code_lex_index);

  printf("\n");
  print_tree(&code_tree, 0);
  //    printf("%d\n%d\n", PROGRAM, code_tree_ptr->type);
  return 0;
}
