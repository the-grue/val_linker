/*                                 TYPES.H                                 */

typedef
 struct acbp_struct                       acbp_type;
typedef acbp_type                              *acbp_ptr;
typedef
 struct bit_16_switch_struct              bit_16_switch_type;
typedef bit_16_switch_type                     *bit_16_switch_ptr;
typedef
 struct boolean_switch_struct             boolean_switch_type;
typedef boolean_switch_type                    *boolean_switch_ptr;
typedef
 Enumeration combine_enum                    combine_type;
typedef
 struct communal_entry_struct             communal_entry_type;
typedef
 struct DTA_struct                        DTA_type;
typedef DTA_type far                           *DTA_ptr;
typedef
 union entry_information_union               entry_information_type;
typedef
 struct EXE_header_struct                 EXE_header_type;
typedef EXE_header_type far                    *EXE_header_ptr;
typedef
 struct external_entry_struct             external_entry_type;
typedef
 struct file_info_struct                  file_info_type;
typedef file_info_type far                     *file_info_ptr;
typedef
 struct file_struct                       file_type;
typedef file_type                              *file_ptr;
typedef
 struct FIX_DAT_struct                    FIX_DAT_type;
typedef FIX_DAT_type                           *FIX_DAT_ptr;
typedef
 struct fixup_struct                      fixup_type;
typedef
 struct group_entry_struct                group_entry_type;
typedef group_entry_type far                   *group_entry_ptr;
typedef group_entry_ptr far                    *group_entry_ptr_array;
typedef
 struct internal_entry_struct             internal_entry_type;
typedef
 struct library_directory_struct          library_directory_type;
typedef library_directory_type                 *library_directory_ptr;
typedef
 struct library_entry_struct              library_entry_type;
typedef
 struct library_file_header_struct        library_file_header_type;
typedef library_file_header_type               *library_file_header_ptr;
typedef
 struct library_symbol_entry_struct       library_symbol_entry_type;
typedef library_symbol_entry_type              *library_symbol_entry_ptr;
typedef
 struct lname_entry_struct                lname_entry_type;
typedef lname_entry_type far                   *lname_entry_ptr;
typedef lname_entry_ptr far                    *lname_entry_ptr_array;
typedef
 Enumeration loc_enum                        loc_type;
typedef
 struct LOCAT_struct                      LOCAT_type;
typedef LOCAT_type                             *LOCAT_ptr;
typedef
 struct lseg_struct                       lseg_type;
typedef lseg_type far                          *lseg_ptr;
typedef lseg_ptr far                           *lseg_ptr_array;
typedef
 struct memory_descriptor_struct          memory_descriptor_type;
typedef memory_descriptor_type far             *memory_descriptor_ptr;
typedef
 struct MOD_TYP_struct                    MOD_TYP_type;
typedef MOD_TYP_type                           *MOD_TYP_ptr;
typedef
 union obj_ptr_union                         obj_ptr_type;
typedef
 struct obj_record_header_struct          obj_record_header_type;
typedef obj_record_header_type                 *obj_record_header_ptr;
typedef
 Enumeration obj_mod_rec_type_enum           object_module_record_types;
typedef
 Enumeration public_entry_class_enum         public_entry_class_type;
typedef
 struct public_entry_struct               public_entry_type;
typedef public_entry_type far                  *public_entry_ptr;
typedef public_entry_ptr far                   *public_entry_ptr_array;
typedef
 struct pool_descriptor_struct            pool_descriptor_type;
typedef pool_descriptor_type                   *pool_descriptor_ptr;
typedef
 struct segment_entry_struct              segment_entry_type;
typedef segment_entry_type far                 *segment_entry_ptr;
typedef segment_entry_ptr far                  *segment_entry_ptr_array;
typedef
 struct switch_table_struct               switch_table_type;
typedef switch_table_type                      *switch_table_ptr;
typedef
struct temp_file_header_struct            temp_file_header_type;
typedef
 struct text_switch_struct                text_switch_type;
typedef text_switch_type                       *text_switch_ptr;
typedef
 struct thread_struct                     thread_type;
typedef
 struct TRD_DAT_struct                    TRD_DAT_type;
typedef TRD_DAT_type                           *TRD_DAT_ptr;
typedef
 Enumeration token_class_enum                token_class_type;
typedef
 struct token_stack_struct                token_stack_type;
typedef token_stack_type far                   *token_stack_ptr;

struct acbp_struct
 {
  unsigned                             p:1;
  unsigned                             b:1;
  unsigned                             c:3;
  unsigned                             a:3;
 };

