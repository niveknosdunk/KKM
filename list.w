@i mytypes.w




@* Unordered Lists.
Unordered lists work as LIFO stacks.
We can push to and pop from a |list|.
We can find the $n$-th entry on a |list|.
We can read the entries of a |list| one by one, deleting those we no longer want.

@(list.h@>=
#include <stdio.h>
#include "mytypes.h"
typedef struct 
{
	SInt32 *body;
	UInt32 size;  // size of each entry, not including padding
	UInt32 padded_size;  // size of each entry, in long words
	UInt32 body_length;  // capacity of body
	UInt32 length;	// number of entries on list
	UInt32 read_index;
	UInt32 read_delete_index;
} list;
#define list_count(l) ((l)->length)
#define list_is_empty(l) ((l)->length == 0)
#define list_clear(l)  ((l)->length = 0)
#define list_entry(l,n) ((l)->body + (n)*((l)->padded_size))

void list_initialize(list **l,UInt32 sz);
void list_pop(list *l,void *q);
void *list_push(list *l,void *q);
void list_abandon(list **l);


@ Initialize a list, setting up storage
@c
#include "list.h"
void list_initialize(list **l,UInt32 sz)
{
	*l = (list *)malloc(sizeof(list));
	if((*l)==NULL) abort_message("Out of memory");
	(*l) -> size = sz;
	(*l) -> padded_size = (sz+sizeof(SInt32)-1)/sizeof(SInt32);
	(*l) -> length = 0;
	(*l) -> body = NULL;
	(*l) -> body_length = 0;
}

  
@ Abandon a list, freeing storage used.
@c

void list_abandon(list **l)
{
	if ((*l)->body_length>0) free((*l)->body);
	free(*l);
	*l = NULL;
}



@ |list_push| pushes a new entry pointed to by |q| to the top of the list.
The variant |plist_push| pushes the pointer |q| directly to the list
(as opposed to the contents pointed to by |q|).

@c

void *list_push(list *l,void *q)
{
	SInt32 *p;
	if(l->body_length <= l->length)
	{
		l->body_length  = l->length + 1000;
		l->body = (SInt32 *)realloc(l->body,l->body_length*l->padded_size*sizeof(SInt32));
		if(l->body==NULL) abort_message("out of memory");
	}
	
	p = list_entry(l, l->length);

	memcpy(p,q,l->size);
	l->length++;
	return p;
}

void plist_push(list *l, void *q)
{
	void *p = q;
	list_push(l,&p);
}


@ |list_pop| pops the top entry into the region pointed to by |q|,
and deletes it from the list.
It copies the top entry to |q|.
The variant |plist_pop| returns the entry from a pointer list directly.

@c

void list_pop(list *l,void *q)
{
	SInt32 *p;
	
	if (l->length==0) abort_message("pop from empty list");
	
	l->length--;
	p = list_entry(l, l->length);
	memcpy(q,p,l->size);
}

void *plist_pop(list *l)
{
	void *p;
	
	list_pop(l,&p);
	return p;
}


@ Routine to read the entries of a list one by one,
and deleting those you don't wish to retain on the list.
To use it, first call |list_read_init| (to begin at the beginning of the list)
or |list_read_init_at| (to begin at a specified item of the list).
Then repeated calls to |list_read| will return the entries on the list,
until a |NULL| is returned signifying the end.
If you wish to delete the entry you last read, just call |list_read_delete|.
If your list is a list of pointers, then 
the companion pointer list routine |plist_read| returns the pointer,
rather than its address.
Since the end of the list is signaled by returning a |NULL| pointer
it will only work well if the list of pointers
contains no |NULL| pointers, otherwise you might stop too early.
So in this case you would need some alternate method to
recognize the end of the list.

{\bf Warning:} if you use |list_read_delete| then you {\bf must} read through
to the end of the list.  
@(list.h@>=
#define list_read_init(l) ((l)->read_index = (l)->read_delete_index = 0)
#define list_read_init_at(l,n) ((l)->read_index = (l)->read_delete_index = \
	((n)<l->length)? (n):l->length)
#define list_read_delete(l) ((l)->read_delete_index--)
void *list_read(list *l);

@ @c
void *list_read(list *l)
{
	void *p,*pp;
	
	if(l->read_index >= l->length)
	{
		// we are at the end of the list
		l->length = l->read_index = l->read_delete_index;
		return(NULL);
	}
	
	pp = list_entry(l,l->read_delete_index);
	

	if(l->read_index != l->read_delete_index) 
	{
		p = list_entry(l,l->read_index);
		memcpy(pp, p, l->size);
	}
	
	l->read_delete_index++;
	l->read_index++;
	return pp;
}

void *plist_read(list *l)
{
	void **r;
	r = list_read(l);
	if(r!=NULL) return *r;
	else return NULL; 
}

@ The following |list_read_insert| will insert an item in a list
you are reading, placing it just before the most recently read item
if there is room, or if not, placing it at the end.
@c
void *list_read_insert(list *l,void *p)
{
	if(l->read_index == l->read_delete_index) 
		list_push(l,p);
	else
	{
		memcpy(list_entry(l,l->read_delete_index), p, l->size);
		l->read_delete_index++;
	}
}


@* FIFO list.
This is a first in first out stack.
Rather than being kept in a continuous block
as the above LIFO lists, we keep it in a variable number of segments,
each linked to the next.



@(list.h@>=
typedef struct 
{
	SInt32 **first;  // The first segment of the list
	SInt32 **last;  // The last segment of the list
	UInt32 size;  // size of each entry, not including padding
	UInt32 padded_size;  // size of each entry, in long word pointers
	UInt32 first_index;	// index of the first entry on the list
	UInt32 last_index; // index just after the last entry on the list
	UInt32 num_segments;  // number of segments
} fifo;
#define fifo_count(l) ((l)->last_index-(l)->first_index+1000*((l)->num_segments-1))
#define fifo_is_empty(l) ((l)->num_segments == 0 && (l)->last_index==(l)->first_index)

void fifo_initialize(fifo **l,UInt32 sz);
void fifo_pop(fifo *l,void *q);
void *fifo_push(fifo *l,void *q);
void fifo_abandon(fifo **l);
void fifo_clear(fifo *l);


@ Initialize a FIFO list, setting up storage
@c
void fifo_initialize(fifo **l,UInt32 sz)
{
	SInt32 *p;

	*l = (fifo *)malloc(sizeof(fifo));
	if((*l)==NULL) abort_message("Out of memory");
	(*l) -> size = sz;
	(*l) -> padded_size = (sz+sizeof(SInt32 *)-1)/sizeof(SInt32 *);
	(*l) -> first_index = 0;
	(*l) -> last_index = 0;
	p = (SInt32 *)malloc((1+1000 * (*l)->padded_size)*sizeof(SInt32 *));
	if(p==NULL) abort_message("out of memory");
	*p++=NULL;
	(*l) -> first = p;
	(*l) -> last = p;
	(*l) -> num_segments = 1;
}

  
@ Abandon a list, freeing storage used.
@c

void fifo_abandon(fifo **l)
{
	SInt32 **p;
	
	while ((*l)->first != NULL) 
	{ 
		p = (*l)->first-1;
		(*l)->first = (SInt32 **)*p; 
		free(p);
	}
	free(*l);
	*l = NULL;
}



@ |fifo_push| pushes a new entry pointed to by |q| to the top of the list.

@c

void *fifo_push(fifo *l,void *q)
{
	SInt32 **p;
	if(l->last_index == 1000)
	{
		p = (SInt32 **)malloc((1+1000 * l->padded_size)*sizeof(SInt32 *));
		if(p==NULL) abort_message("out of memory");
		*p++=NULL;
		*(l->last-1)=(SInt32 *)p;
		l->last = p;
		l->last_index = 0;
		l->num_segments++;
	}
	
	p = l->last + l->last_index*l-> padded_size;

	memcpy(p,q,l->size);
	l->last_index++;
	return p;
}



@ |fifo_pop| pops the bottom entry into the region pointed to by |q|,
and deletes it from the list.


@c

void fifo_pop(fifo *l,void *q)
{
	SInt32 **p;
	
	if (fifo_is_empty(l)) abort_message("pop from empty list");
	
	if(l->first_index==1000)
	{
		p = l->first-1;
		l->first = (SInt32 **)(*p);
		free(p);
		l->first_index=0;
		l->num_segments--;
		if(l->num_segments==0) abort_message("fifo error");
	}
	
	p = l->first + l->first_index*l-> padded_size;
	l->first_index++;
	memcpy(q,p,l->size);
	if (fifo_is_empty(l)) l->last_index = l->first_index = 0;
}



@* Hash list. 
This is a hash list which can do double duty as an ordered list.
It allows up to three integer keys, lex ordered.
The hash function takes the first key, shifts it right by |scaling|
and then takes its residue mod |hashsize|.
This is very primitive, but allows you to have an order preserving hash
function if you want an ordered list.

Internally, there are two lists of the same size, |entries| and |keys|.
|entries| is the list of actual entries on the list and
|keys| is the keys of the entries as well as pointers giving a tree
structure to the list.
Each entry in the hash table |hash| is an index of some entry 
with that hash value, or -1 if there is no such entry.

Each entry has an intrinsic depth.
All entries pointed to by the hash table have depth 0.
Each entry has three indices of other entries, |high|, |low|, and |eq|.

The index |high| is the index of an entry with the same depth
so that the keys |k[i]| match for all $i<depth$ and so |k[depth]|
is larger.
If there is no such entry, then |high| is -1.

Likewise, the index |low| is the index of an entry with the same depth
so that the keys |k[i]| match for all $i<depth$ and so |k[depth]|
is smaller.
If there is no such entry, then |low| is -1.

The index |eq| points to an entry of one higher depth
so that the keys |k[i]| match for all $i\leq depth$.
If there is no such entry, then |eq| is -1.

|free| is the index of an unused list entry.
The unused entries are linked using the |low| index.


@(list.h@>=
struct hlist_key
{
	SInt32 high;
	SInt32 low;
	SInt32 eq;
	SInt32 k[3];  // order keys 
};

typedef struct 
{
	list *keys;
	list *entries;
	UInt16 sz;
	UInt16 hashsize;
	UInt8 scaling;
	UInt8 depth; // number of keys
	UInt16 first_nonempty; // first possibly nonempty hash entry
	SInt32 free; 
	SInt32 hash[];
} hlist;

#define hlist_entry(l,n) list_entry((l)->entries,n)
#define hlist_get_key(l,n,m)  (((struct hlist_key *)list_entry((l)->keys,n))->k[m])
#define hlist_count(l) list_count((l)->entries)
#define hlist_find(l,m) hlist_find_add(l,m,NULL,NULL)
#define hlist_get(l,m,p) hlist_sub_match(l,m,p,0)
#define hlist_min(l,m,p) hlist_sub_min(l,m,p,0)

@ hlist function prototypes
@(list.h@>=

void hlist_initialize(hlist **l,UInt32 sz,UInt16 hashsize,UInt16 depth,UInt16 scale);
void hlist_abandon(hlist **l);
void hlist_clear(hlist *l);
Boolean hlist_is_empty(hlist *l);
Boolean hlist_sub_min(hlist *l,SInt32 *m,void *p,int subdepth);
Boolean hlist_sub_max(hlist *l,SInt32 *m,void *p,int subdepth);
Boolean hlist_sub_match(hlist *l,SInt32 *m, void *p,int subdepth);
SInt32 hlist_add(hlist *l,SInt32 *m,void *p);
SInt32 hlist_find_add(hlist *l,SInt32 *m,void *p,Boolean *flag);
void hlist_delete(hlist *l,SInt32 index);
Boolean hlist_find_delete(hlist *l,SInt32 *m, void *p);
SInt32 hlist_add_delete(hlist *l,SInt32 *m, void *p);
Boolean hlist_id_integrity(hlist*l,int dim,Boolean print);
Boolean hlist_integrity(hlist *l);
float hlist_balance_check(hlist *l);

@ Sometimes we want to travel down a tree taking random branches,
the following more or less randomly returns true or false.
For efficiency, it reuses each random number 64 times
before generating a new one.
Maybe this is not needed, I've never tested but
in any case here we don't need things so random
so it should be no big deal.

@d random_bit ((((random_bit_i++)&2047)==0)?@/
(random_bit_n=random())&1:
((random_bit_n>>(random_bit_i&31))&1))

@c
UInt32 random_bit_n,random_bit_i=2048; 

@ @c
void hlist_initialize(hlist **l,UInt32 sz,UInt16 hashsize,UInt16 depth,UInt16 scale)
{
	UInt16 i;
	
	*l = (hlist *)malloc(sizeof(hlist)+hashsize*sizeof(SInt32));
	if((*l)==NULL) abort_message("Out of memory");
	list_initialize(&((*l)->keys),sizeof(struct hlist_key));
	list_initialize(&((*l)->entries),sz);
	(*l)->free = -1;
	(*l)->sz = sz;
	(*l)->hashsize = hashsize;
	(*l)->depth = depth;
	(*l)->scaling = scale;
	(*l)->first_nonempty = hashsize;
	for(i=0;i<hashsize;i++) (*l)->hash[i] = -1;
}

@ @c
void hlist_abandon(hlist **l)
{
	list_abandon(&((*l)->keys));
	list_abandon(&((*l)->entries));
	free(*l);
	*l = NULL;
}

@ @c
void hlist_clear(hlist *l)
{
	UInt16 i;
	list_clear(l->keys);
	list_clear(l->entries);
	l->free = -1;
	for(i=l->first_nonempty;i<l->hashsize;i++) l->hash[i] = -1;
}

@ @c
Boolean hlist_is_empty(hlist *l)
{
	SInt32 min_index;

	for(min_index = l->first_nonempty; min_index < l->hashsize; min_index++)
		if(l->hash[min_index]>=0) break;
	l->first_nonempty = min_index;
	return min_index>=l->hashsize;
}


@ Find the minimum of all entries in the list |l|
which match the keys in |m|
up to the level |subdepth|.
Return all its keys in |m|, 
and put the minimum list entry in |p|.  
Delete this entry from the list.
It returns false if there are no matches.
If |subdepth| is zero, then it may not actually return the minimum,
it will return the minimum entry on the tree among all
entries with minimum hash function.

Usually, most of these entries will be in a tree at depth $>$ |subdepth|,
but there may be at most one entry at each smaller depth.
We put their indices in |roots|.
@c
Boolean hlist_sub_min(hlist *l,SInt32 *m,void *p,int subdepth)
{
	struct hlist_key *q;
	SInt32 *last,index,*roots[4];
	int depth;  /* the number of valid entries in |roots| */
	int d;  /* index of minimum entry in |roots| */
	int i,j;

	@<for |d|$\le$|subdepth| put pointers to matching depth |d| entries in |roots[d]|@>@;
	
	if(last==NULL) return false;
	
	if(depth == subdepth)
	{
		@<find the minimum at each depth $>$|subdepth| and put pointers in |roots|@>@;
	}
	
	@<find the absolute minimum of all in |roots|@>@;
	
	index = *last;
	q = (struct hlist_key *)list_entry(l->keys,index);
	for(i=subdepth;i<l->depth;i++) m[i] = q->k[i];
	
	if(p!=NULL) memcpy(p,list_entry(l->entries,index),l->sz);

	if(d >=	subdepth)
	{
		/* in this case we can use a faster method than the general case */
		@<delete minimum node@>@;
	}
	else
	{
		@<delete hlist entry pointed to by |last|@>@;
	}
@q+	if(!hlist_integrity(l)) abort_message("hlist internal");@>
	return true;
}

@ Like |hlist_sub_min| except it returns the maximum of the subtree
whose keys match |m| to the level |subdepth|.
It returns false if this subtree is empty.
@c
Boolean hlist_sub_max(hlist *l,SInt32 *m,void *p,int subdepth)
{
	struct hlist_key *q;
	SInt32 *last,index,*roots[4];
	int depth,d;
	int i,j;

	@<for |d|$\le$|subdepth| put pointers to matching depth |d| entries in |roots[d]|@>@;
	
	if(last==NULL) return false;
	
	if(depth == subdepth)
	{
		@<find the maximum at each depth $>$|subdepth| and put pointers in |roots|@>@;
	}
	
	@<find the absolute maximum of all in |roots|@>@;
	
	index = *last;
	q = (struct hlist_key *)list_entry(l->keys,index);
	for(i=subdepth;i<l->depth;i++) m[i] = q->k[i];
	
	if(p!=NULL) memcpy(p,list_entry(l->entries,index),l->sz);

	if(d >=	subdepth)
	{
		@<delete maximum node@>@;
	}
	else
	{
		@<delete hlist entry pointed to by |last|@>@;
	}
	
@q+	if(!hlist_integrity(l)) abort_message("hlist internal");@>
	return true;
}


@ Like |hlist_sub_min| except it returns any convenient entry of the subtree
whose keys match |m| to the level |subdepth|.
It returns false if this subtree is empty.
@c
Boolean hlist_sub_match(hlist *l,SInt32 *m,void *p,int subdepth)
{
	struct hlist_key *q;
	SInt32 *last,index,*roots[4];
	int depth,d;
	int i,j;

	@<for |d|$\le$|subdepth| put pointers to matching depth |d| entries in |roots[d]|@>@;
	if(last==NULL) return false;
	index = *last;
	q = (struct hlist_key *)list_entry(l->keys,index);
	for(i=subdepth;i<l->depth;i++) m[i] = q->k[i];
	
	if(p!=NULL) memcpy(p,list_entry(l->entries,index),l->sz);
	
	@<delete hlist entry pointed to by |last|@>@;
	
	
@q+	if(!hlist_integrity(l)) abort_message("hlist internal");@>
	return true;
}




@ @<find the minimum at each depth $>$|subdepth| and put pointers in |roots|@>=
for(;depth<l->depth;depth++)
{
	do
	{
		index = *last;
		q = (struct hlist_key *)list_entry(l->keys,index);
		if(q->low < 0) break;
		last = &(q->low);
	} while(true);
	roots[depth] = last;
	if(q->eq<0) break;
	last = &(q->eq);
}
roots[depth] = last;

@ @<find the maximum at each depth $>$|subdepth| and put pointers in |roots|@>=
for(;depth<l->depth;depth++)
{
	do
	{
		index = *last;
		q = (struct hlist_key *)list_entry(l->keys,index);
		if(q->high < 0) break;
		last = &(q->high);
	} while(true);
	roots[depth] = last;
	if(q->eq<0) break;
	last = &(q->eq);
}
roots[depth] = last;

@ @<find the absolute minimum of all in |roots|@>=
{
	struct hlist_key *r;

	d = depth;
	q = (struct hlist_key *)list_entry(l->keys,*last);
	
	for(i=d;i>0;)
	{
		i--;
		if(roots[i]==NULL) continue;
		if(*(roots[i])<0) continue;
		r = (struct hlist_key *)list_entry(l->keys,*(roots[i]));
		for(j=i+1;j<l->depth;j++)
		{
			if(r->k[j]>q->k[j]) break;
			else if(r->k[j]<q->k[j])
			{
				last = roots[i];
				q = r;
				d = i;
				break;
			}
		}
	}
}

@ @<find the absolute maximum of all in |roots|@>=
{
	struct hlist_key *r;

	d = depth;
	q = (struct hlist_key *)list_entry(l->keys,*last);
	
	for(i=d;i>0;)
	{
		i--;
		if(roots[i]==NULL) continue;
		if(*(roots[i])<0) continue;
		r = (struct hlist_key *)list_entry(l->keys,*(roots[i]));
		for(j=i+1;j<l->depth;j++)
		{
			if(r->k[j]<q->k[j]) break;
			else if(r->k[j]>q->k[j])
			{
				last = roots[i];
				q = r;
				d = i;
				break;
			}
		}
	}
}

@ @<delete minimum node@>=
{
	struct hlist_key *r;

	if(d==l->depth)
	{
		*last = q->eq;
	}
	else if(q->eq<0)
	{
		*last = q->high;
	}
	else
	{
		/* move a very deep entry to the place formerly occupied 
		   by the deleted entry */
@q+		if(d==depth) abort_message("Huh? delete min1");@>
		r = (struct hlist_key *)list_entry(l->keys,*(roots[depth]));
		*last = *(roots[depth]);
		if(depth<l->depth) *(roots[depth]) = r->high;
		else *(roots[depth]) = r->eq;
		r->high = q->high;
		r->eq = q->eq;
@q+		if(r->low >= 0) abort_message("Huh? delete min2");@>
	}
	q->low = l->free;
	l->free = index;
}

@ @<delete maximum node@>=
{
	struct hlist_key *r;

	if(d==l->depth)
	{
		*last = q->eq;
	}
	else if(q->eq<0)
	{
		*last = q->low;
	}
	else
	{
@q+		if(d==depth) abort_message("Huh? delete max1");@>
		r = (struct hlist_key *)list_entry(l->keys,*(roots[depth]));
		*last = *(roots[depth]);
		if(depth<l->depth) *(roots[depth]) = r->low;
		else *(roots[depth]) = r->eq;
		r->low = q->low;
		r->eq = q->eq;
@q+		if(r->high >= 0) abort_message("Huh? delete max2");@>
	}
	q->low = l->free;
	l->free = index;
}





@ Some matching entries will have depth $\le$ |subdepth|.
Here we find all those and put them on the list |roots|.
On exit, |depth| is the index in |roots| of the last valid entry in |roots|
and |*last| is the list index of that entry, or NULL
if there are no matching entries.

@<for |d|$\le$|subdepth| put pointers to matching depth |d| entries in |roots[d]|@>=
{
	if(subdepth>0) last = l->hash + ((m[0]>>l->scaling)%(l->hashsize));
	else if(!hlist_is_empty(l)) last = l->hash + l->first_nonempty;
	else return false;
	
	if(subdepth>l->depth) subdepth = l->depth;
	
	d = -1;
	 
	for(depth=0;depth<subdepth;depth++)
	{
		while(*last>=0)
		{
			index = *last;
			q = (struct hlist_key *)list_entry(l->keys,index);
			if (m[depth] < q->k[depth]) last = &(q->low);
			else if(m[depth] > q->k[depth]) last = &(q->high);
			else break;
		} 
		
		if(*last>=0) 
		{
			for(i=depth+1;i<subdepth;i++) 
			{
				if(m[i]!=q->k[i]) break;
			}
			if(i<subdepth) roots[depth] = NULL;
			else  
			{
				roots[depth] = last;
				d = depth;
			}
			if(q->eq<0) break;
			last = &(q->eq);
		}
		else break;
	}
	if(*last>=0 && depth==subdepth)
	{
		roots[depth] = last;
		d = depth;
	}
	else
	{
		depth = d;
		if(d>=0) last = roots[depth];
		else last = NULL;
	}
}


@ Add an entry |p| with keys |m| to the ordered list.
Return the index of the entry.
@c

SInt32 hlist_add(hlist *l,SInt32 *m,void *p)
{
	struct hlist_key *q,*r;
	SInt32 index,*last,lastsave;	
	int depth=0,i;

	
	@<point |last| to hash entry@>@;
	
	do
	{
		@<point |last| to the next deeper matching entry@>@;
		if(*last<0 || depth>=l->depth) break;
		last = &(q->eq);
		depth++;
	}
	while(true);
	
	lastsave = *last;
	
	@<copy |p| to |l|, set |index| to its index, and point |q| to it@>@;

	if(lastsave>=0) 
	{
		r = (struct hlist_key *)list_entry(l->keys,lastsave);
		q->eq = r->eq;
		r->eq = index;
	}
@q+	if(!hlist_integrity(l)) abort_message("hlist internal");@>
	return index;
}




@ This searches the ordered list |l| for an entry with key |m|.
If it finds one, it returns its index and sets |flag|.
If it doesn't find one, and |p| is not NULL,
it creates an entry with key |m|
and moves |*p| into that entry and resets |flag|.
@c

SInt32 hlist_find_add(hlist *l,SInt32 *m,void *p,Boolean *flag)
{
	struct hlist_key  *q;
	SInt32 index,*last;
	int depth=0,i;
	
	@<point |last| to hash entry@>@;
	
	@<point |last| to the next deeper matching entry@>@;
	
	if(*last >= 0)
	{
		if(flag!=NULL) *flag = true;
@q+		if(!hlist_integrity(l)) abort_message("hlist internal");@>
		return *last;
	}

	if(p!=NULL)
	{
		@<copy |p| to |l|, set |index| to its index, and point |q| to it@>@;
	}
	else index = -1;
	
	if(flag!=NULL) *flag = false;
@q+	if(!hlist_integrity(l)) abort_message("hlist internal");@>
	return index;
}


@ @<copy |p| to |l|, set |index| to its index, and point |q| to it@>=
{
	struct hlist_key rr;
	
	rr.high = rr.low = rr.eq = -1;
	for(i=0;i<l->depth;i++) rr.k[i] = m[i];
	
	if(l->free>=0)
	{
		index = l->free;
		if(*last<0) *last = index;
		memcpy(list_entry(l->entries,index),p,l->sz);
		q = (struct hlist_key *)list_entry(l->keys,index);
		l->free = q->low;
		memcpy(q,&rr,sizeof(struct hlist_key));
	}
	else
	{
		index = list_count((l)->entries);
		if(*last<0) *last = index;
		list_push(l->entries,p);
		q = (struct hlist_key *)list_push(l->keys,&rr);
	}
}

@ @<point |last| to hash entry@>=
{
	UInt16 he;
	
	he = (((UInt32)m[0])>>l->scaling)%(l->hashsize);
	if(he<l->first_nonempty) l->first_nonempty = he;
	
	last = l->hash + he;
}



@ Delete an entry from a hash list.
More precisely, it deletes an entry from the list with the same keys as
the |index|-th entry.
Since this is only used when there is a one-to-one correspondence
between keys and entries it amounts to the same thing,
but is slightly easier to do.
@c
void hlist_delete(hlist *l,SInt32 index)
{
	struct hlist_key  *q;
	SInt32 *last;
	int depth=0,i;
	SInt32 *m;
	
	q = (struct hlist_key *)list_entry(l->keys,index);
	m = q->k;
	@<point |last| to hash entry@>@;
	
	@<point |last| to the next deeper matching entry@>@;
	
	if(*last < 0) abort_message("attempt to delete nonexistent item");
	
	index = *last;
	
	@<delete hlist entry pointed to by |last|@>@;
@q+	if(!hlist_integrity(l)) abort_message("hlist internal");@>
}


@ Find entry on |l| with key |m|.  If found, 
copy it to |p| (if |p| is not NULL), delete the entry and return true.
Otherwise, return false.
@c
Boolean hlist_find_delete(hlist *l,SInt32 *m,void *p)
{
	struct hlist_key  *q;
	SInt32 *last,index;
	int depth=0,i;
	
	@<point |last| to hash entry@>@;
	
	@<point |last| to the next deeper matching entry@>@;
	
	if(*last < 0) return false;
	
	index = *last;
	
	if(p!=NULL) memcpy(p,list_entry(l->entries,index),l->sz);
	@<delete hlist entry pointed to by |last|@>@;
@q+	if(!hlist_integrity(l)) abort_message("hlist internal");@>
	return true;
}

@ Find entry on |l| with key |m|.  If found, 
copy it to |p|, delete it and return -1.
Otherwise, add it to the list (contents pointed to by |p|),
return its index.
@c
SInt32 hlist_add_delete(hlist *l,SInt32 *m,void *p)
{
	struct hlist_key  *q;
	SInt32 *last,index;
	int depth=0,i;

	@<point |last| to hash entry@>@;
	
	@<point |last| to the next deeper matching entry@>@;
	
	index = *last;
	
	if(index>=0)
	{
		memcpy(p,list_entry(l->entries,index),l->sz);
		@<delete hlist entry pointed to by |last|@>@;
@q+		if(!hlist_integrity(l)) abort_message("hlist internal");@>
		return -1;
	}
	
	@<copy |p| to |l|, set |index| to its index, and point |q| to it@>@;
@q+	if(!hlist_integrity(l)) abort_message("hlist internal");@>
	return index;
}

@ On entry, it is assumed that |depth| is the depth of the entry
with index |*last|, and |m| is a list of keys.
Moreover, the i-th key of the entry with index |*last| matches
|m[i]| for all $i<depth$.
It will then find, if possible, an entry of the same depth or greater
with all keys matching |m|.
It will choose such an entry with minimal depth.
On exit, |last| will be the field pointing to this entry.
If such an entry does not exist, |last| will be the field
which would point to such an entry were it to exist
(and |*last|$<0$, indicating that nothing is being pointed to).


@<point |last| to the next deeper matching entry@>=
{	
	for(;depth<l->depth;depth++)
	{
		while(*last>=0)
		{
			index = *last;
			q = (struct hlist_key *)list_entry(l->keys,index);
			if (m[depth] < q->k[depth]) last = &(q->low);
			else if(m[depth] > q->k[depth]) last = &(q->high);
			else break;
		} 
		if(*last>=0) 
		{
			for(i=depth+1;i<l->depth;i++) if(m[i]!=q->k[i]) break;
			if(i>=l->depth) break;
			last = &(q->eq);
		}
		else break;
	}
}

@ @<delete hlist entry pointed to by |last|@>=
{
	struct hlist_key  *r;
	SInt32 rindex;
	SInt32 *lastr;
	
	index = *last;
	q = (struct hlist_key *)list_entry(l->keys,index);
	if(depth==l->depth) {*last = q->eq; abort_message("Huh? delete");}
	else if(q->eq>=0)
	{
		r = (struct hlist_key *)list_entry(l->keys,q->eq);
		rindex = q->eq;
		lastr = &q->eq;
		depth++;
		if(depth < l->depth)
		{
			@<let |r| be a detached descendent leaf of |r|@>@;
		}
		*last = rindex;
		r->high = q->high;
		r->low = q->low;
		r->eq = q->eq;
	}
	else if(q->low<0) *last = q->high;
	else if(q->high<0) *last = q->low;
	else
	{
		@<move |q| to a leaf node and delete it@>@;
	}
	q->low = l->free;
	l->free = index;
}

@ @<move |q| to a leaf node and delete it@>=
{
	
	while(q->low >=0 && q->high >=0)
	{
		if(random_bit)
		{
			*last = q->high;
			r = (struct hlist_key *)list_entry(l->keys,*last);
			q->high = r->low;
			last = &r->low;
		}
		else
		{
			*last = q->low;
			r = (struct hlist_key *)list_entry(l->keys,*last);
			q->low = r->high;
			last = &r->high;
		}
	}
	
	if(q->low<0) *last = q->high;
	else *last = q->low;
}



@ @<let |r| be a detached descendent leaf of |r|@>=
{
	
	for(;depth<l->depth;depth++) 
	{
		while(r->eq<0 && r->low>=0 && r->high>=0)
		{
			if(random_bit) lastr = &r->high;
			else lastr = &r->low;
			rindex = *lastr;
			r = (struct hlist_key *)list_entry(l->keys,rindex);
		} 
		if(r->eq<0) break;
		lastr = &r->eq;
		rindex = r->eq;
		r = (struct hlist_key *)list_entry(l->keys,rindex);
	}
	if(depth >= l->depth) *lastr = r->eq;
	else if(r->low<0) *lastr = r->high;
	else if(r->high<0) *lastr = r->low;
	else abort_message("Huh? delete detach");
}



@ Check balance of hlist.  Compare with optimum balanced binary tree
(we could do better).

@c
UInt32 optimum_balance(UInt32 n)
{
	UInt32 m=0xffffffff;
	int i;
	
	for(i=0;i<32;i++,m<<=1) if((m&n)==0) break;
	
	return 1+(n+1)*i+m;
}

UInt32  hlist_balance_check_x(hlist *l,SInt32 index,UInt32 *paths)
{
	UInt32 n,p,nlo,plo,nhi,phi,ne,pe;
	struct hlist_key  *q;
	
	if(index<0)
	{
		*paths=0;
		return 0;
	}
	
	q = (struct hlist_key *)list_entry(l->keys,index);
	
	nlo = hlist_balance_check_x(l,q->low,&plo);
	nhi = hlist_balance_check_x(l,q->high,&phi);
	ne = hlist_balance_check_x(l,q->eq,&pe);
	
	*paths = plo+phi+pe+nlo+nhi+ne+1;
	
	return nlo+nhi+ne+1;
}

float hlist_balance_check(hlist *l)
{
	UInt16 i;
	UInt32 total_nodes=0,total_path_length=0,total_optimum=0;
	UInt32 nodes,path_length;
	float x,y;

	for(i=l->first_nonempty;i<l->hashsize;i++)
	{
		if(l->hash[i]>=0)
		{
			nodes = hlist_balance_check_x(l,l->hash[i],&path_length);
			total_nodes+=nodes;
			total_path_length+=path_length;
			total_optimum+=optimum_balance(nodes);
		}
	}
	
	x = total_path_length;
	y = total_optimum;
	
	return (x-y)/(y+.00001);
}

@* Index.