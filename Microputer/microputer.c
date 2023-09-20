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

/**************************** Constants ***************************************/

/* 1 = test mode enabled, disabled otherwise */
#define TEST_MODE 0

/***************************** Macros *****************************************/

/* Prints the function name at the start of the function */
#define START_FUNC printf( "\n\tENTER: [%s] (Ln.%d)\t+\n", __func__, __LINE__)
/* Prints the function name at the end of the function */
#define END_FUNC printf( "\n\t EXIT: [%s] (Ln.%d)\t-\n", __func__, __LINE__ )

/* Max characters per line in the .asm file */
#define MAX_ASM_LINE_LEN 15

/************************* Testing Utility ************************************/

#ifdef TEST_MODE
/// @brief Converts an unsigned short or char to a bit string
/// @param value the unsigned short or char
/// @param bytes the bytes to print 
void bitify(uint16_t value, byte_t bytes) {
    int count = sizeof( byte_t ) * 8 * bytes;

    for (int i = 0; i < count; i++)
    {
        printf( "%d", 
            ((value << i) & (0x8 << (4 + ((bytes - 1) * 8)))) >> (count - 1) );
        
        if ((i + 1) % 4 == 0 && i != 0 && i != count - 1) {
            printf( " " );
        }
    }
}
#endif

/*******************************************************************************
 *             Microputer Initialization & Termination Functions
 ******************************************************************************/

/// @brief Dynamically allocates memory for a new microputer type
/// @param out_mp_pp a pointer to a pointer of the microputer
/// @return 1 if SUCCESS, otherwise ERROR
int create_microputer(microputer_t ** out_mp_pp)
{
    #if TEST_MODE == 1
        START_FUNC;
    #endif 

    /* Allocate memory for the pointer */
    *out_mp_pp = (microputer_t *) malloc( sizeof( microputer_t ) );
    if (*out_mp_pp == NULL) 
    { 
        printf( "\t# [ERROR: malloc failed to allocate for *out_mp_pp] #\n" );
        return ERROR; 
    }
    /* Wipe any existing memory */
    memset( *out_mp_pp, 0, sizeof( microputer_t ) );

    /* Setting the PC register to the first word boundary of memory */
    (*out_mp_pp)->pc = 0;                            
    /* Setting the IR register to 0 (NIL), no instruction executing currently */
    (*out_mp_pp)->ir = 0;     
    /* Setting the number of currently loaded instructions to 0 */
    (*out_mp_pp)->loaded_mem_slots = 0; 

    #if TEST_MODE == 1
        END_FUNC;
    #endif 

    return SUCCESS;
}

/// @brief Frees the memory allocated for the microputer struct
/// @param mp_pp a pointer to a pointer of the microputer
/// @return 1 if SUCCESS, otherwise ERROR
int delete_microputer(microputer_t ** mp_pp)
{
    #if TEST_MODE == 1
        START_FUNC;
    #endif 

    if (*mp_pp != NULL)
    {
        free( *mp_pp );
        *mp_pp = NULL;          // Setting the pointer to NULL for safety
    } else 
    {
        printf( "\t# [ERROR: Can NOT free( *mp_pp ) *mp_pp is NULL!] #\n" );
        return ERROR;
    }

    #if TEST_MODE == 1
        END_FUNC;
    #endif 

    return SUCCESS;
}

/*******************************************************************************
 *                 Program Assembly and Execution Functions
 ******************************************************************************/

