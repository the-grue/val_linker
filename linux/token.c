/*                                 TOKEN.C                                 */

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       complete_a_filename_token                         |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void complete_a_filename_token()
BeginDeclarations
EndDeclarations
BeginCode
 copy_string(token, next_token);
 scan_out_token();
 if ( token_type Is switch_end_token_type
  ) {
   concat_string(token, next_token);
   scan_out_token();
   if ( token_type Is filename_token_type
    ) {
     concat_string(token, next_token);
     scan_out_token();
    };
  };
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          eat_white_space                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void eat_white_space()
BeginDeclarations
EndDeclarations
BeginCode
 While token_break_char Is ' '
  BeginWhile
   token_get_char();
  EndWhile;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                     get_free_token_source_element                       |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
token_stack_ptr get_free_token_source_element()
BeginDeclarations
token_stack_ptr                        elem;
EndDeclarations
BeginCode
 Pop token_stack_free_list InTo elem EndPop;
 if ( elem IsNull
  ) {
   elem = (token_stack_ptr) 
           allocate_memory(Addr(static_pool),
                           Bit_32(sizeof(token_stack_type)));
  };
 return(elem);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                        get_filename_token                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void get_filename_token(bit_16            required,
                        file_info_list   *list)
BeginDeclarations
#define List                           (*list)
token_stack_ptr                        source_element;
#define Source_element                 (*source_element)
EndDeclarations
BeginCode
 Loop
  BeginLoop
   required = (required) AndIf (List.first IsNull);
   if ( token_type Is text_token_type
    ) {
     linker_error(8, "Input syntax error:  \"%Fs\" out of place.\n",
                     String(next_token));
    };
   if ( token_type Is indirect_file_token_type
    ) {
     scan_out_token();
     if ( token_type IsNot filename_token_type
      ) {
       linker_error(8,"Input syntax error:  Expected filename after '@'.\n");
      };
     complete_a_filename_token();
     source_element                    = get_free_token_source_element();
     Source_element.break_char         = token_break_char;
     Source_element.source_file        = input_open(token);
     Source_element.token_string       = Null;
     Source_element.token_string_index = 0;
     Push source_element OnTo token_stack EndPush;
     token_break_char                  = ' ';
     scan_out_token();
     ContinueLoop;
    };
   if ( token_type Is switch_token_type
    ) {
     process_switch();
     ContinueLoop;
    };
   if ( token_type Is continuation_token_type
    ) {
     scan_out_token();
     if ( token_type Is line_end_token_type
      ) {
       scan_out_token();
      };
     ContinueLoop;
    };
   if ( token_type Is filename_token_type
    ) {
     complete_a_filename_token();
     more_tokens = True;
     return;
    };
   if ( ((token_type Is end_of_command_line_token_type)  AndIf
       (List.first IsNull))                            OrIf
      ((required)                                      AndIf
       ((token_type Is separator_token_type)           OrIf
        (token_type Is line_end_token_type)))
    ) {
     if ( (*token_stack.first).source_file IsNot stdin
      ) {
       source_element                    = get_free_token_source_element();
       Source_element.source_file        = stdin;
       Source_element.token_string       = Null;
       Source_element.token_string_index = 0;
       Push source_element OnTo token_stack EndPush;
       token_break_char                  = ' ';
      };
     if ( default_prompt IsNotNull
      ) {
       linker_message(default_prompt, String(default_filename));
       prompt_next_stdin = False;
       default_prompt    = Null;
      };
     scan_out_token();
     ContinueLoop;
    };
   if ( List.first IsNull
    ) {
     if ( (token_type Is separator_token_type)  OrIf
        (token_type Is terminator_token_type)
      ) {
       if ( prompting_for Is 2
        ) {
         if ( comfile.val IsTrue
          ) {
           default_extension = com_extension_string;
          } else {
           if ( sysfile.val IsTrue
            ) {
             default_extension = sys_extension_string;
            } else {
             default_extension = exe_extension_string;
            };
           };
         change_extension(default_filename, default_extension);
        };
       copy_string(token, default_filename);
       more_tokens = False;
       return;
      };
    } else {
     if ( (token_type Is separator_token_type)           OrIf
        (token_type Is terminator_token_type)          OrIf
        (token_type Is end_of_command_line_token_type)
      ) {
       copy_string(token, null_string);
       more_tokens = False;
       return;
     };
    };
   if ( token_type Is line_end_token_type
    ) {
     if ( List.first IsNull
      ) {
       if ( prompting_for Is 2
        ) {
         if ( comfile.val IsTrue
          ) {
           default_extension = com_extension_string;
          } else {
           if ( sysfile.val IsTrue
            ) {
             default_extension = sys_extension_string;
            } else {
             default_extension = exe_extension_string;
            };
           };
         change_extension(default_filename, default_extension);
        };
       copy_string(token, default_filename);
      } else {
       copy_string(token, null_string);
      };
     if ( (*token_stack.first).source_file Is stdin
      ) {
       Pop token_stack InTo source_element EndPop;
       Push source_element OnTo token_stack_free_list EndPush;
      };
     prompt_next_stdin = False;
     more_tokens = False;
     return;
    };
  EndLoop;
 return;
