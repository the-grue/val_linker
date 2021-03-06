/*                                 ORDER.C                                 */

/*
  order_expression::  term   {('or'  | '+' | '|') term}
              term::  factor {('and' | '*' | '&') factor}
            factor::  ('!' | '~' | '-' | 'not')* primary
           primary::  'true'                       |
                      'false'                      |
                      ('segment' '[' name ']' )    |
                      ('class'   '[' name ']' )    |
                      ('group'   '[' name ']' )    |
                      ( '(' order_expression ')' )
*/

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          align_active_segment                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void align_active_segment()
{
bit_32                                 gap;
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
bit_32                                 mask;
public_entry_ptr                       pub;
#define Pub                            (*pub)


 if ( (*Active_segment.lsegs.first).align == 0
  ) { /* Don't align absolute segments */
   return;
  };
 Active_segment.highest_uninitialized_byte = 0L;
 TraverseList(Active_segment.lsegs, lseg)
  BeginTraverse
   if (Lseg.align == absolute_segment) continue;
   if ( Active_segment.combine == common_combine
    ) {  /* Finally, we know how big the common area is, so allocate
             memory for it.  */
     Lseg.data = allocate_memory(&(static_pool), Lseg.length);
    };
   mask                    = align_mask[Lseg.align];
   gap                     = AlignmentGap(next_available_address, mask);
   next_available_address += gap;
   Lseg.address            = next_available_address;
   next_available_address += Lseg.length;
   if ( Lseg.highest_uninitialized_byte != 0
    ) {
     highest_uninitialized_byte                =
     Active_segment.highest_uninitialized_byte =
      Lseg.address + Lseg.highest_uninitialized_byte;
    };
  EndTraverse;
 Active_segment.address = (*Active_segment.lsegs.first).address;
 Active_segment.length  = (*Active_segment.lsegs.last).address +
                          (*Active_segment.lsegs.last).length -
                          Active_segment.address;
 if ( Active_segment.highest_uninitialized_byte == 0
  ) {
   Active_segment.highest_uninitialized_byte =
    (*Active_segment.lsegs.first).address;
  };
 if ( (Active_segment.owning_group != 0) &&
    ((*Active_segment.owning_group).first_segment == 0)
  ) {
   (*Active_segment.owning_group).first_segment = active_segment;
  };
 if ( (DOSSEG.val != 0) && 
    (Active_segment.owning_group != 0) &&
    ((*Active_segment.owning_group).group_name == DGROUP_lname)
  ) {
   if ( (edata_segment == 0) &&
      (Active_segment.class_name == BSS_lname)
    ) {
     edata_segment = active_segment;
     pub = lookup_public(6, (byte *) "_edata", 0);
     if ( (Pub.type_entry == external) || (Pub.type_entry == unused)
      ) {
       Pub.type_entry      = internal;
       Pub.Internal.group  = Active_segment.owning_group;
       Pub.Internal.lseg   = Active_segment.lsegs.first;
       Pub.Internal.frame  = 0;
       Pub.Internal.offset = 0;
      } else {
       linker_error(4, "Could not generate symbol \"_edata\" "
                       "when \"/DOSSEG\" set\n"
                       "because it was explicitly defined.\n");
      };
    };
   if ( (end_segment == 0) &&
      (Active_segment.class_name == STACK_lname)
    ) {
     end_segment = active_segment;
     pub = lookup_public(4, (byte *) "_end", 0);
     if ( (Pub.type_entry == external) || (Pub.type_entry == unused)
      ) {
       Pub.type_entry      = internal;
       Pub.Internal.group  = Active_segment.owning_group;
       Pub.Internal.lseg   = Active_segment.lsegs.first;
       Pub.Internal.frame  = 0;
       Pub.Internal.offset = 0;
      } else {
       linker_error(4, "Could not generate symbol \"_end\" "
                       "when \"/DOSSEG\" set\n"
                       "because it was explicitly defined.\n");
      };
    };
  };
 return;
}
#undef Lseg
#undef Pub

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            get_order_token                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void get_order_token()
{


 while ( token_break_char == ' '
  ) {
   order_token_get_char();
  };
 copy_string(token, null_string);
 if ( IsIdentifier(token_break_char)
  ) {
   while ( IsIdentifier(token_break_char)
    ) {
     concat_char_to_string(token, token_break_char);
     order_token_get_char();
    };
   lowercase_string(token);
  } else {
   if ( token_break_char == '['
    ) {
     while ( token_break_char != ']'
      ) {
       concat_char_to_string(token, token_break_char);
       order_token_get_char();
      };
     order_token_get_char();
     if ( case_ignore.val
      ) {
       lowercase_string(token);
      };
    } else {
     concat_char_to_string(token, token_break_char);
     order_token_get_char();
    };
  };
 return;
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         order_and_align_segments                        |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void order_and_align_segments()
{
byte_ptr                               start_of_expression;


 order_start_time = Now;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  | Before we can start ordering segments, we have to get some housekeeping |
  | done first.                                                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 order_prologue();

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |              OK, lets get to ordering and aligning segments.            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 First(segments_ordered_list) =
 Last(segments_ordered_list)  = Null;
 highest_uninitialized_byte = 0L;
 start_of_expression = String(ordering.val);
 start_of_expression++;
 while ( *start_of_expression != '\000'
  ) {
   if (segment_list.first == 0) break;
   First(segments_unordered_list) =
   Last(segments_unordered_list)  = Null;
   while ( segment_list.first != 0
    ) {
     Pop segment_list InTo active_segment EndPop;
     order_expression_char_ptr = start_of_expression;
     token_break_char = ' ';
     if ( codeview_information_present &&
        (((Active_segment.segment_name == codeview_segment_TYPES) &&
          (Active_segment.class_name == codeview_class_DEBTYP)) ||
         ((Active_segment.segment_name == codeview_segment_SYMBOLS) &&
          (Active_segment.class_name == codeview_class_DEBSYM)))
      ) { /* Eat the codeview segment */
      } else { /* Process all non-codeview segments */
       if ( order_expression()
        ) {
         Insert active_segment AtEnd InList segments_ordered_list EndInsert;
         align_active_segment();
        } else {
         Insert active_segment AtEnd InList segments_unordered_list EndInsert;
        };
      };
    };
   start_of_expression = order_expression_char_ptr;
   start_of_expression--;
   segment_list = segments_unordered_list;
  };

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |          Make one final pass accepting all remaining segments.          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 while ( segment_list.first != 0
  ) {
   Pop segment_list InTo active_segment EndPop;
   Insert active_segment AtEnd InList segments_ordered_list EndInsert;
   align_active_segment();
  };
 segment_list = segments_ordered_list;
 return;
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            order_expression                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 order_expression()
{
bit_16                                 left_operand;


 get_order_token();
 left_operand = order_term();
 while ( TokenIs(or_string)   ||
       TokenIs(plus_string) ||
       TokenIs(bar_string)
  ) {
   get_order_token();
   left_operand |= order_term();
  };
 return(left_operand);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             order_factor                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 order_factor()
{
bit_16                                 unary_not;


 unary_not = False;
 while ( TokenIs(exclamation_string) ||
       TokenIs(tilde_string)       ||
       TokenIs(minus_string)       ||
       TokenIs(not_string)
  ) {
   get_order_token();
   unary_not ^= True;
  };
 return(unary_not ^ order_primary());
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             order_primary                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 order_primary()
{
group_entry_ptr                        group;
#define Group                          (*group)
bit_16                                 operand;


 if ( TokenIs(true_string)
  ) {
   get_order_token();
   return(True);
  };
 if ( TokenIs(false_string)
  ) {
   get_order_token();
   return(False);
  };
 if ( TokenIs(open_paren_string)
  ) {
   operand = order_expression();
   if ( TokenIs(close_paren_string)
    ) {
     get_order_token();
     return(operand);
    } else {
     linker_error(8, "Expression syntax error:\n"
                     "\t\"%Fs\"\n",
                     String(ordering.val));
    };
  };
 if ( TokenStartsWith(segment_string)
  ) {
   get_order_token();
   if ( *String(token) != '['
    ) {
     linker_error(8, "Expression syntax error:\n"
                     "\t\"%Fs\"\n",
                     String(ordering.val));
    };
   cut_string(token, 0, 1);
   operand = match_pattern(token,
                           string((*Active_segment.segment_name).symbol));
   get_order_token();
   return(operand);
  };
 if ( TokenStartsWith(group_string)
  ) {
   get_order_token();
   if ( *String(token) != '['
    ) {
     linker_error(8, "Expression syntax error:\n"
                     "\t\"%Fs\"\n",
                     String(ordering.val));
    };
   cut_string(token, 0, 1);
   group = Active_segment.owning_group;
   if ( group == 0
    ) {
     operand = False;
    } else {
     operand =
      match_pattern(token, string((*Group.group_name).symbol));
    };
   get_order_token();
   return(operand);
  };
 if ( TokenStartsWith(class_string)
  ) {
   get_order_token();
   if ( *String(token) != '['
    ) {
     linker_error(8, "Expression syntax error:\n"
                     "\t\"%Fs\"\n",
                     String(ordering.val));
    };
   cut_string(token, 0, 1);
   operand = match_pattern(token,
                           string((*Active_segment.class_name).symbol));
   get_order_token();
   return(operand);
  };
 linker_error(8, "Expression syntax error:\n"
                 "\t\"%Fs\"\n",
                 String(ordering.val));
 return(False);
}
#undef Group

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              order_prologue                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void order_prologue()
{
group_entry_ptr                        group;
bit_32                                 length;
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
public_entry_ptr                       next_communal;
bit_16                                 offset;
public_entry_ptr                       pub;
#define Pub                            (*pub)
bit_16                                 size;



/*+-------------------------------------------------------------------------+
  |                                                                         |
  | Compute the address base for the executable image.  For .COM files this |
  | will be 100H, for .EXE and .SYS files, this will be 0.                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 if ( comfile.val != 0
  ) {
   address_base = 0x100L;
  } else {
   address_base = 0L;
  };
 next_available_address = 0L;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |   The ordering expression for the segments must be known.  If the       |
  |   "/ORDER:(expression)" switch was specified, then it will be used      |
  |   regardless or whether or not "/DOSSEG" is set.  If the "/ORDER"       |
  |   switch was not specified and "/DOSSEG" was, then the appropriate      |
  |   expression is used.  Otherwise, the linker will just take the         |
  |   segments in the order they were encountered.                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 if ( ordering.val == 0
  ) {
   if ( DOSSEG.val != 0
    ) {
     ordering.val = duplicate_string(&(static_pool),
      string((byte *) ("(seg[*code]|seg[*CODE], "
                       "!(gr[dgroup]|gr[DGROUP]), "
                       "cl[begdata]|cl[BEGDATA], "
                       "cl[*data]|cl[*DATA], "
                       "!(cl[*bss]|cl[*BSS]|cl[*stack]|cl[*STACK]), "
                       "cl[*bss]|cl[*BSS], "
                       "cl[*stack]|cl[*STACK])")));
    } else {
     ordering.val = duplicate_string(&(static_pool),
                                     string((byte *) ("(true)")));
    };
  };

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |  Check to insure a stack segment of appropriate size is present.  For   |
  |  .COM and .SYS files, no stack segment is required.  For .EXE files,    |
  |  one is required.  For .EXE files, we must insure that a stack of at    |
  |  least the size specified in the "/STACK" switch is available.          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 if ( exefile == 0
  ) {
   if ( stack_segment_found != 0
    ) {
     linker_error(4, "Stack segment found for a non .EXE file.\n");
    };
  } else {
   if ( (stack_segment_found == 0) &&
      (stack.set           == 0)
    ) {
     linker_error(4, "No stack segment for .EXE file.\n");
    } else {
     if ( (stack.set != 0) && 
        (Bit_16(Largest_stack_seg.length) < stack.val)
      ) {
       obj_generate_segment(generated_lname,
                            none_lname,
                            stack_combine,
                            2,              /* word aligned */
                            none_lname,
                            exe_file_list.first,
                            0L,             /* not absolute segment */
                            Bit_32(stack.val));
      };
    };
  };

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |  Handle near communals as follows:  If there is at least one near       |
  |  communal, create the segment "c_common" in group "DGROUP" with         |
  |  class "BSS".  Then, for each communal place it in "c_common" in        |
  |  ascending order while changing the dictionary entry from               |
  |  "near_communal" to "internal".                                         |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 if ( near_communals != 0
  ) {
   length = 0L;
   for ( pub=near_communals; pub != 0; pub=Pub.Communal.next_communal
    ) {
     if (Pub.type_entry != near_communal) continue;
     length += Pub.Communal.element_size;
    };
   if ( length > 65536L
    ) {
     linker_error(8, "Near communal size exceeds 64K by %lu bytes.\n",
                     length-65536L);
    };
   lseg = obj_generate_segment(c_common_lname,
                               BSS_lname,
                               blank_common_combine,
                               3,              /* paragraph aligned */
                               none_lname,
                               exe_file_list.first,
                               0L,             /* not absolute segment */
                               length);
   Lseg.highest_uninitialized_byte = length;
   group                        = lookup_group(DGROUP_lname);
   (*Lseg.segment).owning_group = group;
   offset = 0;
   for ( pub=near_communals; pub != 0; pub=next_communal
    ) {
     next_communal        = Pub.Communal.next_communal;
     if (Pub.type_entry != near_communal) continue;
     size                 = Bit_16(Pub.Communal.element_size);
     Pub.type_entry       = internal;
     Pub.Internal.group   = group;
     Pub.Internal.lseg    = lseg;
     Pub.Internal.offset  = offset;
     Pub.Internal.frame   = 0;
     offset              += size;
    };
  };
 
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |  Handle far communals as follows:  Packing as many far communals as     |
  |  will fit (64K), create private segments with the segment and class     |
  |  name of "FAR_BSS".  They do NOT go in DRGROUP.                         |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 lseg   = Null;
 offset = 0;
 for ( pub=far_communals; pub != 0; pub=next_communal
  ) {
   next_communal = Pub.Communal.next_communal;
   if (Pub.type_entry != far_communal) continue; 
   length = Pub.Communal.element_size * Pub.Communal.element_count;
   if ( length > 65536L
    ) {
     Pub.Communal.next_communal = huge_communals;
     huge_communals             = pub;
     continue;
    };
   if ( (lseg == 0) || ((length + Bit_32(offset)) > 65536L)
    ) {
     lseg = obj_generate_segment(FAR_BSS_lname,
                                 FAR_BSS_lname,
                                 blank_common_combine,
                                 3,              /* paragraph aligned */
                                 none_lname,
                                 exe_file_list.first,
                                 0L,             /* not absolute segment */
                                 0L);
     offset = 0;
    };
   Pub.type_entry                   = internal;
   Pub.Internal.group               = Null;
   Pub.Internal.lseg                = lseg;
   Pub.Internal.offset              = offset;
   Pub.Internal.frame               = 0;
   offset                          += Bit_16(length);
   Lseg.highest_uninitialized_byte += length;
   Lseg.length                     += length;
   (*Lseg.segment).length          += length;
  };

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |  Handle huge communals as follows:  Taking as many segments as          |
  |  required, create private segments with the segment and class           |
  |  name of "HUGE_BSS".  They do NOT go in DRGROUP.                        |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 for ( pub=huge_communals; pub != 0; pub=next_communal
  ) {
   next_communal = Pub.Communal.next_communal;
   length = Pub.Communal.element_size * Pub.Communal.element_count;
   lseg = obj_generate_segment(HUGE_BSS_lname,
                               HUGE_BSS_lname,
                               blank_common_combine,
                               3,              /* paragraph aligned */
                               none_lname,
                               exe_file_list.first,
                               0L,             /* not absolute segment */
                               0L);
   if ( Pub.Communal.element_size > 65536L
    ) {
     linker_error(4, "Communal \"%Fs\" has element size exceeding 64K.\n",
                     Pub.symbol);
     offset = 0;
    } else {
     offset = Bit_16(65536L % Pub.Communal.element_size);
    };
   length                          += Bit_32(offset);
   Pub.type_entry                   = internal;
   Pub.Internal.group               = Null;
   Pub.Internal.lseg                = lseg;
   Pub.Internal.offset              = offset;
   Pub.Internal.frame               = 0;
   Lseg.highest_uninitialized_byte += length;
   Lseg.length                     += length;
   (*Lseg.segment).length          += length;
  };

 return;
}
#undef Pub
#undef Lseg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               order_term                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 order_term()
{
bit_16                                 left_operand;


 left_operand = order_factor();
 while ( TokenIs(and_string)       ||
       TokenIs(star_string)      ||
       TokenIs(ampersand_string)
  ) {
   get_order_token();
   left_operand &= order_factor();
  };
 return(left_operand);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           order_token_get_char                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void order_token_get_char()
{


 if ( token_break_char == '\000'
  ) {
   linker_error(8, "Expression syntax error:\n"
                   "\t\"%Fs\"\n",
                   String(ordering.val));
  };
 token_break_char = *order_expression_char_ptr++;
 return;
}
