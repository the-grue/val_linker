/*                                  MAINTEST.C                             */
/*  Test file for main.c                                                   */
#include <stdio.h>

typedef unsigned int bit_32;
typedef unsigned short bit_16;
typedef unsigned char byte;

bit_16 link_step = 0;

struct file_info_struct
 {
  byte placeholder[64];
 };

typedef struct file_info_struct file_info_type;
typedef file_info_type *file_info_ptr;
file_info_ptr temp_file;

void primary_linker_initialization(byte *program_directory);
void get_filenames_from_user(bit_16 argc16, byte *argv[]);
void secondary_linker_initialization(void);
void process_library_directories(void);
void process_object_modules(void);
void process_libraries(void);
void order_and_align_segments(void);
void pass_two(void);
void write_executable_image(void);
void link_map(void);
void file_delete(file_info_ptr file_info);
void end_linker(bit_16 return_code);

#include "../main.c"

void primary_linker_initialization(byte *program_directory){printf("link_step %d\n", link_step);}
void get_filenames_from_user(bit_16 argc16, byte *argv[]){printf("link_step %d\n", link_step);}
void secondary_linker_initialization(void){printf("link_step %d\n", link_step);}
void process_library_directories(void){printf("link_step %d\n", link_step);}
void process_object_modules(void){printf("link_step %d\n", link_step);}
void process_libraries(void){printf("link_step %d\n", link_step);}
void order_and_align_segments(void){printf("link_step %d\n", link_step);}
void pass_two(void){printf("link_step %d\n", link_step);}
void write_executable_image(void){printf("link_step %d\n", link_step);}
void link_map(void){printf("link_step %d\n", link_step);}
void file_delete(file_info_ptr file_info){printf("link_step %d\n", link_step);}
void end_linker(bit_16 return_code){printf("link_step %d\n", link_step);}
