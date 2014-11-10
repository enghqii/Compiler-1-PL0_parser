%{

char 	token[128] = "";
int 	num = 0;
void 	install_token();
void 	install_number();

%}

delim	[ \t\n]
ws 		{delim}+
letter	[a-zA-Z]
digit	[0-9]
id 		{letter}({letter}|{digit})*
number 	{digit}+

%%

{ws} 		{ }
odd 		{install_token();return 1;}
begin 		{install_token();return 1;}
end 		{install_token();return 1;}
if 			{install_token();return 1;}
then 		{install_token();return 1;}
else 		{install_token();return 1;}
while 		{install_token();return 1;}
do 			{install_token();return 1;}
call 		{install_token();return 1;}
const 		{install_token();return 1;}
var 		{install_token();return 1;}
procedure 	{install_token();return 1;}
{id} 		{install_token();return 1;}
{number} 	{install_token();install_number();return 1;}
"+" 		{install_token();return 1;}
"-" 		{install_token();return 1;}
"*" 		{install_token();return 1;}
"/" 		{install_token();return 1;}
"=" 		{install_token();return 1;}
"<>" 		{install_token();return 1;}
"<" 		{install_token();return 1;}
">" 		{install_token();return 1;}
"<=" 		{install_token();return 1;}
">=" 		{install_token();return 1;}
"(" 		{install_token();return 1;}
")" 		{install_token();return 1;}
"," 		{install_token();return 1;}
";" 		{install_token();return 1;}
"." 		{install_token();return 1;}
":=" 		{install_token();return 1;}

%%

void install_token()
{
	strcpy(token, yytext);
}
void install_number()
{
	num = atoi(yytext);
}