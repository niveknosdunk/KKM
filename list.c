#define random_bit ((((random_bit_i++) &2047) ==0) ? \
(random_bit_n= random() ) &1: \
((random_bit_n>>(random_bit_i&31) ) &1) )  \

/*3:*/


#include "list.h"
void list_initialize(list**l,UInt32 sz)
{
*l= (list*)malloc(sizeof(list));
if((*l)==NULL)abort_message("Out of memory");
(*l)->size= sz;
(*l)->padded_size= (sz+sizeof(SInt32)-1)/sizeof(SInt32);
(*l)->length= 0;
(*l)->body= NULL;
(*l)->body_length= 0;
}


/*:3*//*4:*/



void list_abandon(list**l)
{
if((*l)->body_length> 0)free((*l)->body);
free(*l);
*l= NULL;
}



/*:4*//*5:*/



void*list_push(list*l,void*q)
{
SInt32*p;
if(l->body_length<=l->length)
{
l->body_length= l->length+1000;
l->body= (SInt32*)realloc(l->body,l->body_length*l->padded_size*sizeof(SInt32));
if(l->body==NULL)abort_message("out of memory");
}

p= list_entry(l,l->length);

memcpy(p,q,l->size);
l->length++;
return p;
}

void plist_push(list*l,void*q)
{
void*p= q;
list_push(l,&p);
}


/*:5*//*6:*/



void list_pop(list*l,void*q)
{
SInt32*p;

if(l->length==0)abort_message("pop from empty list");

l->length--;
p= list_entry(l,l->length);
memcpy(q,p,l->size);
}

void*plist_pop(list*l)
{
void*p;

list_pop(l,&p);
return p;
}


/*:6*//*8:*/


void*list_read(list*l)
{
void*p,*pp;

if(l->read_index>=l->length)
{

l->length= l->read_index= l->read_delete_index;
return(NULL);
}

pp= list_entry(l,l->read_delete_index);


if(l->read_index!=l->read_delete_index)
{
p= list_entry(l,l->read_index);
memcpy(pp,p,l->size);
}

l->read_delete_index++;
l->read_index++;
return pp;
}

void*plist_read(list*l)
{
void**r;
r= list_read(l);
if(r!=NULL)return*r;
else return NULL;
}

/*:8*//*9:*/


void*list_read_insert(list*l,void*p)
{
if(l->read_index==l->read_delete_index)
list_push(l,p);
else
{
memcpy(list_entry(l,l->read_delete_index),p,l->size);
l->read_delete_index++;
}
}


/*:9*//*11:*/


void fifo_initialize(fifo**l,UInt32 sz)
{
SInt32**p;

*l= (fifo*)malloc(sizeof(fifo));
if((*l)==NULL)abort_message("Out of memory");
(*l)->size= sz;
(*l)->padded_size= (sz+sizeof(SInt32*)-1)/sizeof(SInt32*);
(*l)->first_index= 0;
(*l)->last_index= 0;
p= (SInt32**)malloc((1+1000*(*l)->padded_size)*sizeof(SInt32*));
if(p==NULL)abort_message("out of memory");
*p++= NULL;
(*l)->first= p;
(*l)->last= p;
(*l)->num_segments= 1;
}


/*:11*//*12:*/



void fifo_abandon(fifo**l)
{
SInt32**p;

while((*l)->first!=NULL)
{
p= (*l)->first-1;
(*l)->first= (SInt32**)*p;
free(p);
}
free(*l);
*l= NULL;
}



/*:12*//*13:*/



void*fifo_push(fifo*l,void*q)
{
SInt32**p;
if(l->last_index==1000)
{
p= (SInt32**)malloc((1+1000*l->padded_size)*sizeof(SInt32*));
if(p==NULL)abort_message("out of memory");
*p++= NULL;
*(l->last-1)= (SInt32*)p;
l->last= p;
l->last_index= 0;
l->num_segments++;
}

p= l->last+l->last_index*l->padded_size;

memcpy(p,q,l->size);
l->last_index++;
return p;
}



/*:13*//*14:*/



void fifo_pop(fifo*l,void*q)
{
SInt32**p;

if(fifo_is_empty(l))abort_message("pop from empty list");

if(l->first_index==1000)
{
p= l->first-1;
l->first= (SInt32**)(*p);
free(p);
l->first_index= 0;
l->num_segments--;
if(l->num_segments==0)abort_message("fifo error");
}

p= l->first+l->first_index*l->padded_size;
l->first_index++;
memcpy(q,p,l->size);
if(fifo_is_empty(l))l->last_index= l->first_index= 0;
}



