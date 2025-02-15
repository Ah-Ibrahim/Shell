
/*
 * CS-413 Spring 98
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */


%{
	extern "C" {		
		int yylex();
		void yyerror (char const *s);
	}
	#define yylex yylex
	#include <stdio.h>
	#include "command.h"
%}

%union	{char* string_val;}
%token	<string_val> WORD

%token 	NOTOKEN GREAT GREAT_GREAT LESS NEWLINE ERROR_GREAT ERROR_GREAT_GREAT PIPE	AMPERSAND

%%

goal: commands
	;

commands: command
		| commands command 
		;

command	: simple_command_list ioemodifier_opt BACKGROUND NEWLINE	{
															printf("   Yacc: Execute command\n");
															Command::_currentCommand.execute();
														}
		| NEWLINE	{
						Command::_currentCommand.execute();
					}
		| error NEWLINE	{ 
							yyerrok; 
						}
        ;

		

simple_command_list	: simple_command
					| simple_command_list PIPE simple_command
					;

simple_command 	: command_and_args	{
										Command::_currentCommand.insertSimpleCommand( Command::_currentSimpleCommand );
									}
				;

command_and_args: command_word arg_list	
				;

arg_list: arg_list argument
		| /* can be empty */
		;

argument: WORD 	{
					printf("   Yacc: insert argument \"%s\"\n", $1);
					Command::_currentSimpleCommand->insertArgument( $1 );
				}
		;

command_word: WORD 	{
						printf("   Yacc: insert command \"%s\"\n", $1);
						Command::_currentSimpleCommand = new SimpleCommand();
						Command::_currentSimpleCommand->insertArgument( $1 );
					}	
			;

ioemodifier_opt	: imodifier_opt omodifier_opt emodifier_opt
				;


imodifier_opt	: LESS WORD	{
								printf("   Yacc: insert input \"%s\"\n", $2);
								Command::_currentCommand._inputFile = $2;
							}
				| /* Empty */
				;


omodifier_opt	: GREAT WORD 		{
										printf("   Yacc: insert output \"%s\"\n", $2);
										Command::_currentCommand._outFile = $2;
									}
				| GREAT_GREAT WORD	{
										printf("   Yacc: insert output append \"%s\"\n", $2);
										Command::_currentCommand._outFile = $2;
										Command::_currentCommand._appendOut = 1;
									}
				| /* Empty */
				;

emodifier_opt	: ERROR_GREAT WORD 	{
										printf("   Yacc: insert error \"%s\"\n", $2);
										Command::_currentCommand._errFile = $2;	
									}
				| ERROR_GREAT_GREAT WORD	{
												printf("   Yacc: insert error \"%s\"\n", $2);
												Command::_currentCommand._errFile = $2;	
												Command::_currentCommand._appendErr = 1;
											}
				| /* Empty */
				;

BACKGROUND	: AMPERSAND	{
							Command::_currentCommand._background = 1;
					}
			| /* EMPTY*/
			;
%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif
