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
BeginDeclarations
bit_32                                 gap;
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
bit_32                                 mask;
public_entry_ptr                       pub;
#define Pub                            (*pub)
EndDeclarations
BeginCode
 if ( (*Active_segment.lsegs.first).align IsZero
  ) { /* Don't align absolute segments */
   return;
  };
 Active_segment.highest_uninitialized_byte = 0L;
 TraverseList(Active_segment.lsegs, lseg)
  BeginTraverse
   LoopIf(Lseg.align Is absolute_segment);
   if ( Active_segment.combine Is common_combine
    ) {  /* Finally, we know how big the common area is, so allocate
             memory for it.  */
     Lseg.data = allocate_memory(Addr(static_pool), Lseg.length);
    };
   mask                    = align_mask[Lseg.align];
   gap                     = AlignmentGap(next_available_address, mask);
   next_available_address += gap;
   Lseg.address            = next_available_address;
   next_available_address += Lseg.length;
   if ( Lseg.highest_uninitialized_byte IsNotZero
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
 if ( Active_segment.highest_uninitialized_byte IsZero
  ) {
   Active_segment.highest_uninitialized_byte =
    (*Active_segment.lsegs.first).address;
  };
 if ( (Active_segment.owning_group IsNotNull) AndIf
    ((*Active_segment.owning_group).first_segment IsNull)
  ) {
   (*Active_segment.owning_group).first_segment = active_segment;
  };
 if ( (DOSSEG.val IsTrue) AndIf 
    (Active_segment.owning_group IsNotNull) AndIf
    ((*Active_segment.owning_group).group_name Is DGROUP_lname)
  ) {
   if ( (edata_segment IsNull) AndIf
      (Active_segment.class_name Is BSS_lname)
    ) {
     edata_segment = active_segment;
     pub = lookup_public(6, (byte *) "_edata", 0);
     if ( (Pub.type_entry Is external) OrIf (Pub.type_entry Is unused)
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
   if ( (end_segment IsNull) AndIf
      (Active_segment.class_name Is STACK_lname)
    ) {
     end_segment = active_segment;
     pub = lookup_public(4, (byte *) "_end", 0);
     if ( (Pub.type_entry Is external) OrIf (Pub.type_entry Is unused)
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
EndCode
#undef Lseg
#undef Pub

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            get_order_token                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void get_order_token()
BeginDeclarations
EndDeclarations
BeginCode
 While token_break_char Is ' '
  ) {
   order_token_get_char();
  EndWhile;
 copy_string(token, null_string);
 if ( IsIdentifier(token_break_char)
  ) {
   While IsIdentifier(token_break_char)
    ) {
     concat_char_to_string(token, token_break_char);
     order_token_get_char();
    EndWhile;
   lowercase_string(token);
  } else {
   if ( token_break_char Is '['
    ) {
     While token_break_char IsNot ']'
      ) {
       concat_char_to_string(token, token_break_char);
       order_token_get_char();
      EndWhile;
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
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         order_and_align_segments                        |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void order_and_align_segments()
BeginDeclarations
byte_ptr                               start_of_expression;
EndDeclarations
BeginCode
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
 While *start_of_expression IsNot '\000'
  ) {
   ExitIf(segment_list.first IsNull);
   First(segments_unordered_list) =
   Last(segments_unordered_list)  = Null;
   While segment_list.first IsNotNull
    ) {
     Pop segment_list InTo active_segment EndPop;
     order_expression_char_ptr = start_of_expression;
     token_break_char = ' ';
     if ( codeview_information_present AndIf
        (((Active_segment.segment_name Is codeview_segment_TYPES) AndIf
          (Active_segment.class_name Is codeview_class_DEBTYP)) OrIf
         ((Active_segment.segment_name Is codeview_segment_SYMBOLS) AndIf
          (Active_segment.class_name Is codeview_class_DEBSYM)))
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
    EndWhile;
   start_of_expression = order_expression_char_ptr;
   start_of_expression--;
   segment_list = segments_unordered_list;
  EndWhile;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |          Make one final pass accepting all remaining segments.          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 While segment_list.first IsNotNull
  ) {
   Pop segment_list InTo active_segment EndPop;
   Insert active_segment AtEnd InList segments_ordered_list EndInsert;
   align_active_segment();
  EndWhile;
 segment_list = segments_ordered_list;
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            order_expression                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 order_expression()
BeginDeclarations
bit_16                                 left_operand;
EndDeclarations
BeginCode
 get_order_token();
 left_operand = order_term();
 While TokenIs(or_string)   OrIf
       TokenIs(plus_string) OrIf
       TokenIs(bar_string)
  ) {
   get_order_token();
   left_operand |= order_term();
  EndWhile;
 return(left_operand);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             order_factor                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 order_factor()
BeginDeclarations
bit_16                                 unary_not;
EndDeclarations
BeginCode
 unary_not = False;
 While TokenIs(exclamation_string) OrIf
       TokenIs(tilde_string)       OrIf
       TokenIs(minus_string)       OrIf
       TokenIs(not_string)
  ) {
   get_order_token();
   unary_not ^= True;
  EndWhile;
 return(unary_not ^ order_primary());
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             order_primary                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 order_primary()
BeginDeclarations
group_entry_ptr                        group;
#define Group                          (*group)
bit_16                                 operand;
EndDeclarations
BeginCode
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
   if ( *String(token) IsNot '['
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
   if ( *String(token) IsNot '['
    ) {
     linker_error(8, "Expression syntax error:\n"
                     "\t\"%Fs\"\n",
                     String(ordering.val));
    };
   cut_string(token, 0, 1);
   group = Active_segment.owning_group;
   if ( group IsNull
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
   if ( *String(token) IsNot '['
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
EndCode
#undef Group

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              order_prologue                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void order_prologue()
BeginDeclarations
group_entry_ptr                        group;
bit_32                                 length;
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
public_entry_ptr                       next_communal;
bit_16                                 offset;
public_entry_ptr                       pub;
#define Pub                            (*pub)
bit_16                                 size;
EndDeclarations
BeginCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  | Compute the address base for the executable image.  For .COM files this |
  | will be 100H, for .EXE and .SYS files, this will be 0.                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 if ( comfile.val IsTrue
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

 if ( ordering.val IsNull
  ) {
   if ( DOSSEG.val IsTrue
    ) {
     ordering.val = duplicate_string(Addr(static_pool),
      string((byte *) ("(seg[*code]|seg[*CODE], "
                       "!(gr[dgroup]|gr[DGROUP]), "
                       "cl[begdata]|cl[BEGDATA], "
                       "cl[*data]|cl[*DATA], "
                       "!(cl[*bss]|cl[*BSS]|cl[*stack]|cl[*STACK]), "
                       "cl[*bss]|cl[*BSS], "
                       "cl[*stack]|cl[*STACK])")));
    } else {
     ordering.val = duplicate_string(Addr(static_pool),
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

 if ( exefile IsFalse
  ) {
   if ( stack_segment_found IsTrue
    ) {
     linker_error(4, "Stack segment found for a non .EXE file.\n");
    };
  } else {
   if ( (stack_segment_found IsFalse) AndIf
      (stack.set           IsFalse)
    ) {
     linker_error(4, "No stack segment for .EXE file.\n");
    } else {
     if ( (stack.set IsTrue) AndIf 
        (Bit_16(Largest_stack_seg.length) LessThan stack.val)
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

 if ( near_communals IsNotNull
  ) {
   length = 0L;
   For pub=near_communals; pub IsNotNull; pub=Pub.Communal.next_communal
    BeginFor
     LoopIf(Pub.type_entry IsNot near_communal);
     length += Pub.Communal.element_size;
    EndFor;
   if ( length Exceeds 65536L
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
   For pub=near_communals; pub IsNotNull; pub=next_communal
    BeginFor
     next_communal        = Pub.Communal.next_communal;
     LoopIf(Pub.type_entry IsNot near_communal);
     size                 = Bit_16(Pub.Communal.element_size);
     Pub.type_entry       = internal;
     Pub.Internal.group   = group;
     Pub.Internal.lseg    = lseg;
     Pub.Internal.offset  = offset;
     Pub.Internal.frame   = 0;
     offset              += size;
    EndFor;
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
 For pub=far_communals; pub IsNotNull; pub=next_communal
  BeginFor
   next_communal = Pub.Communal.next_communal;
   LoopIf(Pub.type_entry IsNot far_communal); 
   length = Pub.Communal.element_size * Pub.Communal.element_count;
   if ( length Exceeds 65536L
    ) {
     Pub.Communal.next_communal = huge_communals;
     huge_communals             = pub;
     ContinueLoop;
    };
   if ( (lseg IsNull) OrIf ((length + Bit_32(offset)) Exceeds 65536L)
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
  EndFor;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |  Handle huge communals as follows:  Taking as many segments as          |
  |  required, create private segments with the segment and class           |
  |  name of "HUGE_BSS".  They do NOT go in DRGROUP.                        |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 For pub=huge_communals; pub IsNotNull; pub=next_communal
  BeginFor
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
   if ( Pub.Communal.element_size Exceeds 65536L
    ) {
     linker_error(4, "Communal \"%Fs\" has element size exceeding 64K.\n",
                     Pub.symbol);
     offset = 0;
    } else {
     offset = Bit_16(65536L Mod Pub.Communal.element_size);
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
  EndFor;

 return;
EndCode
#undef Pub
#undef Lseg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               order_term                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 order_term()
BeginDeclarations
bit_16                                 left_operand;
EndDeclarations
BeginCode
 left_operand = order_factor();
 While TokenIs(and_string)       OrIf
       TokenIs(star_string)      OrIf
       TokenIs(ampersand_string)
  ) {
   get_order_token();
   left_operand &= order_factor();
  EndWhile;
 return(left_operand);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           order_token_get_char                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void order_token_get_char()
BeginDeclarations
EndDeclarations
BeginCode
 if ( token_break_char Is '\000'
  ) {
   linker_error(8, "Expression syntax error:\n"
                   "\t\"%Fs\"\n",
                   String(ordering.val));
  };
 token_break_char = *order_expression_char_ptr++;
 return;
EndCode
