/*                                 LINKERIO.C                              */


/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               DOS_error                                 |
  |                                                                         |
  |                             O/S dependent                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void DOS_error(char_ptr format, ... )
{
va_list                                argptr;
bit_16                                 error_number;


 if ( outregs.x.cflag
  ) {
   error_number = outregs.x.ax;
   if ( error_number Exceeds 58
    ) {
     error_number = 59;
    };
   fprintf(stdout,"\nDOS Error (Code %u): \"%s\"\n",
                  outregs.x.ax, DOS_error_text[error_number]);
   va_start(argptr,format);
   vfprintf(stdout,format,argptr);
   end_linker(16);
  };
 return;
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              DOS_int21                                  |
  |                                                                         |
  |                             O/S dependent                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void DOS_int21(char_ptr format, ... )
{
va_list                                argptr;
bit_16                                 error_number;


 intdosx(Addr(inregs), Addr(outregs), Addr(segregs));
 if ( (outregs.x.cflag) AndIf
    (Not ((inregs.h.al  Is 0x41) AndIf (outregs.x.ax Is 2)))
  ) {
   error_number = outregs.x.ax;
   if ( error_number Exceeds 58
    ) {
     error_number = 59;
    };
   fprintf(stdout,"\nDOS Error (Code %u): \"%s\"\n",
                  outregs.x.ax, DOS_error_text[error_number]);
   va_start(argptr,format);
   vfprintf(stdout,format,argptr);
   end_linker(16);
  };
 return;
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           linker_error                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void linker_error(unsigned severity, char_ptr format, ...)
{
va_list                                argptr;


 fprintf(stdout,"\nLinker Error (Severity %d)\n",severity);
 va_start(argptr,format);
 vfprintf(stdout,format,argptr);
 if ( severity Exceeds 7
  ) {
   end_linker(severity);
  } else {
   return;
  };
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            linker_message                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void linker_message(char_ptr format, ...)
{
va_list                                argptr;
static char                            flag = 0;


 if(!flag) { /* Issue Signon message */
  flag = -1;
  fprintf(stdout,"VAL 8086 linker - %s\n",__DATE__); }
 va_start(argptr,format);
 vfprintf(stdout,format,argptr);
 return;
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                 print                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void print(char_ptr format, ...)
{
va_list                                argptr;
bit_16                                 len;


 va_start(argptr,format);
 vsprintf(CharPtr(object_file_element), format, argptr);
 len = strlen(CharPtr(object_file_element));
 if ( len IsZero
  ) {
   return;
  };
 if ( object_file_element[len-1] Is '\n'
  ) {
   object_file_element[len-1] = '\000';
   strcat(CharPtr(object_file_element), "\r\n");
   len++;
  };
 file_write(BytePtr(object_file_element), Bit_32(len));
 return;
}
