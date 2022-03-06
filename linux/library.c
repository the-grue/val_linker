/*                                 LIBRARY.C                               */

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            process_libraries                            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void process_libraries()
{
file_info_ptr                          current_lib_file_info;


 library_processing_start_time = Now;
 while ( library_request_count > 0
  ) {
   TraverseList(lib_file_list, current_lib_file_info)
    BeginTraverse
     process_library(current_lib_file_info);
    EndTraverse;
  };
 write_temp_file(0, Null, 0, 0, 0);
 file_close_for_write();
 return;
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            process_library                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void process_library(file_info_ptr lib_file)
{
#define Lib_file                       (*lib_file)
bit_32                                 pos;
public_entry_ptr                       pub;
#define Pub                            (*pub)


 while ( Lib_file.request_count > 0
  ) {
   file_open_for_read(lib_file);
   Lib_file.pass_count++;
   TraverseList(Lib_file.external_list, pub)
    BeginTraverse
     if ((Pub.type_entry != public_in_library) OrIf
            (Not Pub.Library.requested)) continue;
     pos = Bit_32(Pub.Library.page) * Bit_32(Lib_file.page_size);
     file_position(pos);
     Lib_file.module_count++;
     obj_tmodule();
     if ( Pub.type_entry != internal
      ) {
       linker_error(12, "Library directory error:\n"
                        "\tLibrary:  \"%Fs\"\n"
                        "\t Module:  \"%Fs\"\n"
                        "\t Offset:  %lu\n"
                        "\t  Error:  Above module did not resolve \"%Fs\"\n"
                        "\t          when the library directory indicated it "
                                     "would.\n",
                        Lib_file.filename,
                        (*tmodule_name).symbol,
                        pos,
                        Pub.symbol);
      };
    EndTraverse;
   file_close_for_read();
  };
 return;
}
#undef Lib_file
#undef Pub

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       process_library_directories                       |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void process_library_directories()
{
file_info_ptr                          current_lib_file_info;


 library_directory_start_time = Now;
 public_sort_array = (public_entry_ptr_array)
                       allocate_memory
                        (Addr(dynamic_pool),
                         Bit_32(sizeof(public_entry_ptr)) *
                          Bit_32(MAX_PUBLICS_IN_LIBRARY));
 TraverseList(lib_file_list, current_lib_file_info)
  BeginTraverse
   process_library_directory(current_lib_file_info);
  EndTraverse;
 release_pool(Addr(dynamic_pool));
 return;
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                        process_library_directory                        |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void process_library_directory(file_info_ptr lib_file)
{
#define Lib_file                       (*lib_file)
bit_16                                 entry_number;
bit_16                                 current_block;
library_directory_ptr                  directory_page;
#define Directory_page                 (*directory_page)
library_file_header_ptr                lib_hdr;
#define Lib_hdr                        (*lib_hdr)
bit_16                                 n_directory_blocks;
public_entry_ptr                       pub;
#define Pub                            (*pub)
public_entry_ptr_array                 public_array;
library_symbol_entry_ptr               symbol;
bit_16                                 symbol_count;
bit_16                                 symbol_index;
#define Symbol                         (*symbol)
bit_16                                *symbol_in_page;
#define Symbol_in_page                 (*symbol_in_page)


 lib_hdr = (library_file_header_ptr) object_file_element;
 file_open_for_read(lib_file);
 file_IO_limit(sizeof(library_file_header_type));
 file_read(object_file_element, sizeof(library_file_header_type));
 if ( Lib_hdr.flag != 0xF0
  ) {
   linker_error(4,"File \"%Fs\" is not in LIB file format.\n"
                  "\tThis file has been ignored.\n",
                  (*lib_file).filename);
   Delete lib_file FromList lib_file_list EndDelete;
   file_close_for_read();
   return;
  };
 Lib_file.page_size            = Lib_hdr.page_size + 3;
 Lib_file.request_count        = 0;
 First(Lib_file.external_list) =
 Last(Lib_file.external_list)  = Null;
 n_directory_blocks            = Lib_hdr.n_directory_blocks;
 file_position(Lib_hdr.directory_position);
 file_IO_limit(0);
 symbol_count = 0;
 public_array = public_sort_array;
 for ( current_block=0;
     current_block <  n_directory_blocks;
     current_block++
  ) {
   file_read(object_file_element, 512);
   directory_page = (library_directory_ptr) object_file_element;
   for ( entry_number=0; entry_number < 37; entry_number++
    ) {
     symbol_index =
      Directory_page.offset_to_symbol[entry_number] ShiftedLeft 1;
     if (symbol_index == 0) continue;
     symbol = (library_symbol_entry_ptr)
              Addr(Directory_page.offset_to_symbol[symbol_index]);
     symbol_in_page = (bit_16 *)
                      Addr(Symbol.symbol[Symbol.length_of_symbol]);
     if ( case_ignore.val
      ) {
       far_to_lower(Symbol.symbol, Symbol.length_of_symbol);
      };
     pub = lookup_public(Symbol.length_of_symbol, Symbol.symbol, 0);
/*+-------------------------------------------------------------------------+
  |                                                                         |
  | The Microsoft linker does not seem to concern itself with the ambiguity |
  | which occurs when more than one library contains a module resolving an  |
  | external reference.  It just seems to take the module from the first    |
  | library which can resolve the external.  This linker behaves in the     |
  | same manner except it will log when it ignores a module in a library.   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
     if ( Pub.type_entry != unused
      ) {  /* Log the ambiguity. */
       linker_error(4,"The symbol \"%Fs\" is defined in the libraries\n"
                      "\t\"%Fs\" and \"%Fs\".\n"
                      "\tThe latter has been ignored.\n",
                      Pub.symbol,
                      (*Pub.Library.lib_file).filename, Lib_file.filename);
      } else {
       Pub.type_entry        = public_in_library;
       Pub.Library.page      = Symbol_in_page;
       Pub.Library.lib_file  = lib_file;
       Pub.Library.requested = False;
       if ( symbol_count >= MAX_PUBLICS_IN_LIBRARY
        ) {
         linker_error(8,"Limitation of not more than %u public symbols in "
                        "a library\n"
                        "\texceeded for library \"%Fs\".\n",
                        MAX_PUBLICS_IN_LIBRARY,
                        Lib_file.filename);
        };
       *public_array++ = pub;
       symbol_count++;
      };
    };
  };
 file_close_for_read();
 if ( symbol_count > 0
  ) {
   sort_directory(0, symbol_count-1);
   public_array = public_sort_array;
   for ( entry_number=0; entry_number < symbol_count; entry_number++
    ) {
     Insert *public_array++ AtEnd InList Lib_file.external_list EndInsert;
    };
  };
 return;
}
#undef Pub
#undef Lib_file
#undef Lib_hdr
#undef Library_page
#undef Symbol
#undef Symbol_in_page

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              sort_directory                             |
  |                                                                         |
  +-------------------------------------------------------------------------+

Well, there is always much ado concerning the best sort method for a
particular application.  What is used here is a simple "quick sort".
See Knuth, "Searching and Sorting", page 114 for a description of the
algorithm.  What is really important here is why we are sorting.  After
the OBJ modules are processed, we must scan the libraries to see what
modules from the library must be included.  If the symbols in the
symbol table are in ascending module order, then we will include modules
from the library in ascending order.  This may not seem too important
at first glance, but consider that the library will be heavily buffered
and we can reduce I/O overhead by processing modules in the same buffer.
We may not get all the modules in one pass (a module at the end of the
library could require a module at the beginning of the library).  However,
experience has shown that all modules from a library can usually be
included in less than three passes.  The linker statistics will show
the number of passes for and modules included from each library.  If
you want to test this theory, just comment out the sort in the
procedure "process_library_directory" and compare the number of passes
before and after.

Now, all this could be avoided if the librarian had placed the modules
in the library in some sort of an approximation of a topologically
sorted order.  (A true topological sort is not usually possible because
relational loops are not all that infrequent.)  However, if modules with
relational loops were placed close to each other, it is very likely
that the I/O buffering could cause the library to be fully processed in
one pass.  This would improve performance, but it would not insure that
a library could be processed in exactly one pass.

Consider the case where a relational loop occurs with modules which are
not in the same library (this case forces multiple passes).  E.G.,
suppose module A1 in library A requires module B1 in library B.  Further,
suppose module B1 in turn requires module A2 back in library A.  When
processing library A (the first time), we have no way of knowing we also
need A2 until after we process B1 in library B.  We then have to go back
to library A and pick up A2.  The more this happens, the longer the link
time.*/
void sort_directory(bit_16 left, bit_16 right)
{
bit_16                                 i;
bit_16                                 j;
public_entry_ptr                       temp;


 if ( left >= right
  ) {
   return;
  };
 i = left;
 j = right;
 while ( i < j
  ) {
   while ( i < j
    ) {
     if ( (*public_sort_array[i]).Library.page >
        (*public_sort_array[j]).Library.page
      ) {
       temp                 = public_sort_array[i];
       public_sort_array[i] = public_sort_array[j];
       public_sort_array[j] = temp;
       break;
      };
     j--;
    };
   while ( i < j
    ) {
     if ( (*public_sort_array[i]).Library.page >
        (*public_sort_array[j]).Library.page
      ) {
       temp                 = public_sort_array[i];
       public_sort_array[i] = public_sort_array[j];
       public_sort_array[j] = temp;
       break;
      };
     i++;
    };
  };
  if ( i > 0
   ) {
    sort_directory(left, i-1);
   };
 sort_directory(i+1,  right);
 return;
}
