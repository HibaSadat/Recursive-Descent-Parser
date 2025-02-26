#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <ctype.h>

enum tokenType {
    READ, WRITE, ID, NUMBER, LPAREN, RPAREN, SEMICOLON, COMMA, ASSIGN, PLUS, MINUS, TIMES, DIV, SCAN_EOF
};

char* mnemonic[] = { "READ", "WRITE", "ID", "NUMBER", "LPAREN", "RPAREN", "SEMICOLON", "COMMA", "ASSIGN", "PLUS", "MINUS", "TIMES", "DIV", "SCAN_EOF" };

char lexeme[256] = { '\0' };
unsigned int lexLen = 0;
FILE* src;
enum tokenType currentToken;
void lexical_error(char ch);

enum tokenType scan() {
    static int currentCh = ' ';
    char* reserved[2] = { "read", "write" };
    lexLen = 0;
    lexeme[0] = '\0';
    extern FILE* src;

    if (feof(src)) {
        return SCAN_EOF;
    }

    while ((currentCh = fgetc(src)) != EOF) {
        if (isspace(currentCh)) {
            continue;
        }
        else if (isalpha(currentCh) || currentCh == '_') { 
            lexeme[0] = currentCh;
            lexLen = 1;
            int tempCh;
            while ((tempCh = fgetc(src)), (isalnum(tempCh) || tempCh == '_')) {
                lexeme[lexLen++] = tempCh;
            }
            lexeme[lexLen] = '\0';
            if (tempCh != EOF) {
                ungetc(tempCh, src);
            }
            if (strcmp(lexeme, "read") == 0) {
                return READ;
            }
            else if (strcmp(lexeme, "write") == 0) {
                return WRITE;
            }
            else {
                return ID;
            }
        }
        else if (isdigit(currentCh)) {
            lexeme[0] = currentCh;
            lexLen = 1;
            int tempCh;
            while ((tempCh = fgetc(src)), isdigit(tempCh)) {
                lexeme[lexLen++] = tempCh;
            }
            lexeme[lexLen] = '\0';
            if (tempCh != EOF) {
                ungetc(tempCh, src);
            }
            return NUMBER;
        }
        else if (currentCh == '+') {
            return PLUS;
        }
        else if (currentCh == '-') {
            return MINUS;
        }
        else if (currentCh == '*') {
            return TIMES;
        }
        else if (currentCh == '/') {
            return DIV;
        }
        else if (currentCh == ';') {
            return SEMICOLON;
        }
        else if (currentCh == ',') {
            return COMMA;
        }
        else if (currentCh == '(') {
            return LPAREN;
        }
        else if (currentCh == ')') {
            return RPAREN;
        }
        else if (currentCh == ':') {
            int tempCh;
            tempCh = fgetc(src);
            if (tempCh == '=') {
                return ASSIGN;
            }
            else {
                if (tempCh != EOF) {
                    ungetc(tempCh, src);
                }
                lexical_error(':');
            }
        }
        else {
            lexical_error(currentCh);
        }
    }
    return SCAN_EOF;
}

unsigned int numErrs = 0;
void lexical_error(char ch) {
    fprintf(stderr, "Lexical Error. Unexpected character: %c.\n", ch);
    exit(1);
}

extern enum tokenType currentToken;
extern char lexeme[256];
extern FILE* src;

void program();
void stmt_list();
void stmt();
void expr();
void term();
void factor();
void expr_list();
void id_list();
void match(enum tokenType expected);
void parse_error(char* errMsg, char* lexeme);

void program() {
    currentToken = scan(); 
    stmt_list();
    match(SCAN_EOF); 
}

void stmt_list() {
    if (currentToken == ID || currentToken == READ || currentToken == WRITE) {
        stmt();
        stmt_list();
    }
}

void stmt() {
    switch (currentToken) {
    case ID:
        match(ID);
        match(ASSIGN);
        expr();
        match(SEMICOLON);
        break;
    case READ:
        match(READ);
        match(LPAREN);
        id_list();
        match(RPAREN);
        match(SEMICOLON);
        break;
    case WRITE:
        match(WRITE);
        match(LPAREN);
        expr_list();
        match(SEMICOLON);
        break;
    default:
        parse_error("Invalid statement", lexeme);
        break;
    }
}

void expr() {
    term();
    while (currentToken == PLUS || currentToken == MINUS) {
        match(currentToken);
        term();
    }
}

void term() {
    factor();
    while (currentToken == TIMES || currentToken == DIV) {
        match(currentToken); 
        factor();
    }
}

void factor() {
    if (currentToken == LPAREN) {
        match(LPAREN);
        expr();
        match(RPAREN);
    }
    else if (currentToken == ID) {
        match(ID);
    }
    else if (currentToken == NUMBER) {
        match(NUMBER);
    }
    else {
        parse_error("Error in expression: Expected ID, NUMBER, or '('.", lexeme);
    }
}

void expr_list() {
    expr();
    while (currentToken == COMMA) {
        match(COMMA);
        expr();
    }
    match(RPAREN);
}

void id_list() {
    match(ID);
    while (currentToken == COMMA) {
        match(COMMA);
        match(ID);
    }
}

void match(enum tokenType expected) {
    if (currentToken == expected) {
        currentToken = scan();
    }
    else {
        parse_error("Expected symbol", mnemonic[expected]);
        exit(1);
    }
}

void parse_error(char* errMsg, char* lexeme) {
    fprintf(stderr, "%s: %s\n ", errMsg, lexeme);
    exit(1);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source file>\n", argv[0]);
        exit(1);
    }

    if (fopen_s(&src, argv[1], "r")) {
        fprintf(stderr, "Error opening source file: %s ", argv[1]);
        exit(1);
    }

    program();
    printf("Parsing complete. No errors.\n");

    fclose(src);
    return 0;
}