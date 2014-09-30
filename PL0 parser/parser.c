#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
//#include <stdbool.h>

typedef enum
{
	NONE = 0,
	IDENTIFIER,
	KEYWORD,
	NUMBER,
	OTHER,
} TOKEN_TYPE;

FILE * fp;
char ch = '\0';  // current char

char token[128] = "";
TOKEN_TYPE type = NONE;
int num = 0; // be set if type is NUMBER

void error(int err)
{
	fprintf(stderr, "ERROR %d\n", err);
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
		type = NUMBER;
	}
	else if( isalpha(ch) )
	{
		while( isalpha(ch) || isdigit(ch) )
		{
			token[tokenIndex++] = ch;
			ch = fgetc(fp);
		}
		token[tokenIndex] = '\0';
		type = IDENTIFIER;
	}
	else if( ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == ',' || ch == '=' || ch == ';' || ch == ':' || ch == '<' || ch == '>')
	{
		token[0] = ch;
		token[1] = '\0';
		type = OTHER;

		ch = fgetc(fp);
	}
	else if(ch == EOF)
	{
		return -1;
	}

	else
	{
		// dummy
		printf("CH : %d(%c)\n", ch, ch);
		type = OTHER;

		ch = fgetc(fp);
	}

	// eliminate white spaces
	while( isspace(ch) ){ ch = fgetc(fp); }
	return 0;
}

void Block()
{
}

void Statement()
{
}

void Condition()
{
}

void Expression()
{
}

void Term()
{
}

void Factor()
{
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
		/* main */
		while( !(NextToken() < 0) )
		{
			printf("%s\t\t", token);

			switch (type)
			{
			case IDENTIFIER:
				printf("IDENTIFIER\n");
				break;
			case KEYWORD:
				printf("KEY WORD\n");
				break;
			case NUMBER:
				printf("NUMBER\n");
				break;
			case OTHER:
				printf("OTHER\n");
				break;
			}
		}
	}

	{
		CleanUP();
	}
	return 0;
}