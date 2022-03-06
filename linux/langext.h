/*                                 LANGEXT.H                               */

/* Fixup some C syntax I really don't like
   De gustibus non est desputandum
   (Concerning taste there is not argument -- or
    Only fools argue where taste is concerned) */

/* Linked List commands */

#define ListTypeOf(exp) \
struct exp##_list_struct \
 { \
  exp##_ptr                            first; \
  exp##_ptr                            last; \
 }; \
typedef struct exp##_list_struct       exp##_list

struct Generic_Element_struct
 {
  struct Generic_Element_struct far   *next;
 };

typedef struct Generic_Element_struct  Generic_Element_type;
typedef Generic_Element_type far         *Generic_Element_ptr;

ListTypeOf(Generic_Element);

#define TraverseList(lst,elm)  for(elm=lst.first; elm != 0; Next(elm))
#define BeginTraverse        {
#define EndTraverse          }

#define Insert               ListInsert((Generic_Element_ptr)
#define After                ,0,(Generic_Element_ptr)
#define AtEnd                ,1,Null
#define AtBeginning          ,2,Null
#define InList               ,(Generic_Element_list far *) &(
#define EndInsert            ))

#define Delete               ListDelete((Generic_Element_ptr)
#define FromList             ,(Generic_Element_list far *) &(
#define EndDelete            ))

#define Push                 ListInsert((Generic_Element_ptr)
#define OnTo                 ,2,Null,(Generic_Element_list far *) &(
#define EndPush              ))

#define Pop                  ListPop((Generic_Element_list far *) &(
#define InTo                 ),(Generic_Element_ptr *) &(
#define EndPop               ))

#define LastInList(ptr)      ((*ptr).next == NULL)
#define Next(ptr)            ptr = (*ptr).next
#define First(list)          list.first
#define Last(list)           list.last

/* Make operations less cryptic */

/* Logical operators */
#define IsIdentifier(x)      (((x>='A') && (x<='Z')) || \
                              ((x>='a') && (x<='z')) || \
                               (x=='_'))
#define IsNumber(x)          ((x>='0') && (x<='9'))

/* Some other operators */
/*#define Addr(exp)            &(exp)*/
#define Mod                  %
#define ShiftedLeft          <<
#define ShiftedRight         >>

/* Some useful constants */

#define False                0
#define Null                 0
#define True                 1

/* Some helpful types */
typedef unsigned char           bit_8;
typedef unsigned int            bit_16;
typedef unsigned long           bit_32;
typedef unsigned char           byte;
typedef byte far               *byte_ptr;
typedef void far               *far_ptr;
typedef signed char             int_8;
typedef signed int              int_16;
typedef signed long             int_32;
typedef void  /* near */       *near_ptr;
typedef char  /* near */       *char_ptr;

struct string_struct
 {
  bit_16                     max_length;
  bit_16                     length;
  byte                       text[1];
 };

typedef struct string_struct string_type;
typedef string_type far        *string_ptr;

#define String(str)          ((byte_ptr) ((*str).text))
#define Length(str)          (*str).length
#define MaxLength(str)       (*str).max_length
#define FirstCharIn(str)     *String(str)
#define LastCharIn(str)      String(str)[Length(str)-1]

/* Some helpful type casts */

#define Bit_8(exp)           ((bit_8)      (exp))
#define Bit_16(exp)          ((bit_16)     (exp))
#define Bit_32(exp)          ((bit_32)     (exp))
#define Int_8(exp)           ((int_8)      (exp))
#define Int_16(exp)          ((int_16)     (exp))
#define Int_32(exp)          ((int_32)     (exp))
#define CharPtr(exp)         ((char *)     (exp))
#define Byte(exp)            ((byte)       (exp))
#define BytePtr(exp)         ((byte_ptr )  (exp))
#define NearPtr(exp)         ((near_ptr)   (exp))
#define FarPtr(exp)          ((far_ptr)    (exp))
#define StringPtr(exp)       ((string_ptr) (exp))
#define Offset(exp)          ((bit_16)     (exp))
#define Segment(exp)         ((bit_16)     (((bit_32) (exp)) >> 16L))
#define Low(exp)             ((bit_16)     (exp))
#define High(exp)            ((bit_16)     (((bit_32) (exp)) >> 16L))
#define MakeFarPtr(seg,off)  ((far_ptr)    ((((bit_32) (seg)) << 16L)|(off)))