/*:14*//*17:*/


UInt32 random_bit_n,random_bit_i= 2048;

/*:17*//*18:*/


void hlist_initialize(hlist**l,UInt32 sz,UInt16 hashsize,UInt16 depth,UInt16 scale)
{
UInt16 i;

*l= (hlist*)malloc(sizeof(hlist)+hashsize*sizeof(SInt32));
if((*l)==NULL)abort_message("Out of memory");
list_initialize(&((*l)->keys),sizeof(struct hlist_key));
list_initialize(&((*l)->entries),sz);
(*l)->free= -1;
(*l)->sz= sz;
(*l)->hashsize= hashsize;
(*l)->depth= depth;
(*l)->scaling= scale;
(*l)->first_nonempty= hashsize;
for(i= 0;i<hashsize;i++)(*l)->hash[i]= -1;
}

/*:18*//*19:*/


void hlist_abandon(hlist**l)
{
list_abandon(&((*l)->keys));
list_abandon(&((*l)->entries));
free(*l);
*l= NULL;
}

/*:19*//*20:*/


void hlist_clear(hlist*l)
{
UInt16 i;
list_clear(l->keys);
list_clear(l->entries);
l->free= -1;
for(i= l->first_nonempty;i<l->hashsize;i++)l->hash[i]= -1;
}

/*:20*//*21:*/


Boolean hlist_is_empty(hlist*l)
{
SInt32 min_index;

for(min_index= l->first_nonempty;min_index<l->hashsize;min_index++)
if(l->hash[min_index]>=0)break;
l->first_nonempty= min_index;
return min_index>=l->hashsize;
}


/*:21*//*22:*/


Boolean hlist_sub_min(hlist*l,SInt32*m,void*p,int subdepth)
{
struct hlist_key*q;
SInt32*last,index,*roots[4];
int depth;
int d;
int i,j;

/*31:*/


{
if(subdepth> 0)last= l->hash+((m[0]>>l->scaling)%(l->hashsize));
else if(!hlist_is_empty(l))last= l->hash+l->first_nonempty;
else return false;

if(subdepth> l->depth)subdepth= l->depth;

d= -1;

for(depth= 0;depth<subdepth;depth++)
{
while(*last>=0)
{
index= *last;
q= (struct hlist_key*)list_entry(l->keys,index);
if(m[depth]<q->k[depth])last= &(q->low);
else if(m[depth]> q->k[depth])last= &(q->high);
else break;
}

if(*last>=0)
{
for(i= depth+1;i<subdepth;i++)
{
if(m[i]!=q->k[i])break;
}
if(i<subdepth)roots[depth]= NULL;
else
{
roots[depth]= last;
d= depth;
}
if(q->eq<0)break;
last= &(q->eq);
}
else break;
}
if(*last>=0&&depth==subdepth)
{
roots[depth]= last;
d= depth;
}
else
{
depth= d;
if(d>=0)last= roots[depth];
else last= NULL;
}
}


/*:31*/



if(last==NULL)return false;

if(depth==subdepth)
{
/*25:*/


for(;depth<l->depth;depth++)
{
do
{
index= *last;
q= (struct hlist_key*)list_entry(l->keys,index);
if(q->low<0)break;
last= &(q->low);
}while(true);
roots[depth]= last;
if(q->eq<0)break;
last= &(q->eq);
}
roots[depth]= last;

/*:25*/


}

/*27:*/


{
struct hlist_key*r;

d= depth;
q= (struct hlist_key*)list_entry(l->keys,*last);

for(i= d;i> 0;)
{
i--;
if(roots[i]==NULL)continue;
if(*(roots[i])<0)continue;
r= (struct hlist_key*)list_entry(l->keys,*(roots[i]));
for(j= i+1;j<l->depth;j++)
{
if(r->k[j]> q->k[j])break;
else if(r->k[j]<q->k[j])
{
last= roots[i];
q= r;
d= i;
break;
}
}
}
}

/*:27*/



index= *last;
q= (struct hlist_key*)list_entry(l->keys,index);
for(i= subdepth;i<l->depth;i++)m[i]= q->k[i];

if(p!=NULL)memcpy(p,list_entry(l->entries,index),l->sz);

if(d>=subdepth)
{

/*29:*/


{
struct hlist_key*r;

if(d==l->depth)
{
*last= q->eq;
}
else if(q->eq<0)
{
*last= q->high;
}
else
{


#ifdef debug
if(d==depth)abort_message("Huh? delete min1");
#endif
r= (struct hlist_key*)list_entry(l->keys,*(roots[depth]));
*last= *(roots[depth]);
if(depth<l->depth)*(roots[depth])= r->high;
else*(roots[depth])= r->eq;
r->high= q->high;
r->eq= q->eq;
#ifdef debug
if(r->low>=0)abort_message("Huh? delete min2");
#endif
}
q->low= l->free;
l->free= index;
}

/*:29*/


}
else
{
/*40:*/


{
struct hlist_key*r;
SInt32 rindex;
SInt32*lastr;

index= *last;
q= (struct hlist_key*)list_entry(l->keys,index);
if(depth==l->depth){*last= q->eq;abort_message("Huh? delete");}
else if(q->eq>=0)
{
r= (struct hlist_key*)list_entry(l->keys,q->eq);
rindex= q->eq;
lastr= &q->eq;
depth++;
if(depth<l->depth)
{
/*42:*/


{

for(;depth<l->depth;depth++)
{
while(r->eq<0&&r->low>=0&&r->high>=0)
{
if(random_bit)lastr= &r->high;
else lastr= &r->low;
rindex= *lastr;
r= (struct hlist_key*)list_entry(l->keys,rindex);
}
if(r->eq<0)break;
lastr= &r->eq;
rindex= r->eq;
r= (struct hlist_key*)list_entry(l->keys,rindex);
}
if(depth>=l->depth)*lastr= r->eq;
else if(r->low<0)*lastr= r->high;
else if(r->high<0)*lastr= r->low;
else abort_message("Huh? delete detach");
}



/*:42*/


}
*last= rindex;
r->high= q->high;
r->low= q->low;
r->eq= q->eq;
}
else if(q->low<0)*last= q->high;
else if(q->high<0)*last= q->low;
else
{
/*41:*/


{

while(q->low>=0&&q->high>=0)
{
if(random_bit)
{
*last= q->high;
r= (struct hlist_key*)list_entry(l->keys,*last);
q->high= r->low;
last= &r->low;
}
else
{
*last= q->low;
r= (struct hlist_key*)list_entry(l->keys,*last);
q->low= r->high;
last= &r->high;
}
}

if(q->low<0)*last= q->high;
else*last= q->low;
}



/*:41*/


}
q->low= l->free;
l->free= index;
}

/*:40*/


}
#ifdef debug
if(!hlist_integrity(l))abort_message("hlist internal");
#endif
return true;
}