/// @brief Writes the assembly file buffer to a .asm file
/// @param asm_file_buffer_p the assembly file buffer
/// @param asm_file_name_p the name of the file
/// @param lines the number of lines to write to the .asm file
/// @return 1 if SUCCESS, otherwise ERROR
int write_assembly_file(char (*asm_file_buffer_p)[ MAX_ASM_LINE_LEN ], 
    const char * asm_file_name_p, byte_t lines)
{
    #if TEST_MODE == 1
        START_FUNC;
    #endif 

    FILE * file_p = fopen( asm_file_name_p, "w" );  // open file to write
    char mem_num[ 5 ];
    int result = SUCCESS;
    int i = 0;

    #if TEST_MODE == 1 
        printf( "\n\tASSEMBLY INSTRUCTIONS: \n" );
    #endif

    while (i < lines && result != EOF)
    {
        /* Creating and then writing the mem address preface for each line */
        snprintf( mem_num, 5, "%d: ", i * 2 );
        result |= fputs( mem_num, file_p );
        /* Writing the assembly instruction to the .asm file */
        result |= fputs( asm_file_buffer_p[ i ], file_p );

        #if TEST_MODE == 1 
            printf( "%d: %s\n", i * 2, asm_file_buffer_p[ i ] );
        #endif
        if (i < lines - 1)
        {
            /* Writing the new line character */
            result |= fputs( "\n", file_p );
        }
        if (result < 0)
        {
            printf( "\t# [ERROR: Can not write buffer to .asm file!] #\n" );
            goto FUNC_EXIT;
        }
        i++;
    }

    FUNC_EXIT:
    fclose( file_p );

    #if TEST_MODE == 1
        END_FUNC;
    #endif 

    return result;
}

/// @brief Loads the program into memory and disassembles the program
/// @param mp_p microputer pointer
/// @param bin_file_name_p the machine code file
/// @param asm_file_name_p the assembly file  
/// @return 1 if SUCCESS, otherwise ERROR
int disassemble_micro_program(microputer_t * mp_p, 
    const char * bin_file_name_p, const char * asm_file_name_p)
{
    #if TEST_MODE == 1
        START_FUNC;
    #endif 

    FILE * file_p = fopen( bin_file_name_p, "rb" );
    byte_t op_code = 0;
    uint16_t instr = 0;
    int result = SUCCESS;
    int counter = 0;
    /* Declaring a pointer to an array (i.e. 2D array) to use as buffer*/
    char (*asm_file_buffer_p)[ MAX_ASM_LINE_LEN ] = 
        (char (*)[]) malloc( sizeof( asm_file_buffer_p ) 
            * (MEM_BYTE_SIZE / WORD_SIZE) );

    if (!file_p) 
    {
        printf( "\t# [ERROR: file '%s' could NOT be opened!] #\n",
            bin_file_name_p );
        result = ERROR;
        goto EXIT_FUNC;
    }
    
    while (fread( &mp_p->mem[ counter ], 
        sizeof( char ), 1, file_p ) == 1 && counter < MEM_BYTE_SIZE) 
    {
        #if TEST_MODE == 1
            printf( "Memory [%d]{ ", counter );
            bitify( mp_p->mem[ counter ], 1 );
            printf( " }\n" );
        #endif

        if ( (counter + 1) % 2 == 0 )
        {
            instr = mp_p->mem[ counter - 1 ] << 8;  // higher order bits
            instr |= mp_p->mem[ counter ];          // lower order bits
            op_code = (byte_t) ((instr & (0b111 << 13)) >> 13);

            #if TEST_MODE == 1
                printf( "Instruction: \n" );
                printf( "\top_code: %d\n", op_code );
                printf( "\tinstr:   " );
                bitify( instr, 2 );
                printf( "\n" );
            #endif
            /* Call the instruction's disassembler; no need to error check
               since any series of bits is translatable */
            (*mp_p->instr_set[ op_code ].disassembler)( 
                instr, asm_file_buffer_p[ counter / 2 ] );
        }   
        counter++;
    }
    /* Setting the PC register to the first word boundary of memory */
    mp_p->pc = 0;        
    mp_p->loaded_mem_slots = counter;   
    write_assembly_file( asm_file_buffer_p, asm_file_name_p, counter / 2 );

EXIT_FUNC:
    fclose( file_p );
    free( asm_file_buffer_p );

    #if TEST_MODE == 1
        END_FUNC;
    #endif 
    
    return result;
}