EndCode
#undef List
#undef Source_element

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             input_open                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
FILE *input_open(string_ptr fn)
BeginDeclarations
FILE                                  *infile;
EndDeclarations
BeginCode
 infile = fopen((char *)near_string(fn), "r");
 if ( infile Is NULL
  ) {
   linker_error(8, "Could not open file \"%Fs\" for input.\n",
                   String(fn));
  };
 return(infile);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             process_switch                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void process_switch()
BeginDeclarations
switch_table_ptr                       current_switch;
#define Current_switch                 (*current_switch)
EndDeclarations
BeginCode
 current_switch = switch_table;
 scan_out_token();
 copy_string(token, next_token);
 lowercase_string(token);
 While Current_switch.full_name IsNotNull
  BeginWhile
   if ( compare_string(token, string((byte *)Current_switch.abbr_name)) IsZero
    ) {
     Current_switch.switch_processor(current_switch);
     return;
    };
   if ( (Length(token) NotLessThan Current_switch.min_length) AndIf
      (far_compare(String(token), (byte *) Current_switch.full_name,
        Length(token)) IsZero)
    ) {
     Current_switch.switch_processor(current_switch);
     return;
    };
   current_switch++;
  EndWhile;
 linker_error(8,"Syntax error:  \"%Fs\" is an unknown switch.\n",
                String(token));
 return;
EndCode
#undef Current_switch

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           scan_bit_16_switch                            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void scan_bit_16_switch(switch_table_ptr current_switch)
BeginDeclarations
#define Current_switch                 (*current_switch)
#define Affected_thing (*((bit_16_switch_ptr)(*current_switch).affected_thing))
EndDeclarations
BeginCode
 scan_out_token();
 copy_string(token, next_token);
 if ( token_type IsNot switch_end_token_type
  ) {
   linker_error(8,"Syntax error:  \":\" did not follow switch \"%s\"\n",
                  Current_switch.full_name);
  };
 scan_out_token();
 copy_string(token, next_token);
  if ( (Not token_is_number)                                OrIf
     (token_numeric_value LessThan    Affected_thing.min) OrIf
     (token_numeric_value GreaterThan Affected_thing.max)
   ) {
   linker_error(8,"Syntax error:  Switch \"%s\" requires a numeric value\n"
                  "               between %u and %u\n",
                  Current_switch.full_name,
                  Affected_thing.min, Affected_thing.max);
   } else {
    Affected_thing.val = token_numeric_value;
    Affected_thing.set = True;
   };
 scan_out_token();
 copy_string(token, next_token);
 return;
EndCode
#undef Current_switch
#undef Affected_thing

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             scan_opt_bit_16                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void scan_opt_bit_16(switch_table_ptr current_switch)
BeginDeclarations
#define Current_switch                 (*current_switch)
#define Affected_thing (*((bit_16_switch_ptr)(*current_switch).affected_thing))
EndDeclarations
BeginCode
 scan_out_token();
 copy_string(token, next_token);
 Affected_thing.set = True;
 if ( token_type IsNot switch_end_token_type
  ) {
   Affected_thing.val = Affected_thing.def;
   return;
  };
 scan_out_token();
 copy_string(token, next_token);
  if ( (Not token_is_number)                                OrIf
     (token_numeric_value LessThan    Affected_thing.min) OrIf
     (token_numeric_value GreaterThan Affected_thing.max)
   ) {
   linker_error(8,"Syntax error:  Switch \"%s\" requires a numeric value\n"
                  "               between %u and %u\n",
                  Current_switch.full_name,
                  Affected_thing.min, Affected_thing.max);
   } else {
    Affected_thing.val = token_numeric_value;
   };
 scan_out_token();
 copy_string(token, next_token);
 return;
