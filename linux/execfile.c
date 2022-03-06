/*                                EXECFILE.C                               */

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             make_EXE_header                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void make_EXE_header()
BeginDeclarations
bit_16                                 checksum;
bit_32                                 image_size;
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
bit_32                                 n_uninitialized_bytes;
segment_entry_ptr                      seg;
#define Seg                            (*seg)
EndDeclarations
BeginCode
 image_size                      = exe_header_size+highest_uninitialized_byte;
 Exe_header.signature            = 0x5A4D;
 Exe_header.image_length_MOD_512 = Bit_16(image_size Mod 512L);
 Exe_header.image_length_DIV_512 = Bit_16(image_size / 512L) + 1;
 Exe_header.n_header_paragraphs  = Bit_16(exe_header_size ShiftedRight 4L);
 n_uninitialized_bytes           = (*segment_list.last).address + 
                                   (*segment_list.last).length - 
                                   highest_uninitialized_byte;
 Exe_header.min_paragraphs_above = Bit_16((n_uninitialized_bytes + 
                  AlignmentGap(n_uninitialized_bytes, 0xFL)) ShiftedRight 4L);
 Exe_header.max_paragraphs_above = CPARMAXALLOC.val;
 if ( stack_segment_found IsTrue
  ) {
   Exe_header.initial_SS = CanonicFrame(Largest_stack_seg.address);
   Exe_header.initial_SP = Bit_16(largest_stack_seg_length +
                           Largest_stack_seg.address -
                            (Bit_32(Exe_header.initial_SS) ShiftedLeft 4L));
  } else {
   Exe_header.initial_SS = 0;
   Exe_header.initial_SP = 0;
  };
 Exe_header.initial_CS                 = initial_CS;
 Exe_header.initial_IP                 = initial_IP;
 Exe_header.offset_to_relocation_table = 0x1E;
 Exe_header.always_one                 = 1;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |    Run a checksum on all the bytes in the soon-to-exist EXE file.       |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 if ( exechecksum.val IsTrue
  ) {
   checksum = word_checksum(Bit_16(exe_header_size), 0, BytePtr(exe_header));
   TraverseList(segment_list, seg)
    BeginTraverse
     if (Seg.address NotLessThan highest_uninitialized_byte) break;
     TraverseList(Seg.lsegs, lseg)
      BeginTraverse
       if (Lseg.address NotLessThan highest_uninitialized_byte) break;
       checksum += word_checksum(Bit_16(Lseg.length), 
                                 Bit_16(Lseg.address),
                                 Lseg.data);
      EndTraverse;
    EndTraverse;
  } else {
   checksum = 0xFFFF;
  };
 Exe_header.checksum = Complement checksum;
 file_write(BytePtr(exe_header), exe_header_size);
 return;
