////////////////////////////////////////////////////////////////////////////////
/// Header file to model the microputer
/// @author Thomas Pelegrin
/// @date 09.19.2023
////////////////////////////////////////////////////////////////////////////////

/***************************** Imports ****************************************/

#include "stdint.h"

/**************************** Constants ***************************************/

#define WORD_SIZE 2
#define MAX_INSTR_NAME_SIZE 3
#define NUM_INSTRUCTIONS 8
#define NUM_REGISTERS 16
#define MEM_BYTE_SIZE 32
#define SUCCESS 0
#define ERROR 1

/********************* Microputer Structs & Types *****************************/

/* unsigned char to represent a single byte */
typedef unsigned char byte_t;

struct microputer_s;

/* Represents an instruction */
struct instruction_s {
    int (*handler)(struct microputer_s * mp_p); 
    void (*disassembler)(uint16_t instr, char * out_p);
} typedef instruction_t;

/* Represents a microputer CPU */
struct microputer_s {
    /* Contains pointers to the handlers for each type of instruction */
    instruction_t instr_set[ NUM_INSTRUCTIONS ]; 
    byte_t reg[ NUM_REGISTERS ];     
    byte_t mem[ MEM_BYTE_SIZE ];             
    byte_t loaded_mem_slots;
    uint16_t pc;                                    
    uint16_t ir;                                   
} typedef microputer_t;

/********************* Public Microputer Functions ****************************/

int create_microputer(microputer_t ** mp_pp);
int delete_microputer(microputer_t ** mp_pp);
void create_instruction_set(microputer_t * mp_p);
int disassemble_micro_program(microputer_t * mp_p, 
    const char * bin_file_name_p, const char * asm_file_name_p);
int execute_micro_program(microputer_t * mp_p);