/*:22*//*23:*/


Boolean hlist_sub_max(hlist*l,SInt32*m,void*p,int subdepth)
{
struct hlist_key*q;
SInt32*last,index,*roots[4];
int depth,d;
int i,j;

/*31:*/


{
if(subdepth> 0)last= l->hash+((m[0]>>l->scaling)%(l->hashsize));
else if(!hlist_is_empty(l))last= l->hash+l->first_nonempty;
else return false;

if(subdepth> l->depth)subdepth= l->depth;

d= -1;

for(depth= 0;depth<subdepth;depth++)
{
while(*last>=0)
{
index= *last;
q= (struct hlist_key*)list_entry(l->keys,index);
if(m[depth]<q->k[depth])last= &(q->low);
else if(m[depth]> q->k[depth])last= &(q->high);
else break;
}

if(*last>=0)
{
for(i= depth+1;i<subdepth;i++)
{
if(m[i]!=q->k[i])break;
}
if(i<subdepth)roots[depth]= NULL;
else
{
roots[depth]= last;
d= depth;
}
if(q->eq<0)break;
last= &(q->eq);
}
else break;
}
if(*last>=0&&depth==subdepth)
{
roots[depth]= last;
d= depth;
}
else
{
depth= d;
if(d>=0)last= roots[depth];
else last= NULL;
}
}


/*:31*/



if(last==NULL)return false;

if(depth==subdepth)
{
/*26:*/


for(;depth<l->depth;depth++)
{
do
{
index= *last;
q= (struct hlist_key*)list_entry(l->keys,index);
if(q->high<0)break;
last= &(q->high);
}while(true);
roots[depth]= last;
if(q->eq<0)break;
last= &(q->eq);
}
roots[depth]= last;

/*:26*/


}

/*28:*/


{
struct hlist_key*r;

d= depth;
q= (struct hlist_key*)list_entry(l->keys,*last);

for(i= d;i> 0;)
{
i--;
if(roots[i]==NULL)continue;
if(*(roots[i])<0)continue;
r= (struct hlist_key*)list_entry(l->keys,*(roots[i]));
for(j= i+1;j<l->depth;j++)
{
if(r->k[j]<q->k[j])break;
else if(r->k[j]> q->k[j])
{
last= roots[i];
q= r;
d= i;
break;
}
}
}
}

/*:28*/



index= *last;
q= (struct hlist_key*)list_entry(l->keys,index);
for(i= subdepth;i<l->depth;i++)m[i]= q->k[i];

if(p!=NULL)memcpy(p,list_entry(l->entries,index),l->sz);

if(d>=subdepth)
{
/*30:*/


{
struct hlist_key*r;

if(d==l->depth)
{
*last= q->eq;
}
else if(q->eq<0)
{
*last= q->low;
}
else
{
#ifdef debug
if(d==depth)abort_message("Huh? delete max1");
#endif
r= (struct hlist_key*)list_entry(l->keys,*(roots[depth]));
*last= *(roots[depth]);
if(depth<l->depth)*(roots[depth])= r->low;
else*(roots[depth])= r->eq;
r->low= q->low;
r->eq= q->eq;
#ifdef debug
if(r->high>=0)abort_message("Huh? delete max2");
#endif
}
q->low= l->free;
l->free= index;
}





/*:30*/


}
else
{
/*40:*/


{
struct hlist_key*r;
SInt32 rindex;
SInt32*lastr;

index= *last;
q= (struct hlist_key*)list_entry(l->keys,index);
if(depth==l->depth){*last= q->eq;abort_message("Huh? delete");}
else if(q->eq>=0)
{
r= (struct hlist_key*)list_entry(l->keys,q->eq);
rindex= q->eq;
lastr= &q->eq;
depth++;
if(depth<l->depth)
{
/*42:*/


{

for(;depth<l->depth;depth++)
{
while(r->eq<0&&r->low>=0&&r->high>=0)
{
if(random_bit)lastr= &r->high;
else lastr= &r->low;
rindex= *lastr;
r= (struct hlist_key*)list_entry(l->keys,rindex);
}
if(r->eq<0)break;
lastr= &r->eq;
rindex= r->eq;
r= (struct hlist_key*)list_entry(l->keys,rindex);
}
if(depth>=l->depth)*lastr= r->eq;
else if(r->low<0)*lastr= r->high;
else if(r->high<0)*lastr= r->low;
else abort_message("Huh? delete detach");
}



/*:42*/


}
*last= rindex;
r->high= q->high;
r->low= q->low;
r->eq= q->eq;
}
else if(q->low<0)*last= q->high;
else if(q->high<0)*last= q->low;
else
{
/*41:*/


{

while(q->low>=0&&q->high>=0)
{
if(random_bit)
{
*last= q->high;
r= (struct hlist_key*)list_entry(l->keys,*last);
q->high= r->low;
last= &r->low;
}
else
{
*last= q->low;
r= (struct hlist_key*)list_entry(l->keys,*last);
q->low= r->high;
last= &r->high;
}
}

if(q->low<0)*last= q->high;
else*last= q->low;
}



/*:41*/


}
q->low= l->free;
l->free= index;
}

/*:40*/


}

#ifdef debug
if(!hlist_integrity(l))abort_message("hlist internal");
#endif
return true;
}


