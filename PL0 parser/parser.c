#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
//#include <stdbool.h>

//TODO : keyword 일떄 IDENTIFIER로 판단 X

typedef enum
{
	TYPE_NONE = 0,

	TYPE_IDENTIFIER,
	TYPE_KEYWORD,
	TYPE_NUMBER,

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
	
	TYPE_ASSIGN,
	TYPE_GREATER_EQUAL,
	TYPE_LESS_EQUAL,
	TYPE_NOT_EQUAL,

	TYPE_OTHER,

} TOKEN_TYPE;

FILE *		fp;
char		ch			= '\0';  // current char

TOKEN_TYPE	type		= TYPE_NONE;
char		token[128]	= "";
int			num			= 0;

void error(int err)
{
	fprintf(stderr, "ERROR %d\n", err);
	exit(-1);
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
		type = TYPE_IDENTIFIER;
	}
	else if( ch == '+' || ch == '-' || ch == '*' ||	ch == '/' || ch == ',' || 
		ch == '=' || ch == ';' || ch == '.' || ch == '(' || ch == ')' ||
		ch == ':' || ch == '>' || ch == '<')
	{
		char before = ch;
		token[0] = ch;
		token[1] = '\0';
		type = (TOKEN_TYPE)ch;

		ch = fgetc(fp);

		if( before == ':' && ch == '=' )
		{
			// assign
			token[1] = ch;
			token[2] = '\0';
			type = TYPE_ASSIGN;

			ch = fgetc(fp);
		}
		else if( before == '<' && ch == '=')
		{
			// LT
			token[1] = ch;
			token[2] = '\0';
			type = TYPE_LESS_EQUAL;

			ch = fgetc(fp);
		}
		else if( before == '>' && ch == '=')
		{
			// GT
			token[1] = ch;
			token[2] = '\0';
			type = TYPE_GREATER_EQUAL;

			ch = fgetc(fp);
		}
		else if( before == '<' && ch == '>')
		{
			// neq
			token[1] = ch;
			token[2] = '\0';
			type = TYPE_NOT_EQUAL;

			ch = fgetc(fp);
		}

	}
	else if(ch == EOF)
	{
		return -1;
	}

	else
	{
		// A char i can't parse
		printf("CH : %d(%c)\n", ch, ch);
		type = TYPE_NONE;

		ch = fgetc(fp);
	}

	printf("NOW token %s\n", token);

	// eliminate white spaces
	while( isspace(ch) ){ ch = fgetc(fp); }
	return 0;
}

void Expression();

void Factor()
{
	int i = 0;

	if( type == TYPE_IDENTIFIER )
	{
		// TODO
		NextToken();
	}

	if( type == TYPE_NUMBER )
	{
		NextToken();
	}

	if( type == TYPE_LPAREN )
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

	if( strcmp("if", token) == 0 )
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

	if( strcmp("begin", token) == 0 )
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

	if( strcmp("while", token) == 0 )
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

	if( type == TYPE_IDENTIFIER )
	{
		// TODO : check if symbol alive

		NextToken();
		if( type == TYPE_ASSIGN )
		{
			NextToken();
		}
		else
			error(13);
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
				// SYMTAB 에 insert
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
	if( strcmp(token, "procedure") == 0 )
	{
		NextToken();
		if(type == TYPE_IDENTIFIER)
		{
			// TODO : enter
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
}

void SetUP()
{
	fp = fopen("input.txt", "r");
	while( isspace(ch = fgetc(fp)) ){}
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
		NextToken();
		Block();
		if( strcmp(".", token) != 0 )
		{
			error(9);
		}

		/* main */
		/*
		while( !(NextToken() < 0) )
		{
			printf("%s\t\t", token);

			switch (type)
			{
			case TYPE_IDENTIFIER:
				printf("IDENTIFIER\n");
				break;
			case TYPE_KEYWORD:
				printf("KEY WORD\n");
				break;
			case TYPE_NUMBER:
				printf("NUMBER\n");
				break;
			case TYPE_OTHER:
				printf("OTHER\n");
				break;
			default:
				printf("%c\n", type);
				break;
			}
		}
		*/
	}

	{
		CleanUP();
	}
	return 0;
}