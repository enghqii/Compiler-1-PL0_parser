#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
//#include <stdbool.h>

#define FALSE 0
#define TRUE 1

#define MAX_LEVEL 3		/* maximum depth of block nesting */
#define MAX_CODE 200	/* size of code array */

typedef enum {

	TYPE_NONE = 0,

	TYPE_IDENTIFIER,
	TYPE_KEYWORD,
	TYPE_NUMBER,
	
	TYPE_ASSIGN,
	TYPE_GREATER_EQUAL,
	TYPE_LESS_EQUAL,
	TYPE_NOT_EQUAL,

	TYPE_PLUS = '+',
	TYPE_MINUS = '-',
	TYPE_MULTIPLY = '*',
	TYPE_DIVIDE = '/',
	
	TYPE_COMMA = ',',
	TYPE_EQUAL = '=',
	TYPE_SEMICOLON = ';',
	TYPE_PERIOD = '.',
	
	TYPE_COLON = ':',
	TYPE_GREATER = '>',
	TYPE_LESS = '<',
	
	TYPE_LPAREN = '(',
	TYPE_RPAREN = ')',

	TYPE_OTHER,

} TOKEN_TYPE;

char keyWords[][10] = { "const", "var", "procedure", "call", "begin", "end", "if", "then", "while", "do", "odd"};

/* SYMTAB */
typedef enum {
	SYM_NONE,
	SYM_CONST,
	SYM_VAR,
	SYM_PROCEDURE,
} SYMBOL_TYPE;

char* get_type_string(SYMBOL_TYPE type) {

	switch (type) {
	case SYM_NONE:
		return "NONE";
	case SYM_CONST:
		return "CONST";
	case SYM_VAR:
		return "VAR";
	case SYM_PROCEDURE:
		return "PROC";
	default:
		return "";
	}
}

typedef struct _Symbol {
	char name[128];
	SYMBOL_TYPE type;
	int level;
	int addr;
} Symbol;

typedef struct _SymbolTable {
	int tx;
	Symbol symtab[256];
} SymbolTable;

/* CODE */
typedef enum {
	LIT = 0,
	OPR,
	LOD,
	STO,
	CAL,
	INT,
	JMP,
	JPC,
} Operator;

typedef struct _Instruction {
	Operator	opcode;
	int			level;
	int			disp;
} Instruction;

typedef struct _Code {
	int cx;							/* code allocation index */
	Instruction inst[MAX_CODE];
} Code;

/* GLOBAL vars */
FILE *		fp			= NULL;
char		ch			= '\0';		/* current char */

TOKEN_TYPE	type		= TYPE_NONE;
char		token[128]	= "";
int			num			= 0;

SymbolTable	SYMTAB;
Code		code;

int			lev = 0;

/* code generation functions */

void print_symboltable() {

	int i=0;
	
	
	printf("\n=========================\n");
	printf("<SYMTAB>\n");
	printf("name | type | level | addr\n");

	for(i = 0; i < SYMTAB.tx; i++)
	{
		printf("%5s %6s %7d %d\n",
		SYMTAB.symtab[i].name,
		get_type_string(SYMTAB.symtab[i].type),
		SYMTAB.symtab[i].level,
		SYMTAB.symtab[i].addr
		);
	}

	printf("=========================\n");
}

void level_up() {
	lev ++;
}

void level_down() {
	
	print_symboltable();

	{
		int i;
		for(i = SYMTAB.tx - 1; i >= 0; i--){
			if(SYMTAB.symtab[i].level < lev)
				break;
		}

		SYMTAB.tx = i + 1;
	}

	lev --;
}

void gen(Operator opcode, int level, int disp) {

	if(code.cx > MAX_CODE) {
		printf("Program is too long\n");
		exit(-1);

	} else {
		code.inst[code.cx].opcode = opcode;
		code.inst[code.cx].level = level;
		code.inst[code.cx].disp = disp;

		code.cx++;
	}
}

void error(int err) {
	fprintf(stderr, "ERROR %d\n", err);
	exit(-1);
}