/*:23*//*24:*/


Boolean hlist_sub_match(hlist*l,SInt32*m,void*p,int subdepth)
{
struct hlist_key*q;
SInt32*last,index,*roots[4];
int depth,d;
int i,j;

/*31:*/


{
if(subdepth> 0)last= l->hash+((m[0]>>l->scaling)%(l->hashsize));
else if(!hlist_is_empty(l))last= l->hash+l->first_nonempty;
else return false;

if(subdepth> l->depth)subdepth= l->depth;

d= -1;

for(depth= 0;depth<subdepth;depth++)
{
while(*last>=0)
{
index= *last;
q= (struct hlist_key*)list_entry(l->keys,index);
if(m[depth]<q->k[depth])last= &(q->low);
else if(m[depth]> q->k[depth])last= &(q->high);
else break;
}

if(*last>=0)
{
for(i= depth+1;i<subdepth;i++)
{
if(m[i]!=q->k[i])break;
}
if(i<subdepth)roots[depth]= NULL;
else
{
roots[depth]= last;
d= depth;
}
if(q->eq<0)break;
last= &(q->eq);
}
else break;
}
if(*last>=0&&depth==subdepth)
{
roots[depth]= last;
d= depth;
}
else
{
depth= d;
if(d>=0)last= roots[depth];
else last= NULL;
}
}


/*:31*/


if(last==NULL)return false;
index= *last;
q= (struct hlist_key*)list_entry(l->keys,index);
for(i= subdepth;i<l->depth;i++)m[i]= q->k[i];

if(p!=NULL)memcpy(p,list_entry(l->entries,index),l->sz);

/*40:*/


{
struct hlist_key*r;
SInt32 rindex;
SInt32*lastr;

index= *last;
q= (struct hlist_key*)list_entry(l->keys,index);
if(depth==l->depth){*last= q->eq;abort_message("Huh? delete");}
else if(q->eq>=0)
{
r= (struct hlist_key*)list_entry(l->keys,q->eq);
rindex= q->eq;
lastr= &q->eq;
depth++;
if(depth<l->depth)
{
/*42:*/


{

for(;depth<l->depth;depth++)
{
while(r->eq<0&&r->low>=0&&r->high>=0)
{
if(random_bit)lastr= &r->high;
else lastr= &r->low;
rindex= *lastr;
r= (struct hlist_key*)list_entry(l->keys,rindex);
}
if(r->eq<0)break;
lastr= &r->eq;
rindex= r->eq;
r= (struct hlist_key*)list_entry(l->keys,rindex);
}
if(depth>=l->depth)*lastr= r->eq;
else if(r->low<0)*lastr= r->high;
else if(r->high<0)*lastr= r->low;
else abort_message("Huh? delete detach");
}



/*:42*/


}
*last= rindex;
r->high= q->high;
r->low= q->low;
r->eq= q->eq;
}
else if(q->low<0)*last= q->high;
else if(q->high<0)*last= q->low;
else
{
/*41:*/


{

while(q->low>=0&&q->high>=0)
{
if(random_bit)
{
*last= q->high;
r= (struct hlist_key*)list_entry(l->keys,*last);
q->high= r->low;
last= &r->low;
}
else
{
*last= q->low;
r= (struct hlist_key*)list_entry(l->keys,*last);
q->low= r->high;
last= &r->high;
}
}

if(q->low<0)*last= q->high;
else*last= q->low;
}



/*:41*/


}
q->low= l->free;
l->free= index;
}

/*:40*/




#ifdef debug
if(!hlist_integrity(l))abort_message("hlist internal");
#endif
return true;
}




