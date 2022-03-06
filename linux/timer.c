/*                                 TIMER.C                                 */


/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            get_time                                     |
  |                                                                         |
  |                          O/S dependent                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 get_time(void)
{
bit_32                                 hhmmsscc;


 inregs.h.ah = 0x2C;                   /* Get time function */
 intdosx(&(inregs), &(outregs), &(segregs));
/* DOS_int21("Failed to get the system time.\n");*/
 hhmmsscc = Bit_32(outregs.h.dl)            +
            Bit_32(outregs.h.dh)  *    100L +
            Bit_32(outregs.h.cl)  *   6000L +
            Bit_32(outregs.h.ch)  * 360000L;
 return(hhmmsscc);
}


/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         elapsed_time                                    |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
char_ptr elapsed_time(bit_32 start_time, bit_32 stop_time)
{
 bit_16                                hh;
 bit_16                                mm;
 bit_16                                ss;
 bit_16                                cc;
 bit_32                                t;


 if ( start_time > stop_time
  ) { /* We passed midnight and must add 24 hours to stop time */
   stop_time += 8640000L;
  };
 t   = stop_time - start_time;
 cc  = Bit_16(t % 100L);                     t /= 100L;
 ss  = Bit_16(t % 60L);                      t /= 60L;
 mm  = Bit_16(t % 60L);                      t /= 60L;
 hh  = Bit_16(t);
 if ( hh != 0
  ) {
   sprintf(time_array,"%u:%02u:%02u.%02u",hh,mm,ss,cc);
  } else if ( mm != 0
   ) {
    sprintf(time_array,"%u:%02u.%02u",mm,ss,cc);
   } else {
    sprintf(time_array,"%u.%02u",ss,cc);
 };
 return(time_array);
}
