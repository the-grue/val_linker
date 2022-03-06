/*                                 DICTARY.C                               */

/*
There are several dictionaries which the linker maintains.  These
dictionaries are used as:

    1)  A dictionary of public (internal and external) symbols.
    2)  A dictionary of segments.
    3)  A dictionary of group names.
    4)  A dictionary of "LNAMES" and other names encountered in
        an object module.

All the dictionaries have a similar data structure.

Since the dictionary uses a hashing scheme, two linked list are used
as the principle dictionary data structure.  One linked list (based
on the "next" field) is used to link elements together in the classical
manner.  The other (based on the "next_congruent" field) is used to
link congruent elements together.  (The hashing scheme divides the
symbol table into partitions.  Elements which are in the same partition
are said to be congruent with each other.)  The dictionary of "LNAMES"
does not have the "next" link.

The second linked list forms a list of all symbols which are congruent
modulo the hash table size.  In other words, all the symbols having the
same remainder when you divide the sum of the characters forming the symbol
by the hash table size are in the same list.  Graphically, here is what
it looks like:

   hash_table
   +--------+[0]
   |        |
   +--------+[1]
   |        |
   +--------+
   ~        ~                       next next_congruent rest of entry
   +--------+[i]                   +----+--------------+-----//------+
   |   *----+--------------------->|    |      *       |             |
   +--------+                      +----+------+-------+-----//------+
   ~        ~                                  |
   +--------+[HASH_TABLE_SIZE-1]               V
   |        |                      +----+--------------+-----//------+
   +--------+                      |    |      *       |             |
                                   +----+------+-------+-----//------+
                                               |
Note:  The "next" field in the                 ~
       list is the classical                   |
       linking.  It is not                     V
       mapped out here.            +----+--------------+-----//------+
                                   |    |      *       |             |
                                   +----+------+-------+-----//------+
                                               |
                                             -----
                                              ---
                                               -

In addition to using a standard hash table for improving symbol table
lookup performance, a additional heuristic algorithm is added.  When a
lookup is performed for a symbol, that symbol is moved to the beginning
of the list of symbols in that congruency.  In this manner, symbols used
frequently will remain close to the beginning of the list and thereby
reduce searching time. */

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              lookup_group                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
group_entry_ptr lookup_group(lname_entry_ptr  group_lname)
{
#define Group_lname                    (*group_lname)
bit_16                                 hash_val;
group_entry_ptr                        group_entry;
#define Group_entry                    (*group_entry)
group_entry_ptr                        prior;
#define Prior                          (*prior)


 hash_val    = Group_lname.lname_checksum Mod group_table_hash_size.val;
 group_entry = group_hash_table[hash_val];
 prior       = Null;
 while ( group_entry != 0
  ) {
   if ( group_lname Is Group_entry.group_name
    ) {
     if ( prior != 0
      ) {  /* Move group to beginning of list */
       Prior.next_congruent       = Group_entry.next_congruent;
       Group_entry.next_congruent = group_hash_table[hash_val];
       group_hash_table[hash_val] = group_entry;
      };
     return(group_entry);
    };
   prior       = group_entry;
   group_entry = Group_entry.next_congruent;
  };
 group_entry = (group_entry_ptr)
                allocate_memory(Addr(static_pool),
                                Bit_32(sizeof(group_entry_type)));
 Insert group_entry AtEnd InList group_list EndInsert;
 Group_entry.group_name     = group_lname;
 Group_entry.next_congruent = group_hash_table[hash_val];
 group_hash_table[hash_val] = group_entry;
 Group_entry.first_segment  = Null;
 return(group_entry);
}
#undef Group_lname
#undef Group_entry
#undef Prior

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              lookup_lname                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
lname_entry_ptr lookup_lname(bit_16 len, byte *sym)
{
bit_16                                 charsum;
bit_16                                 hash_val;
lname_entry_ptr                        lname_entry;
#define Lname_entry                    (*lname_entry)
lname_entry_ptr                        prior;
#define Prior                          (*prior)


 charsum     = checksum(len, sym);
 hash_val    = charsum Mod lname_table_hash_size.val;
 lname_entry = lname_hash_table[hash_val];
 prior       = Null;
 while ( lname_entry != 0
  ) {
   if ( len Is Lname_entry.length
    ) {
     if ( far_compare(sym, Lname_entry.symbol, len) IsZero
      ) {
       if ( prior != 0
        ) {  /* Move lname to beginning of list */
         Prior.next_congruent       = Lname_entry.next_congruent;
         Lname_entry.next_congruent = lname_hash_table[hash_val];
         lname_hash_table[hash_val] = lname_entry;
        };
       return(lname_entry);
      };
    };
   prior       = lname_entry;
   lname_entry = Lname_entry.next_congruent;
  };
 lname_entry = (lname_entry_ptr)
                allocate_memory(Addr(static_pool),
                                Bit_32(sizeof(lname_entry_type))+Bit_32(len));
 Lname_entry.lname_checksum = charsum;
 Lname_entry.length         = len;
 far_move(Lname_entry.symbol, sym, len);
 Lname_entry.symbol[len]    = '\000';
 Lname_entry.next_congruent = lname_hash_table[hash_val];
 lname_hash_table[hash_val] = lname_entry;
 return(lname_entry);
}
#undef Lname_entry
#undef Prior

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             lookup_public                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
public_entry_ptr lookup_public(bit_16 len, byte *sym, bit_16 module)
{
bit_16                                 hash_val;
public_entry_ptr                       pub_entry;
#define Pub_entry                      (*pub_entry)
public_entry_ptr                       prior;
#define Prior                          (*prior)


 hash_val  = checksum(len, sym) Mod public_table_hash_size.val;
 pub_entry = public_hash_table[hash_val];
 prior     = Null;
 while ( pub_entry != 0
  ) {
   if ( len Is Pub_entry.length
    ) {
     if ( (far_compare(sym, Pub_entry.symbol, len) IsZero) AndIf
        (module Is Pub_entry.module)
      ) {
       if ( prior != 0
        ) {  /* Move public to beginning of list */
         Prior.next_congruent        = Pub_entry.next_congruent;
         Pub_entry.next_congruent    = public_hash_table[hash_val];
         public_hash_table[hash_val] = pub_entry;
        };
       return(pub_entry);
      };
    };
   prior     = pub_entry;
   pub_entry = Pub_entry.next_congruent;
  };
 pub_entry = (public_entry_ptr)
              allocate_memory(Addr(static_pool),
                              Bit_32(sizeof(public_entry_type))+Bit_32(len));
 Pub_entry.type_entry        = unused;
 Pub_entry.module            = module;
 Pub_entry.length            = len;
 far_move(Pub_entry.symbol, sym, len);
 Pub_entry.symbol[len]       = '\000';
 Pub_entry.next_congruent    = public_hash_table[hash_val];
 public_hash_table[hash_val] = pub_entry;
 return(pub_entry);
}
#undef Pub_entry
#undef Prior

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              lookup_segment                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
segment_entry_ptr lookup_segment(lname_entry_ptr  segment_lname,
                                 lname_entry_ptr  class_lname,
                                 combine_type     combine)
{
#define Segment_lname                  (*segment_lname)
#define Class_lname                    (*class_lname)
bit_16                                 hash_val;
segment_entry_ptr                      segment_entry;
#define Segment_entry                  (*segment_entry)
segment_entry_ptr                      prior;
#define Prior                          (*prior)


 hash_val      = (Segment_lname.lname_checksum +
                  Class_lname.lname_checksum)
                 Mod segment_table_hash_size.val;
 segment_entry = segment_hash_table[hash_val];
 if ( (combine != 0) AndIf (combine IsNot blank_common_combine)
  ) {  /* All non-zero combine types except blank common will be combined. */
   prior         = Null;
   while ( segment_entry != 0
    ) {
     if ( (segment_lname       Is Segment_entry.segment_name) AndIf
        (class_lname         Is Segment_entry.class_name)   AndIf
        (combine             Is Segment_entry.combine)
      ) {
       if ( prior != 0
        ) {  /* Move segment to beginning of list */
         Prior.next_congruent         = Segment_entry.next_congruent;
         Segment_entry.next_congruent = segment_hash_table[hash_val];
         segment_hash_table[hash_val] = segment_entry;
        };
       return(segment_entry);
      };
     prior       = segment_entry;
     segment_entry = Segment_entry.next_congruent;
    };
  };
 segment_entry = (segment_entry_ptr)
                allocate_memory(Addr(static_pool),
                                Bit_32(sizeof(segment_entry_type)));
 Insert segment_entry AtEnd InList segment_list EndInsert;
 Segment_entry.segment_name   = segment_lname;
 Segment_entry.class_name     = class_lname;
 Segment_entry.combine        = Bit_8(combine);
 Segment_entry.owning_group   = Null;
 Segment_entry.lsegs.first    =
 Segment_entry.lsegs.last     = Null;
 Segment_entry.address        =
 Segment_entry.length         = 0L;
 Segment_entry.next_congruent = segment_hash_table[hash_val];
 segment_hash_table[hash_val] = segment_entry;
 return(segment_entry);
}
#undef Segment_lname
#undef Segment_class_lname
#undef Segment_entry
#undef Prior