/*:24*//*32:*/



SInt32 hlist_add(hlist*l,SInt32*m,void*p)
{
struct hlist_key*q,*r;
SInt32 index,*last,lastsave;
int depth= 0,i;


/*35:*/


{
UInt16 he;

he= (((UInt32)m[0])>>l->scaling)%(l->hashsize);
if(he<l->first_nonempty)l->first_nonempty= he;

last= l->hash+he;
}



/*:35*/



do
{
/*39:*/


{
for(;depth<l->depth;depth++)
{
while(*last>=0)
{
index= *last;
q= (struct hlist_key*)list_entry(l->keys,index);
if(m[depth]<q->k[depth])last= &(q->low);
else if(m[depth]> q->k[depth])last= &(q->high);
else break;
}
if(*last>=0)
{
for(i= depth+1;i<l->depth;i++)if(m[i]!=q->k[i])break;
if(i>=l->depth)break;
last= &(q->eq);
}
else break;
}
}

/*:39*/


if(*last<0||depth>=l->depth)break;
last= &(q->eq);
depth++;
}
while(true);

lastsave= *last;

/*34:*/


{
struct hlist_key rr;

rr.high= rr.low= rr.eq= -1;
for(i= 0;i<l->depth;i++)rr.k[i]= m[i];

if(l->free>=0)
{
index= l->free;
if(*last<0)*last= index;
memcpy(list_entry(l->entries,index),p,l->sz);
q= (struct hlist_key*)list_entry(l->keys,index);
l->free= q->low;
memcpy(q,&rr,sizeof(struct hlist_key));
}
else
{
index= list_count((l)->entries);
if(*last<0)*last= index;
list_push(l->entries,p);
q= (struct hlist_key*)list_push(l->keys,&rr);
}
}

/*:34*/



if(lastsave>=0)
{
r= (struct hlist_key*)list_entry(l->keys,lastsave);
q->eq= r->eq;
r->eq= index;
}
#ifdef debug
if(!hlist_integrity(l))abort_message("hlist internal");
#endif
return index;
}




/*:32*//*33:*/