/// @brief Executes the microprogram on the passed microputer
/// @param mp_p microputer pointer
/// @return 1 if SUCCESS, otherwise ERROR
int execute_micro_program(microputer_t * mp_p)
{
    #if TEST_MODE == 1
        START_FUNC;
    #endif 

    byte_t op_code;
    do
    {
        /* Combining the higher and lower order bits to into one value */
        mp_p->ir = mp_p->mem[ mp_p->pc++ ] << 8;      // higher order bits
        mp_p->ir |= mp_p->mem[ mp_p->pc++ ];          // lower order bits
        op_code = (byte_t) ((mp_p->ir & (0b111 << 13)) >> 13);

        /* Call the instruction's handler and check for runtime errors */
        if ((*mp_p->instr_set[ op_code ].handler)( mp_p ) != SUCCESS)
        {
            printf( "\t# [ERROR: %hu handler returned error code!] #\n", 
                (uint16_t) op_code );
            return ERROR;
        }
    } while (mp_p->pc < mp_p->loaded_mem_slots);

    #if TEST_MODE == 1
        END_FUNC;
    #endif 

    return SUCCESS;
}

/*******************************************************************************
 *                  Instruction Args Extractor Functions
 ******************************************************************************/

/// @brief Extracts the args from the LDI machine code instruction 
/// @param instr the 16-bit binary instruction
/// @param out_instr_args_p pointer containing the extracted args
/// @return 1 if SUCCESS, otherwise ERROR
void ldi_extract_instr_args(uint16_t instr, byte_t * out_instr_args_p)
{
    #if TEST_MODE == 1
        START_FUNC;
    #endif 

    out_instr_args_p[ 0 ] = (byte_t) ((instr & (0xF << 9)) >> 9);   // Ri
    out_instr_args_p[ 1 ] = (byte_t) ((instr & (0xFF << 1)) >> 1);  // imm. data

    #if TEST_MODE == 1
        END_FUNC;
    #endif 
}

/// @brief Extracts the args from the logical/ADD machine code instruction 
/// @param instr the 16-bit binary instruction
/// @param out_instr_args_p pointer containing the extracted args
/// @return 1 if SUCCESS, otherwise ERROR
void bit_op_extract_instr_args(uint16_t instr, byte_t * out_instr_args_p)
{
    #if TEST_MODE == 1
        START_FUNC;
    #endif 

    out_instr_args_p[ 0 ] = (byte_t) ((instr & (0b111 << 13)) >> 13);
    out_instr_args_p[ 1 ] = (byte_t) ((instr & (0xF << 9)) >> 9);
    out_instr_args_p[ 2 ] = (byte_t) ((instr & (0xF << 5)) >> 5);
    out_instr_args_p[ 3 ] = (byte_t) ((instr & (0xF << 1)) >> 1);

    #if TEST_MODE == 1
        END_FUNC;
    #endif 
}

/// @brief Extracts the args from the PRT machine code instruction 
/// @param instr the 16-bit binary instruction
/// @param out_instr_args_p pointer containing the extracted args
void prt_extract_instr_args(uint16_t instr, byte_t * out_instr_args_p)
{
    #if TEST_MODE == 1
        START_FUNC;
    #endif 

    out_instr_args_p[ 0 ] = (byte_t) ((instr & (0xF << 9)) >> 9);

    #if TEST_MODE == 1
        END_FUNC;
    #endif 
}

/// @brief Extracts the args from the RDD machine code instruction 
/// @param instr the 16-bit binary instruction
/// @param out_instr_args_p pointer containing the extracted args
void rdd_extract_instr_args(uint16_t instr, byte_t * out_instr_args_p)
{
    #if TEST_MODE == 1
        START_FUNC;
    #endif 

    out_instr_args_p[ 0 ] = (byte_t) ((instr & (0xF << 9)) >> 9);

    #if TEST_MODE == 1
        END_FUNC;
    #endif 
}

/// @brief Extracts the args from the BLT machine code instruction 
/// @param instr the 16-bit binary instruction
/// @param out_instr_args_p pointer containing the extracted args
void blt_extract_instr_args(uint16_t instr, byte_t * out_instr_args_p)
{
    #if TEST_MODE == 1
        START_FUNC;
    #endif 

    out_instr_args_p[ 0 ] = (byte_t) ((instr & (0xF << 9)) >> 9); // Ri index
    out_instr_args_p[ 1 ] = (byte_t) ((instr & (0xF << 5)) >> 5); // Rj index
    out_instr_args_p[ 2 ] = instr & 0b11111;                      // address

    #if TEST_MODE == 1
        END_FUNC;
    #endif 
}

/*******************************************************************************
 *                     Instruction disassembler Functions
 ******************************************************************************/

