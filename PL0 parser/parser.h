#ifndef _PARSER_H_
#define _PARSER_H_

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

	/* SYMTAB */
	typedef enum {
		SYM_NONE,
		SYM_CONST,
		SYM_VAR,
		SYM_PROCEDURE,
	} SYMBOL_TYPE;

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

#endif