SInt32 hlist_find_add(hlist*l,SInt32*m,void*p,Boolean*flag)
{
struct hlist_key*q;
SInt32 index,*last;
int depth= 0,i;

/*35:*/


{
UInt16 he;

he= (((UInt32)m[0])>>l->scaling)%(l->hashsize);
if(he<l->first_nonempty)l->first_nonempty= he;

last= l->hash+he;
}



/*:35*/



/*39:*/


{
for(;depth<l->depth;depth++)
{
while(*last>=0)
{
index= *last;
q= (struct hlist_key*)list_entry(l->keys,index);
if(m[depth]<q->k[depth])last= &(q->low);
else if(m[depth]> q->k[depth])last= &(q->high);
else break;
}
if(*last>=0)
{
for(i= depth+1;i<l->depth;i++)if(m[i]!=q->k[i])break;
if(i>=l->depth)break;
last= &(q->eq);
}
else break;
}
}

/*:39*/



if(*last>=0)
{
if(flag!=NULL)*flag= true;
#ifdef debug
if(!hlist_integrity(l))abort_message("hlist internal");
#endif
return*last;
}

if(p!=NULL)
{
/*34:*/


{
struct hlist_key rr;

rr.high= rr.low= rr.eq= -1;
for(i= 0;i<l->depth;i++)rr.k[i]= m[i];

if(l->free>=0)
{
index= l->free;
if(*last<0)*last= index;
memcpy(list_entry(l->entries,index),p,l->sz);
q= (struct hlist_key*)list_entry(l->keys,index);
l->free= q->low;
memcpy(q,&rr,sizeof(struct hlist_key));
}
else
{
index= list_count((l)->entries);
if(*last<0)*last= index;
list_push(l->entries,p);
q= (struct hlist_key*)list_push(l->keys,&rr);
}
}

/*:34*/


}
else index= -1;

if(flag!=NULL)*flag= false;
#ifdef debug
if(!hlist_integrity(l))abort_message("hlist internal");
#endif
return index;
}


/*:33*//*36:*/


void hlist_delete(hlist*l,SInt32 index)
{
struct hlist_key*q;
SInt32*last;
int depth= 0,i;
SInt32*m;

q= (struct hlist_key*)list_entry(l->keys,index);
m= q->k;
/*35:*/


{
UInt16 he;

he= (((UInt32)m[0])>>l->scaling)%(l->hashsize);
if(he<l->first_nonempty)l->first_nonempty= he;

last= l->hash+he;
}



/*:35*/



/*39:*/


{
for(;depth<l->depth;depth++)
{
while(*last>=0)
{
index= *last;
q= (struct hlist_key*)list_entry(l->keys,index);
if(m[depth]<q->k[depth])last= &(q->low);
else if(m[depth]> q->k[depth])last= &(q->high);
else break;
}
if(*last>=0)
{
for(i= depth+1;i<l->depth;i++)if(m[i]!=q->k[i])break;
if(i>=l->depth)break;
last= &(q->eq);
}
else break;
}
}

/*:39*/



if(*last<0)abort_message("attempt to delete nonexistent item");

index= *last;

/*40:*/


{
struct hlist_key*r;
SInt32 rindex;
SInt32*lastr;

index= *last;
q= (struct hlist_key*)list_entry(l->keys,index);
if(depth==l->depth){*last= q->eq;abort_message("Huh? delete");}
else if(q->eq>=0)
{
r= (struct hlist_key*)list_entry(l->keys,q->eq);
rindex= q->eq;
lastr= &q->eq;
depth++;
if(depth<l->depth)
{
/*42:*/


{

for(;depth<l->depth;depth++)
{
while(r->eq<0&&r->low>=0&&r->high>=0)
{
if(random_bit)lastr= &r->high;
else lastr= &r->low;
rindex= *lastr;
r= (struct hlist_key*)list_entry(l->keys,rindex);
}
if(r->eq<0)break;
lastr= &r->eq;
rindex= r->eq;
r= (struct hlist_key*)list_entry(l->keys,rindex);
}
if(depth>=l->depth)*lastr= r->eq;
else if(r->low<0)*lastr= r->high;
else if(r->high<0)*lastr= r->low;
else abort_message("Huh? delete detach");
}



/*:42*/


}
*last= rindex;
r->high= q->high;
r->low= q->low;
r->eq= q->eq;
}
else if(q->low<0)*last= q->high;
else if(q->high<0)*last= q->low;
else
{
/*41:*/


{

while(q->low>=0&&q->high>=0)
{
if(random_bit)
{
*last= q->high;
r= (struct hlist_key*)list_entry(l->keys,*last);
q->high= r->low;
last= &r->low;
}
else
{
*last= q->low;
r= (struct hlist_key*)list_entry(l->keys,*last);
q->low= r->high;
last= &r->high;
}
}

if(q->low<0)*last= q->high;
else*last= q->low;
}



/*:41*/


}
q->low= l->free;
l->free= index;
}

/*:40*/


#ifdef debug
if(!hlist_integrity(l))abort_message("hlist internal");
#endif
}


