/*                                 STRING.C                                */

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                        allocate_string                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr allocate_string(pool_descriptor_ptr pool, bit_16 len)
{
string_ptr                             temp;
#define Temp                           (*temp)


 temp = StringPtr(allocate_memory(pool, Bit_32(sizeof(string_type))+
                                        Bit_32(len)));
 Temp.max_length = len;
 Temp.length     = 0;
 Temp.text[0]    = '\000';
 return(temp);
}
#undef Temp

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         compare_string                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
int compare_string(string_ptr left, string_ptr right)
{
bit_16                                 left_len;
bit_16                                 right_len;


 left_len  = Length(left);
 right_len = Length(right);
 if ( (left_len == 0) && (right_len == 0)
  ) {
   return(0);
  };
 if ( left_len < right_len
  ) {
   return(-1);
  };
 if ( left_len > right_len
  ) {
   return(1);
  };
 return(far_compare(String(left), String(right), left_len));
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         compare_short_string                            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
int compare_short_string(string_ptr left, string_ptr right)
{
bit_16                                 left_len;


 left_len = Length(left);
 if ( left_len == 0
  ) {
   return(0);
  };
 return(far_compare(String(left), String(right), left_len));
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         concat_string                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr concat_string(string_ptr dest, string_ptr source)
{
bit_16                                 temp;


 if ( (Length(source) + Length(dest)) > MaxLength(dest)
  ) {
   linker_error(8,"Destination string to small (%u bytes) to hold "
                  "concatination of:\n"
                  "\t\"%Fs\" (%u bytes) and\n"
                  "\t\"%Fs\" (%u bytes)\n",
                  MaxLength(dest),
                  String(dest), Length(dest),
                  String(source), Length(dest));
  };
 temp = Length(source);
 far_move(Addr(String(dest)[Length(dest)]), String(source), temp + 1);
 Length(dest) += temp;
 return(dest);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         concat_char_to_string                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr concat_char_to_string(string_ptr dest, byte c)
{


 if ( (Length(dest) + 1) > MaxLength(dest)
  ) {
   linker_error(8,"Destination string to small (%u bytes) to add \"%c\" "
                  "to string:\n"
                  "\t\"%Fs\" (%u bytes)\n",
                  MaxLength(dest), c,
                  String(dest), Length(dest));
  };
 String(dest)[Length(dest)++] = c;
 String(dest)[Length(dest)]   = '\000';
 return(dest);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         copy_string                                     |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr copy_string (string_ptr dest, string_ptr source)
{


 if ( Length(source) > MaxLength(dest)
  ) {
   linker_error(8,"Destination string to small (%u bytes) to hold:\n"
                  "\t\"%Fs\" (%u bytes)\n",
                  MaxLength(dest),
                  String(source), Length(dest));
  };
 Length(dest) = Length(source);
 far_move(String(dest), String(source), Length(source)+1);
 return(dest);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            cut_string                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr cut_string(string_ptr s, bit_16 at, bit_16 len)
{
bit_16                                 length_to_right;
bit_16                                 string_length;


 string_length = Length(s);
 if ( string_length <= at
  ) {
   return(s);
  };
 if ( string_length < (at + len)
  ) {
   len = string_length - at;
  };
 Length(s)       -= len;
 length_to_right  = string_length - (at+len);
 far_move(Addr(String(s)[at]), Addr(String(s)[at+len]), length_to_right+1);
 return(s);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            duplicate_string                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr duplicate_string(pool_descriptor_ptr pool, string_ptr s)
{
string_ptr                             temp;
#define Temp                           (*temp)


 temp = StringPtr(allocate_memory(pool,
                                  Bit_32(sizeof(string_type))+
                                  Bit_32(Length(s))));
 Temp.max_length = Length(s);
 copy_string(temp, s);
 return(temp);
}
#undef Temp

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           edit_number_string                            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr edit_number_string(string_ptr s, char_ptr format, ...)
{
va_list                                argptr;
bit_16                                 i;


 va_start(argptr,format);
 vsprintf((char_ptr) temp_near_string, format, argptr);
 copy_string(s, string(temp_near_string));
 i = Length(s);
 while ( i > 3
  ) {
   i -= 3;
   paste_string(s, i, comma_string);
  };
 return(s);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           index_string                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 index_string(string_ptr s, bit_16 from, string_ptr pattern)
{
bit_16                                 i;
bit_16                                 iteration_count;
byte_ptr                               left;
bit_16                                 len;
byte_ptr                               pat;
bit_16                                 pattern_length;


 pattern_length = Length(pattern);
 pat            = String(pattern);
 len            = Length(s);
 if ( (len - from) < pattern_length
  ) {
   return(0xFFFF);
  };
 iteration_count = len - pattern_length + 1;
 left  = Addr(String(s)[from]);
 for ( i=from; i < iteration_count; i++
  ) {
   if ( far_compare(left++, pat, pattern_length) == 0
    ) {
     return(i);
    };
  };
 return(0xFFFF);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       lowercase_string                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr lowercase_string(string_ptr s)
{


 far_to_lower(String(s), Length(s));
 return(s);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                      make_constant_string                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr make_constant_string(pool_descriptor_ptr pool, byte *s)
{
bit_16                                 len;
string_ptr                             temp;


 len  = strlen(CharPtr(s));
 temp = StringPtr(allocate_memory(pool, 
                                  Bit_32(sizeof(string_type))+Bit_32(len)));
 Length(temp)    =
 MaxLength(temp) = len++;
 far_move(String(temp), BytePtr(s), len);
 return(temp);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             match_pattern                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 match_pattern(string_ptr pattern, string_ptr s)
{
bit_16                                 i;
bit_16                                 n_searches;
byte_ptr                               source;


 if ( FirstCharIn(pattern) Is '*'
  ) {
   cut_string(pattern, 0, 1);
   if ( LastCharIn(pattern) Is '*'
    ) { /* We must perform exhaustive search */
     cut_string(pattern, 0, Length(pattern)-1);
     if ( Length(pattern) > Length(s)
      ) {
       return(False);
      };
     n_searches = Length(s) - Length(pattern) + 1;
     source = String(s);
     for ( i=0; i<n_searches; i++
      ) {
       if ( far_match(String(pattern), source++, Length(pattern))
        ) {
         return(True);
        };
      };
    } else { /* We must match only the tail */
     if ( Length(pattern) > Length(s)
      ) {
       return(False);
      } else {
       return(far_match(String(pattern),
                        BytePtr(Addr(String(s)[Length(s)-Length(pattern)])),
                        Length(pattern)));
      };
    };
  } else { /* We must match only the front */
   return(far_match(String(pattern), String(s), Length(pattern)));
  };
 return(False);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             near_string                                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
byte *near_string(string_ptr s)
{


 far_move(temp_near_string, String(s), Length(s)+1);
 return(temp_near_string);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            paste_string                                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr paste_string(string_ptr dest, bit_16 at, string_ptr s)
{
bit_16                                 length_inserted_string;
bit_16                                 length_string_to_right;
bit_16                                 length_string;


 length_string          = Length(dest);
 length_inserted_string = Length(s);
 if ( (length_string + length_inserted_string) > MaxLength(dest)
  ) {
   linker_error(8,"Destination string too small (%u bytes) to insert:\n"
                  "\t\"%Fs\" (%u bytes) into\n"
                  "\t\"%Fs\" (%u bytes)\n",
                  MaxLength(dest),
                  s, length_inserted_string,
                  String(dest), length_string);
  };
 if ( length_inserted_string == 0
  ) {
   return(dest);
  };
 if ( at > length_string
  ) {
   at = length_string;
  };
 length_string_to_right = length_string - at;
 far_move_left(Addr(String(dest)[at+length_inserted_string]),
               Addr(String(dest)[at]),
               length_string_to_right+1);
 far_move(Addr(String(dest)[at]), String(s), length_inserted_string);
 Length(dest) += length_inserted_string;
 return(dest);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         reverse_index_string                            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 reverse_index_string(string_ptr s, bit_16 from, string_ptr pattern)
{
bit_16                                 i;
bit_16                                 iteration_count;
bit_16                                 len;
byte_ptr                               right;
byte_ptr                               pat;
bit_16                                 pattern_length;


 pattern_length = Length(pattern);
 len            = Length(s);
 pat            = String(pattern);
 if ( len < pattern_length
  ) {
   return(0xFFFF);
  };
 if ( from > (len - pattern_length)
  ) {
   from = len - pattern_length;
  };
 iteration_count = from + 1;
 right = Addr(String(s)[from]);
 for ( i=0; i < iteration_count; i++, from--
  ) {
   if ( far_compare(right--, pat, pattern_length) == 0
    ) {
     return(from);
    };
  };
 return(0xFFFF);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               string                                    |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr string(byte_ptr s)
{
bit_16                                 len;


 len  = far_index(s, 0);
 if ( len > MaxLength(temp_string)
  ) {
   linker_error(8,"Destination string too small (%u bytes) to hold:\n"
                  "\t\"%s\" (%u bytes)\n",
                  MaxLength(temp_string),
                  s, len);
  };
 far_move(String(temp_string), s, len+1);
 Length(temp_string) = len;
 return(temp_string);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               substr                                    |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr substr(string_ptr s, bit_16 at, bit_16 len)
{
bit_16                                 string_length;


 string_length = Length(s);
 if ( string_length == 0
  ) {
   return(null_string);
  };
 if ( at >= string_length
  ) {
   at = string_length - 1;
  };
 if ( len > Bit_16(string_length - at)
  ) {
   len = string_length - at;
  };
 if ( len > MaxLength(temp_string)
  ) {
   linker_error(8,"Destination string too small in SUBSTR operation\n");
  };
 far_move(String(temp_string), Addr(String(s)[at]), len+1);
 Length(temp_string) = len;
 return(temp_string);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            trunc_string                                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr trunc_string(string_ptr s, bit_16 at)
{


 if ( Length(s) <= at+1
  ) {
   return(s);
  };
 Length(s)     = at;
 String(s)[at] = '\000';
 return(s);
}