EndCode
#undef Current_switch
#undef Affected_thing

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         scan_out_token                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void scan_out_token()
BeginDeclarations
bit_16                                 paren_count;
EndDeclarations
BeginCode
 eat_white_space();
 copy_string(next_token, null_string);
 token_is_hex_number =
 token_is_number     = False;
 switch ( token_break_char
  ) {
   case '\n':
    prompt_next_stdin = True;
    concat_char_to_string(next_token, token_break_char);
    if ( token_stack.first Is token_stack.last
     ) {
      token_type       = end_of_command_line_token_type;
     } else {
      token_type       = line_end_token_type;
     };
    token_break_char = ' ';  /* Make it look like we advanced a character */
    break;
   case ',':
    concat_char_to_string(next_token, token_break_char);
    token_type       = separator_token_type;
    token_break_char = ' ';  /* Make it look like we advanced a character */
    break;
   case ';':
    concat_char_to_string(next_token, token_break_char);
    token_type       = terminator_token_type;
    break;
   case '+':
    concat_char_to_string(next_token, token_break_char);
    token_type       = continuation_token_type;
    token_break_char = ' ';  /* Make it look like we advanced a character */
    break;
   case '/':
    concat_char_to_string(next_token, token_break_char);
    token_type       = switch_token_type;
    token_break_char = ' ';  /* Make it look like we advanced a character */
    break;
   case ':':
    concat_char_to_string(next_token, token_break_char);
    token_type       = switch_end_token_type;
    token_break_char = ' ';  /* Make it look like we advanced a character */
    break;
   case '@':
    concat_char_to_string(next_token, token_break_char);
    token_type       = indirect_file_token_type;
    token_break_char = ' ';  /* Make it look like we advanced a character */
    break;
   case '(':
    paren_count = 1;
    token_type  = text_token_type;
    concat_char_to_string(next_token, token_break_char);
    While paren_count IsNotZero
     BeginWhile
      token_get_char();
      if ( token_break_char IsNot '\n'
       ) {
        concat_char_to_string(next_token, token_break_char);
       } else {
        if ( (*token_stack.first).source_file Is stdin
         ) {
          linker_message("continue parenthesized text:  ");
         };
       };
      if ( token_break_char Is '('
       ) {
        paren_count++;
       };
      if ( token_break_char Is ')'
       ) {
        paren_count--;
       };
     EndWhile;
    token_break_char = ' ';  /* Make it look like we advanced a character */
    break;
   default:
    token_is_number     = True;
    token_numeric_value = 0;
    While (token_break_char IsNot ',')  AndIf
          (token_break_char IsNot ';')  AndIf
          (token_break_char IsNot '+')  AndIf
          (token_break_char IsNot '/')  AndIf
          (token_break_char IsNot '@')  AndIf
          (token_break_char IsNot ':')  AndIf
          (token_break_char IsNot ' ')  AndIf
          (token_break_char IsNot '\n')
     BeginWhile
      concat_char_to_string(next_token, token_break_char);
      if ( (Length(next_token) Is 2) AndIf (String(next_token)[0] Is '0') AndIf
         ((String(next_token)[1] Is 'x') OrIf (String(next_token)[1] Is 'X'))
       ) {
        token_is_hex_number = True;
       } else {
        if ( token_is_hex_number IsFalse
         ) {
          token_is_number = token_is_number AndIf
                            isdigit(token_break_char);
          if ( token_is_number
           ) {
            token_numeric_value = (token_numeric_value * 10) +
                                  Bit_16(token_break_char - '0');
           };
         } else {
          token_is_hex_number =
          token_is_number     = token_is_number AndIf
                                isxdigit(token_break_char);
          if ( token_is_number
           ) {
            if ( isdigit(token_break_char)
             ) {
              token_numeric_value = (token_numeric_value * 16) +
                                    Bit_16(token_break_char - '0');
             } else {
              token_numeric_value = (token_numeric_value * 16) +
                                 Bit_16(toupper(token_break_char) - 'A' + 10);
             };
           };
         };
       };
      token_get_char();
     EndWhile;
    token_type = filename_token_type;
    break;
  };
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           scan_help_switch                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void scan_help_switch(switch_table_ptr current_switch)
BeginDeclarations
#define Affected_thing (*((boolean_switch_ptr)(*current_switch).affected_thing))
FILE                                  *help_file;
EndDeclarations
BeginCode
 Affected_thing.val = True;
 help_file = fopen(CharPtr(near_string(help_filename)), "r");
 if ( help_file IsNull
  ) {
   printf("Could not open help file \"%Fs\".\n", String(help_filename));
  } else {
   While fgets(CharPtr(object_file_element), MAX_ELEMENT_SIZE, help_file) 
         IsNotNull
    BeginWhile
     if ( *CharPtr(object_file_element) Is '\f'
       ) {
        linker_message("Press [RETURN] to continue...");
        gets(CharPtr(object_file_element));
       } else {
        linker_message(CharPtr(object_file_element));
      };
    EndWhile;
   fclose(help_file);
  };
 exit(0);
 return;
