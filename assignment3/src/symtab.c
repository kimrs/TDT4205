#include "symtab.h"
#include <stdlib.h>

// These variables is located in vslc.c
extern int arch;
extern int outputStage; 

static char **strings, **more_strings;

static int strings_size = 100 	/*
								 * currently set to 100 as memoryleakage prevents the 
								 * re-allocation of memory. However, the expansion code
								 * should work.
								 */
, strings_index = -1;


void symtab_init ( void )
{
    strings = malloc(strings_size * sizeof(*strings));
}

void symtab_finalize ( void )
{
	for(int i = 0; i <= strings_index; i++)
		free(strings[i]);

	free(strings);
	
}
/*
 * Have some problems reallocating. Might be because of memory leakage.
 * works fine when not simplifying.
 */
int strings_add ( char *str )
{	strings_index++;
    if(outputStage == 7)
        fprintf ( stderr, "Add strings (%s), index: %d \n", str, strings_index );
	
	if(strings_index >= strings_size)//Expand table
	{
		//http://www.cplusplus.com/reference/cstdlib/realloc/1
		strings_size *= 2;
	
		more_strings = realloc(strings, strings_size * sizeof(char*));
		
		if(strings != NULL) 
		{
			strings = more_strings;
			//fprintf(stderr, "Reallocation sucsessfull\n");
		} else {
			fprintf(stderr, "Error reallocating, index: %d\n", strings_index);
			free(strings);
			exit (1);
		}
	}
	strings[strings_index] = str;
	
 	return strings_index;   
}

// Prints the data segment of the assembly code
// which includes all the string constants found
// ARM and x86 have different formats
void strings_output ( FILE *stream )
{
    if(arch == 1) { //ARM
    	 fputs (
			".syntax unified\n"
			".cpu cortex-a15\n"
			".fpu vfpv3-d16\n"
			".data\n"
			".align	2\n"
			".DEBUG: .ascii \"Hit Debug\\n\\000\"\n"
			".DEBUGINT: .ascii \"Hit Debug, r0 was: %d\\n\\000\"\n"
		    ".INTEGER: .ascii \"%d \\000\"\n"
			".FLOAT: .ascii \"%f \\000\"\n"
			".NEWLINE: .ascii \"\\n \\000\"\n",
		    stream
		);
		for ( int i=0; i<=strings_index; i++ ) {
		    fprintf ( stream, ".STRING%d: .ascii %s\n", i, strings[i] );
		    fprintf ( stream, ".ascii \"\\000\"\n", i, strings[i] ); // ugly hack
		}
		fputs ( ".globl main\n", stream );
		fputs ( ".align	2\n", stream );
    }
    else { //x86
		fputs (
		    ".data\n"
		    ".INTEGER: .string \"%d \"\n"
			".FLOAT: .string \"%f \"\n"
			".NEWLINE: .string \"\\n \"\n",
		    stream
		);
		for ( int i=0; i<=strings_index; i++ )
		    fprintf ( stream, ".STRING%d: .string %s\n", i, strings[i] );
		fputs ( ".globl main\n", stream );
    }
}