/// @brief Assembles the LDI machine code instruction into assembly
/// @param instr the 16-bit binary instruction
/// @param out_line_buffer_p pointer to a assembly instruction string 
void ldi_disassembler(uint16_t instr, char * out_line_buffer_p)
{
    #if TEST_MODE == 1
        START_FUNC;
    #endif 

    byte_t out_instr_args_p[ 2 ];
    ldi_extract_instr_args( instr, out_instr_args_p );
    snprintf( out_line_buffer_p, MAX_ASM_LINE_LEN, "LDI R%hu %hu", 
        (uint16_t) out_instr_args_p[ 0 ], 
        (uint16_t) out_instr_args_p[ 1 ] );

    #if TEST_MODE == 1
        END_FUNC;
    #endif 
}

/// @brief Assembles the logical/ADD machine code instruction into assembly
/// @param instr the 16-bit binary instruction
/// @param out_line_buffer_p pointer to a assembly instruction string 
void bit_op_disassembler(uint16_t instr, char * out_line_buffer_p)
{
    #if TEST_MODE == 1
        START_FUNC;
    #endif 

    byte_t out_instr_args_p[ 4 ];
    char * op_name;
    bit_op_extract_instr_args( instr, out_instr_args_p );

    switch ( out_instr_args_p[ 0 ] )  
    {
        case 0b001: op_name = "ADD"; break;  
        case 0b010: op_name = "AND"; break;    
        case 0b011: op_name = "OR"; break;     
        case 0b100: op_name = "XOR"; break;    
        default: return; 
    }
    snprintf( out_line_buffer_p, MAX_ASM_LINE_LEN, "%s R%hu R%hu R%hu", 
        op_name, 
        (uint16_t) out_instr_args_p[ 1 ], 
        (uint16_t) out_instr_args_p[ 2 ], 
        (uint16_t) out_instr_args_p[ 3 ] );

    #if TEST_MODE == 1
        END_FUNC;
    #endif 
}

/// @brief Assembles the PRT machine code instruction into assembly
/// @param instr 
/// @param out_line_buffer_p pointer to a assembly instruction string 
void prt_disassembler(uint16_t instr, char * out_line_buffer_p)
{
    #if TEST_MODE == 1
        START_FUNC;
    #endif 

    byte_t out_instr_args_p[ 1 ];
    prt_extract_instr_args( instr, out_instr_args_p );
    snprintf( out_line_buffer_p, MAX_ASM_LINE_LEN, "PRT R%hu", 
        (uint16_t) out_instr_args_p[ 0 ] );

    #if TEST_MODE == 1
        END_FUNC;
    #endif 
}

/// @brief Assembles the RDD machine code instruction into assembly
/// @param instr the 16-bit binary instruction
/// @param out_line_buffer_p pointer to a assembly instruction string 
void rdd_disassembler(uint16_t instr, char * out_line_buffer_p)
{
    #if TEST_MODE == 1
        START_FUNC;
    #endif 

    byte_t out_instr_args_p[ 1 ];
    rdd_extract_instr_args( instr, out_instr_args_p );
    snprintf( out_line_buffer_p, MAX_ASM_LINE_LEN, "RDD R%hu", 
        (uint16_t) out_instr_args_p[ 0 ] );

    #if TEST_MODE == 1
        END_FUNC;
    #endif 
}

/// @brief Assembles the BLT machine code instruction into assembly
/// @param instr the 16-bit binary instruction
/// @param out_line_buffer_p pointer to a assembly instruction string
void blt_disassembler(uint16_t instr, char * out_line_buffer_p)
{
    #if TEST_MODE == 1
        START_FUNC;
    #endif 

    byte_t out_instr_args_p[ 3 ];
    blt_extract_instr_args( instr, out_instr_args_p );
    snprintf( out_line_buffer_p, MAX_ASM_LINE_LEN, "BLT R%hu R%hu %hu", 
        (uint16_t) out_instr_args_p[ 0 ], 
        (uint16_t) out_instr_args_p[ 1 ], 
        (uint16_t) out_instr_args_p[ 2 ] );

    #if TEST_MODE == 1
        END_FUNC;
    #endif 
}