EndCode
#undef Affected_thing

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           scan_reset_bit_16                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void scan_reset_bit_16(switch_table_ptr current_switch)
BeginDeclarations
#define Affected_thing (*((bit_16_switch_ptr)(*current_switch).affected_thing))
EndDeclarations
BeginCode
 scan_out_token();
 copy_string(token, next_token);
 Affected_thing.set = False;
 Affected_thing.val = Affected_thing.def;
 return;
EndCode
#undef Affected_thing

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           scan_reset_switch                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void scan_reset_switch(switch_table_ptr current_switch)
BeginDeclarations
#define Current_switch                 (*current_switch)
#define Affected_thing (*((boolean_switch_ptr)(*current_switch).affected_thing))
EndDeclarations
BeginCode
 Affected_thing.val = False;
 scan_out_token();
 copy_string(token, next_token);
 return;
EndCode
#undef Current_switch
#undef Affected_thing

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            scan_set_switch                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void scan_set_switch(switch_table_ptr current_switch)
BeginDeclarations
#define Current_switch                 (*current_switch)
#define Affected_thing (*((boolean_switch_ptr)(*current_switch).affected_thing))
EndDeclarations
BeginCode
 Affected_thing.val = True;
 scan_out_token();
 copy_string(token, next_token);
 return;
EndCode
#undef Current_switch
#undef Affected_thing

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            scan_text_switch                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void scan_text_switch(switch_table_ptr current_switch)
BeginDeclarations
#define Current_switch                 (*current_switch)
#define Affected_thing (*((text_switch_ptr)(*current_switch).affected_thing))
EndDeclarations
BeginCode
 scan_out_token();
 copy_string(token, next_token);
 if ( token_type IsNot switch_end_token_type
  ) {
   linker_error(8,"Syntax error:  \":\" did not follow switch \"%s\"\n",
                  Current_switch.full_name);
  };
 scan_out_token();
 copy_string(token, next_token);
 if ( token_type IsNot text_token_type
  ) {
   linker_error(8, "Syntax error:  Parenthesized text did not follow\n"
                   "\t\"%s\" switch.  Instead found \"%Fs\".\n",
                  Current_switch.full_name, String(token));
  };
 Affected_thing.val = duplicate_string(Addr(static_pool), next_token);
 scan_out_token();
 copy_string(token, next_token);
 return;
EndCode
#undef Affected_thing
#undef Current_switch

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           token_get_char                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void token_get_char()
BeginDeclarations
int_16                                 c;
token_stack_ptr                        tos;
#define Tos                            (*tos)
#define Tos_string                     (*tos).token_string
EndDeclarations
BeginCode
 Loop
  BeginLoop
   if ( ((*token_stack.first).source_file Is stdin) AndIf
      (prompt_next_stdin)
    ) {
     linker_message("continue:  ");
     prompt_next_stdin = False;
    };
   tos = token_stack.first;
   if ( tos IsNull
    ) {
     token_break_char = ';';
     return;
    };
   if ( Tos.source_file IsNull
    ) { /* Input is from a string */
     if ( Tos.token_string_index LessThan Length(Tos_string)
      ) {
       token_break_char = String(Tos_string)[Tos.token_string_index++];
       ExitLoop;
      } else {
       if ( token_stack.first Is token_stack.last
        ) {
         token_break_char = '\n';
         return;
        } else {
         Pop token_stack InTo tos EndPop;
         Push tos OnTo token_stack_free_list EndPush;
         ContinueLoop;
        };
      };
    } else { /* Input is from a file */
     c = fgetc(Tos.source_file);
     if ( c Is EOF
      ) {
       if ( Tos.source_file IsNot stdin
        ) {
         fclose(Tos.source_file);
        };
       Pop token_stack InTo tos EndPop;
       token_break_char = Tos.break_char;
       Push tos OnTo token_stack_free_list EndPush;
       ExitLoop;
      } else {
       token_break_char = Byte(c);
       ExitLoop;
      };
    };
  EndLoop;
 if ( token_break_char Is '\r'
  ) {
   token_break_char = '\n';
  };
 if ( token_break_char Is '\t'
  ) {
   token_break_char = ' ';
  };
 return;
EndCode
#undef Tos
#undef Tos_string