EndCode
#undef Lseg
#undef Seg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         write_executable_image                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void write_executable_image()
BeginDeclarations
bit_32                                 data_index;
bit_32                                 gap;
bit_32                                 partial_length;
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
segment_entry_ptr                      seg;
#define Seg                            (*seg)
EndDeclarations
BeginCode
 exec_image_start_time = Now;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         Validate start address.                         |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 if ( start_address_found IsTrue
  ) {
   fixup                 = start_address;
   initial_CS            = CanonicFrame(frame());
   initial_IP            = Bit_16(target() - frame());
   if ( (comfile.val IsTrue)      AndIf 
      (initial_CS IsNotZero)    AndIf 
      (initial_IP IsNot 0x0100)
    ) {  /* COM file start address must be 0000:0100 */
      linker_error(4, "Start address for COM file is not 0000:0100.\n");
    } else {
     if ( (sysfile.val IsTrue)   AndIf
        (initial_CS IsNotZero) AndIf 
        (initial_IP IsNotZero)
      ) {  /* SYS file start address must be 0000:0000 */
        linker_error(4, "Start address for SYS file is not 0000:0000.\n");
      };
    };
  } else {  /* No start address found. */
   linker_error(4,"No start address.\n");
   initial_CS = 0;
   initial_IP = 0;
  };
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                        Validate stack segment.                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 if ( (comfile.val IsTrue) AndIf (stack_segment_found IsTrue)
  ) {  /* COM file should not have a stack segment. */
    linker_error(4, "COM file should not have a stack segment.\n");
  } else {
   if ( (sysfile.val IsTrue) AndIf (stack_segment_found IsTrue)
    ) {  /* SYS file should not have a stack segment. */
      linker_error(4, "SYS file should not have a stack segment.\n");
    } else {
     if ( (exefile IsTrue) AndIf (stack_segment_found IsFalse)
      ) {  /* EXE file should have a stack segment. */
       linker_error(4, "EXE file should have a stack segment.\n");
      };
    };
  };
 
 if ( pause.val IsTrue
  ) {
   printf("About to write \"%Fs\".\n", (*exe_file_list.first).filename);
   printf("Press [RETURN] key to continue.\n");
   gets(CharPtr(object_file_element));
  };
 file_open_for_write(exe_file_list.first);
 if ( exefile IsTrue
  ) {
   make_EXE_header();
  };
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |     Well, we have everything we need to write the executable image.     |
  |                            So let's do it!                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 /* We will use object_file_element as a source for the gaps caused by
    the alignment of segments.  We will fill the gaps with zeros. */
 far_set(BytePtr(object_file_element), 0, MAX_ELEMENT_SIZE);
 next_available_address = address_base;
 TraverseList(segment_list, seg)
  BeginTraverse
   if ((*Seg.lsegs.first).align Is absolute_segment) continue;
   if (Seg.address NotLessThan highest_uninitialized_byte) break;
   TraverseList(Seg.lsegs, lseg)
    BeginTraverse
     if (Lseg.address NotLessThan highest_uninitialized_byte) break;
     if ( Lseg.address LessThan next_available_address
      ) {
       if ((Lseg.address+Lseg.length) NotGreaterThan 
              next_available_address) continue;
       data_index     = next_available_address - Lseg.address;
       partial_length = Lseg.length - data_index;
       if ( Seg.combine IsNot blank_common_combine
        ) {
         file_write(Addr(Lseg.data[Bit_16(data_index)]), partial_length);
        } else {
         write_gap(partial_length);
        };
		next_available_address += partial_length;
      } else {
       gap = Lseg.address - next_available_address;
       if ( gap IsNotZero
        ) {
         write_gap(gap);
         next_available_address += gap;
        };
       if ( (Lseg.address + Lseg.length) Exceeds highest_uninitialized_byte
        ) {
         partial_length = (Lseg.address + Lseg.length) - 
                          highest_uninitialized_byte;
         if ( Seg.combine IsNot blank_common_combine
          ) {
           file_write(Lseg.data, partial_length);
          } else {
           write_gap(partial_length);
          };
          next_available_address += partial_length;
        } else {
         if ( Seg.combine IsNot blank_common_combine
          ) {
           file_write(Lseg.data, Lseg.length);
          } else {
           write_gap(Lseg.length);
          };
         next_available_address += Lseg.length;
        };
      };
    EndTraverse;
  EndTraverse;
 file_close_for_write();
 return;
EndCode
#undef Lseg
#undef Seg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                write_gap                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void write_gap(bit_32 length)
BeginDeclarations
EndDeclarations
BeginCode
 while ( length Exceeds 0
  ) {
   if ( length Exceeds Bit_32(MAX_ELEMENT_SIZE)
    ) {
     file_write(BytePtr(object_file_element), Bit_32(MAX_ELEMENT_SIZE));
     length -= Bit_32(MAX_ELEMENT_SIZE);
    } else {
     file_write(BytePtr(object_file_element), length);
     length = 0L;
    };
  };
 return;
EndCode
