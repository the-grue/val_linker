/*                                 FIXUP.C                                 */

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            fixup_FIXUPP_record                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void fixup_FIXUPP_record()
{
byte_ptr                               byte_location;
int_32                                 IP_distance_to_target;
bit_16                                 fbval;
bit_32                                 foval;
bit_32                                 frame_address;
bit_32                                 location_address;
bit_32                                 target_address;
bit_16 far                            *word_location;


 file_read(BytePtr(Addr(fixup)), sizeof(fixup));
 frame_address  = frame();
 target_address = target();
 if ( ((target_address < frame_address) ||
     (target_address > (frame_address + 65535L))) &&
    (fixup.frame_method != 6) && (frame_absolute == 0)
  ) {
   linker_error(4, "Fixup error:\n"
                   "\t Module:  \"%Fs\"\n"
                   "\t   File:  \"%Fs\"\n"
                   "\tSegment:  \"%Fs\"\n"
                   "\t Offset:  %u\n"
                   "\t  Error:  Target not within frame.\n",
                   (*(*temp_file_header.lseg).tmodule).symbol,
                   (*(*temp_file_header.lseg).file).filename,
                (*(*(*temp_file_header.lseg).segment).segment_name).symbol,
                   temp_file_header.offset);
  };
 byte_location  = 
              Addr((*temp_file_header.lseg).data[temp_file_header.offset]);
 word_location  = (bit_16 far *) byte_location;
 if ( fixup.mode == 0
  ) { /* Self-relative fixup */
   location_address = (*temp_file_header.lseg).address +
                      Bit_32(temp_file_header.offset) +
                      1L;
   if ( fixup.location_type == offset_location
    ) {
     location_address++;
    };
   if ( (location_address < frame_address) ||
      (location_address > (frame_address + 65535L)) ||
      (frame_absolute != 0)
    ) {
     linker_error(4, "Fixup error:\n"
                     "\t Module:  \"%Fs\"\n"
                     "\t   File:  \"%Fs\"\n"
                     "\tSegment:  \"%Fs\"\n"
                     "\t Offset:  %u\n"
                     "\t  Error:  Location not within frame.\n",
                     (*(*temp_file_header.lseg).tmodule).symbol,
                     (*(*temp_file_header.lseg).file).filename,
                 (*(*(*temp_file_header.lseg).segment).segment_name).symbol,
                     temp_file_header.offset);
    };
   switch ( fixup.location_type
    ) {
     case lobyte_location:
      location_address = (*temp_file_header.lseg).address +
                         Bit_32(temp_file_header.offset) +
                         1L;
      IP_distance_to_target = Int_32(target_address) -
                              Int_32(location_address);
      if ( (IP_distance_to_target < -128L) ||
         (IP_distance_to_target > 127L)
       ) {
        linker_error(4, "Byte self-relative fixup error:\n"
                        "\t Module:  \"%Fs\"\n"
                        "\t   File:  \"%Fs\"\n"
                        "\tSegment:  \"%Fs\"\n"
                        "\t Offset:  %u\n"
                        "\t  Error:  Distance to target out of range.\n",
                        (*(*temp_file_header.lseg).tmodule).symbol,
                        (*(*temp_file_header.lseg).file).filename,
                (*(*(*temp_file_header.lseg).segment).segment_name).symbol,
                        temp_file_header.offset);
       };
      *byte_location += Bit_8(IP_distance_to_target);
      break;
     case offset_location:
      IP_distance_to_target = target_address - location_address;
      *word_location += Bit_16(IP_distance_to_target);
      break;
     case base_location:       /* Undefined action */
     case pointer_location:    /* Undefined action */
     case hibyte_location:     /* Undefined action */
      break;
    };
  } else { /* Segment-relative fixup */
   fbval          = Bit_16(frame_address ShiftedRight 4);
   foval          = target_address - frame_address;
   if ( (frame_absolute == 0)                     &&
      (exefile == 0)                            &&
      ((fixup.location_type == base_location)      ||
       (fixup.location_type == pointer_location))
    ) {  /* Count the relocation items we should not be getting. */
     n_relocation_items++;
    };
   switch ( fixup.location_type
    ) {
     case lobyte_location:
      *byte_location += Bit_8(foval);
      break;
     case offset_location:
      *word_location += Bit_16(foval);
      break;
     case base_location:
      *word_location += fbval;
      if ( exefile != 0
       ) {
        Exe_header.relocation_table[Exe_header.n_relocation_items++] =
         segment_offset(temp_file_header.lseg, temp_file_header.offset);
       };
      break;
     case pointer_location:
      *word_location++ += Bit_16(foval);
      *word_location   += fbval;
      if ( exefile != 0
       ) {
        Exe_header.relocation_table[Exe_header.n_relocation_items++] =
         segment_offset(temp_file_header.lseg, temp_file_header.offset+2);
       };
      break;
     case hibyte_location:
      *byte_location += Bit_8(foval ShiftedRight 8);
      break;
    };
  };
 return;
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           fixup_FORREF_record                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void fixup_FORREF_record()
{
bit_16                                 len;
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
bit_16                                 offset;
bit_8                                  size;


 lseg          = temp_file_header.lseg;
 len           = temp_file_header.rec_len;
 file_read(BytePtr(object_file_element), len);
 obj_ptr.b8       = object_file_element;
 end_of_record.b8 = Addr(obj_ptr.b8[len]);
 size             = *obj_ptr.b8++;
 switch ( size
  ) {
   case 0:
    while ( obj_ptr.b8 != end_of_record.b8
     ) {
      offset = *obj_ptr.b16++;
      Lseg.data[offset] += *obj_ptr.b8++;
     };
    break;
   case 1:
    while ( obj_ptr.b8 != end_of_record.b8
     ) {
      offset = *obj_ptr.b16++;
      *((bit_16 far *) Addr(Lseg.data[offset])) += *obj_ptr.b16++;
     };
    break;
   case 2:
    while ( obj_ptr.b8 != end_of_record.b8
     ) {
      offset = *obj_ptr.b16++;
      *((bit_32 far *) Addr(Lseg.data[offset])) += *obj_ptr.b32++;
     };
    break;
   default:
    linker_error(4, "Translator error:\n"
                    "\t Module:  \"%Fs\"\n"
                    "\t   File:  \"%Fs\"\n"
                    "\tSegment:  \"%Fs\"\n"
                    "\t  Error:  Invalid FORREF record.\n",
                    (*(*temp_file_header.lseg).tmodule).symbol,
                    (*(*temp_file_header.lseg).file).filename,
            (*(*(*temp_file_header.lseg).segment).segment_name).symbol);
    break;
  };
 return;
}
#undef Lseg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           fixup_LEDATA_record                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void fixup_LEDATA_record()
{
lseg_ptr                               lseg;
#define Lseg                           (*lseg)


 lseg          = temp_file_header.lseg;
 lseg_data_ptr = Addr(Lseg.data[temp_file_header.offset]);
 file_read(lseg_data_ptr, temp_file_header.rec_len);
 return;
}
#undef Lseg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            fixup_LIDATA_IDB                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void fixup_LIDATA_IDB()
{
bit_16                                 block_count;
bit_8                                 *content;
bit_16                                 i;
bit_16                                 j;
bit_16                                 len;
bit_16                                 repeat_count;


 repeat_count = *obj_ptr.b16++;
 block_count  = *obj_ptr.b16++;
 if ( block_count != 0
  ) {  /* Handle recursive case:  Content is iterated data block */
   content = obj_ptr.b8;
   for ( i=0; i<repeat_count; i++
    ) {
     obj_ptr.b8 = content;
     for ( j=0; j<block_count; j++
      ) {
       fixup_LIDATA_IDB();
      };
    };
  } else {  /* Handle non-recursive case:  Content is data. */
   len = Bit_16(*obj_ptr.b8++);
   for ( i=0; i<repeat_count; i++
    ) {
     far_move(lseg_data_ptr, obj_ptr.b8, len);
     lseg_data_ptr += len;
    };
   obj_ptr.b8 += len;
  };
 return;
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           fixup_LIDATA_record                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void fixup_LIDATA_record()
{
lseg_ptr                               lseg;
#define Lseg                           (*lseg)


 lseg          = temp_file_header.lseg;
 lseg_data_ptr = Addr(Lseg.data[temp_file_header.offset]);
 file_read(BytePtr(object_file_element), temp_file_header.rec_len);
 obj_ptr.b8       = object_file_element;
 end_of_record.b8 = Addr(obj_ptr.b8[temp_file_header.rec_len]);
 while ( obj_ptr.b8 != end_of_record.b8
  ) {
   fixup_LIDATA_IDB();
  };
 return;
}
#undef Lseg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                 frame                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 frame()
{
bit_32                                 frame_address;
group_entry_ptr                        grp;
#define Grp                            (*grp)
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
public_entry_ptr                       pub;
#define Pub                            (*pub)
segment_entry_ptr                      seg;
#define Seg                            (*seg)


 switch ( fixup.frame_method
  ) {
   case 0:  /* Frame is segment relative */
    lseg           = (lseg_ptr) fixup.frame_referent;
    frame_absolute = Lseg.align == absolute_segment;
    seg            = Lseg.segment;
    frame_address  = Seg.address;
    break;
   case 1:  /* Frame is group relative */
    grp            = (group_entry_ptr) fixup.frame_referent;
    seg            = Grp.first_segment;
    lseg           = Seg.lsegs.first;
    frame_absolute = Lseg.align == absolute_segment;
    frame_address  = Seg.address;
    break;
   case 2:  /* Frame is relative to external */
    pub = (public_entry_ptr) fixup.frame_referent;
    frame_address = public_frame_address(pub);
    break;
   case 3:  /* Frame is absolute */
    frame_absolute = True;
    frame_address  = Bit_32(fixup.frame_referent);
    break;
   case 4:  /* Frame is segment containing location */
    lseg           = temp_file_header.lseg;
    seg            = Lseg.segment;
    frame_absolute = Lseg.align == absolute_segment;
    frame_address  = Seg.address;
    break;
   case 5:  /* Frame is defined by target */
    switch ( fixup.target_method
     ) {
      case 0:  /* Target is segment relative */
       lseg           = (lseg_ptr) fixup.target_referent;
       seg            = Lseg.segment;
       frame_absolute = Lseg.align == absolute_segment;
       frame_address  = Seg.address;
       break;
      case 1:  /* Target is group relative */
       grp = (group_entry_ptr) fixup.target_referent;
       seg            = Grp.first_segment;
       lseg           = Seg.lsegs.first;
       frame_absolute = Lseg.align == absolute_segment;
       frame_address  = Seg.address;
       break;
      case 2:  /* Target is relative to an external */
       pub = (public_entry_ptr) fixup.target_referent;
       frame_address = public_frame_address(pub);
       break;
      case 3:  /* Target is absolute */
       frame_absolute = True;
       frame_address  = Bit_32(fixup.target_referent);
       break;
     };
    break;
   case 6:  /* No frame */
    frame_absolute = False;
    frame_address = 0L;
    break;
  };
 return(frame_address & 0xFFFFFFF0L);
}
#undef Grp
#undef Lseg
#undef Pub
#undef Seg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               pass_two                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void pass_two()
{


 fixup_start_time = Now;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |      First, we will figure out how long the EXE header will be.         |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 if ( exefile != 0
  ) {
   exe_header_size  = Bit_32(sizeof(EXE_header_type)) - 
                      Bit_32(sizeof(bit_32)) + 
                     (Bit_32(sizeof(bit_32)) * Bit_32(n_relocation_items));
   if ( align_exe_header.val != 0
    ) {
     exe_header_size += AlignmentGap(exe_header_size, 0xFL);
    } else {
     exe_header_size += AlignmentGap(exe_header_size, 0x1FFL);
    };
   exe_header       = (EXE_header_ptr)
                       allocate_memory(Addr(static_pool),
                                       exe_header_size);
   far_set(BytePtr(exe_header), 0, Bit_16(exe_header_size));
  };
 file_open_for_read(temp_file);
 file_read(BytePtr(Addr(temp_file_header)), sizeof(temp_file_header));
 while ( temp_file_header.rec_typ != 0
  ) {
   switch ( temp_file_header.rec_typ
    ) {
     case FIXUPP_record:
      fixup_FIXUPP_record();
      break;
     case FORREF_record:
      fixup_FORREF_record();
      break;
     case LEDATA_record:
      fixup_LEDATA_record();
      break;
     case LIDATA_record:
      fixup_LIDATA_record();
      break;
     default:
      linker_error(16, "Internal logic error:  Invalid temp file record.\n");
      break;
    };
   file_read(BytePtr(Addr(temp_file_header)), sizeof(temp_file_header));
  };
 file_close_for_read();
 return;
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          public_frame_address                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 public_frame_address(public_entry_ptr pub)
{
bit_32                                 address;
#define Pub                            (*pub)
segment_entry_ptr                      seg;
#define Seg                            (*seg)


 frame_absolute = False;
 if ( Pub.type_entry != internal
  ) {
   seg = (*temp_file_header.lseg).segment;
   if ( ! fixup.external_error_detected
    ) {
     linker_error(4, "\tModule \"%Fs\" in file \"%Fs\"\n"
                     "\treferences unresolved external \"%Fs\"\n"
                     "\tat offset %04XH in segment \"%Fs\".\n",
                     (*(*temp_file_header.lseg).tmodule).symbol,
                     (*(*temp_file_header.lseg).file).filename,
                     Pub.symbol,
                     temp_file_header.offset,
                     (*Seg.segment_name).symbol);
     fixup.external_error_detected = True;
    };
   address = 0L;
  } else {
   if ( Pub.Internal.group == 0
    ) {
     if ( Pub.Internal.lseg == 0
      ) {
       frame_absolute = True;
       address        = (Bit_32(Pub.Internal.frame) ShiftedLeft 4);
      } else {
       frame_absolute = (*Pub.Internal.lseg).align == absolute_segment;
       address        = (*(*Pub.Internal.lseg).segment).address;
      };
    } else {
     frame_absolute = 
               (*(*(*Pub.Internal.group).first_segment).lsegs.first).align ==
               absolute_segment;
     address        = (*(*Pub.Internal.group).first_segment).address;
    };
  };
 return(address);
}
#undef Pub
#undef Seg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         public_target_address                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 public_target_address(public_entry_ptr pub)
{
bit_32                                 address;
#define Pub                            (*pub)
segment_entry_ptr                      seg;
#define Seg                            (*seg)


 if ( Pub.type_entry != internal
  ) {
   seg = (*temp_file_header.lseg).segment;
   if ( ! fixup.external_error_detected
    ) {
     linker_error(4, "\tModule \"%Fs\" in file \"%Fs\"\n"
                     "\treferences unresolved external \"%Fs\"\n"
                     "\tat offset %04XH in segment \"%Fs\".\n",
                     (*(*temp_file_header.lseg).tmodule).symbol,
                     (*(*temp_file_header.lseg).file).filename,
                     Pub.symbol,
                     temp_file_header.offset,
                     (*Seg.segment_name).symbol);
     fixup.external_error_detected = True;
    };
   address = 0L;
  } else {
   if ( Pub.Internal.lseg == 0
    ) {
     address = (Bit_32(Pub.Internal.frame) ShiftedLeft 4);
    } else {
     address = (*Pub.Internal.lseg).address;
    };
  };
 return(address + Bit_32(Pub.Internal.offset));
}
#undef Pub
#undef Seg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              segment_offset                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 segment_offset(lseg_ptr lseg, bit_16 offset)
{
#define Lseg                           (*lseg)


 return ((Frame(lseg) ShiftedLeft 12L) | (Bit_32(offset) + Target(lseg)));
}
#undef Lseg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                 target                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 target()
{
group_entry_ptr                        grp;
#define Grp                            (*grp)
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
public_entry_ptr                       pub;
#define Pub                            (*pub)
bit_32                                 target_address;


 switch ( fixup.target_method
  ) {
   case 0:  /* Target is segment relative */
    lseg = (lseg_ptr) fixup.target_referent;
    target_address = Lseg.address + Bit_32(fixup.target_offset);
    break;
   case 1:  /* Target is group relative */
    grp = (group_entry_ptr) fixup.target_referent;
    target_address = (*Grp.first_segment).address +
                      Bit_32(fixup.target_offset);
    break;
   case 2:  /* Target is relative to an external */
    pub = (public_entry_ptr) fixup.target_referent;
    target_address = public_target_address(pub) +
                      Bit_32(fixup.target_offset);
    break;
   case 3:  /* Target is absolute */
    target_address = Bit_32(fixup.target_referent) +
                      Bit_32(fixup.target_offset);
    break;
  };
 return(target_address);
}
#undef Grp
#undef Lseg
#undef Pub
