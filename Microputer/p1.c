////////////////////////////////////////////////////////////////////////////////
/// Simulates a CPU executing machine code and converting to assembly
/// @author Thomas Pelegrin
/// @date 09.19.2023
////////////////////////////////////////////////////////////////////////////////

/***************************** Imports ****************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "microputer.h"

/************************* Program Functions **********************************/

/// @brief Sets up everything you need and starts the program
/// @param in_bin_file the file to read the machine code from
/// @param out_asm_file the file to write the assembly to
/// @return 1 if SUCCESS, otherwise ERROR
int start(char * in_bin_file, char * out_asm_file)
{
    int result = SUCCESS;
    microputer_t * mp_p = NULL;

    printf( "\nInput File (machine code): \t'%s'\n", in_bin_file );
    printf( "Output File   (.asm file): \t'%s'\n\n", out_asm_file );

    /* Memory allocation for the microputer struct */
    result = create_microputer( &mp_p );
    if (result != SUCCESS)
    {
        printf( "\t# [ERROR: attempting to create microputer] #\n" );
        goto FUNC_EXIT;
    }

    /* Creates the pre-defined instruction set used for this project */
    create_instruction_set( mp_p );

    /* Disassemble the machine code into .asm file & load program into memory */
    result = disassemble_micro_program( mp_p, in_bin_file, out_asm_file );
    if (result != SUCCESS)
    {
        printf( "\t# [ERROR: loading machine code program into memory] #\n" );
        goto FUNC_EXIT;
    }

    result = execute_micro_program( mp_p );
    if (result != SUCCESS)
    {
        printf( "\t# [ERROR: executing program's instructions] #\n" );
        goto FUNC_EXIT;
    }

    FUNC_EXIT:

    /* Freeing memory allocation and prepares to exit */
    result = delete_microputer( &mp_p );
    if (result != SUCCESS)
    {
        printf( "\t# [ERROR: attempting to delete microputer] #\n" );
    }

    return result;
}

/// @brief Runs the program for the 3 provided test files
/// @return if all succeed SUCCESS, otherwise ERROR
int run_default_tests() 
{
    int result = SUCCESS;
    char in_bin_files[ 3 ][ 34 ] = { 
        { "project1_reference_files/inp1.dat" }, 
        { "project1_reference_files/inp2.dat" }, 
        { "project1_reference_files/inp3.dat" } };
    char out_asm_files[ 3 ][ 25 ] = {
        { "disassembled_output1.asm" },
        { "disassembled_output2.asm" },
        { "disassembled_output3.asm" }
    };

    for (int i = 0; i < 3; i++)
    {
        result = start( in_bin_files[ i ], out_asm_files[ i ] );
        if (result != SUCCESS)
        {
            break;
        }
    }
    return result;
}

/// @brief Main function for running the program
/// @param argc the argument count
/// @param argv for the input file and output file, in that order
/// @return 1 if SUCCESS, otherwise ERROR
int main(int argc, char *argv[])
{
    int result = SUCCESS;
    char * in_bin_file = "project1_reference_files/inp3.dat";
    char * out_asm_file = "disassembled_output.asm";

    if (argc >= 2 && strlen( argv[ 1 ] ) && strlen( argv[ 2 ] ))
    {
        result = start( in_bin_file, out_asm_file );
    } else 
    {
        printf( "\n\t# [WARNING: Files were not specified properly] #\n" );
        printf( "\t# [Executing tests with the 3 default files...] #\n" );
        run_default_tests();
    }
    return result;
}