#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
//#include <stdbool.h>

#define FALSE 0
#define TRUE 1

typedef enum
{
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

typedef enum{
	SYM_NONE,
	SYM_CONST,
	SYM_VAR,
	SYM_PROCEDURE,
} SYMBOL_TYPE;

typedef struct _Symbol{
	char name[128];
	SYMBOL_TYPE type;
	//int level;
} Symbol;

typedef struct _SymbolTable{
	int n;
	Symbol symtab[256];
} SymbolTable;

SymbolTable SYMTAB;

char keyWords[][10] = { "const", "var", "procedure", "call", "begin", "end", "if", "then", "while", "do", "odd"};

FILE *		fp			= NULL;
char		ch			= '\0';  // current char

TOKEN_TYPE	type		= TYPE_NONE;
char		token[128]	= "";
int			num			= 0;

void error(int err)
{
	fprintf(stderr, "ERROR %d\n", err);
	exit(-1);
}

int IsSymbol(SymbolTable SYMTAB, char * name)
{
	int i=0;
	for(i = 0; i < SYMTAB.n; i++)
	{
		if(strcmp(SYMTAB.symtab[i].name, name) == 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}

void enter(SymbolTable SYMTAB, char * name, SYMBOL_TYPE type)
{
	if( IsSymbol(SYMTAB, name) == FALSE ) 
	{
		Symbol s;
		strcpy(s.name, name);
		s.type = type;
		SYMTAB.symtab[SYMTAB.n] = s;
		SYMTAB.n++;
	}
}

int isKeyWord(char * str)
{
	int i = 0;
	for(i = 0; i < sizeof(keyWords) / sizeof(char[10]); i++ )
	{
		if( strcmp(str, keyWords[i]) == 0 )
			return TRUE;
	}
	return FALSE;
}

int isSpecialChar(char c)
{
	if( c == '+' || c == '-' || c == '*' ||	c == '/' || c == ',' || 
		c == '=' || c == ';' || c == '.' || c == '(' || c == ')' ||
		c == ':' || c == '>' || c == '<')
	{
		return TRUE;
	}
	return FALSE;
}

int NextToken()
{
	int tokenIndex = 0;
	token[0] = '\0';

	if( isdigit(ch) )
	{
		while( isdigit(ch) )
		{
			token[tokenIndex++] = ch;
			ch = fgetc(fp);
		}
		token[tokenIndex] = '\0';
		num = atoi(token);
		type = TYPE_NUMBER;
	}
	else if( isalpha(ch) )
	{
		while( isalpha(ch) || isdigit(ch) )
		{
			token[tokenIndex++] = ch;
			ch = fgetc(fp);
		}
		token[tokenIndex] = '\0';

		if(isKeyWord(token))
		{
			type = TYPE_KEYWORD;
		}
		else
		{
			type = TYPE_IDENTIFIER;
		}
	}
	else if( isSpecialChar(ch) )
	{
		char before = ch;
		token[0] = ch;
		token[1] = '\0';
		type = (TOKEN_TYPE)ch;

		ch = fgetc(fp);

		if( before == ':' && ch == '=' )
		{
			token[1] = ch;
			token[2] = '\0';
			type = TYPE_ASSIGN;

			ch = fgetc(fp);
		}
		else if( before == '<' && ch == '=')
		{
			token[1] = ch;
			token[2] = '\0';
			type = TYPE_LESS_EQUAL;

			ch = fgetc(fp);
		}
		else if( before == '>' && ch == '=')
		{
			token[1] = ch;
			token[2] = '\0';
			type = TYPE_GREATER_EQUAL;

			ch = fgetc(fp);
		}
		else if( before == '<' && ch == '>')
		{
			token[1] = ch;
			token[2] = '\0';
			type = TYPE_NOT_EQUAL;

			ch = fgetc(fp);
		}

	}
	else if(ch == EOF)
	{
		return FALSE;
		// error(0);
	}

	else
	{
		// A char i can't parse
		printf("CH : %d(%c)\n", ch, ch);
		type = TYPE_NONE;

		ch = fgetc(fp);
	}

	printf("NEXT token : %s\n", token);

	// eliminate white spaces
	while( isspace(ch) ){ ch = fgetc(fp); }
	return TRUE;
}

// FWD decl
void Expression();

void Factor()
{
	int i = 0;

	if( type == TYPE_IDENTIFIER )
	{
		// TODO
		NextToken();
	}

	else if( type == TYPE_NUMBER )
	{
		NextToken();
	}

	else if( type == TYPE_LPAREN )
	{
		NextToken();
		Expression();

		if( type == TYPE_RPAREN )
		{
			NextToken();
		}
		else
			error(22);
	}
	else
		error(23);

}

void Term()
{
	Factor();
	while( type == TYPE_MULTIPLY || type == TYPE_DIVIDE )
	{
		NextToken();
		Factor();
	}
}

void Expression()
{
	if( type == TYPE_PLUS || type == TYPE_MINUS )
	{
		NextToken();
		Term();
	}
	else
		Term();

	while( type == TYPE_PLUS || type == TYPE_MINUS )
	{
		NextToken();
		Term();
	}
}

void Condition()
{
	if( strcmp("odd", token) == 0 )
	{
		NextToken();
		Expression();
	}
	else
	{
		Expression();

		if( type != TYPE_EQUAL && 
			type != TYPE_NOT_EQUAL &&
			type != TYPE_LESS && 
			type != TYPE_LESS_EQUAL && 
			type != TYPE_GREATER && 
			type != TYPE_GREATER_EQUAL )
		{
			error(20);
		}
		else
		{
			NextToken();
			Expression();
		}
	}
}

void Statement()
{
	if( strcmp("call", token) == 0 )
	{
		NextToken();

		if(type != TYPE_IDENTIFIER )
		{
			error(14);
		}
		else
		{
			// TODO : check if symbol alive
			NextToken();
		}
	}

	else if( strcmp("if", token) == 0 )
	{
		NextToken();
		Condition();

		if( strcmp(token, "then") == 0 )
		{
			NextToken();
		}
		else
			error(16);

		Statement();
	}

	else if( strcmp("begin", token) == 0 )
	{
		do
		{
			NextToken();
			Statement();
		}
		while(type == TYPE_SEMICOLON);

		if( strcmp("end", token) == 0 )
		{
			NextToken();
		}
		else
			error(17);
	}

	else if( strcmp("while", token) == 0 )
	{
		NextToken();
		Condition();
		if( strcmp("do", token) == 0 )
		{
			NextToken();
		}
		else
			error(18);

		Statement();
	}

	else if( type == TYPE_IDENTIFIER )
	{
		// TODO : check if symbol alive

		NextToken();
		if( type == TYPE_ASSIGN )
		{
			NextToken();
		}
		else
			error(13);

		Expression();
	}
}

void ConstDeclaration()
{
	if( type == TYPE_IDENTIFIER )
	{
		NextToken();
		if( type == TYPE_EQUAL )
		{
			NextToken();
			if( type == TYPE_NUMBER )
			{
				// SYMTAB �� insert
				enter(SYMTAB, token, SYM_CONST);
				NextToken();
			}
			else
				error(2);
		}
		else
			error(3);
	}
	else
		error(4);

}

void VarDeclaration()
{
	if( type == TYPE_IDENTIFIER )
	{
		// TODO : enter
		enter(SYMTAB, token, SYM_VAR);
		NextToken();
	}
	else
		error(4);
}

void Block()
{
	if( strcmp(token, "const") == 0 )
	{
		do
		{
			NextToken();
			ConstDeclaration();
		}
		while( type == TYPE_COMMA );

		if( type == TYPE_SEMICOLON )
		{
			NextToken();
		}
		else
			error(5);
	}

	if( strcmp(token, "var") == 0 )
	{
		do
		{
			NextToken();
			VarDeclaration();
		}while( type == TYPE_COMMA );

		if( type == TYPE_SEMICOLON )
		{
			NextToken();
		}
		else
			error(5);
	}

	while( strcmp(token, "procedure") == 0 )
	{
		NextToken();
		if(type == TYPE_IDENTIFIER)
		{
			// TODO : enter
			enter(SYMTAB, token, SYM_PROCEDURE);
			NextToken();
		}
		else
			error(4);

		if( type == TYPE_SEMICOLON )
		{
			NextToken();
		}
		else
			error(5);

		Block();
		
		if( type == TYPE_SEMICOLON )
		{
			NextToken();
		}
		else
			error(5);
	}

	Statement();
	return ;
}

void SetUP()
{
	fp = fopen("input.txt", "r");
	while( isspace(ch = fgetc(fp)) ){}

	SYMTAB.n = 0;
}

void CleanUP()
{
	fclose(fp);
}

int main()
{
	{
		SetUP();
	}

	{
		/* main */
		NextToken();
		Block();

		if( strcmp(".", token) != 0 )
		{
			error(9);
		}
	}

	{
		CleanUP();
	}
	return 0;
}