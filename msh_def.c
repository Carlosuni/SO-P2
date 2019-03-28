/*-
 * msh.c
 *
 * Minishell C source
 * Show how to use "obtain_order" input interface function
 *
 * THIS FILE IS TO BE MODIFIED
 */

#include <stddef.h>			/* NULL */
#include <stdio.h>			/* setbuf, printf */

extern int obtain_order();		/* See parser.y for description */

int main(void)
{
	char ***argvv;
	int command_counter;
	int num_commands;
	int args_counter;
	char *filev[3];
	int bg;
	int ret;

	setbuf(stdout, NULL);			/* Unbuffered */
	setbuf(stdin, NULL);

	while (1) 
	{
		fprintf(stderr, "%s", "msh> ");	/* Prompt */
		ret = obtain_order(&argvv, filev, &bg);
		if (ret == 0) break;		/* EOF */
		if (ret == -1) continue;	/* Syntax error */
		num_commands = ret - 1;		/* Line */
		if (num_commands == 0) continue;	/* Empty line */

/*
 * THE PART THAT MUST BE REMOVED STARTS HERE
 * THE FOLLOWING LINES ONLY GIVE AN IDEA OF HOW TO USE THE STRUCTURES
 * argvv AND filev. THESE LINES MUST BE REMOVED.
 */

		for (command_counter = 0; command_counter < num_commands; command_counter++) 
		{
			for (args_counter = 0; (argvv[command_counter][args_counter] != NULL); args_counter++)
			{
				printf("%s ", argvv[command_counter][args_counter]);
			}
			printf("\n");
		}
	
		if (filev[0] != NULL) printf("< %s\n", filev[0]);/* IN */

		if (filev[1] != NULL) printf("> %s\n", filev[1]);/* OUT */
	
		if (filev[2] != NULL) printf(">& %s\n", filev[2]);/* ERR */
	
		if (bg) printf("&\n");

/*
 * END OF THE PART TO BE REMOVED
 */
	
	} //fin while 

	return 0;

} //end main