Enumeration align_enum
 {
  absolute_segment,
  byte_aligned,
  word_aligned,
  paragraph_aligned,
  page_aligned,
  dword_aligned
 EndEnumeration;

struct bit_16_switch_struct
 {
  bit_16                               val;
  bit_16                               min;
  bit_16                               max;
  bit_16                               def;
  bit_16                               set;
 };

struct boolean_switch_struct
 {
  bit_16                               val;
 };

Enumeration combine_enum
 {
  private_combine                      = 0,
  public_combine                       = 2,
  stack_combine                        = 5,
  common_combine                       = 6,
  blank_common_combine                 = 9
 EndEnumeration;

struct DTA_struct
 {
  byte                                 DOS_reserved[21];
  bit_8                                attribute;
  bit_16                               time_stamp;
  bit_16                               date_stamp;
  bit_32                               file_size;
  byte                                 filename[13];
  byte                                 filler[85];
 };

struct EXE_header_struct
 {
  bit_16                               signature;
  bit_16                               image_length_MOD_512;
  bit_16                               image_length_DIV_512;
  bit_16                               n_relocation_items;
  bit_16                               n_header_paragraphs;
  bit_16                               min_paragraphs_above;
  bit_16                               max_paragraphs_above;
  bit_16                               initial_SS;
  bit_16                               initial_SP;
  bit_16                               checksum;
  bit_16                               initial_IP;
  bit_16                               initial_CS;
  bit_16                               offset_to_relocation_table;
  bit_16                               overlay;
  bit_16                               always_one;
  bit_32                               relocation_table[1];
 };

struct external_entry_struct
 {
  bit_16                               reserved;
 };

struct internal_entry_struct
 {
  group_entry_ptr                      group;
  lseg_ptr                             lseg;
  bit_16                               frame;
  bit_16                               offset;
 };

struct library_entry_struct
 {
  file_info_ptr                        lib_file;
  bit_16                               page;
  bit_8                                requested;
 };

struct communal_entry_struct
 {
  public_entry_ptr                     next_communal;
  bit_32                               element_size;
  bit_32                               element_count;
 };

union entry_information_union
 {
  library_entry_type                   library_type;
  external_entry_type                  external_type;
  internal_entry_type                  internal_type;
  communal_entry_type                  communal_type;
 };

Enumeration public_entry_class_enum
 {
  unused,
  public_in_library,
  external,
  internal,
  far_communal,
  near_communal
 EndEnumeration;

struct public_entry_struct
 {
  public_entry_ptr                     next;
  public_entry_ptr                     next_congruent;
  bit_8                                type_entry;
  entry_information_type               entry;
  bit_16                               module;
  bit_16                               length;
  byte                                 symbol[1];
 };
#define Library                        entry.library_type
#define External                       entry.external_type
#define Internal                       entry.internal_type
#define Communal                       entry.communal_type

ListTypeOf(public_entry);

struct file_info_struct
 {
  file_info_ptr                        next;
  bit_8                                attribute;
  bit_16                               time_stamp;
  bit_16                               date_stamp;
  bit_32                               file_size;
  bit_16                               page_size;
  bit_16                               request_count;
  bit_16                               pass_count;
  bit_16                               module_count;
  public_entry_list                    external_list;
  byte                                 filename[1];
 };

ListTypeOf(file_info);

struct file_struct
 {
  file_info_ptr                        file_info;
  bit_16                               file_handle;
  bit_32                               start_of_buffer_position;
  bit_32                               next_buffer_position;
  byte_ptr                             buffer;
  byte_ptr                             current_byte;
  bit_16                               buffer_size;
  bit_16                               IO_limit;
  bit_16                               bytes_in_buffer;
  bit_16                               bytes_left_in_buffer;
  bit_16                               byte_position;
 };

struct FIX_DAT_struct
 {
  unsigned                             targt:2;
  unsigned                             p:1;
  unsigned                             t:1;
  unsigned                             frame:3;
  unsigned                             f:1;
 };

struct fixup_struct
 {
  void far                            *frame_referent;
  void far                            *target_referent;
  bit_16                               target_offset;
  unsigned                             location_type:3;
  unsigned                             mode:1;
  unsigned                             frame_method:3;
  unsigned                             target_method:2;
  unsigned                             external_error_detected:1;
  unsigned                             reserved:6;
 };

struct group_entry_struct
 {
  group_entry_ptr                      next;
  group_entry_ptr                      next_congruent;
  lname_entry_ptr                      group_name;
  segment_entry_ptr                    first_segment;
 };

ListTypeOf(group_entry);

struct library_directory_struct
 {
  bit_8                                offset_to_symbol[38];
  byte                                 symbol_area[474];
 };

struct library_file_header_struct
 {
  bit_8                                flag;
  bit_16                               page_size;
  bit_32                               directory_position;
  bit_16                               n_directory_blocks;
 };

 struct library_symbol_entry_struct
  {
   bit_8                               length_of_symbol;
   byte                                symbol[1];
  };

struct lname_entry_struct
 {
  lname_entry_ptr                      next_congruent;
  bit_16                               lname_checksum;
  bit_16                               length;
  byte                                 symbol[1];
 };

struct LOCAT_struct
 {
  unsigned                             data_record_offset:10;
  unsigned                             loc:3;
  unsigned                             s:1;
  unsigned                             m:1;
  unsigned                             type_fixupp_record:1;
 };

Enumeration loc_enum
 {
  lobyte_location,
  offset_location,
  base_location,
  pointer_location,
  hibyte_location
 EndEnumeration;

struct lseg_struct
 {
  lseg_ptr                             next;
  segment_entry_ptr                    segment;
  lname_entry_ptr                      tmodule;
  file_info_ptr                        file;
  bit_32                               address;
  bit_32                               length;
  bit_32                               highest_uninitialized_byte;
  bit_8                                align;
  byte_ptr                             data;
 };

ListTypeOf(lseg);

struct memory_descriptor_struct
 {
  memory_descriptor_ptr                next;
  byte_ptr                             chunk;
  bit_32                               available;
  bit_32                               size;
  byte_ptr                             unused_base;
 };

struct MOD_TYP_struct
 {
  unsigned                             l:1;
  unsigned                             zeros:5;
  unsigned                             mattr:2;
 };

union obj_ptr_union
 {
  bit_8                               *b8;
  bit_16                              *b16;
  bit_32                              *b32;
  acbp_ptr                             acbp;
  FIX_DAT_ptr                          FIX_DAT;
  LOCAT_ptr                            LOCAT;
  MOD_TYP_ptr                          MOD_TYP;
  TRD_DAT_ptr                          TRD_DAT;
 };

Enumeration obj_mod_rec_type_enum
 {
  THEADR_record                        = 0x80,
  COMENT_record                        = 0x88,
  MODEND_record                        = 0x8a,
  EXTDEF_record                        = 0x8c,
  TYPDEF_record                        = 0x8e,
  PUBDEF_record                        = 0x90,
  LINNUM_record                        = 0x94,
  LNAMES_record                        = 0x96,
  SEGDEF_record                        = 0x98,
  GRPDEF_record                        = 0x9a,
  FIXUPP_record                        = 0x9c,
  LEDATA_record                        = 0xa0,
  LIDATA_record                        = 0xa2,
  COMDEF_record                        = 0xb0,
  FORREF_record                        = 0xb2,
  MODEXT_record                        = 0xb4,
  MODPUB_record                        = 0xb6
 EndEnumeration;

struct obj_record_header_struct
 {
  bit_8                                rec_typ;
  bit_16                               rec_len;
  byte                                 variant_part[1];
 };

struct pool_descriptor_struct
 {
  memory_descriptor_ptr                memory_descriptor_list;
  char_ptr                             pool_id;
  bit_32                               pool_size;
  bit_32                               used_bytes;
 };

struct segment_entry_struct
 {
  segment_entry_ptr                    next;
  segment_entry_ptr                    next_congruent;
  lname_entry_ptr                      segment_name;
  lname_entry_ptr                      class_name;
  bit_8                                combine;
  bit_32                               address;
  bit_32                               length;
  bit_32                               highest_uninitialized_byte;
  lseg_list                            lsegs;
  group_entry_ptr                      owning_group;
 };

ListTypeOf(segment_entry);

struct switch_table_struct
 {
  bit_8                                min_length;
  char                                *abbr_name;
  char                                *full_name;
  void                                *affected_thing;
  void                  (*switch_processor)(switch_table_ptr current_switch);
 };

struct temp_file_header_struct
 {
  bit_8                                rec_typ;
  bit_16                               rec_len;
  lseg_ptr                             lseg;
  bit_16                               offset;
 };

struct text_switch_struct
 {
  string_ptr                           val;
 };

struct thread_struct
 {
  bit_8                                method;
  void far                            *referent;
  bit_8                                thread_defined;
 };

Enumeration token_class_enum
 {
  continuation_token_type,
  end_of_command_line_token_type,
  filename_token_type,
  indirect_file_token_type,
  line_end_token_type,
  number_token_type,
  separator_token_type,
  switch_token_type,
  switch_end_token_type,
  text_token_type,
  terminator_token_type
 EndEnumeration;

struct token_stack_struct
 {
  token_stack_ptr                      next;
  FILE                                *source_file;
  byte                                 break_char;
  string_ptr                           token_string;
  bit_16                               token_string_index;
 };

ListTypeOf(token_stack);

struct TRD_DAT_struct
 {
  unsigned                             thred:2;
  unsigned                             method:3;
  unsigned                             z:1;
  unsigned                             d:1;
  unsigned                             type_fixupp_record:1;
 };