/*:36*//*37:*/


Boolean hlist_find_delete(hlist*l,SInt32*m,void*p)
{
struct hlist_key*q;
SInt32*last,index;
int depth= 0,i;

/*35:*/


{
UInt16 he;

he= (((UInt32)m[0])>>l->scaling)%(l->hashsize);
if(he<l->first_nonempty)l->first_nonempty= he;

last= l->hash+he;
}



/*:35*/



/*39:*/


{
for(;depth<l->depth;depth++)
{
while(*last>=0)
{
index= *last;
q= (struct hlist_key*)list_entry(l->keys,index);
if(m[depth]<q->k[depth])last= &(q->low);
else if(m[depth]> q->k[depth])last= &(q->high);
else break;
}
if(*last>=0)
{
for(i= depth+1;i<l->depth;i++)if(m[i]!=q->k[i])break;
if(i>=l->depth)break;
last= &(q->eq);
}
else break;
}
}

/*:39*/



if(*last<0)return false;

index= *last;

if(p!=NULL)memcpy(p,list_entry(l->entries,index),l->sz);
/*40:*/


{
struct hlist_key*r;
SInt32 rindex;
SInt32*lastr;

index= *last;
q= (struct hlist_key*)list_entry(l->keys,index);
if(depth==l->depth){*last= q->eq;abort_message("Huh? delete");}
else if(q->eq>=0)
{
r= (struct hlist_key*)list_entry(l->keys,q->eq);
rindex= q->eq;
lastr= &q->eq;
depth++;
if(depth<l->depth)
{
/*42:*/


{

for(;depth<l->depth;depth++)
{
while(r->eq<0&&r->low>=0&&r->high>=0)
{
if(random_bit)lastr= &r->high;
else lastr= &r->low;
rindex= *lastr;
r= (struct hlist_key*)list_entry(l->keys,rindex);
}
if(r->eq<0)break;
lastr= &r->eq;
rindex= r->eq;
r= (struct hlist_key*)list_entry(l->keys,rindex);
}
if(depth>=l->depth)*lastr= r->eq;
else if(r->low<0)*lastr= r->high;
else if(r->high<0)*lastr= r->low;
else abort_message("Huh? delete detach");
}



/*:42*/


}
*last= rindex;
r->high= q->high;
r->low= q->low;
r->eq= q->eq;
}
else if(q->low<0)*last= q->high;
else if(q->high<0)*last= q->low;
else
{
/*41:*/


{

while(q->low>=0&&q->high>=0)
{
if(random_bit)
{
*last= q->high;
r= (struct hlist_key*)list_entry(l->keys,*last);
q->high= r->low;
last= &r->low;
}
else
{
*last= q->low;
r= (struct hlist_key*)list_entry(l->keys,*last);
q->low= r->high;
last= &r->high;
}
}

if(q->low<0)*last= q->high;
else*last= q->low;
}



/*:41*/


}
q->low= l->free;
l->free= index;
}

/*:40*/


#ifdef debug
if(!hlist_integrity(l))abort_message("hlist internal");
#endif
return true;
}

/*:37*//*38:*/


