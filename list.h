/*2:*/
#line 12 "./temp.w"

#include <stdio.h> 
#include "mytypes.h"
typedef struct
{
SInt32*body;
UInt32 size;
UInt32 padded_size;
UInt32 body_length;
UInt32 length;
UInt32 read_index;
UInt32 read_delete_index;
}list;
#define list_count(l) ((l)->length)
#define list_is_empty(l) ((l)->length == 0)
#define list_clear(l)  ((l)->length =  0)
#define list_entry(l,n) ((l)->body + (n)*((l)->padded_size))

void list_initialize(list**l,UInt32 sz);
void list_pop(list*l,void*q);
void*list_push(list*l,void*q);
void list_abandon(list**l);


/*:2*//*7:*/
#line 138 "./temp.w"

#define list_read_init(l) ((l)->read_index =  (l)->read_delete_index =  0)
#define list_read_init_at(l,n) ((l)->read_index =  (l)->read_delete_index =  \
 ((n)<l->length)? (n):l->length)
#define list_read_delete(l) ((l)->read_delete_index--)
void*list_read(list*l);

/*:7*//*10:*/
#line 203 "./temp.w"

typedef struct
{
SInt32**first;
SInt32**last;
UInt32 size;
UInt32 padded_size;
UInt32 first_index;
UInt32 last_index;
UInt32 num_segments;
}fifo;
#define fifo_count(l) ((l)->last_index-(l)->first_index+1000*((l)->num_segments-1))
#define fifo_is_empty(l) ((l)->num_segments == 1 && (l)->last_index==(l)->first_index)





void fifo_initialize(fifo**l,UInt32 sz);
void fifo_pop(fifo*l,void*q);
void*fifo_push(fifo*l,void*q);
void fifo_abandon(fifo**l);
void fifo_clear(fifo*l);


/*:10*//*15:*/
#line 362 "./temp.w"

struct hlist_key
{
SInt32 high;
SInt32 low;
SInt32 eq;
SInt32 k[3];
};

typedef struct
{
list*keys;
list*entries;
UInt16 sz;
UInt16 hashsize;
UInt8 scaling;
UInt8 depth;
UInt16 first_nonempty;
SInt32 free;
SInt32 hash[];
}hlist;

#define hlist_entry(l,n) list_entry((l)->entries,n)
#define hlist_get_key(l,n,m)  (((struct hlist_key *)list_entry((l)->keys,n))->k[m])
#define hlist_count(l) list_count((l)->entries)
#define hlist_find(l,m) hlist_find_add(l,m,NULL,NULL)
#define hlist_get(l,m,p) hlist_sub_match(l,m,p,0)
#define hlist_min(l,m,p) hlist_sub_min(l,m,p,0)

/*:15*//*16:*/
#line 392 "./temp.w"


void hlist_initialize(hlist**l,UInt32 sz,UInt16 hashsize,UInt16 depth,UInt16 scale);
void hlist_abandon(hlist**l);
void hlist_clear(hlist*l);
Boolean hlist_is_empty(hlist*l);
Boolean hlist_sub_min(hlist*l,SInt32*m,void*p,int subdepth);
Boolean hlist_sub_max(hlist*l,SInt32*m,void*p,int subdepth);
Boolean hlist_sub_match(hlist*l,SInt32*m,void*p,int subdepth);
SInt32 hlist_add(hlist*l,SInt32*m,void*p);
SInt32 hlist_find_add(hlist*l,SInt32*m,void*p,Boolean*flag);
void hlist_delete(hlist*l,SInt32 index);
Boolean hlist_find_delete(hlist*l,SInt32*m,void*p);
SInt32 hlist_add_delete(hlist*l,SInt32*m,void*p);
Boolean hlist_id_integrity(hlist*l,int dim,Boolean print);
Boolean hlist_integrity(hlist*l);
float hlist_balance_check(hlist*l);

/*:16*/