/*******************************************************************************
 *                     Instruction Handler Functions
******************************************************************************/

/// @brief Executes the LDI instruction on the passed microputer
/// @param mp_p microputer pointer
/// @return 1 if SUCCESS, otherwise ERROR
int ldi_handler(microputer_t * mp_p) 
{
    #if TEST_MODE == 1
        START_FUNC;
    #endif 
    byte_t out_instr_args_p[ 2 ];
    ldi_extract_instr_args( mp_p->ir, out_instr_args_p );
    byte_t * ri = &mp_p->reg[ out_instr_args_p[ 0 ] ];
    byte_t imm = out_instr_args_p[ 1 ];

    /* Loads data into the specified register */
    *ri = imm;

    #if TEST_MODE == 1
        printf( "LDI R%hu(%hu) %hu\n", 
            (uint16_t) out_instr_args_p[ 0 ], (uint16_t) *ri,
            (uint16_t) imm );
    #endif

    #if TEST_MODE == 1
        END_FUNC;
    #endif

    return SUCCESS;
}

/// @brief Executes the logical/ADD instruction on the passed microputer
/// @param mp_p microputer pointer
/// @return 1 if SUCCESS, otherwise ERROR
int bit_op_handler(microputer_t * mp_p)
{
    #if TEST_MODE == 1
        START_FUNC;
    #endif 

    byte_t out_instr_args_p[ 4 ];
    bit_op_extract_instr_args( mp_p->ir, out_instr_args_p );
    byte_t op_code = out_instr_args_p[ 0 ]; 
    byte_t * ri = &mp_p->reg[ out_instr_args_p[ 1 ] ];
    byte_t * rj = &mp_p->reg[ out_instr_args_p[ 2 ] ];
    byte_t * rk = &mp_p->reg[ out_instr_args_p[ 3 ] ];

    /* Logical op the first two registers together and stores it in the third */
    switch ( op_code )  
    {
        case 0b001: *rk = *ri + *rj; break;     // ADD
        case 0b010: *rk = *ri & *rj; break;     // AND
        case 0b011: *rk = *ri | *rj; break;     // OR
        case 0b100: *rk = *ri ^ *rj; break;     // XOR
        default: return ERROR; 
    }

    #if TEST_MODE == 1
        printf( "%s R%hu(%hu) R%hu(%hu) R%hu(%hu)\n", 
             op_code == 0b001 ? "ADD" : 
                op_code == 0b010 ? "AND" :
                    op_code == 0b011 ? "OR" : "XOR",
            (uint16_t) out_instr_args_p[ 1 ], (uint16_t) *ri,
            (uint16_t) out_instr_args_p[ 2 ], (uint16_t) *rj,
            (uint16_t) out_instr_args_p[ 3 ], (uint16_t) *rk );
    #endif

    #if TEST_MODE == 1
        END_FUNC; 
    #endif

    return SUCCESS;
}

/// @brief Executes the PRT instruction on the passed microputer
/// @param mp_p microputer pointer
/// @return 1 if SUCCESS, otherwise ERROR
int prt_handler(microputer_t * mp_p) 
{
    #if TEST_MODE == 1
        START_FUNC;
    #endif 
    /* Prints the contents of the register (in decimal)*/
    byte_t out_instr_args_p[ 1 ];
    prt_extract_instr_args( mp_p->ir, out_instr_args_p );

    printf( "R%hu = %hu\n", 
        (uint16_t) out_instr_args_p[ 0 ],
         (uint16_t) mp_p->reg[ out_instr_args_p[ 0 ] ] );

    #if TEST_MODE == 1
        END_FUNC;
    #endif 

    return SUCCESS;
}

/// @brief Executes the RDD instruction on the passed microputer
/// @param mp_p microputer pointer
/// @return 1 if SUCCESS, otherwise ERROR
int rdd_handler(microputer_t * mp_p) 
{
    #if TEST_MODE == 1
        START_FUNC;
    #endif 

    byte_t out_instr_args_p[ 1 ];
    rdd_extract_instr_args( mp_p->ir, out_instr_args_p );
    byte_t * ri = &mp_p->reg[ out_instr_args_p[ 0 ] ];
    unsigned int input = 0;
 
    printf( "Enter a value for R%d: ", (uint16_t) out_instr_args_p[ 0 ] ); 
    scanf( "%d", &input );                      // read in an integer
    *ri = (byte_t) (input & 0x00FF) ;     // take lower 8 bits

    #if TEST_MODE == 1
        printf( "RDD R%hu(%hu)\n", 
        (uint16_t) out_instr_args_p[ 0 ], (uint16_t) *ri );
    #endif

    #if TEST_MODE == 1
        END_FUNC;
    #endif 

    return SUCCESS;
}

