/*                                 LIST.C                                  */

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               ListDelete                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void ListDelete(Generic_Element_ptr       elem,
                Generic_Element_list far *lst)
BeginDeclarations
#define Elem                           (*elem)
#define Lst                            (*lst)
Generic_Element_ptr                    current;
#define Current                        (*current)
Generic_Element_ptr                    prior;
#define Prior                          (*prior)
EndDeclarations
BeginCode
 prior = Null;
 TraverseList(Lst, current)
  BeginTraverse
   if ( current Is elem
    Then
     if ( prior Is Null
      Then
       First(Lst) = Elem.next;
       if ( First(Lst) Is Null
        Then
         Last(Lst) = Null;
        };
      Else
       Prior.next = Elem.next;
       if ( Last(Lst) Is elem
        Then
         Last(Lst) = prior;
        };
      };
     ExitLoop;
    };
   prior = current;
  EndTraverse;
 return;
EndCode
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
BeginDeclarations
#define Elem                           (*elem)
#define Aftr                           (*aftr)
#define Lst                            (*lst)
EndDeclarations
BeginCode
 Using type_insert
  BeginCase
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |  Handle:   Insert ptr After aftr InList lst EndInsert;                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
   When 0:
    Elem.next  = Aftr.next;
    Aftr.next  = elem;
    if ( Elem.next IsNull
     Then
      Last(Lst) = elem;
     };
    break;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |  Handle:   Insert ptr AtEnd InList lst EndInsert;                       |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
   When 1:
    if ( Last(Lst) IsNull
     Then
      First(Lst) = elem;
     Else
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
   When 2:
    Elem.next  = First(Lst);
    First(Lst) = elem;
    if ( Last(Lst) IsNull
     Then
      Last(Lst) = elem;
     };
    break;
  EndCase;
 return;
EndCode
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
BeginDeclarations
#define Elem                           (*elem)
#define Lst                            (*lst)
EndDeclarations
BeginCode
 if ( First(Lst) IsNull
  Then
   Elem = Null;
   return;
  };
 Elem       = First(Lst);
 First(Lst) = (*Elem).next;
 if ( First(Lst) IsNull
  Then
   Last(Lst) = Null;
  };
 return;
EndCode
#undef Elem
#undef Lst