int FindSymbol(char * name) {

	int i = 0;
	for(i = SYMTAB.tx - 1; i >= 0; i--) 	{
		if(strcmp(SYMTAB.symtab[i].name, name) == 0) {
			return i;
		}
	}
	return -1;
}

void enter(char * name, SYMBOL_TYPE type, int addr) {

	printf("enter : [%s]\n", name);

	if( FindSymbol(name) < 0 ) {
		Symbol s;

		strcpy(s.name, name);
		s.type	= type;
		s.level = lev;
		s.addr	= addr;
		
		if(type == SYM_VAR) {

			Symbol before = SYMTAB.symtab[SYMTAB.tx - 1];

			if(before.type == SYM_VAR){
				s.addr = before.addr + 1;
			}else{
				s.addr = 0;
			}
		}
		
		SYMTAB.symtab[SYMTAB.tx] = s;
		SYMTAB.tx++;
	}
}

// Tokenising functions 

int isKeyWord(char * str) {
	int i = 0;
	for(i = 0; i < sizeof(keyWords) / sizeof(char[10]); i++ ) {
		if( strcmp(str, keyWords[i]) == 0 )
			return TRUE;
	}
	return FALSE;
}

int isSpecialChar(char c) {
	if( c == '+' || c == '-' || c == '*' ||	c == '/' || c == ',' || 
		c == '=' || c == ';' || c == '.' || c == '(' || c == ')' ||
		c == ':' || c == '>' || c == '<') {
		return TRUE;
	}
	return FALSE;
}

int NextToken() {

	int tokenIndex = 0;
	token[0] = '\0';

	if( isdigit(ch) ) {
		while( isdigit(ch) ) {
			token[tokenIndex++] = ch;
			ch = fgetc(fp);
		}
		token[tokenIndex] = '\0';
		num = atoi(token);
		type = TYPE_NUMBER;

	} else if( isalpha(ch) ) {

		while( isalpha(ch) || isdigit(ch) ) {
			token[tokenIndex++] = ch;
			ch = fgetc(fp);
		}
		token[tokenIndex] = '\0';

		if(isKeyWord(token)) {
			type = TYPE_KEYWORD;
		}
		else {
			type = TYPE_IDENTIFIER;
		}

	} else if( isSpecialChar(ch) ) {
		char before = ch;
		token[0] = ch;
		token[1] = '\0';
		type = (TOKEN_TYPE)ch;

		ch = fgetc(fp);

		if( before == ':' && ch == '=' ) {
			token[1] = ch;
			token[2] = '\0';
			type = TYPE_ASSIGN;
			ch = fgetc(fp);

		} else if( before == '<' && ch == '=') {
			token[1] = ch;
			token[2] = '\0';
			type = TYPE_LESS_EQUAL;
			ch = fgetc(fp);

		} else if( before == '>' && ch == '=') {
			token[1] = ch;
			token[2] = '\0';
			type = TYPE_GREATER_EQUAL;
			ch = fgetc(fp);
		}
		else if( before == '<' && ch == '>') {
			token[1] = ch;
			token[2] = '\0';
			type = TYPE_NOT_EQUAL;
			ch = fgetc(fp);
		}

	} else if(ch == EOF) {
		return FALSE;

	} else {
		// A char i can't parse
		printf("CH : %d(%c)\n", ch, ch);
		type = TYPE_NONE;

		ch = fgetc(fp);
	}

	/*printf("NEXT token : %s\n", token);*/

	// eliminate white spaces
	while( isspace(ch) ){ ch = fgetc(fp); }
	return TRUE;
}

/* Parsing functions */

void Expression();

