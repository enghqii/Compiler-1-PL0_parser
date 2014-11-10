#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
//#include <stdbool.h>

#include "parser.h"

char keyWords[][10] = { "const", "var", "procedure", "call", "begin", "end", "if", "then", "while", "do", "odd"};

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

/* GLOBAL vars */
FILE *		fp			= NULL;

/* lex */
extern int		yylex();
extern FILE *	yyin;
extern char		token[128];
extern int		num;

TOKEN_TYPE	type		= TYPE_NONE;
int			lev			= 0;
int			dx[MAX_LEVEL]= {0,};

SymbolTable	SYMTAB;
Code		code;

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
	if(lev + 1 > MAX_LEVEL){
		// abort.
	}
	lev ++;
}

void level_down() {
	//print_symboltable();
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

	//printf("enter : [%s]\n", name);

	if( FindSymbol(name) < 0 ) {
		Symbol s;

		strcpy(s.name, name);
		s.type	= type;
		s.level = lev;
		s.addr	= addr;

		if(type == SYM_VAR) {
			s.addr = dx[lev];
			dx[lev]++;
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
	if( c == '+' || c == '-' || c == '*' || c == '/' || c == ',' ||
		c == '=' || c == ';' || c == '.' || c == '(' || c == ')' ||
		c == ':' || c == '>' || c == '<') {
			return TRUE;
	}
	return FALSE;
}

/* has side effect on the 'token' and 'num'*/
int NextToken() {

	int i = yylex();
	printf("now token is : [%s]", token);

	if( isdigit(token[0]) ){
		type = TYPE_NUMBER;

	} else if ( isalpha(token[0]) ) { 

		if(isKeyWord(token)) {
			type = TYPE_KEYWORD;
		} else {
			type = TYPE_IDENTIFIER;
		}

	} else if( isSpecialChar(token[0]) ) {

		type = (TOKEN_TYPE)token[0];

		if( strcmp(token, ":=") == 0 ) {
			type = TYPE_ASSIGN;
		} else if ( strcmp(token, "<=") == 0 ) {
			type = TYPE_LESS_EQUAL;
		} else if ( strcmp(token, "<=") == 0 ) {
			type = TYPE_GREATER_EQUAL;
		} else if ( strcmp(token, "<=") == 0 ) {
			type = TYPE_NOT_EQUAL;
		}

	} else {
		type = TYPE_NONE;
	}

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
		code.inst[cx1].disp = code.cx;

	} else if( strcmp("begin", token) == 0 ) {
		do {
			NextToken();
			Statement();
		} while(type == TYPE_SEMICOLON);

		__asm{nop}

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
		enter(token, SYM_VAR, -1);
		NextToken();
	} else {
		error(4);
	}
}

int mainAddr = 0;

void Block() {
	int cx0;
	int tx0 = SYMTAB.tx;

	level_up();
	dx[lev] = 3;


	if(tx0 - 1 >= 0)
		SYMTAB.symtab[tx0 - 1].addr = code.cx; // set proc addr to now code index
	else
		mainAddr = code.cx;

	gen(JMP, 0, 0); // set disp later -- 1

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

	if(tx0 - 1 >= 0){
		code.inst[SYMTAB.symtab[tx0 -1].addr].disp = code.cx; // set JMP disp -- 1
		SYMTAB.symtab[tx0 -1].addr = code.cx;

	}else{
		code.inst[mainAddr].disp = code.cx;
		mainAddr = code.cx;
	}

	cx0 = code.cx;
	gen(INT, 0, dx[lev]); // local var alloc
	Statement();
	gen(OPR, 0, 0);

	level_down();

	return ;
}

/* interpreter */

void print_a_code(int p, Instruction* inst){

	printf("[%3d] ", p);

	switch(inst->opcode){
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

	printf(" %d, %d\n", inst->level, inst->disp);
}

void printCode() {

	int i = 0;

	printf("\n<CODE>\n");

	for(i = 0; i < code.cx; i++) {
		print_a_code(i, &code.inst[i]);
	}
	printf("=================================\n");
}

void print_stack(int *S, int t){
	int i = 0;
	printf("\n<Print Stack Trace>\n");
	for(i = 1; i <= t; i++){
		printf("[%d] %d\n", i, S[i]);
	}
	printf("=================================\n");
}

int base(int level, int * S, int b){
	int b1 = b;

	while(level > 0){
		b1 = S[b1];
		level--;
	}
	return b1;
}

void interpret(){

	int S[512] = {0,};
	int p = 0; // PC
	int t = 0; // top
	int b = 1; // base

	// I won't use 0 index
	S[0] = -1;

	do{
		Instruction inst = code.inst[p];
		p = p + 1;

		if(p > code.cx){
			printf("Force break\n");
			break;
		}

		switch (inst.opcode){
		case LIT:
			t++;
			S[t] = inst.disp;
			break;
		case OPR:

			switch(inst.disp){
			case 0:
				print_stack(S, t);
				t = b - 1;
				p = S[t+3];
				b = S[t+2];
				break;
			case 1:
				S[t] = -S[t];
				break;
			case 2:
				t--;
				S[t] = S[t] + S[t+1];
				break;
			case 3:
				t--;
				S[t] = S[t] - S[t+1];
				break;
			case 4:
				t--;
				S[t] = S[t] * S[t+1];
				break;
			case 5:
				t--;
				if(S[t+1] != 0)
					S[t] = S[t] / S[t+1];
				else{
					printf("DIV by 0\n");
					exit(-1);
				}
				break;
			case 6:
				S[t] = (S[t] % 2 == 0 ? 0 : 1);
				break;
			case 8:
				t--;
				S[t] = (S[t] == S[t+1]);
				break;
			case 9:
				t--;
				S[t] = (S[t] != S[t+1]);
				break;
			case 10:
				t--;
				S[t] = (S[t] < S[t+1]);
				break;
			case 11:
				t--;
				S[t] = (S[t] >= S[t+1]);
				break;
			case 12:
				t--;
				S[t] = (S[t] > S[t+1]);
				break;
			case 13:
				t--;
				S[t] = (S[t] <= S[t+1]);
				break;
			default:
				printf("Invalid OPR disp");
				exit(-1);
			}

			break;
		case LOD:
			t++;
			{
				int bas = base(inst.level, S, b);
				S[t] = S[bas + inst.disp];
			}
			break;
		case STO:
			{
				int bas = base(inst.level, S, b);
				S[bas + inst.disp] = S[t];
			}
			t--;
			break;
		case CAL:
			S[t+1] = base(inst.level, S, b);
			S[t+2] = b;
			S[t+3] = p;
			b = t+1;
			p = inst.disp;
			break;
		case INT:
			t = t + inst.disp;
			break;
		case JMP:
			p = inst.disp;
			break;
		case JPC:
			if(S[t] == 0){
				p = inst.disp;
			}
			t--;
			break;
		default:
			printf("Unknown opcode. abort.\n");
			exit(-1);
		}

	}while(p != 0);

	printf("execution done\n");
}

/* and other functions */

int SetUP() {

	if(fp = fopen("input.txt", "r")){
		//while( isspace(ch = fgetc(fp)) ){}
		yyin = fp;

		SYMTAB.tx	= 0;
		code.cx		= 0;

		return 0;
	}else{
		return -1;
	}
}

void CleanUP() {
	fclose(fp);
}

int main() {

	if(SetUP() < 0){
		printf("no input file.\n");
		return -1;
	}

	{
		/* main */
		NextToken();
		lev = -1;
		Block();

		if( strcmp(".", token) != 0 ) {
			error(9);
		}

		printf("Compile : NO Error found\n");
		printCode();
		interpret();
	}

	{
		CleanUP();
	}
	return 0;
}