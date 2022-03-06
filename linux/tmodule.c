/*                                 TMODULE.C                               */

/*
Object modules are parsed via recursive descent as defined below:

     obj_t_module::      obj_THEADR obj_seg_grp {obj_component} obj_modtail

     obj_seg_grp::       {obj_LNAMES | obj_SEGDEF | obj_EXTDEF}
                         {obj_TYPDEF | obj_EXTDEF | obj_GRPDEF}

     obj_component::     obj_data | obj_debug_record

     obj_data::          obj_content_def | obj_thread_def | obj_COMDEF |
                         obj_TYPDEF | obj_PUBDEF | obj_EXTDEF |
                         obj_FORREF | obj_MODPUB | obj_MODEXT


     obj_debug_record::  obj_LINNUM

     obj_content_def::   obj_data_record {obj_FIXUPP}

     obj_thread_def::    obj_FIXUPP  (containing only thread fields)

     obj_data_record::   obj_LIDATA | obj_LEDATA

     obj_modtail::       obj_MODEND
*/

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_COMDEF                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_COMDEF()
{
bit_32                                 element_count;
bit_32                                 element_size;
bit_8                                  element_type;
bit_8                                  expected_type;
bit_16                                 len;
public_entry_ptr                       pub;
#define Pub                            (*pub)


 if ( Current_record_header.rec_typ IsNot COMDEF_record
  ) {
   return(False);
  };
 while ( obj_ptr.b8 IsNot end_of_record.b8
  ) {
   if ( n_externals >= max_externals.val
    ) {
     linker_error(12, "Internal limit exceeded:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Too many externals.  Max of %u exceeded.\n"
                      "\t         Retry with larger \"/maxexternals:n\" "
                                  "switch.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      max_externals.val);
    };
   len         = obj_name_length();
   if ( case_ignore.val
    ) {
     far_to_lower(BytePtr(obj_ptr.b8), len);
    };
   pub         = lookup_public(len, obj_ptr.b8, 0);
   obj_ptr.b8 += len;
   obj_name_length();  /* Eat the type index. */
   externals[++n_externals] = pub;
   element_type  = *obj_ptr.b8++;
   switch ( element_type
    ) {
     case 0x61:
      expected_type = far_communal;
      element_count = obj_leaf_descriptor();
      element_size  = obj_leaf_descriptor();
      break;
     case 0x62:
      expected_type = near_communal;
      element_size  = obj_leaf_descriptor();
      element_count = 1L;
      break;
     default:
      linker_error(12, "Translator error:\n"
                       "\tModule:  \"%Fs\"\n"
                       "\t  File:  \"%Fs\"\n"
                       "\tOffset:  %lu\n"
                       "\t Error:  Communal type of \"%02X\" is illegal.\n",
                       (*tmodule_name).symbol,
                       (*infile.file_info).filename,
                       current_record_offset,
                       element_type);
    };
   if ( Pub.type_entry Is unused
    ) {
     Insert pub AtEnd InList external_list EndInsert;
     Pub.type_entry             = expected_type;
     Pub.Communal.element_size  = element_size;
     Pub.Communal.element_count = element_count;
     switch ( element_type
     ) {
      case 0x61:
       Pub.Communal.next_communal = far_communals;
       far_communals              = pub;
       break;
      case 0x62:
       Pub.Communal.next_communal = near_communals;
       near_communals             = pub;
       break;
     };
    } else {
     if ( Pub.type_entry Is expected_type
      ) {
       if ( (element_size              * element_count)              > 
          (Pub.Communal.element_size * Pub.Communal.element_count)
        ) { /* We need the largest common */
         Pub.Communal.element_size  = element_size;
         Pub.Communal.element_count = element_count;
        };
      } else {
       if ( (Pub.type_entry Is near_communal) OrIf
          (Pub.type_entry Is far_communal)
        ) {
         linker_error(4, "Translator error:\n"
                         "\tModule:  \"%Fs\"\n"
                         "\t  File:  \"%Fs\"\n"
                         "\tOffset:  %lu\n"
                         "\t Error:  Communal \"%Fs\" is declared both near "
                                     "and far.\n",
                         (*tmodule_name).symbol,
                         (*infile.file_info).filename,
                         current_record_offset,
                         Pub.symbol);
        };
      };
    };
  };
 obj_next_record();
 return(True);
}
#undef Pub

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_COMENT                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_COMENT()
{
bit_8                                  comment_class;


 if ( Current_record_header.rec_typ IsNot COMENT_record
  ) {
   return(False);
  };
 obj_ptr.b8++;
 comment_class = *obj_ptr.b8++;
 switch ( comment_class
  ) {
   case 158:
    DOSSEG.val = True;
    break;
   case 161:
    codeview_information_present = True;
    break;
   default:
    break;
  };
 return(True);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             obj_component                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/* obj_component:: obj_data | obj_debug_record */
bit_16 obj_component()
{


 if ( obj_data() OrIf obj_debug_record()
  ) {
   return(True);
  };
 return(False);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            obj_content_def                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/* obj_content_def:: obj_data_record {obj_FIXUPP} */
bit_16 obj_content_def()
{


 if ( Not obj_data_record()
  ) {
   return(False);
  };
 while ( obj_FIXUPP()
  ) {
  };
 return(True);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_data                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/* obj_data:: obj_content_def |
              obj_thread_def  |
              obj_TYPDEF      |
              obj_PUBDEF      |
              obj_EXTDEF */

bit_16 obj_data()
{


 if ( obj_content_def() OrIf
    obj_thread_def()  OrIf
    obj_TYPDEF()      OrIf
    obj_PUBDEF()      OrIf
    obj_EXTDEF()      OrIf
    obj_FORREF()      OrIf
    obj_COMDEF()      OrIf
    obj_MODEXT()      OrIf
    obj_MODPUB()
  ) {
   return(True);
  };
 return(False);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            obj_data_record                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/*  obj_data_record:: obj_LIDATA | obj_LEDATA */

bit_16 obj_data_record()
{


 if ( obj_LIDATA() OrIf obj_LEDATA()
  ) {
   return(True);
  };
 return(False);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           obj_debug_record                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/* obj_debug_record:: obj_LINNUM */

bit_16 obj_debug_record()
{


 if ( obj_LINNUM()
  ) {
   return(True);
  };
 return(False);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_EXTDEF                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_EXTDEF()
{
bit_16                                 len;
public_entry_ptr                       pub;
#define Pub                            (*pub)


 if ( Current_record_header.rec_typ IsNot EXTDEF_record
  ) {
   return(False);
  };
 while ( obj_ptr.b8 IsNot end_of_record.b8
  ) {
   if ( n_externals >= max_externals.val
    ) {
     linker_error(12, "Internal limit exceeded:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Too many externals.  Max of %u exceeded.\n"
                      "\t         Retry with larger \"/maxexternals:n\" "
                                  "switch.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      max_externals.val);
    };
   len         = obj_name_length();
   if ( case_ignore.val
    ) {
     far_to_lower(BytePtr(obj_ptr.b8), len);
    };
   pub         = lookup_public(len, obj_ptr.b8, 0);
   obj_ptr.b8 += len;
   obj_name_length();  /* Eat the type index. */
   externals[++n_externals] = pub;
   if ( Pub.type_entry Is unused
    ) {
     Insert pub AtEnd InList external_list EndInsert;
     Pub.type_entry = external;
    } else {
     if ( (Pub.type_entry Is public_in_library) AndIf
        (Not Pub.Library.requested)
      ) {
       library_request_count++;
       (*Pub.Library.lib_file).request_count++;
       Pub.Library.requested = True;
      };
    };
  };
 obj_next_record();
 return(True);
}
#undef Pub

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_FIXUPP                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_FIXUPP()
{


 if ( Current_record_header.rec_typ IsNot FIXUPP_record
  ) {
   return(False);
  };
 FIXUPP_contains_only_threads = True;
 while ( obj_ptr.b8 IsNot end_of_record.b8
  ) {
   if ( (*obj_ptr.TRD_DAT).type_fixupp_record IsZero
    ) {
     obj_FIXUPP_thread();
    } else {
     FIXUPP_contains_only_threads = False;
     obj_FIXUPP_fixup();
    };
  };
 obj_next_record();
 return(True);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           obj_FIXUPP_fixup                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void obj_FIXUPP_fixup()
{
FIX_DAT_type                           FIX_DAT;
bit_16                                 frame_method;
LOCAT_type                             LOCAT;
bit_16                                 target_method;
bit_8                                  temp;
bit_16                                 thread_number;


/*+-------------------------------------------------------------------------+
  |                                                                         |
  | The LOCAT field in a FIXUPP record has its low and high bytes swapped   |
  | because the high order bit must be 0 for threads and 1 for fixups.      |
  | Since that bit could not be placed in the offset, the bytes were        |
  | swapped instead.                                                        |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 temp                  = obj_ptr.b8[0];
 obj_ptr.b8[0]         = obj_ptr.b8[1];
 obj_ptr.b8[1]         = temp;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |          Pick up the two required fields (LOCAT and FIX_DAT)            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 LOCAT                 = *obj_ptr.LOCAT++;
 FIX_DAT               = *obj_ptr.FIX_DAT++;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |          A fixup consists of a location, mode, frame and target.        |
  |                         Process the location part.                      |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 fixup_index           = LOCAT.data_record_offset;
 fixup.location_type   = LOCAT.loc;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         Process the mode part.                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 fixup.mode            = LOCAT.m;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         Process the frame part.                         |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 if ( FIX_DAT.f IsZero
  ) {  /* Frame is specified explicitly */
   frame_method         = FIX_DAT.frame;
   fixup.frame_method   = frame_method;
   switch ( frame_method
    ) {
     case 0:
      fixup.frame_referent = (void far *) snames[obj_index_segment()];
      break;
     case 1:
      fixup.frame_referent = (void far *) gnames[obj_index_group()];
      break;
     case 2:
      fixup.frame_referent = (void far *) externals[obj_index_external()];
      break;
     case 3:
      fixup.frame_referent =
       (void far *) (Bit_32(*obj_ptr.b16++) ShiftedLeft 4);
     default:
      fixup.frame_referent = Null;
      break;
    };
  } else {  /* Frame is specified by a thread */
   thread_number        = FIX_DAT.frame;
   if ( Not frame_thread[thread_number].thread_defined
    ) {
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Reference to frame thread %u which has "
                                  "been defined.n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      thread_number);
    };
   fixup.frame_referent = frame_thread[thread_number].referent;
   fixup.frame_method   = frame_thread[thread_number].method;
  };

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         Process the target part.                        |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 if ( FIX_DAT.t IsZero
  ) {  /* Target is specified explicitly */
   target_method       = FIX_DAT.targt;
   fixup.target_method = target_method;
   switch ( target_method
    ) {
     case 0:  /* Target is the segment referenced by the index */
      fixup.target_referent = (void far *) snames[obj_index_segment()];
      break;
     case 1:  /* Target is the lowest seg in the group referenced 
                 by the index */
      fixup.target_referent = (void far *) gnames[obj_index_group()];
      break;
     case 2:
      fixup.target_referent = (void far *) externals[obj_index_external()];
      break;
     case 3:
      fixup.target_referent =
       (void far *) (Bit_32(*obj_ptr.b16++) ShiftedLeft 4);
      break;
    };
  } else {  /* Target is specified by a thread */
   thread_number         = FIX_DAT.targt;
   if ( Not target_thread[thread_number].thread_defined
    ) {
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Reference to target thread %u which has "
                                  "been defined.n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      thread_number);
    };
   fixup.target_referent = target_thread[thread_number].referent;
   fixup.target_method   = target_thread[thread_number].method;
  };

 if ( FIX_DAT.p IsZero
  ) {  /* There is a target displacement */
   fixup.target_offset = *obj_ptr.b16++;
  } else {  /* The target displacement is zero */
   fixup.target_offset = 0;
  };

 fixup.external_error_detected = False;

 if ( (fixup.mode IsZero) AndIf
                             ((fixup.location_type Is base_location)    OrIf
                              (fixup.location_type Is pointer_location) OrIf
                              (fixup.location_type Is hibyte_location))
  ) { /* Undefined fixup action */
   linker_error(4, "Possible translator error:\n"
                   "\tModule:  \"%Fs\"\n"
                   "\t  File:  \"%Fs\"\n"
                   "\tOffset:  %lu\n"
                   "\t Error:  Base, pointer or hibyte self-relative fixups\n"
                   "\t         are undefined.\n",
                   (*tmodule_name).symbol,
                   (*infile.file_info).filename,
                   current_record_offset);
  };

 if ( last_LxDATA_record_type Is LEDATA_record
  ) {
   if ( ((fixup.location_type Is base_location)     OrIf
       (fixup.location_type Is pointer_location)) AndIf
      (exefile != 0)
    ) { /* Base and pointer locations will require a relocation item
            in the EXE header */
     n_relocation_items++;
    };
   write_temp_file(Current_record_header.rec_typ,
                   last_LxDATA_lseg,
                   last_LxDATA_offset + fixup_index,
                   BytePtr(Addr(fixup)),
                   sizeof(fixup));
  } else {
   if ( fixup.mode IsZero
    ) {
     linker_error(4, "Translator warning:\n"
                     "\tModule:  \"%Fs\"\n"
                     "\t  File:  \"%Fs\"\n"
                     "\tOffset:  %lu\n"
                     "\t Error:  Self-relative fixup not permitted for "
                                 "LIDATA.\n",
                     (*tmodule_name).symbol,
                     (*infile.file_info).filename,
                     current_record_offset);
    } else {
     obj_fixup_LIDATA();
    };
  };

 return;
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            obj_fixup_LIDATA                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void obj_fixup_LIDATA()
{
obj_ptr_type                           old_obj_ptr;


 LIDATA_index  = 0;
 LIDATA_offset = last_LxDATA_offset;
 old_obj_ptr   = obj_ptr;
 obj_ptr.b8    = Last_LIDATA_record_header.variant_part;
 end_of_last_LIDATA_record.b8 =
  (byte *)
   Addr(Last_LIDATA_record_header.variant_part
    [Last_LIDATA_record_header.rec_len-1]);
 obj_index_segment();
 obj_ptr.b16++;
 while ( obj_ptr.b8 IsNot end_of_last_LIDATA_record.b8
  ) {
   obj_fixup_LIDATA_IDB();
  };
 obj_ptr = old_obj_ptr;
 return;
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           obj_fixup_LIDATA_IDB                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void obj_fixup_LIDATA_IDB()
{
bit_16                                 block_count;
bit_8                                 *content;
bit_16                                 i;
bit_16                                 j;
bit_16                                 len;
bit_16                                 old_index;
bit_16                                 repeat_count;


 repeat_count = *obj_ptr.b16++;  LIDATA_index += sizeof(bit_16);
 block_count  = *obj_ptr.b16++;  LIDATA_index += sizeof(bit_16);
 content      = obj_ptr.b8;
 old_index    = LIDATA_index;
 if ( block_count IsNotZero
  ) {  /* Handle recursive case:  Content is iterated data block */
   for ( i=0; i<repeat_count; i++
    ) {
     obj_ptr.b8 = content;
     LIDATA_index = old_index;
     for ( j=0; j<block_count; j++
      ) {
       obj_fixup_LIDATA_IDB();
      };
    };
  } else {  /* Handle non-recursive case:  Content is data. */
   for ( i=0; i<repeat_count; i++
    ) {
     obj_ptr.b8   = content;
     LIDATA_index = old_index;
     len          = Bit_16(*obj_ptr.b8++);  LIDATA_index += sizeof(bit_8);
     if ( (fixup_index >= LIDATA_index)        AndIf
        (fixup_index <   (LIDATA_index + len))
      ) {
       write_temp_file(Current_record_header.rec_typ,
                       last_LxDATA_lseg,
                       LIDATA_offset + fixup_index - LIDATA_index,
                       BytePtr(Addr(fixup)),
                       sizeof(fixup));
       if ( ((fixup.location_type Is base_location)     OrIf
           (fixup.location_type Is pointer_location)) AndIf
          (exefile != 0)
        ) { /* Base and pointer locations will require a relocation item
                in the EXE header */
         n_relocation_items++;
        };
      };
     LIDATA_offset += len;
    };
   obj_ptr.b8    += len;
   LIDATA_index  += len;
  };
 return;
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           obj_FIXUPP_thread                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void obj_FIXUPP_thread()
{
bit_16                                 method;
bit_16                                 thread;
TRD_DAT_type                           TRD_DAT;


 TRD_DAT = *obj_ptr.TRD_DAT++;
 thread  = TRD_DAT.thred;
 method  = TRD_DAT.method;
 if ( TRD_DAT.d IsZero
  ) {  /* This is a target thread */
   target_thread[thread].method = Bit_8(method);
   target_thread[thread].thread_defined = True;
   switch ( method
    ) {
     case 0:
      target_thread[thread].referent =
       (void far *) snames[obj_index_segment()];
      break;
     case 1:
      target_thread[thread].referent =
       (void far *) gnames[obj_index_group()];
      break;
     case 2:
      target_thread[thread].referent =
       (void far *) externals[obj_index_external()];
      break;
     case 3:
      target_thread[thread].referent =
       (void far *) (Bit_32(*obj_ptr.b16++) ShiftedLeft 4);
     default:
      target_thread[thread].referent = Null;
      break;
    };
  } else {  /* This is a frame thread */
   frame_thread[thread].method = Bit_8(method);
   frame_thread[thread].thread_defined = True;
   switch ( method
    ) {
     case 0:
      frame_thread[thread].referent =
       (void far *) snames[obj_index_segment()];
      break;
     case 1:
      frame_thread[thread].referent =
       (void far *) gnames[obj_index_group()];
      break;
     case 2:
      frame_thread[thread].referent =
       (void far *) externals[obj_index_external()];
      break;
     case 3:
      frame_thread[thread].referent =
       (void far *) (Bit_32(*obj_ptr.b16++) ShiftedLeft 4);
     default:
      frame_thread[thread].referent = Null;
      break;
    };
  };
 return;
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_FORREF                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_FORREF()
{
bit_16                                 len;
bit_16                                 segment_index;


 if ( Current_record_header.rec_typ IsNot FORREF_record
  ) {
   return(False);
  };
 segment_index = obj_index_segment();
 len           = Current_record_header.rec_len - 2;
 if ( segment_index > 127
  ) {
   len--;
  };
 write_temp_file(Current_record_header.rec_typ,
                 snames[segment_index],
                 0,
                 BytePtr(obj_ptr.b8),
                 len);
 obj_next_record();
 return(True);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           obj_generate_segment                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
lseg_ptr obj_generate_segment(lname_entry_ptr segment_lname,
                              lname_entry_ptr class_lname,
                              combine_type    combine,
                              bit_8           align,
                              lname_entry_ptr tmodule,
                              file_info_ptr   file,
                              bit_32          address,
                              bit_32          length)
{
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
segment_entry_ptr                      seg;
#define Seg                            (*seg)


 if ( combine Is stack_combine
  ) {
   length += AlignmentGap(length, 1L); /* Stacks should be an integral
                                          number of words. */
  };
 seg             = lookup_segment(segment_lname, class_lname, combine);
 if ( (combine IsNot common_combine) OrIf (Seg.lsegs.first IsNull)
  ) {
   Seg.address   = address;
   Seg.length   += length;
   lseg          = (lseg_ptr) 
                    allocate_memory(Addr(static_pool),
                                    Bit_32(sizeof(lseg_type)));
   Lseg.segment  = seg;
   Lseg.tmodule  = tmodule;
   Lseg.file     = file;
   Lseg.address  = address;
   Lseg.length   = length;
   Lseg.align    = align;
   if ( (combine IsNot common_combine)      AndIf
      (combine IsNot blank_common_combine)
    ) {  /* Don't allocate common data yet.  (We will wait until we
             know how long the common block will be.) */
     Lseg.data   = allocate_memory(Addr(static_pool), length);
    };

   Lseg.highest_uninitialized_byte = 0L;

   Insert lseg AtEnd InList Seg.lsegs EndInsert;
  } else {  /* Not the first occurrence of this common */
   lseg = Seg.lsegs.first;
   if ( length > Seg.length
    ) {  /* Expand common block to be big enough to hold this entry. */
     Seg.length  =
     Lseg.length = length;
    };
   if ( align > Lseg.align
    ) {  /* Align to largest boundary. */
     Lseg.align = align;
    };
  };
 if ( Seg.combine Is stack_combine
  ) {
   if ( Not stack_segment_found
    ) {
     largest_stack_seg        = seg;
     largest_stack_seg_length = Seg.length;
     stack_segment_found      = True;
    } else {
     if ( Seg.length > largest_stack_seg_length
      ) {
       largest_stack_seg        = seg;
       largest_stack_seg_length = Seg.length;
      };
    };
  };
 return(lseg);
}
#undef Lseg 
#undef Seg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_GRPDEF                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_GRPDEF()
{
group_entry_ptr                        group;
#define Group                          (*group)
bit_16                                 group_index;
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
segment_entry_ptr                      seg;
#define Seg                            (*seg)
bit_16                                 segment_index;


 if ( Current_record_header.rec_typ IsNot GRPDEF_record
  ) {
   return(False);
  };
 group_index         = obj_index_LNAME();
 group               = lookup_group(lnames[group_index]);
 if ( n_groups >= max_groups.val
  ) {
   linker_error(12, "Internal limit exceeded:\n"
                    "\tModule:  \"%Fs\"\n"
                    "\t  File:  \"%Fs\"\n"
                    "\tOffset:  %lu\n"
                    "\t Error:  Too many GRPDEFs.  Max of %u exceeded.\n"
                    "\t         Retry with larger \"/maxgroups:n\" switch.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset,
                    max_groups.val);
  };
 gnames[++n_groups]  = group;
 while ( obj_ptr.b8 IsNot end_of_record.b8
  ) {
   if ( *obj_ptr.b8++ IsNot 0xFF
    ) {
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  GRPDEF record has a group component "
                                  "descriptor which\n"
                      "\t         does not start with 0xFF.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset);
    };
   segment_index = obj_index_segment();
   lseg          = snames[segment_index];
   seg           = Lseg.segment;
   if ( Seg.owning_group IsNull
    ) {
     Seg.owning_group = group;
    } else {
     if ( Seg.owning_group IsNot group
      ) {
       linker_error(4, "Attempt to place segment \"%Fs\" into group \"%Fs\"\n"
                       "\twhen it is already in group \"%Fs\".\n"
                       "\tRequest to place in group \"%Fs\" ignored.\n",
                       (*Seg.segment_name).symbol, (*Group.group_name).symbol,
                       (*(*Seg.owning_group).group_name).symbol,
                       (*Group.group_name).symbol);
     };
    };
  };
 obj_next_record();
 return(True);
}
#undef Seg
#undef Group
#undef Lseg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           obj_index_external                            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_index_external()
{
bit_16                                 index;


 if ( *obj_ptr.b8 < 128
  ) {
   index = Bit_16(*obj_ptr.b8++);
  } else {
   index = (Bit_16(*obj_ptr.b8++ - 128) ShiftedLeft 8) +
           Bit_16(*obj_ptr.b8++);
  };
   if ( index > n_externals
    ) {
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Invalid external index (%u) with only %u "
                                  "externals defined.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      index, n_externals);
    };
 return(index);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             obj_index_group                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_index_group()
{
bit_16                                 index;


 if ( *obj_ptr.b8 < 128
  ) {
   index = Bit_16(*obj_ptr.b8++);
  } else {
   index = (Bit_16(*obj_ptr.b8++ - 128) ShiftedLeft 8) +
           Bit_16(*obj_ptr.b8++);
  };
   if ( index > n_groups
    ) {
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Invalid group index (%u) with only %u "
                                  "groups defined.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      index, n_groups);
    };
 return(index);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             obj_index_LNAME                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_index_LNAME()
{
bit_16                                 index;


 if ( *obj_ptr.b8 < 128
  ) {
   index = Bit_16(*obj_ptr.b8++);
  } else {
   index = (Bit_16(*obj_ptr.b8++ - 128) ShiftedLeft 8) +
           Bit_16(*obj_ptr.b8++);
  };
   if ( index > n_lnames
    ) {
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Invalid LNAME index (%u) with only %u "
                                  "LNAMEs defined.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      index, n_lnames);
    };
 return(index);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           obj_index_segment                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_index_segment()
{
bit_16                                 index;


 if ( *obj_ptr.b8 < 128
  ) {
   index = Bit_16(*obj_ptr.b8++);
  } else {
   index = (Bit_16(*obj_ptr.b8++ - 128) ShiftedLeft 8) +
           Bit_16(*obj_ptr.b8++);
  };
   if ( index > n_segments
    ) {
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Invalid segment index (%u) with only %u "
                                  "segments defined.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      index, n_segments);
    };
 return(index);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         obj_iterated_data_block                         |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void obj_iterated_data_block()
{
bit_16                                 block_count;
bit_8                                 *content;
bit_16                                 i;
bit_16                                 j;
bit_16                                 len;
bit_16                                 repeat_count;


 repeat_count = *obj_ptr.b16++;
 block_count  = *obj_ptr.b16++;
 if ( block_count IsNotZero
  ) {  /* Handle recursive case:  Content is iterated data block */
   content = obj_ptr.b8;
   for ( i=0; i<repeat_count; i++
    ) {
     obj_ptr.b8 = content;
     for ( j=0; j<block_count; j++
      ) {
       obj_iterated_data_block();
      };
    };
  } else {  /* Handle non-recursive case:  Content is data. */
   len = Bit_16(*obj_ptr.b8++);
   for ( i=0; i<repeat_count; i++
    ) {
     far_move(Addr(last_LxDATA_Lseg.data[LIDATA_offset]), 
              obj_ptr.b8, len);
     LIDATA_offset += len;
    };
   obj_ptr.b8 += len;
  };
 return;
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                     obj_iterated_data_block_length                      |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 obj_iterated_data_block_length()
{
bit_16                                 block_count;
bit_16                                 i;
bit_16                                 len;
bit_32                                 length;
bit_16                                 repeat_count;


 repeat_count = *obj_ptr.b16++;
 block_count  = *obj_ptr.b16++;
 if ( repeat_count IsZero
  ) { /* This is a translator error. */
   linker_error(12, "Translator error:\n"
                    "\tModule:  \"%Fs\"\n"
                    "\t  File:  \"%Fs\"\n"
                    "\tOffset:  %lu\n"
                    "\t Error:  Repeat count in LIDATA iterated data block "
                                "is zero.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset);
  };
 length       = 0L;
 if ( block_count IsNotZero
  ) {  /* Handle recursive case:  Content is iterated data block */
   for ( i=0; i<block_count; i++
    ) {
     length     += Bit_32(repeat_count) * obj_iterated_data_block_length();
    };
  } else {  /* Handle non-recursive case:  Content is data. */
   len         = Bit_16(*obj_ptr.b8++);
   obj_ptr.b8 += len;
   length      = Bit_32(repeat_count) * Bit_32(len);
  };
 return(length);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           obj_leaf_descriptor                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 obj_leaf_descriptor()
{
bit_8                                  element_size;


 element_size = *obj_ptr.b8++;
 if ( element_size < 129
  ) {
   return(Bit_32(element_size));
  } else {
   if ( element_size Is 129
    ) {
     return(Bit_32(*obj_ptr.b16++));
    } else {
     if ( element_size Is 132
      ) {
       obj_ptr.b8--;
       return((*obj_ptr.b32++) And 0x00FFFFFFL);
      } else {
       if ( element_size Is 136
        ) {
         return(*obj_ptr.b32++);
        } else {
         linker_error(12, "Translator error:\n"
                        "\tModule:  \"%Fs\"\n"
                        "\t  File:  \"%Fs\"\n"
                        "\tOffset:  %lu\n"
                        "\t Error:  Communal element size of %u is illegal.\n",
                        (*tmodule_name).symbol,
                        (*infile.file_info).filename,
                        current_record_offset,
                        element_size);
        };
      };
    };
  };
 return(0L);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_LEDATA                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_LEDATA()
{
bit_32                                 next_byte;
bit_16                                 len;
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
bit_16                                 offset;
bit_16                                 segment_index;


 if ( Current_record_header.rec_typ IsNot LEDATA_record
  ) {
   return(False);
  };
 last_LxDATA_record_type = Current_record_header.rec_typ;
 segment_index           = obj_index_segment();
 last_LxDATA_lseg        =
 lseg                    = snames[segment_index];
 len                     = Current_record_header.rec_len - 4;
 if ( segment_index > 127
  ) {
   len--;
  };
 last_LxDATA_offset =
 offset             = *obj_ptr.b16++;
 next_byte          = Bit_32(offset) + Bit_32(len);
 if ( next_byte > Lseg.length
  ) {
   linker_error(12, "Translator error:\n"
                    "\tModule:  \"%Fs\"\n"
                    "\t  File:  \"%Fs\"\n"
                    "\tOffset:  %lu\n"
                    "\t Error:  Attempt to initialize past end of LSEG.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset);
  };
 if ( next_byte > Lseg.highest_uninitialized_byte
  ) {
   Lseg.highest_uninitialized_byte = next_byte;
  };
 if ( (*last_LxDATA_Lseg.segment).combine IsNot common_combine
  ) {
   far_move(Addr(Lseg.data[offset]), obj_ptr.b8, len);
  } else {  /* We must save the initialization data out to the tmp file until
           later when we know the length. */
   write_temp_file(Current_record_header.rec_typ,
                   last_LxDATA_lseg,
                   last_LxDATA_offset,
                   BytePtr(obj_ptr.b8),
                   len);
  };
 obj_next_record();
 return(True);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_LIDATA                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_LIDATA()
{
bit_16                                 len;
bit_32                                 LIDATA_length;
bit_32                                 next_byte;
bit_16                                 segment_index;


 if ( Current_record_header.rec_typ IsNot LIDATA_record
  ) {
   return(False);
  };
 far_move(BytePtr(last_LIDATA_record), 
          BytePtr(object_file_element),
          Current_record_header.rec_len + sizeof(obj_record_header_type) - 1);
 last_LxDATA_record_type = Current_record_header.rec_typ;
 segment_index           = obj_index_segment();
 last_LxDATA_lseg        = snames[segment_index];
 LIDATA_offset           =
 last_LxDATA_offset      = *obj_ptr.b16++;
 LIDATA_length           = obj_LIDATA_length();
 next_byte               = last_LxDATA_offset + LIDATA_length;
 if ( next_byte > last_LxDATA_Lseg.length
  ) {
   linker_error(12, "Translator error:\n"
                    "\tModule:  \"%Fs\"\n"
                    "\t  File:  \"%Fs\"\n"
                    "\tOffset:  %lu\n"
                    "\t Error:  Attempt to initialize past end of LSEG.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset);
  };
 if ( next_byte > last_LxDATA_Lseg.highest_uninitialized_byte
  ) {
   last_LxDATA_Lseg.highest_uninitialized_byte = next_byte;
  };
 if ( (*last_LxDATA_Lseg.segment).combine IsNot common_combine
  ) {
   while ( obj_ptr.b8 IsNot end_of_record.b8
   ) {
   obj_iterated_data_block();
   };
  } else {  /* We must save the initialization data out to the tmp file until
           later when we know the length. */
   len                     = Current_record_header.rec_len - 4;
   if ( segment_index > 127
    ) {
     len--;
    };
   write_temp_file(Current_record_header.rec_typ,
                   last_LxDATA_lseg,
                   last_LxDATA_offset,
                   BytePtr(obj_ptr.b8),
                   len);
  };
 obj_next_record();
 return(True);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           obj_LIDATA_length                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 obj_LIDATA_length()
{
bit_32                                 length;
bit_8                                  *start;


 start  = obj_ptr.b8;
 length = 0L;
 while ( obj_ptr.b8 IsNot end_of_record.b8
  ) {
   length += obj_iterated_data_block_length();
  };
 obj_ptr.b8 = start;
 return(length);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_LINNUM                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_LINNUM()
{


 if ( Current_record_header.rec_typ IsNot LINNUM_record
  ) {
   return(False);
  };
 obj_next_record();
 return(True);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_LNAMES                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_LNAMES()
{


 if ( Current_record_header.rec_typ IsNot LNAMES_record
  ) {
   return(False);
  };
 while ( obj_ptr.b8 IsNot end_of_record.b8
  ) {
   if ( n_lnames >= max_lnames.val
    ) {
     linker_error(12, "Internal limit exceeded:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Too many LNAMES.  Max of %u exceeded.\n"
                      "\t         Retry with larger \"/maxlnames:n\" switch.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      max_lnames.val);
    };
   lnames[++n_lnames] = obj_name();
  };
 obj_next_record();
 return(True);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_MODEND                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_MODEND()
{
FIX_DAT_type                           END_DAT;
bit_16                                 frame_method;
MOD_TYP_type                           MOD_TYP;
bit_16                                 target_method;
bit_16                                 thread_number;


 if ( Current_record_header.rec_typ IsNot MODEND_record
  ) {
   return(False);
  };
 MOD_TYP = *obj_ptr.MOD_TYP++;

 if ( MOD_TYP.zeros IsNotZero
  ) {
   linker_error(4, "Translator error:\n"
                   "\tModule:  \"%Fs\"\n"
                   "\t  File:  \"%Fs\"\n"
                   "\tOffset:  %lu\n"
                   "\t Error:  Bits 1 thru 5 of MOD TYP must be zero.\n",
                   (*tmodule_name).symbol,
                   (*infile.file_info).filename,
                   current_record_offset);
  };

 if ( (MOD_TYP.mattr IsNot 1) AndIf (MOD_TYP.mattr IsNot 3)
  ) {  /* We have no starting address */
   return(True);
  };

 if ( MOD_TYP.l IsNot 1
  ) {
   linker_error(4, "Translator error:\n"
                   "\tModule:  \"%Fs\"\n"
                   "\t  File:  \"%Fs\"\n"
                   "\tOffset:  %lu\n"
                   "\t Error:  Bit 0 of MOD TYP must be one.\n",
                   (*tmodule_name).symbol,
                   (*infile.file_info).filename,
                   current_record_offset);
  };

 if ( start_address_found != 0
  ) {
   linker_error(4, "Multiple start address encountered.  The start address\n"
                   "in module \"%Fs\" of file \"%Fs\" has been ignored.\n",
                   (*tmodule_name).symbol,
                   (*infile.file_info).filename);
  };

 start_address_found = True;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                  Pick up the required field END_DAT.                    |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 END_DAT = *obj_ptr.FIX_DAT++;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         Process the frame part.                         |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 if ( END_DAT.f IsZero
  ) {  /* Frame is specified explicitly */
   frame_method                 = END_DAT.frame;
   start_address.frame_method   = frame_method;
   switch ( frame_method
    ) {
     case 0:
      start_address.frame_referent =
       (void far *) snames[obj_index_segment()];
      break;
     case 1:
      start_address.frame_referent =
       (void far *) gnames[obj_index_group()];
      break;
     case 2:
      start_address.frame_referent =
       (void far *) externals[obj_index_external()];
      break;
     case 3:
      start_address.frame_referent =
       (void far *) (Bit_32(*obj_ptr.b16++) ShiftedLeft 4);
     default:
      start_address.frame_referent = Null;
      break;
    };
  } else {  /* Frame is specified by a thread */
   thread_number                = END_DAT.frame;
   if ( Not frame_thread[thread_number].thread_defined
    ) {
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Reference to frame thread %u which has "
                                  "been defined.n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      thread_number);
    };
   start_address.frame_referent = frame_thread[thread_number].referent;
   start_address.frame_method   = frame_thread[thread_number].method;
  };

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         Process the target part.                        |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 if ( END_DAT.t IsZero
  ) {  /* Target is specified explicitly */
   target_method               = END_DAT.targt;
   start_address.target_method = target_method;
   switch ( target_method
    ) {
     case 0:
      start_address.target_referent =
       (void far *) snames[obj_index_segment()];
      break;
     case 1:
      start_address.target_referent =
       (void far *) gnames[obj_index_group()];
      break;
     case 2:
      start_address.target_referent =
       (void far *) externals[obj_index_external()];
      break;
     case 3:
      start_address.target_referent =
       (void far *) (Bit_32(*obj_ptr.b16++) ShiftedLeft 4);
      break;
    };
  } else {  /* Target is specified by a thread */
   thread_number                 = END_DAT.targt;
   if ( Not target_thread[thread_number].thread_defined
    ) {
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Reference to target thread %u which has "
                                  "been defined.n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      thread_number);
    };
   start_address.target_referent = target_thread[thread_number].referent;
   start_address.target_method   = target_thread[thread_number].method;
  };

 if ( END_DAT.p IsZero
  ) {  /* There is a target displacement */
   start_address.target_offset = *obj_ptr.b16++;
  } else {  /* The target displacement is zero */
   linker_error(12, "Translator error:\n"
                    "\tModule:  \"%Fs\"\n"
                    "\t  File:  \"%Fs\"\n"
                    "\tOffset:  %lu\n"
                    "\t Error:  Only primary fixups allowed in MODEND.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset);
   start_address.target_offset = 0;
  };
 return(True);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_MODEXT                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_MODEXT()
{
bit_16                                 len;
public_entry_ptr                       pub;
#define Pub                            (*pub)


 if ( Current_record_header.rec_typ IsNot MODEXT_record
  ) {
   return(False);
  };
 while ( obj_ptr.b8 IsNot end_of_record.b8
  ) {
   if ( n_externals >= max_externals.val
    ) {
     linker_error(12, "Internal limit exceeded:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Too many externals.  Max of %u exceeded.\n"
                      "\t         Retry with larger \"/maxexternals:n\" "
                                  "switch.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      max_externals.val);
    };
   len         = obj_name_length();
   if ( case_ignore.val
    ) {
     far_to_lower(BytePtr(obj_ptr.b8), len);
    };
   pub         = lookup_public(len, obj_ptr.b8, tmodule_number);
   obj_ptr.b8 += len;
   obj_name_length();  /* Eat the type index. */
   externals[++n_externals] = pub;
   if ( Pub.type_entry Is unused
    ) {
     Insert pub AtEnd InList external_list EndInsert;
     Pub.type_entry = external;
    };
  };
 obj_next_record();
 return(True);
}
#undef Pub

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_MODPUB                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_MODPUB()
{
bit_16                                 group_index;
bit_16                                 frame;
bit_16                                 len;
public_entry_ptr                       pub;
#define Pub                            (*pub)
bit_16                                 segment_index;


 if ( Current_record_header.rec_typ IsNot MODPUB_record
  ) {
   return(False);
  };
 group_index = obj_index_group();
 segment_index = obj_index_segment();
 if ( (segment_index IsZero) AndIf (group_index IsZero)
  ) {
   frame = *obj_ptr.b16++;
  };
 while ( obj_ptr.b8 IsNot end_of_record.b8
  ) {
   len = obj_name_length();
   if ( case_ignore.val
    ) {
     far_to_lower(BytePtr(obj_ptr.b8), len);
    };
   pub = lookup_public(len, obj_ptr.b8, tmodule_number);
   obj_ptr.b8 += len;
   if ( Pub.type_entry Is internal
    ) {
     linker_error(4, "Duplicate definition of public \"%Fs\".\n"
                     "\tDefinition in module \"%Fs\" of file \"%Fs\" "
                     "ignored.\n",
                     Pub.symbol,
                     (*tmodule_name).symbol,(*infile.file_info).filename);
     obj_ptr.b16++;      /* Eat offset. */
     obj_name_length();  /* Eat type index. */
    } else {
     if ( Pub.type_entry Is unused
      ) {
       Insert pub AtEnd InList external_list EndInsert;
      };
     Pub.type_entry       = internal;
     Pub.Internal.group   = gnames[group_index];
     Pub.Internal.lseg    = snames[segment_index];
     Pub.Internal.frame   = frame;
     Pub.Internal.offset  = *obj_ptr.b16++;
     obj_name_length();  /* Eat type index. */
    };
  };
 obj_next_record();
 return(True);
}
#undef Pub

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              obj_modtail                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/* obj_modtail:: obj_MODEND */

bit_16 obj_modtail()
{


 if ( obj_MODEND()
  ) {
   return(True);
  };
 return(False);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_name                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
lname_entry_ptr obj_name()
{
lname_entry_ptr                        name;
bit_16                                 len;


 len = obj_name_length();
 if ( len IsZero
  ) {
   name = none_lname;
  } else {
   if ( case_ignore.val
    ) {
     far_to_lower(BytePtr(obj_ptr.b8), len);
    };
   name        = lookup_lname(len, obj_ptr.b8);
   obj_ptr.b8 += len;
  };
 return(name);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            obj_name_length                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_name_length()
{


 if ( *obj_ptr.b8 < 128
  ) {
   return(Bit_16(*obj_ptr.b8++));
  } else {
   return((Bit_16(*obj_ptr.b8++ - 128) ShiftedLeft 8) +
          (Bit_16(*obj_ptr.b8++)));
  };
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             obj_next_record                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void obj_next_record()
{


 do
  {
   file_read(object_file_element,  sizeof(obj_record_header_type) - 1);
   while ( (Current_record_header.rec_typ Is LINNUM_record) OrIf
         ((Current_record_header.rec_typ Is COMENT_record) AndIf
          (Current_record_header.rec_len > MAX_OBJECT_FILE_READ_SIZE))
    ) {
     file_position(Bit_32(infile.byte_position) +
                   infile.start_of_buffer_position +
                   Bit_32(Current_record_header.rec_len));
     file_read(object_file_element,  sizeof(obj_record_header_type) - 1);
    };
   current_record_offset = Bit_32(infile.byte_position) +
                           infile.start_of_buffer_position -
                           Bit_32(sizeof(obj_record_header_type)-1);
   if ( Current_record_header.rec_len > MAX_OBJECT_FILE_READ_SIZE
    ) {
     linker_error(12, "Probable invalid OBJ format "
                      "or possible translator error:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Record too long.\n"
                      "\t         Max record length supported by this "
                                 "linker is %u bytes.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      MAX_OBJECT_FILE_READ_SIZE);
    };
   file_read(Current_record_header.variant_part, 
             Current_record_header.rec_len);
   if ( (objchecksum.val != 0) AndIf
      (Bit_8(checksum(Current_record_header.rec_len +
                      sizeof(obj_record_header_type)-1,
                     (byte *) current_record_header)) IsNotZero)
    ) {
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Checksum error.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset);
    };
   obj_ptr.b8 = Current_record_header.variant_part;
   end_of_record.b8 =
    (byte *)
     Addr(Current_record_header.variant_part[Current_record_header.rec_len-1]);
   } while (( obj_COMENT()
  ));
 return;
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_PUBDEF                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_PUBDEF()
{
bit_16                                 group_index;
bit_16                                 frame;
bit_16                                 len;
public_entry_ptr                       pub;
#define Pub                            (*pub)
bit_16                                 segment_index;


 if ( Current_record_header.rec_typ IsNot PUBDEF_record
  ) {
   return(False);
  };
 group_index = obj_index_group();
 segment_index = obj_index_segment();
 if ( (segment_index IsZero) AndIf (group_index IsZero)
  ) {
   frame = *obj_ptr.b16++;
  };
 while ( obj_ptr.b8 IsNot end_of_record.b8
  ) {
   len = obj_name_length();
   if ( case_ignore.val
    ) {
     far_to_lower(BytePtr(obj_ptr.b8), len);
    };
   pub = lookup_public(len, obj_ptr.b8, 0);
   obj_ptr.b8 += len;
   if ( Pub.type_entry Is internal
    ) {
     linker_error(4, "Duplicate definition of public \"%Fs\".\n"
                     "\tDefinition in module \"%Fs\" of file \"%Fs\" "
                     "ignored.\n",
                     Pub.symbol,
                     (*tmodule_name).symbol,(*infile.file_info).filename);
     obj_ptr.b16++;      /* Eat offset. */
     obj_name_length();  /* Eat type index. */
    } else {
     if ( Pub.type_entry Is unused
      ) {
       Insert pub AtEnd InList external_list EndInsert;
      };
     if ( (Pub.type_entry Is public_in_library) AndIf
        (Pub.Library.requested)
      ) {
       library_request_count--;
       (*Pub.Library.lib_file).request_count--;
      };
     Pub.type_entry       = internal;
     Pub.Internal.group   = gnames[group_index];
     Pub.Internal.lseg    = snames[segment_index];
     Pub.Internal.frame   = frame;
     Pub.Internal.offset  = *obj_ptr.b16++;
     obj_name_length();  /* Eat type index. */
    };
  };
 obj_next_record();
 return(True);
}
#undef Pub

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_SEGDEF                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_SEGDEF()
{
acbp_type                              acbp;
bit_32                                 address;
bit_8                                  align;
lname_entry_ptr                        class_lname;
bit_8                                  combine;
bit_32                                 length;
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
bit_16                                 segment_index;
lname_entry_ptr                        segment_lname;


 if ( Current_record_header.rec_typ IsNot SEGDEF_record
  ) {
   return(False);
  };
 acbp    = *obj_ptr.acbp++;
 align   = Bit_8(acbp.a);
 if ( align Is absolute_segment
  ) {
   address  = (Bit_32(*obj_ptr.b16++) ShiftedLeft 4L);  /* Frame */
   address += Bit_32(*obj_ptr.b8++);                    /* Offset */
  } else {
   address = 0L;
  };
 if ( align > dword_aligned
  ) {
   linker_error(12, "Translator error:\n"
                    "\tModule:  \"%Fs\"\n"
                    "\t  File:  \"%Fs\"\n"
                    "\tOffset:  %lu\n"
                    "\t Error:  Align type of %u is undefined.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset,
                    align);
  };
 combine = Bit_8(acbp.c);
 if ( (combine Is 4) OrIf (combine Is 7)
  ) { /* Treat combine types 4 and 7 the same as 2. */
   combine = public_combine;
  };
 if ( (combine Is 1) OrIf (combine Is 3)
  ) { /* This is a translator error. */
   linker_error(12, "Translator error:\n"
                    "\tModule:  \"%Fs\"\n"
                    "\t  File:  \"%Fs\"\n"
                    "\tOffset:  %lu\n"
                    "\t Error:  Combine type of %u is undefined.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset,
                    combine);
  };
 length = Bit_32(*obj_ptr.b16++);
 if ( acbp.b IsNotZero
  ) {
   if ( length IsNotZero
    ) {
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  SEGDEF has acbp.b of 1 and length not "
                                  "zero.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset);
    };
   length = 65536L;
  };
 segment_index         = obj_index_LNAME();
 segment_lname         = lnames[segment_index];
 class_lname           = lnames[obj_index_LNAME()];
 lseg = obj_generate_segment(segment_lname,
                             class_lname,
                             combine,
                             align,
                             tmodule_name,
                             infile.file_info,
                             address,
                             length);
 if ( n_segments >= max_segments.val
  ) {
   linker_error(12, "Internal limit exceeded:\n"
                    "\tModule:  \"%Fs\"\n"
                    "\t  File:  \"%Fs\"\n"
                    "\tOffset:  %lu\n"
                    "\t Error:  Too many SEGDEFs.  Max of %u exceeded.\n"
                    "\t         Retry with larger \"/maxsegments:n\" "
                                "switch.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset,
                    max_segments.val);
  };
 snames[++n_segments]  = lseg;

 obj_next_record();
 return(True);
}
#undef Lseg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              obj_seg_grp                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/* obj_seg_grp:: {obj_LNAMES | obj_SEGDEF | obj_EXTDEF}
                 {obj_TYPDEF | obj_EXTDEF | obj_GRPDEF} */
bit_16 obj_seg_grp()
{


 while ( obj_LNAMES() OrIf obj_SEGDEF() OrIf obj_EXTDEF()
  ) {
  };
 while ( obj_TYPDEF() OrIf obj_EXTDEF() OrIf obj_GRPDEF()
  ) {
  };
 return(True);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_THEADR                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_THEADR()
{


 if ( Current_record_header.rec_typ IsNot THEADR_record
  ) {
   return(False);
  };
 tmodule_name = obj_name();
 obj_next_record();
 return(True);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             obj_thread_def                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/* obj_thread_def:: obj_FIXUPP  (containing only thread fields) */

bit_16 obj_thread_def()
{


 if ( obj_FIXUPP()
  ) {
   if ( FIXUPP_contains_only_threads
    ) {
     return(True);
    } else {
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  \"THREAD DEF\" FIXUPP encountered which "
                                  "did not contain\n"
                      "\t          only thread defs.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset);
    };
  };
 return(False);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              obj_tmodule                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/* obj_t_module:: obj_THEADR obj_seg_grp {obj_component} obj_modtail */

bit_16 obj_tmodule()
{


 far_set(BytePtr(externals), 0,
         sizeof(public_entry_ptr)*(max_externals.val+1));
 far_set(BytePtr(gnames),    0,
         sizeof(group_entry_ptr)*(max_groups.val+1));
 far_set(BytePtr(lnames),    0,
         sizeof(lname_entry_ptr)*(max_lnames.val+1));
 far_set(BytePtr(snames),    0,
         sizeof(lseg_ptr)*(max_segments.val+1));
 far_set(BytePtr(target_thread), 0, sizeof(thread_type)*4);
 far_set(BytePtr(frame_thread),  0, sizeof(thread_type)*4);

 n_externals =
 n_groups    =
 n_lnames    =
 n_segments  = 0;
 tmodule_number++;
 tmodule_name = lookup_lname(31, (byte *) "(THEADR record not encountered)");
 obj_next_record();
 if ( Not obj_THEADR()
  ) {
   linker_error(12, "Translator error:\n"
                    "\tModule:  \"%Fs\"\n"
                    "\t  File:  \"%Fs\"\n"
                    "\tOffset:  %lu\n"
                    "\t Error:  T-MODULE record missing.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset);
  };
 if ( Not obj_seg_grp()
  ) {
   linker_error(12, "Translator error:\n"
                    "\tModule:  \"%Fs\"\n"
                    "\t  File:  \"%Fs\"\n"
                    "\tOffset:  %lu\n"
                    "\t Error:  Segment/Group definition record(s) missing.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset);
  };
 while ( obj_component()
  ) {
  };
 if ( Not obj_modtail()
  ) {
   linker_error(12, "Translator error:\n"
                    "\tModule:  \"%Fs\"\n"
                    "\t  File:  \"%Fs\"\n"
                    "\tOffset:  %lu\n"
                    "\t Error:  MODEND record missing.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset);
  };
 return(True);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_TYPDEF                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_TYPDEF()
{


 if ( Current_record_header.rec_typ IsNot TYPDEF_record
  ) {
   return(False);
  };
 obj_next_record();
 return(True);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              write_temp_file                            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void write_temp_file(bit_8           rec_typ,
                     lseg_ptr        lseg,
                     bit_16          offset,
                     byte_ptr        data,
                     bit_16          len)
{


 temp_file_header.rec_typ       = rec_typ;
 temp_file_header.rec_len       = len;
 temp_file_header.lseg          = lseg;
 temp_file_header.offset        = offset;
 file_write(BytePtr(Addr(temp_file_header)), 
            Bit_32(sizeof(temp_file_header)));
 if ( len > 0
  ) {
   file_write(data, Bit_32(len));
  };
 return;
}