/// @brief Executes the BLT instruction on the passed microputer
/// @param mp_p microputer pointer
/// @return 1 if SUCCESS, otherwise ERROR
int blt_handler(microputer_t * mp_p) 
{
    #if TEST_MODE == 1
        START_FUNC;
    #endif 

    byte_t out_instr_args_p[ 3 ];
    blt_extract_instr_args( mp_p->ir, out_instr_args_p );
    byte_t ri = mp_p->reg[ out_instr_args_p[ 0 ] ];
    byte_t rj = mp_p->reg[ out_instr_args_p[ 1 ] ];
    byte_t addr = out_instr_args_p[ 2 ];

    /* Jumps to the address if the first register is less than second */
    if ( ri < rj )
    {
        if (addr % 2 != 0)
        {
            printf( " # [ERROR: Address must be on word boundary!] #" );
            return ERROR;
        }
        mp_p->pc = addr;    // set PC register to address (index of the instr)
    }

    #if TEST_MODE == 1
        printf( "BLT R%hu(%hu) R%hu(%hu) %hu  ->  PC(%hu)", 
        (uint16_t) out_instr_args_p[ 0 ], (uint16_t) ri,
        (uint16_t) out_instr_args_p[ 1 ], (uint16_t) rj,
        (uint16_t) addr, (uint16_t) mp_p->pc );
    #endif

    #if TEST_MODE == 1
        END_FUNC;
    #endif

    return SUCCESS;
}

/*******************************************************************************
 *                          Instruction Set Setup
 ******************************************************************************/

/// @brief Creates the instruction set for passed microputer
/// @param mp_p microputer pointer
/// @return 1 if SUCCESS, otherwise ERROR
void create_instruction_set(microputer_t * mp_p) 
{
    #if TEST_MODE == 1
        START_FUNC;
    #endif 

    /* Setting up ADD handler and disassembler; index is the op code */
    mp_p->instr_set[ 0b000 ].disassembler = ldi_disassembler;   
    mp_p->instr_set[ 0b000 ].handler = ldi_handler;      

    /* Setting up ADD handler and disassembler; index is the op code */
    mp_p->instr_set[ 0b001 ].disassembler = bit_op_disassembler;
    mp_p->instr_set[ 0b001 ].handler = bit_op_handler;

    /* Setting up AND handler and disassembler; index is the op code */
    mp_p->instr_set[ 0b010 ].disassembler = bit_op_disassembler;
    mp_p->instr_set[ 0b010 ].handler = bit_op_handler;

    /* Setting up OR handler and disassembler; index is the op code */
    mp_p->instr_set[ 0b011 ].disassembler = bit_op_disassembler;
    mp_p->instr_set[ 0b011 ].handler = bit_op_handler;

    /* Setting up XOR handler and disassembler; index is the op code */
    mp_p->instr_set[ 0b100 ].disassembler = bit_op_disassembler;
    mp_p->instr_set[ 0b100 ].handler = bit_op_handler;

    /* Setting up PRT handler and disassembler; index is the op code */
    mp_p->instr_set[ 0b101 ].disassembler = prt_disassembler;
    mp_p->instr_set[ 0b101 ].handler = prt_handler;

    /* Setting up RDD handler and disassembler; index is the op code */
    mp_p->instr_set[ 0b110 ].disassembler = rdd_disassembler;
    mp_p->instr_set[ 0b110 ].handler = rdd_handler;

    /* Setting up BLT handler and disassembler; index is the op code */
    mp_p->instr_set[ 0b111 ].disassembler = blt_disassembler;
    mp_p->instr_set[ 0b111 ].handler = blt_handler;

    #if TEST_MODE == 1
        END_FUNC;
    #endif 
}