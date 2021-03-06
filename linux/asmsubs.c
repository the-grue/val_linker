/*                                ASMSUBS.C                                 */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <dos.h>
#include <string.h>

#include "langext.h"
#include "defines.h"
#include "types.h"
#include "subs.h"

#pragma inline

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               checksum                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 checksum(bit_16 len, byte *sym)
{


/* The following assembly code is the equivalent of the following C code:
 for ( i=0; i < len; i++
  ) {
   sum += sym[i];
  };
 return(sum); */

 asm            mov     si,sym
 asm            mov     cx,len
 asm            xor     ax,ax
 asm            mov     bx,ax
hash_loop:
 asm            mov     bl,[si]
 asm            add     ax,bx
 asm            inc     si
 asm            loop    hash_loop
 return(_AX);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          far_compare                                    |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
int_16 far_compare (byte_ptr left, byte_ptr right, bit_16 len)
{


/* The following assembly code is the equivalent of the following C code:
 while ( len-- != 0
  ) {
   if ( *left != *right
    ) {
     if ( *left < *right
      ) {
       return(-1);
      } else {
       return(1);
      };
    };
   left++;
   right++;
  };
 return(0); */

 asm            cld
 asm            push    ds
 asm            mov     cx,len
 asm            les     di,right
 asm            lds     si,left
 asm    rep     cmpsb
 asm            pop     ds
 asm            jb      left_less_than_right
 asm            ja      left_greater_than_right

 return(0);
left_less_than_right:
 return(-1);
left_greater_than_right:
 return(1);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            far_index                                    |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 far_index(byte_ptr dest, byte c)
{
bit_16                                 i;


/* The following assembly code is the equivalent of the following C code:
 for ( i=0; (*dest++ != c) && (i != 0xFFFF); i++
  ) {
  };
 return(i); */

 asm            xor     cx,cx
 asm            mov     al,c
 asm            les     di,dest
search_loop:
 asm            cmp     al,es:[di]
 asm            je      search_done
 asm            inc     di
 asm            inc     cx
 asm            jne     search_loop
 asm            dec     cx
search_done:
 asm            mov     i,cx

 return(i);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              far_match                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 far_match (byte_ptr pattern, byte_ptr s, bit_16 len)
{


/* The following assembly code is the equivalent of the following C code:
 while ( len > 0
  ) {
   if ( *pattern == '*'
    ) {
     return(True);
    };
   if ( (*pattern != '?') && (*pattern != *source)
    ) {
     return(False);
    };
   source++;
   pattern++;
  };
 return(True); */

 asm            mov     cx,len
 asm            les     di,pattern
 asm            push    ds
 asm            lds     si,s
pattern_loop:
 asm            mov     al,es:[di]
 asm            cmp     al,'?'
 asm            je      a_match
 asm            cmp     al,'*'
 asm            je      succeeded
 asm            cmp     al,[si]
 asm            jne     failed
a_match:
 asm            inc     si
 asm            inc     di
 asm            loop    pattern_loop
succeeded:
 asm            pop     ds
 return(True);
failed:
 asm            pop     ds
 return(False);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          far_move                                       |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
byte_ptr far_move (byte_ptr dest, byte_ptr source, bit_16 len)
{


/* The following assembly code is the equivalent of the following C code:
 while ( len-- != 0
  ) {
   *dest++ = *source++;
  }; */

 asm            cld
 asm            push    ds
 asm            mov     cx,len
 asm            les     di,dest
 asm            lds     si,source
 asm    rep     movsb
 asm            pop     ds

 return(dest);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          far_move_left                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
byte_ptr far_move_left (byte_ptr dest, byte_ptr source, bit_16 len)
{


/* The following assembly code is the equivalent of the following C code:
 dest   += len - 1;
 source += len - 1;
 while ( len-- != 0
  ) {
   *dest-- = *source--;
  }; */

 asm            std
 asm            push    ds
 asm            mov     cx,len
 asm            les     di,dest
 asm            lds     si,source
 asm            add     di,cx
 asm            add     si,cx
 asm            dec     si
 asm            dec     di
 asm    rep     movsb
 asm            pop     ds

 return(dest);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                far_set                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
byte_ptr far_set (byte_ptr dest, byte source, bit_16 len)
{


/* The following assembly code is the equivalent of the following C code:
 while ( len-- != 0
  ) {
   *dest++ = source;
  }; */

 asm            cld
 asm            mov     cx,len
 asm            les     di,dest
 asm            mov     al,source
 asm            stosb
 asm            dec     cx
 asm    rep     stosb

 return(dest);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          far_to_lower                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
byte_ptr far_to_lower (byte_ptr dest, bit_16 len)
{


/* The following assembly code is the equivalent of the following C code:
 while ( len-- != 0
  ) {
   tolower(*dest++);
  }; */

 asm            mov     cx,len
 asm            les     di,dest
lower_case_loop:
 asm            mov     al,es:[di]
 asm            cmp     al,'A'
 asm            jb      next_byte
 asm            cmp     al,'Z'
 asm            ja      next_byte
 asm            add     al,'a'-'A'
 asm            mov     es:[di],al
next_byte:
 asm            inc     di
 asm            loop    lower_case_loop

 return(dest);
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         library_directory_hash                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void library_directory_hash(byte_ptr      sym,
                            bit_16        len,
                            bit_16       *starting_block,
                            bit_16       *delta_block,
                            bit_16       *starting_entry,
                            bit_16       *delta_entry)
{


/* The following assembly code is the equivalent of the following C code:
{
byte                                  *beg_str;
byte                                  *end_str;
bit_16                                 c;
byte                                   temp[33];


 beg_str = temp;
 end_str = &(temp[len]);
 temp[0] = Byte(len);
 far_move(BytePtr(&(temp[1]), sym, len);
 *starting_block =
 *delta_block    =
 *starting_entry =
 *delta_entry    = 0;
 while ( len-- > 0
  ) {
   c = (Bit_16(*beg_str++) & 0xFF) | 0x20;
   *starting_block = c ^ ((*starting_block <<   2)  |
                            (*starting_block >> 14));
   *delta_entry    = c ^ ((*delta_entry    >>  2)  |
                            (*delta_entry    <<  14));
   c = (Bit_16(*end_str--) & 0xFF) | 0x20;
   *delta_block    = c ^ ((*delta_block    <<   2)  |
                            (*delta_block    >> 14));
   *starting_entry = c ^ ((*starting_entry >>  2)  |
                            (*starting_entry <<  14));
  }; */
 asm            xor     ax,ax
 asm            mov     cl,2
 asm            xor     bx,bx
 asm            xor     dx,dx
 asm            mov     bl,len
 asm            mov     dl,len
 asm            or      bx,0x20
 asm            or      dx,0x20
 asm            les     si,sym
 asm            mov     di,len
 asm            dec     di
up_loop:
 asm            mov     al,es:[si]
 asm            or      al,0x20
 asm            rol     bx,cl
 asm            ror     dx,cl
 asm            xor     bx,ax
 asm            xor     dx,ax
 asm            inc     si
 asm            dec     di
 asm            jnz     up_loop
 asm            mov     di,starting_block
 asm            mov     [di],bx
 asm            mov     di,delta_entry
 asm            mov     [di],dx
 asm            mov     di,len
 asm            xor     bx,bx
 asm            xor     dx,dx
down_loop:
 asm            mov     al,es:[si]
 asm            or      al,0x20
 asm            rol     bx,cl
 asm            ror     dx,cl
 asm            xor     bx,ax
 asm            xor     dx,ax
 asm            dec     si
 asm            dec     di
 asm            jnz     down_loop
 asm            mov     di,delta_block
 asm            mov     [di],bx
 asm            mov     di,starting_entry
 asm            mov     [di],dx
 return;
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              word_checksum                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 word_checksum(bit_16 len, bit_16 address, byte_ptr data)
{


/* The following assembly code is the equivalent of the following C code:
{
struct word_struct
 {
  bit_8                                low_byte;
  bit_8                                high_byte;
 };
bit_16                                 sum;
struct word_struct                  word;
bit_16                                *word_ptr;

 word = (bit_16 *) &(word);
 for ( i=0; i < len; i++
  ) {
   *word_ptr = 0;
   if ( (address-- & 1)
    ) {
     word.high_byte = *data++;
    } else {
     word.low_byte = *data++;
    };
   sum += *word_ptr;
  };
 return(sum); */

 asm            xor     ax,ax
 asm            les     di,data
 asm            mov     bx,address
 asm            mov     cx,len
/* Handle case where we are starting at an odd location */
 asm            and     bx,1
 asm            je      at_even_location
 asm            mov     ah,es:[di]
 asm            inc     di
 asm            dec     cx
at_even_location:
/* Loop while we have at least two bytes left */
 asm            cmp     cx,2
 asm            jb      at_end_of_sum
 asm            add     ax,es:[di]
 asm            inc     di
 asm            inc     di
 asm            dec     cx
 asm            loop    at_even_location
at_end_of_sum:
 asm            cmp     cx,1
 asm            jne     checksum_done
 asm            add     al,es:[di]
 asm            adc     ah,0
checksum_done:
 return(_AX);
}