SInt32 hlist_add_delete(hlist*l,SInt32*m,void*p)
{
struct hlist_key*q;
SInt32*last,index;
int depth= 0,i;

/*35:*/


{
UInt16 he;

he= (((UInt32)m[0])>>l->scaling)%(l->hashsize);
if(he<l->first_nonempty)l->first_nonempty= he;

last= l->hash+he;
}



/*:35*/



/*39:*/


{
for(;depth<l->depth;depth++)
{
while(*last>=0)
{
index= *last;
q= (struct hlist_key*)list_entry(l->keys,index);
if(m[depth]<q->k[depth])last= &(q->low);
else if(m[depth]> q->k[depth])last= &(q->high);
else break;
}
if(*last>=0)
{
for(i= depth+1;i<l->depth;i++)if(m[i]!=q->k[i])break;
if(i>=l->depth)break;
last= &(q->eq);
}
else break;
}
}

/*:39*/



index= *last;

if(index>=0)
{
memcpy(p,list_entry(l->entries,index),l->sz);
/*40:*/


{
struct hlist_key*r;
SInt32 rindex;
SInt32*lastr;

index= *last;
q= (struct hlist_key*)list_entry(l->keys,index);
if(depth==l->depth){*last= q->eq;abort_message("Huh? delete");}
else if(q->eq>=0)
{
r= (struct hlist_key*)list_entry(l->keys,q->eq);
rindex= q->eq;
lastr= &q->eq;
depth++;
if(depth<l->depth)
{
/*42:*/


{

for(;depth<l->depth;depth++)
{
while(r->eq<0&&r->low>=0&&r->high>=0)
{
if(random_bit)lastr= &r->high;
else lastr= &r->low;
rindex= *lastr;
r= (struct hlist_key*)list_entry(l->keys,rindex);
}
if(r->eq<0)break;
lastr= &r->eq;
rindex= r->eq;
r= (struct hlist_key*)list_entry(l->keys,rindex);
}
if(depth>=l->depth)*lastr= r->eq;
else if(r->low<0)*lastr= r->high;
else if(r->high<0)*lastr= r->low;
else abort_message("Huh? delete detach");
}



/*:42*/


}
*last= rindex;
r->high= q->high;
r->low= q->low;
r->eq= q->eq;
}
else if(q->low<0)*last= q->high;
else if(q->high<0)*last= q->low;
else
{
/*41:*/


{

while(q->low>=0&&q->high>=0)
{
if(random_bit)
{
*last= q->high;
r= (struct hlist_key*)list_entry(l->keys,*last);
q->high= r->low;
last= &r->low;
}
else
{
*last= q->low;
r= (struct hlist_key*)list_entry(l->keys,*last);
q->low= r->high;
last= &r->high;
}
}

if(q->low<0)*last= q->high;
else*last= q->low;
}



/*:41*/


}
q->low= l->free;
l->free= index;
}

/*:40*/


#ifdef debug
if(!hlist_integrity(l))abort_message("hlist internal");
#endif
return-1;
}

/*34:*/


{
struct hlist_key rr;

rr.high= rr.low= rr.eq= -1;
for(i= 0;i<l->depth;i++)rr.k[i]= m[i];

if(l->free>=0)
{
index= l->free;
if(*last<0)*last= index;
memcpy(list_entry(l->entries,index),p,l->sz);
q= (struct hlist_key*)list_entry(l->keys,index);
l->free= q->low;
memcpy(q,&rr,sizeof(struct hlist_key));
}
else
{
index= list_count((l)->entries);
if(*last<0)*last= index;
list_push(l->entries,p);
q= (struct hlist_key*)list_push(l->keys,&rr);
}
}

/*:34*/


#ifdef debug
if(!hlist_integrity(l))abort_message("hlist internal");
#endif
return index;
}

/*:38*//*43:*/


UInt32 optimum_balance(UInt32 n)
{
UInt32 m= 0xffffffff;
int i;

for(i= 0;i<32;i++,m<<= 1)if((m&n)==0)break;

return 1+(n+1)*i+m;
}

UInt32 hlist_balance_check_x(hlist*l,SInt32 index,UInt32*paths)
{
UInt32 n,p,nlo,plo,nhi,phi,ne,pe;
struct hlist_key*q;

if(index<0)
{
*paths= 0;
return 0;
}

q= (struct hlist_key*)list_entry(l->keys,index);

nlo= hlist_balance_check_x(l,q->low,&plo);
nhi= hlist_balance_check_x(l,q->high,&phi);
ne= hlist_balance_check_x(l,q->eq,&pe);

*paths= plo+phi+pe+nlo+nhi+ne+1;

return nlo+nhi+ne+1;
}

float hlist_balance_check(hlist*l)
{
UInt16 i;
UInt32 total_nodes= 0,total_path_length= 0,total_optimum= 0;
UInt32 nodes,path_length;
float x,y;

for(i= l->first_nonempty;i<l->hashsize;i++)
{
if(l->hash[i]>=0)
{
nodes= hlist_balance_check_x(l,l->hash[i],&path_length);
total_nodes+= nodes;
total_path_length+= path_length;
total_optimum+= optimum_balance(nodes);
}
}

x= total_path_length;
y= total_optimum;

return(x-y)/(y+.00001);
}

/*:43*/
