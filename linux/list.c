/*                                 LIST.C                                  */

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               ListDelete                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void ListDelete(Generic_Element_ptr       elem,
                Generic_Element_list far *lst)
{
#define Elem                           (*elem)
#define Lst                            (*lst)
Generic_Element_ptr                    current;
#define Current                        (*current)
Generic_Element_ptr                    prior;
#define Prior                          (*prior)


 prior = Null;
 TraverseList(Lst, current)
  BeginTraverse
   if ( current == elem
    ) {
     if ( prior == Null
      ) {
       First(Lst) = Elem.next;
       if ( First(Lst) == Null
        ) {
         Last(Lst) = Null;
        };
      } else {
       Prior.next = Elem.next;
       if ( Last(Lst) == elem
        ) {
         Last(Lst) = prior;
        };
      };
     break;
    };
   prior = current;
  EndTraverse;
 return;
}
#undef Elem
#undef Lst
#undef Current
#undef Prior

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           ListInsert                                    |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void ListInsert(Generic_Element_ptr       elem,
                bit_16                    type_insert,
                Generic_Element_ptr       aftr,
                Generic_Element_list far *lst)
{
#define Elem                           (*elem)
#define Aftr                           (*aftr)
#define Lst                            (*lst)


 switch ( type_insert
  ) {
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |  Handle:   Insert ptr After aftr InList lst EndInsert;                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
   case 0:
    Elem.next  = Aftr.next;
    Aftr.next  = elem;
    if ( Elem.next == 0
     ) {
      Last(Lst) = elem;
     };
    break;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |  Handle:   Insert ptr AtEnd InList lst EndInsert;                       |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
   case 1:
    if ( Last(Lst) == 0
     ) {
      First(Lst) = elem;
     } else {
      (*Last(Lst)).next = elem;
     };
    Last(Lst)  = elem;
    Elem.next  = Null;
    break;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |  Handle:   Insert ptr AtBeginning InList lst EndInsert;                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
   case 2:
    Elem.next  = First(Lst);
    First(Lst) = elem;
    if ( Last(Lst) == 0
     ) {
      Last(Lst) = elem;
     };
    break;
  };
 return;
}
#undef Elem
#undef Aftr
#undef Lst

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                ListPop                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void ListPop(Generic_Element_list far *lst,
             Generic_Element_ptr      *elem)
{
#define Elem                           (*elem)
#define Lst                            (*lst)


 if ( First(Lst) == 0
  ) {
   Elem = Null;
   return;
  };
 Elem       = First(Lst);
 First(Lst) = (*Elem).next;
 if ( First(Lst) == 0
  ) {
   Last(Lst) = Null;
  };
 return;
}
#undef Elem
#undef Lst
