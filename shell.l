
/*
 *
 * CS-413 Spring 98
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <string.h>
#include "y.tab.h"

%}

%%

\n 	{
		return NEWLINE;
	}

[ \t] 	{
		/* Discard spaces and tabs */
		}	

"<"		{
			return LESS;
		}

">>"	{
			return GREAT_GREAT;
		}

">" 	{
			return GREAT;
		}

"2>>"   {
			return ERROR_GREAT_GREAT;
		}

"2>"	{
			return ERROR_GREAT;
		}

"|"		{
			return PIPE;
		}

"&"		{
			return AMPERSAND;
		}

[^ \t\n][^ \t\n]*  {
		/* Assume that file names have only alpha chars */
		yylval.string_val = strdup(yytext);
		return WORD;
	}

.	{
		/* Invalid character in input */
		return NOTOKEN;
	}

%%