void Factor() {
	int i = 0;

	if( type == TYPE_IDENTIFIER ) {

		int index = FindSymbol(token);
		if( index >= 0 ){

			switch (SYMTAB.symtab[index].type){

			case SYM_CONST :
				gen(LIT, 0, SYMTAB.symtab[index].addr);
				break;
			case SYM_VAR :
				gen(LOD, lev - SYMTAB.symtab[index].level, SYMTAB.symtab[index].addr);
				break;
			default:
				error(21);
			}

		}else{
			error(21);
		}

		NextToken();
	} else if( type == TYPE_NUMBER ) {
		gen(LIT, 0, num);
		NextToken();
	} else if( type == TYPE_LPAREN ) {
		NextToken();
		Expression();

		if( type == TYPE_RPAREN ) {
			NextToken();
		} else {
			error(22);
		}
	} else {
		error(23);
	}
}

void Term() {
	Factor();
	while( type == TYPE_MULTIPLY || type == TYPE_DIVIDE ) {
		TOKEN_TYPE mulop = type;
		
		NextToken();
		Factor();

		if(mulop == TYPE_MULTIPLY) {
			gen(OPR, 0, 4);
		} else {
			gen(OPR, 0, 5);
		}
	}
}

void Expression() {

	if( type == TYPE_PLUS || type == TYPE_MINUS ) {
		TOKEN_TYPE addop = type;

		NextToken();
		Term();

		if(addop == TYPE_MINUS) {
			gen(OPR, 0, 1);
		}
	} else {
		Term();
	}

	while( type == TYPE_PLUS || type == TYPE_MINUS ) {
		TOKEN_TYPE addop = type;
		
		NextToken();
		Term();

		if(addop == TYPE_PLUS) {
			gen(OPR, 0, 2);
		} else {
			gen(OPR, 0, 3);
		}
	}
}

void Condition() {
	if( strcmp("odd", token) == 0 ) {
		NextToken();
		Expression();

		gen(OPR, 0, 6);
	} else {
		Expression();

		if( type != TYPE_EQUAL && 
			type != TYPE_NOT_EQUAL &&
			type != TYPE_LESS && 
			type != TYPE_LESS_EQUAL && 
			type != TYPE_GREATER && 
			type != TYPE_GREATER_EQUAL ) {
			error(20);
		} else {
			TOKEN_TYPE relop = type;
			
			NextToken();
			Expression();

			switch(relop) {
			case TYPE_EQUAL:
				gen(OPR, 0, 8);
				break;
			case TYPE_NOT_EQUAL:
				gen(OPR, 0, 9);
				break;
			case TYPE_LESS:
				gen(OPR, 0, 10);
				break;
			case TYPE_GREATER_EQUAL:
				gen(OPR, 0, 11);
				break;
			case TYPE_GREATER:
				gen(OPR, 0, 12);
				break;
			case TYPE_LESS_EQUAL:
				gen(OPR, 0, 13);
				break;
			}
		}
	}
}

void Statement() {

	if( strcmp("call", token) == 0 ) {
		NextToken();

		if(type != TYPE_IDENTIFIER ) {
			error(14);
		} else {

			int index = FindSymbol(token);
			if(index >= 0 && SYMTAB.symtab[index].type == SYM_PROCEDURE){
				
				gen(CAL, lev - SYMTAB.symtab[index].level, SYMTAB.symtab[index].addr);

			}else{
				error(15);
			}

			NextToken();
		}
	} else if( strcmp("if", token) == 0 ) {
		int cx1;

		NextToken();
		Condition();

		if( strcmp(token, "then") == 0 ) {
			NextToken();
		} else {
			error(16);
		}

		cx1 = code.cx;
		gen(JPC, 0, 0);

		Statement();

	} else if( strcmp("begin", token) == 0 ) {
		do {
			NextToken();
			Statement();
		} while(type == TYPE_SEMICOLON);

		if( strcmp("end", token) == 0 ) {
			NextToken();
		} else {
			error(17);
		}
		
	} else if( strcmp("while", token) == 0 ) {
		int cx1 = code.cx;
		int cx2;

		NextToken();
		Condition();

		cx2 = code.cx;
		gen(JPC, 0, 0);

		if( strcmp("do", token) == 0 ) {
			NextToken();
		} else {
			error(18);
		}

		Statement();
		gen(JMP, 0, cx1);
		code.inst[cx2].disp = code.cx;
	
	} else if( type == TYPE_IDENTIFIER ) {
		// TODO : check if symbol alive
		int index = FindSymbol(token);

		if(index < 0) {
			error(11);

		} else {

			if(SYMTAB.symtab[index].type != SYM_VAR) {
				error(12);
			}
		}

		NextToken();
		if( type == TYPE_ASSIGN ) {
			NextToken();
		} else {
			error(13);
		}

		Expression();

		if(index >= 0){
			gen(STO, lev - SYMTAB.symtab[index].level, SYMTAB.symtab[index].addr);
		}
	}
}

void ConstDeclaration() {

	if( type == TYPE_IDENTIFIER ) {
		char const_name[64];
		strcpy(const_name, token);

		NextToken();
		if( type == TYPE_EQUAL ) {
			NextToken();
			if( type == TYPE_NUMBER ) {
				// SYMTAB ¿¡ insert
				enter(const_name, SYM_CONST, num);
				NextToken();
			} else {
				error(2);
			}
		} else{
			error(3);
		}
	} else {
		error(4);
	}
}

void VarDeclaration() {

	if( type == TYPE_IDENTIFIER ) {
		// TODO : enter
		enter(token, SYM_VAR, -1);
		NextToken();
	} else {
		error(4);
	}
}

void Block() {
	int cx0;
	int dx = 3;
	int tx0 = SYMTAB.tx;

	level_up();

	if(SYMTAB.tx - 1 > 0)
		SYMTAB.symtab[SYMTAB.tx - 1].addr = code.cx;
	gen(JMP, 0, 0);

	if( strcmp(token, "const") == 0 ) {
		do {
			NextToken();
			ConstDeclaration();
		} while( type == TYPE_COMMA );

		if( type == TYPE_SEMICOLON ) {
			NextToken();
		}
		else {
			error(5);
		}
	}

	if( strcmp(token, "var") == 0 ) {
		do {
			NextToken();
			VarDeclaration();
		} while( type == TYPE_COMMA );

		if( type == TYPE_SEMICOLON ) {
			NextToken();
		} else {
			error(5);
		}
	}

	while( strcmp(token, "procedure") == 0 ) {

		NextToken();
		if(type == TYPE_IDENTIFIER) {
			enter(token, SYM_PROCEDURE, -1);
			NextToken();
		} else {
			error(4);
		}

		if( type == TYPE_SEMICOLON ) {
			NextToken();
		} else {
			error(5);
		}

		Block();
		
		if( type == TYPE_SEMICOLON ) {
			NextToken();
		} else {
			error(5);
		}
	}

	code.inst[SYMTAB.symtab[tx0].addr].disp = code.cx;
	if(tx0 - 1 > 0)
		SYMTAB.symtab[tx0 -1].addr = code.cx;

	cx0 = code.cx;
	Statement();
	gen(OPR, 0, 0);

	level_down();

	return ;
}

/* and other functions */

void SetUP() {

	fp = fopen("input.txt", "r");
	while( isspace(ch = fgetc(fp)) ){}

	SYMTAB.tx	= 0;
	code.cx		= 0;
}

void CleanUP() {

	fclose(fp);
}

void printCode() {

	int i = 0;

	printf("<CODE>\n");

	for(i = 0; i < code.cx; i++) {

		switch(code.inst[i].opcode){
		case LIT:
			printf("LIT");
			break;
		case OPR:
			printf("OPR");
			break;
		case LOD:
			printf("LOD");
			break;
		case STO:
			printf("STO");
			break;
		case CAL:
			printf("CAL");
			break;
		case INT:
			printf("INT");
			break;
		case JMP:
			printf("JMP");
			break;
		case JPC:
			printf("JPC");
			break;
		}

		printf(" %d, %d\n", code.inst[i].level, code.inst[i].disp);
	}
}

int main() {
	
	{
		SetUP();
	}

	{
		/* main */
		NextToken();
		lev = -1;
		Block();

		if( strcmp(".", token) != 0 ) {
			error(9);
		}

		printf("NO Error found\n");
		printCode();
	}

	{
		CleanUP();
	}
	return 0;
}