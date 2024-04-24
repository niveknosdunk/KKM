#define debug9 1
#define edge_orient(i) (((i) ==1) ?-1:1) 
/*1:*/


#include "Morse.h"
#include "globals.h"
/*8:*/


void LocalCancel12(simplex_id v,simplex_id sigma,simplex_id sigmap,simplex_id tau)
{
simplex_id u;
simplex_id w,wp;

u= sigmap;
w= tau;
unmake_triangle_critical(&w);
while(!edge_is_critical(&u))
{
wp= r12(&u);
pair12(&u,w,true);
w= wp;
u= other_edge(v,u,w);
}
if(!edges_equal(u,sigma))abort_message("localcancel12 internal error");
unmake_edge_critical(&u);
pair12(&u,w,true);
}

/*:8*//*9:*/


void Cancel23(simplex_id sigma,simplex_id taup,simplex_id tau)
{
simplex_id s,sp;
simplex_id t;

s= sigma;
t= taup;
unmake_triangle_critical(&s);

while(!tetrahedron_is_critical(t))
{
sp= r32(t);
pair23(&s,t,true);
s= sp;
t= other_coface(s,t);
}
if(t!=tau)abort_message("cancel23 internal error");
unmake_tetrahedron_critical(t);
pair23(&s,t,true);
}

/*:9*//*10:*/




void Cancel01(simplex_id v,int n,simplex_id kappa)
{
simplex_id u;
simplex_id e,ep;

u= get_vertex(kappa,n);

e= kappa;
unmake_edge_critical(&e);

while(!vertex_is_critical(u))
{
ep= r01(u);
pair01(u,e,true);
e= ep;
u= other_vertex_in_edge(u,e);
}
if(u!=v)abort_message("cancel01 internal error");
unmake_vertex_critical(v);
pair01(v,e,true);
}


/*:10*//*11:*/


void Cancel12(simplex_id sigma,simplex_id kappa,list*grad_path,list*changed)
{
simplex_id t,nextt;
simplex_id e;

t= kappa;
unmake_triangle_critical(&t);

do
{
e= id_list_pop(grad_path);
if(edge_is_critical(&e))break;
nextt= r12(&e);
pair12(&e,t,true);
id_list_push(changed,t);
t= nextt;
}while(true);

if(!edges_equal(e,sigma))abort_message("cancel12 internal error");
unmake_edge_critical(&e);
pair12(&e,t,true);
id_list_push(changed,t);
}

/*:11*/


/*2:*/


void LocalCancel(simplex_id v,UInt32 n)
{
simplex_id f,s;
simplex_id e[2],ep[2];
simplex_id tid[2],tidp[2];
int j,i;
Boolean in;

list_read_init_at(crit[2],n);
while(!id_is_null(f= id_list_read(crit[2])))
{
/*3:*/



e[0]= get_edge(f,1);
e[1]= get_edge(f,2);

/*:3*/

;
/*4:*/



for(i= 0;i<2;i++)
{
ep[i]= e[i];
while(!edge_is_critical(ep+i))
{
if(edge_is_paired_down(ep+i))
{
ep[i]= NULL_ID;
break;
}
s= r12(ep+i);
ep[i]= other_edge(v,ep[i],s);
}
}

/*:4*/

;

if(!edges_equal(ep[0],ep[1]))
{
/*6:*/


if(id_is_null(ep[0]))LocalCancel12(v,ep[1],e[1],f);
else if(id_is_null(ep[1]))LocalCancel12(v,ep[0],e[0],f);
else if(min_value(ep[0],1)> min_value(ep[1],1))
LocalCancel12(v,ep[0],e[0],f);
else LocalCancel12(v,ep[1],e[1],f);

/*:6*/

;
list_read_delete(crit[2]);
}
else
{

for(i= 0;i<2;i++)
{
/*5:*/


tidp[i]= tid[i]= coface(f,i);
s= f;
in= false;
while(tetrahedron_is_in_K(tid[i]))
{
in= is_in_lower_Star(tid[i],v);
if(!in)break;
if(tetrahedron_is_critical(tid[i]))break;
s= r32(tid[i]);
tid[i]= other_coface(s,tid[i]);
}
if(!tetrahedron_is_in_K(tid[i])||!in)
tid[i]= NULL_ID;

/*:5*/

;
}
if(tid[0]!=tid[1])
{
/*7:*/


if(id_is_null(tid[0]))Cancel23(f,tidp[1],tid[1]);
else if(id_is_null(tid[1]))Cancel23(f,tidp[0],tid[0]);
else if(min_value(tid[1],3)> min_value(tid[0],3))
Cancel23(f,tidp[0],tid[0]);
else Cancel23(f,tidp[1],tid[1]);


/*:7*/

;
list_read_delete(crit[2]);
}
}
}
}


/*:2*//*13:*/



void ExtractCancel1(SInt32 p)
{
hlist*c0;
hlist*c1;
struct ccrit1
{
simplex_id s;
SInt32 c[2];
}s;
int i;
simplex_id v[2];
SInt32 thisk;
SInt32 evalue;
UInt8 persist_scaling;

/*14:*/


{
UInt32 mask= 0xffffe000;

if(p> 65535)p= 65535;
else if(p<0)p= 0;

for(persist_scaling= 0;persist_scaling<4;persist_scaling++,mask<<= 1)
if((p&mask)==0)break;
}

/*:14*/



hlist_initialize(&c0,sizeof(SInt32),113,1,0);
hlist_initialize(&c1,sizeof(struct ccrit1),1+(p>>persist_scaling),1,persist_scaling);

/*15:*/


{
simplex_id e;

list_read_init(crit[1]);
while((e= id_list_read(crit[1]))!=-1)
{
if(!edge_is_critical(&e))list_read_delete(crit[1]);
else
{
evalue= value(e);

v[0]= FindGrad01(get_vertex(e,0),evalue-p);
v[1]= FindGrad01(get_vertex(e,1),evalue-p);
if(v[0]!=v[1])
{
/*16:*/


{
SInt32 link;
SInt32 persist;

link= -1;
s.s= e;
s.c[0]= (id_is_null(v[0]))?-1:hlist_find_add(c0,v,&link,NULL);
s.c[1]= (id_is_null(v[1]))?-1:hlist_find_add(c0,v+1,&link,NULL);
i= /*21:*/


(id_is_null(v[0]))?1:
(id_is_null(v[1]))?0:
(vertex_value(v[0])> vertex_value(v[1]))?0:
(vertex_value(v[0])<vertex_value(v[1]))?1:
vertex_is_critical(v[0])?1:0


/*:21*/

;
persist= evalue-vertex_value(v[i]);
hlist_add(c1,&persist,&s);
}

/*:16*/

;
}
}
}
}

/*:15*/

;

while(!hlist_is_empty(c1))
{
hlist_min(c1,&thisk,&s);
/*17:*/


v[0]= (s.c[0]<0)?NULL_ID:hlist_get_key(c0,s.c[0],0);
v[1]= (s.c[1]<0)?NULL_ID:hlist_get_key(c0,s.c[1],0);


/*:17*/


i= /*21:*/


(id_is_null(v[0]))?1:
(id_is_null(v[1]))?0:
(vertex_value(v[0])> vertex_value(v[1]))?0:
(vertex_value(v[0])<vertex_value(v[1]))?1:
vertex_is_critical(v[0])?1:0


/*:21*/

;
evalue= value(s.s);
if(vertex_is_critical(v[i])&&thisk==evalue-vertex_value(v[i]))
/*18:*/


{
Cancel01(v[i],i,s.s);
*((SInt32*)hlist_entry(c0,s.c[i]))= s.c[1-i];

}

/*:18*/


else/*19:*/


{
SInt32 m;

evalue= value(s.s);

for(i= 0;i<2;i++)
{
if(s.c[i]<0)continue;
/*20:*/


{
SInt32 a;

for(a= s.c[i];a>=0;)
{
v[i]= hlist_get_key(c0,a,0);
if(vertex_is_critical(v[i]))break;
a= *((SInt32*)hlist_entry(c0,a));
}
if(a!=s.c[i])
{
*((SInt32*)hlist_entry(c0,s.c[i]))= a;
}
if(vertex_value(v[i])<=(evalue-p))
{
v[i]= NULL_ID;
s.c[i]= -1;
}
else s.c[i]= a;
}

/*:20*/


}


if(s.c[1]!=s.c[0])
{
i= /*21:*/


(id_is_null(v[0]))?1:
(id_is_null(v[1]))?0:
(vertex_value(v[0])> vertex_value(v[1]))?0:
(vertex_value(v[0])<vertex_value(v[1]))?1:
vertex_is_critical(v[0])?1:0


/*:21*/

;
m= evalue-vertex_value(v[i]);

if(m<p)hlist_add(c1,&m,&s);
}
}

/*:19*/


}
hlist_abandon(&c0);
hlist_abandon(&c1);
}

/*:13*//*22:*/



void ExtractCancel3(SInt32 p)
{
hlist*c3;
hlist*c2;
struct ccrit2
{
simplex_id s;
SInt32 c[2];
}r;
int i;
simplex_id t[2];
SInt32 thisk;
SInt32 fvalue,tvalue;
UInt8 persist_scaling;

/*14:*/


{
UInt32 mask= 0xffffe000;

if(p> 65535)p= 65535;
else if(p<0)p= 0;

for(persist_scaling= 0;persist_scaling<4;persist_scaling++,mask<<= 1)
if((p&mask)==0)break;
}

/*:14*/



hlist_initialize(&c3,sizeof(SInt32),113,1,0);
hlist_initialize(&c2,sizeof(struct ccrit2),1+(p>>persist_scaling),1,persist_scaling);

/*23:*/


{
simplex_id s;

list_read_init(crit[2]);
while((s= id_list_read(crit[2]))!=-1)
{
if(!triangle_is_critical(&s))list_read_delete(crit[2]);
else
{
fvalue= value(s);

t[0]= FindGrad23(coface0(s),fvalue+p);
t[1]= FindGrad23(coface1(s),fvalue+p);
if(t[0]!=t[1])
/*24:*/


{
SInt32 link;
SInt32 persist;

link= -1;
r.s= s;
r.c[0]= (id_is_null(t[0]))?-1:hlist_find_add(c3,t,&link,NULL);
r.c[1]= (id_is_null(t[1]))?-1:hlist_find_add(c3,t+1,&link,NULL);

i= /*29:*/


(id_is_null(t[0]))?1:
(id_is_null(t[1]))?0:
(value(t[0])> value(t[1]))?1:
(value(t[0])<value(t[1]))?0:
tetrahedron_is_critical(t[0])?1:0





/*:29*/

;
persist= value(t[i])-fvalue;
hlist_add(c2,&persist,&r);
}



/*:24*/


}
}
}

/*:23*/

;
while(!hlist_is_empty(c2))
{
hlist_min(c2,&thisk,&r);
/*25:*/


t[0]= (r.c[0]<0)?NULL_ID:hlist_get_key(c3,r.c[0],0);
t[1]= (r.c[1]<0)?NULL_ID:hlist_get_key(c3,r.c[1],0);




/*:25*/


i= /*29:*/


(id_is_null(t[0]))?1:
(id_is_null(t[1]))?0:
(value(t[0])> value(t[1]))?1:
(value(t[0])<value(t[1]))?0:
tetrahedron_is_critical(t[0])?1:0





/*:29*/

;
fvalue= value(r.s);
tvalue= value(t[i]);
if(tetrahedron_is_critical(t[i])&&thisk==tvalue-fvalue)/*26:*/


{
Cancel23(r.s,coface(r.s,i),t[i]);
*((SInt32*)hlist_entry(c3,r.c[i]))= r.c[1-i];
}

/*:26*/


else/*27:*/


{
SInt32 m;

fvalue= value(r.s);

for(i= 0;i<2;i++)
{
if(r.c[i]<0)continue;
/*28:*/


{
SInt32 a;

for(a= r.c[i];a>=0;)
{
t[i]= hlist_get_key(c3,a,0);
if(tetrahedron_is_critical(t[i]))break;
a= *((SInt32*)hlist_entry(c3,a));
}
*((SInt32*)hlist_entry(c3,r.c[i]))= a;
if(value(t[i])>=fvalue+p)
{
t[i]= NULL_ID;
r.c[i]= -1;
}
else r.c[i]= a;
}

/*:28*/


}


if(r.c[1]!=r.c[0])
{
i= /*29:*/


(id_is_null(t[0]))?1:
(id_is_null(t[1]))?0:
(value(t[0])> value(t[1]))?1:
(value(t[0])<value(t[1]))?0:
tetrahedron_is_critical(t[0])?1:0





/*:29*/

;

m= value(t[i])-fvalue;

if(m<p)hlist_add(c2,&m,&r);
}
}

/*:27*/


}
hlist_abandon(&c2);
hlist_abandon(&c3);
}

/*:22*//*30:*/


void ExtractCancel2(SInt32 p)
{
list*changed;
list*grad_path;
hlist*goodpairs;
UInt32 lastp;
UInt8 persist_scaling;

/*14:*/


{
UInt32 mask= 0xffffe000;

if(p> 65535)p= 65535;
else if(p<0)p= 0;

for(persist_scaling= 0;persist_scaling<4;persist_scaling++,mask<<= 1)
if((p&mask)==0)break;
}

/*:14*/



list_initialize(&changed,sizeof(simplex_id));
list_initialize(&grad_path,sizeof(simplex_id));
hlist_initialize(&goodpairs,sizeof(simplex_id),
1+(p>>persist_scaling),1,persist_scaling);
lastp= 0;
do
{
/*31:*/


{
simplex_id f;
simplex_id e;
UInt32 persist;

list_read_init(crit[2]);
while(!id_is_null(f= id_list_read(crit[2])))
{
if(!triangle_is_critical(&f))list_read_delete(crit[2]);
else
{
e= FindGradPaths12(f,p,NULL,0);
if(!id_is_null(e))
{
persist= value(f)-value(e);
hlist_add(goodpairs,&persist,&f);
}
}
}
}




/*:31*/


if(hlist_is_empty(goodpairs))break;
/*32:*/


{
simplex_id f;
simplex_id e;
UInt32 bp,thisp;

while(!hlist_is_empty(goodpairs))
{
hlist_min(goodpairs,&bp,&f);
e= FindGradPaths12(f,p,grad_path,0);
if(id_is_null(e))continue;
thisp= value(f)-value(e);
if(thisp<=bp)
{
list_clear(changed);
Cancel12(e,f,grad_path,changed);
/*33:*/


{
int i,j,k;
simplex_id t;

k= 0;
while(!list_is_empty(changed)&&k<10000)
{
f= id_list_pop(changed);
if(id_is_null(f))continue;
for(i= 0;i<2;i++)
{
t= coface(f,i);
if(tetrahedron_is_in_K(t)&&!tetrahedron_is_critical(t))
{
if(splitrejoin(t,changed))k++;
}
}
}
if(!list_is_empty(changed))printf("May be infinite loop in split-rejoin");
}


/*:33*/

;

if(thisp<lastp)printf("\n persistence out of order: %d > %d\n",lastp,thisp);

lastp= thisp;
}
else hlist_add(goodpairs,&thisp,&f);
}
}






/*:32*/


}while(true);
list_abandon(&changed);
list_abandon(&grad_path);
hlist_abandon(&goodpairs);
}

/*:30*/


/*34:*/


Boolean splitrejoin(simplex_id t,list*changed)
{
int i,j;
simplex_id e,ep;
simplex_id s,sp;

for(i= 0;i<4;i++)
{
s= get_face(t,i);
if(triangle_is_paired_down(&s))
{
e= r21(&s);
for(j= 0;j<3;j++)
{
ep= get_edge(s,j);
if(!edges_equal(ep,e)
&&(!edge_is_paired_up(&ep)||triangle_in_tetrahedron(r12(&ep),t)<0))break;
}
if(j==3)
{
sp= r32(t);
#ifdef debug9
if(!triangles_equal(r32(t),sp))abort_message("Split-rejoin error");
#endif
pair23(&s,t,true);
pair12(&e,sp,true);
id_list_push(changed,sp);
return true;
}
}
}
return false;
}




/*:34*//*35:*/


simplex_id FindGrad01(simplex_id u,SInt32 m)
{
simplex_id v;
simplex_id e;

v= u;
while(!vertex_is_critical(v)&&vertex_value(v)> m)
{
e= r01(v);
v= other_vertex_in_edge(v,e);
}
if(vertex_value(v)<=m)return NULL_ID;
return v;
}

/*:35*//*36:*/


simplex_id FindGrad23(simplex_id tau,SInt32 m)

{
simplex_id t;
simplex_id s;

if(id_is_null(tau))return NULL_ID;

t= tau;
while(tetrahedron_is_in_K(t)&&value(t)<m)
{
if(tetrahedron_is_critical(t))return t;
s= r32(t);
if(triangle_is_deadend(&s))
{
make_tetrahedron_deadend(t);
return NULL_ID;
}
t= other_coface(s,t);
if(tetrahedron_is_deadend(t))
{
make_triangle_deadend(&s);
return NULL_ID;
}
}
if(!tetrahedron_is_in_K(t)&&t!=tau)make_triangle_deadend(&s);
return NULL_ID;
}

/*:36*//*37:*/


simplex_id FindGrad23orientation(simplex_id sigma,int*orientation)
{
simplex_id t;
simplex_id s;
int orient;

t= id2tetra_id(sigma);
orient= face_index(sigma);

while(tetrahedron_is_in_K(t))
{
if(tetrahedron_is_critical(t))
{
if(orient&1)*orientation= -1;else*orientation= 1;
return t;
}
s= r32(t);
orient+= face_index(s);
if(triangle_is_deadend(&s))
{
make_tetrahedron_deadend(t);
return NULL_ID;
}
s= other_face_id(s);
t= id2tetra_id(s);
if(tetrahedron_is_deadend(t))
{
make_triangle_deadend(&s);
return NULL_ID;
}
orient++;
orient+= face_index(s);
}
return NULL_ID;
}

/*:37*//*38:*/


simplex_id FindGradPaths12(simplex_id sigma,SInt32 p,list*grad_path,int flags)
{
static hlist*graph;
static list*to_do;
static list*crits;
static Boolean first= true;
struct edge_graph
{
SInt32 up;
SInt32 count;
simplex_id e;
}r,*q;
simplex_id e;
simplex_id f;
simplex_id vlist[2];
SInt32 m;
SInt32 sigvalue= value(sigma);

/*39:*/


if(first)
{
hlist_initialize(&graph,sizeof(struct edge_graph),703,2,0);
list_initialize(&crits,sizeof(SInt32));
list_initialize(&to_do,sizeof(SInt32));
first= false;
}
else
{
hlist_clear(graph);
list_clear(crits);
list_clear(to_do);
}


/*:39*/



m= -1;
e= NULL_ID;
f= sigma;
/*40:*/


{
int i;
SInt32 n;
Boolean flag;
simplex_id ep;
int livecount;

livecount= 0;

for(i= 0;i<3;i++)
{
ep= get_edge(f,i);
if(!edges_equal(ep,e)&&!edge_is_paired_downx(ep)&&!edge_is_deadendx(&ep))
{
livecount++;













r.up= m;
r.count= 0;
r.e= ep;
get_edge_vertices(ep,vlist);

n= hlist_find_add(graph,vlist,&r,&flag);
q= (struct edge_graph*)hlist_entry(graph,n);
q->count++;

if(!flag)
{

if(edge_is_critical(&ep))list_push(crits,&n);
else list_push(to_do,&n);

q->e= ep;
}
}
}
if(livecount==0&&!id_is_null(e))make_edge_deadend(&e);
}

/*:40*/



while(!list_is_empty(to_do))
{
list_pop(to_do,&m);
q= (struct edge_graph*)hlist_entry(graph,m);
e= q->e;
f= r12(&e);
/*40:*/


{
int i;
SInt32 n;
Boolean flag;
simplex_id ep;
int livecount;

livecount= 0;

for(i= 0;i<3;i++)
{
ep= get_edge(f,i);
if(!edges_equal(ep,e)&&!edge_is_paired_downx(ep)&&!edge_is_deadendx(&ep))
{
livecount++;













r.up= m;
r.count= 0;
r.e= ep;
get_edge_vertices(ep,vlist);

n= hlist_find_add(graph,vlist,&r,&flag);
q= (struct edge_graph*)hlist_entry(graph,n);
q->count++;

if(!flag)
{

if(edge_is_critical(&ep))list_push(crits,&n);
else list_push(to_do,&n);

q->e= ep;
}
}
}
if(livecount==0&&!id_is_null(e))make_edge_deadend(&e);
}

/*:40*/


}

if(flags&1)/*70:*/


{
simplex_id ee;

e= NULL_ID;
while(!list_is_empty(crits))
{
list_pop(crits,&m);
q= (struct edge_graph*)hlist_entry(graph,m);
ee= q->e;
if(false)

{
e= ee;
break;
}
}
}/*:70*/


else/*41:*/


{
SInt32 best_value,val,bestm;

bestm= -1;

while(!list_is_empty(crits))
{
list_pop(crits,&m);
q= (struct edge_graph*)hlist_entry(graph,m);
val= value(q->e);

if(val<=sigvalue-p)continue;
if(q->count==1&&(bestm<0||val> best_value))
/*42:*/


{

while(q->up>=0)
{
q= (struct edge_graph*)hlist_entry(graph,q->up);
if(q->count> 1)break;
}
if(q->count==1)
{
best_value= val;
bestm= m;
}
}

/*:42*/


}
if(bestm>=0)
{
q= (struct edge_graph*)hlist_entry(graph,bestm);
e= q->e;
}
else e= NULL_ID;
if(!id_is_null(e)&&grad_path!=NULL)/*43:*/


{
list_clear(grad_path);

m= bestm;

while(m>=0)
{
q= (struct edge_graph*)hlist_entry(graph,m);
id_list_push(grad_path,q->e);
m= q->up;
}
}


/*:43*/



}

/*:41*/



return e;
}

/*:38*//*45:*/



void
check_grad12_struct(hlist*edges,int options)
{
int i,k,kk,id;
struct grad12_struct*p,*q;
simplex_id e,ep,f,vlist[2];

for(i= 0;i<hlist_count(edges);i++)
{
p= (struct grad12_struct*)hlist_entry(edges,i);
e= p->e;
kk= p->flags&3;
if(kk!=3)
{
if(!edge_is_paired_up(&e))abort_message("check grad12 err 1");
f= r12(&e);
for(k= 0;k<3;k++)
{
if(k==kk)continue;
ep= get_edge(f,k);
if((options&3)!=0&&edge_is_deadend(&ep))continue;
/*46:*/


{
simplex_id epp;
int j;

get_edge_vertices(ep,vlist);
id= hlist_find(edges,vlist);
if(id<0)
{
if(!edge_is_deadend(&ep)&&!edge_is_paired_down(&ep))
abort_message("check grad12 err 2");
}
else
{
q= (struct grad12_struct*)hlist_entry(edges,id);

id= q->links[(q->flags&3)%3];

while(id>=0)
{
if(id==i)break;
q= (struct grad12_struct*)hlist_entry(edges,id);
epp= q->e;

j= edge_verts_in_triangle(vlist,r12(&epp));
if(j<0)abort_message("check grad12 err 3");
id= q->links[j];
}
if(id<0)abort_message("check grad12 err 4");
}
}



/*:46*/


}
}

}
}

/*:45*//*47:*/


SInt32
find_all_grad12_paths(simplex_id sigma,hlist*edges,int options)
{
int kk= -1;
simplex_id ep;
SInt32 todo= -2;
SInt32 this;
SInt32 critid= -1;
struct grad12_struct*p;
simplex_id f;

hlist_clear(edges);

do
{
/*48:*/


{
this= todo;
if(this>=0)
{
p= (struct grad12_struct*)hlist_entry(edges,this);
ep= p->e;
kk= p->flags&3;
if(kk)
{
todo= p->links[0];
p->links[0]= -1;
}
else
{
todo= p->links[1];
p->links[1]= -1;
}
f= r12(&ep);
}
else f= sigma;
}

/*:48*/


/*49:*/


{
int k;
simplex_id e;
int livecount= 0;

for(k= 0;k<3;k++)
{
if(k==kk)continue;
e= get_edge(f,k);
/*50:*/


{
struct grad12_struct r,*q;
simplex_id s,vlist[2];
Boolean flag;
int j;
SInt32 id;



if(((options&3)==0||!edge_is_deadend(&e))&&!edge_is_paired_down(&e))
{
livecount++;
get_edge_vertices(e,vlist);
r.fake= r.count= r.count2= 0;
id= hlist_find_add(edges,vlist,&r,&flag);
q= (struct grad12_struct*)hlist_entry(edges,id);
if(flag)
{
p= (struct grad12_struct*)hlist_entry(edges,this);
p->links[k]= q->links[(q->flags&3)%3];
q->links[(q->flags&3)%3]= this;
}
else
{


if(edge_is_paired_up(&e))
{
q->links[1]= q->links[2]= -1;
s= r12(&e);
q->e= e;
q->flags= edge_in_triangle(e,s);
if(q->flags<0)abort_message("help");
q->links[q->flags]= this;
if(q->flags)q->links[0]= todo;
else q->links[1]= todo;
todo= id;
}
else if(edge_is_critical(&e))
{
q->e= e;
q->links[0]= this;
q->links[1]= critid;
q->flags= 3;
critid= id;
}
}
}
}

/*:50*/


}
if(livecount==0&&(options&3)!=0&&this>=0)
{
make_triangle_deadend(&f);
make_edge_deadend(&ep);
}
}


/*:49*/


}while(todo>=0);

check_grad12_struct(edges,options);

if((options&3)==2)
/*51:*/


{
SInt32 j;
simplex_id e,s;
list*todo_list;

list_initialize(&todo_list,sizeof(SInt32));

todo= critid;

while(todo>=0)
{
/*52:*/


{
SInt32 next,id;
struct grad12_struct*q;
simplex_id vl[2];
int i;

list_clear(todo_list);
list_push(todo_list,&todo);

while(!list_is_empty(todo_list))
{
list_pop(todo_list,&next);
p= (struct grad12_struct*)hlist_entry(edges,next);

if(p->flags&4)continue;
p->flags|= 4;

i= p->flags&3;
id= p->links[i%3];

e= p->e;
vl[0]= hlist_get_key(edges,next,0);
vl[1]= hlist_get_key(edges,next,1);

while(id>=0)
{
q= (struct grad12_struct*)hlist_entry(edges,id);

ep= q->e;

if((q->flags&4)==0)
{
list_push(todo_list,&id);
}
i= edge_verts_in_triangle(vl,r12(&ep));
if(i<0)abort_message("error");
id= q->links[i];
}
}
}


/*:52*/


p= (struct grad12_struct*)hlist_entry(edges,todo);
todo= p->links[1];
}

list_abandon(&todo_list);

for(j= 0;j<hlist_count(edges);j++)
{
p= (struct grad12_struct*)hlist_entry(edges,j);
if(p->flags&4)continue;
e= p->e;
make_edge_deadend(&e);
s= r12(&e);
make_triangle_deadend(&s);
}
}

/*:51*/



if(options&4)
/*53:*/


{
list*todo_list;
SInt32 id= -1,*idp;
struct grad12_struct*q;
int error_check;

list_initialize(&todo_list,sizeof(SInt32));
list_push(todo_list,&id);

while(!list_is_empty(todo_list))
{
list_read_init(todo_list);
error_check= 1;
while((idp= (SInt32*)list_read(todo_list))!=NULL)
{
if(*idp<0)
{
list_read_delete(todo_list);
kk= -1;
f= sigma;
}
else
{
p= (struct grad12_struct*)hlist_entry(edges,*idp);
if(p->flags&8){list_read_delete(todo_list);continue;}


/*54:*/


{
simplex_id e;
int i;
simplex_id vl[2];
SInt32 ct= 0;

ep= p->e;
vl[0]= hlist_get_key(edges,*idp,0);
vl[1]= hlist_get_key(edges,*idp,1);

kk= p->flags&3;
id= p->links[kk%3];

while(id>=0)
{
q= (struct grad12_struct*)hlist_entry(edges,id);
if((q->flags&8)==0)break;
e= q->e;
i= edge_verts_in_triangle(vl,r12(&e));
#ifdef debug9
if(i<0)abort_message("count error2");
#endif
ct+= edge_orient(i)*q->count;
id= q->links[i];
}

if(id<0)
/*57:*/


{
p->flags&= 0xffffffef;
p->flags|= 8;
list_read_delete(todo_list);

if(id==-2)
{

ct+= edge_orient(edge_in_triangle(ep,sigma));
}
if(edge_is_critical(&ep))p->count= ct;
else p->count= -ct*edge_orient(p->flags&3);
if(p->count> 1000||p->count<-1000)abort_message("large count");
}

/*:57*/


else p->flags|= 16;

if(id>=0||edge_is_critical(&ep))f= NULL_ID;
else f= r12(&ep);
}

/*:54*/


}

if(!id_is_null(f))
{
error_check= 0;
/*55:*/


{
int k;
simplex_id e;

for(k= 0;k<3;k++)
{
if(k==kk)continue;
e= get_edge(f,k);
/*56:*/


{
int flag;
SInt32 qid;
simplex_id vll[2];

if(!edge_is_paired_down(&e)&&((options&3)==0||!edge_is_deadend(&e)))
{
get_edge_vertices(e,vll);
qid= hlist_find(edges,vll);
#ifdef debug9
if(qid<0)abort_message("count error 2");
#endif
q= (struct grad12_struct*)hlist_entry(edges,qid);
if((q->flags&32)==0)
{
list_read_insert(todo_list,&qid);
q->flags|= 32;
}
q->flags&= 0xffffffef;

}
}

/*:56*/


}
}

/*:55*/


}
}
if(error_check&&!list_is_empty(todo_list))
{


/*58:*/


{
simplex_id vl[2];
simplex_id e;
int i;

printf("\nbacktrack: ");
id= *(SInt32*)list_entry(todo_list,0);

while(id>=0)
{
p= (struct grad12_struct*)hlist_entry(edges,id);
ep= p->e;
vl[0]= hlist_get_key(edges,id,0);
vl[1]= hlist_get_key(edges,id,1);
printf(" %d %d,",id,p->flags);
if(p->flags&4096)break;
p->flags|= 4096;
id= p->links[(p->flags&3)%3];
while(id>=0)
{
p= (struct grad12_struct*)hlist_entry(edges,id);
if((p->flags&8)==0)break;
e= p->e;
i= edge_verts_in_triangle(vl,r12(&e));
id= p->links[i];
}
}

printf("\n");
if(p->flags&4096)abort_message("cyclic gradient field");

}

/*:58*/











todo= critid;

while(todo>=0)
{
p= (struct grad12_struct*)hlist_entry(edges,todo);
if(p->flags&8==0)break;
todo= p->links[1];
}

if(todo>=0)
{

list_read_init(todo_list);
while((idp= (SInt32*)list_read(todo_list))!=NULL)
{
simplex_id vl[2];
simplex_id e;
int i;

p= (struct grad12_struct*)hlist_entry(edges,*idp);
ep= p->e;
vl[0]= hlist_get_key(edges,*idp,0);
vl[1]= hlist_get_key(edges,*idp,1);
printf("%d %d ",*idp,p->flags);
id= p->links[(p->flags&3)%3];
while(id>=0)
{
q= (struct grad12_struct*)hlist_entry(edges,id);
printf("%d-",q->flags);
e= q->e;
i= edge_verts_in_triangle(vl,r12(&e));
id= q->links[i];
}
printf("\n");
}
abort_message("count error");
}
else{printf("extra todo_list\n");break;}
}
}

list_abandon(&todo_list);
}

/*:53*/



if(options&8)
/*60:*/


{
list*todo_list;
SInt32*idp;
int error_check;
int i;

list_initialize(&todo_list,sizeof(SInt32));


todo= critid;

while(todo>=0)
{
p= (struct grad12_struct*)hlist_entry(edges,todo);
p->flags|= 256;
list_push(todo_list,&todo);
todo= p->links[1];
}


for(i= 0;i<list_count(todo_list);i++)
{
todo= *(SInt32*)list_entry(todo_list,i);
p= (struct grad12_struct*)hlist_entry(edges,todo);
/*65:*/


{
SInt32 id;
struct grad12_struct*q;
simplex_id vl[2],e;

kk= p->flags&3;
id= p->links[kk%3];

e= p->e;
vl[0]= hlist_get_key(edges,todo,0);
vl[1]= hlist_get_key(edges,todo,1);

while(id>=0)
{
q= (struct grad12_struct*)hlist_entry(edges,id);

ep= q->e;

if((q->flags&256)==0)
{
list_push(todo_list,&id);
q->flags|= 256;
}
kk= edge_verts_in_triangle(vl,r12(&ep));
if(kk<0)abort_message("error");
id= q->links[kk];
}
}

/*:65*/


}



while(!list_is_empty(todo_list))
{
list_read_init(todo_list);
error_check= 1;
while((idp= (SInt32*)list_read(todo_list))!=NULL)
{
p= (struct grad12_struct*)hlist_entry(edges,*idp);
if(p->flags&1024){list_read_delete(todo_list);continue;}


/*61:*/


{
SInt32 id;
SInt32 ct= 0;
struct grad12_struct*q;
simplex_id e;
simplex_id vl[2];

ep= p->e;
vl[0]= hlist_get_key(edges,*idp,0);
vl[1]= hlist_get_key(edges,*idp,1);

kk= p->flags&3;
id= p->links[kk%3];

while(id>=0)
{
q= (struct grad12_struct*)hlist_entry(edges,id);
if((q->flags&1024)==0)break;
e= q->e;
i= edge_verts_in_triangle(vl,r12(&e));
#ifdef debug9
if(i<0)abort_message("count error2");
#endif
ct+= edge_orient(i)*q->count2;
id= q->links[i];
}

if(id<0)
/*62:*/


{

p->flags&= 0xfffff7ff;
p->flags|= 1024;
list_read_delete(todo_list);

if(id==-2)
{

ct+= edge_orient(edge_in_triangle(ep,sigma));
}
if(edge_is_critical(&ep))p->count2= ct;
else p->count2= -ct*edge_orient(p->flags&3);

error_check= 0;
}

/*:62*/


else p->flags|= 2048;

if(id<0&&!edge_is_critical(&ep))
/*63:*/


{
int k;

f= r12(&ep);

for(k= 0;k<3;k++)
{
if(k==kk)continue;
e= get_edge(f,k);
/*64:*/


{
SInt32 qid;
simplex_id vll[2];

if(!edge_is_paired_down(&e))
{
get_edge_vertices(e,vll);
qid= hlist_find(edges,vll);
if(qid>=0)
{
q= (struct grad12_struct*)hlist_entry(edges,qid);
q->flags&= 0xfffff7ff;

}
else if((options&3)==0||!edge_is_deadend(&e))
abort_message("count2 error 37");
}
}


/*:64*/


}
}

/*:63*/


}

/*:61*/



}
if(error_check&&!list_is_empty(todo_list))
{
/*59:*/


{
simplex_id vl[2];
simplex_id e;
SInt32 id;

printf("\nbacktrack2: ");
id= *(SInt32*)list_entry(todo_list,0);

while(id>=0)
{
p= (struct grad12_struct*)hlist_entry(edges,id);
ep= p->e;
vl[0]= hlist_get_key(edges,id,0);
vl[1]= hlist_get_key(edges,id,1);
printf(" %d %d,",id,p->flags);
id= p->links[(p->flags&3)%3];
while(id>=0)
{
p= (struct grad12_struct*)hlist_entry(edges,id);
if((p->flags&1024)==0)break;
e= p->e;
i= edge_verts_in_triangle(vl,r12(&e));
id= p->links[i];
}
}
printf("\n");
if(p->flags&4096)abort_message("cyclic gradient field");

}

/*:59*/


abort_message("count2 error");
}
}

list_abandon(&todo_list);
}

/*:60*/



return critid;
}

/*:47*//*66:*/


SInt32
find_all_backward_grad12_paths(simplex_id tau,hlist*triangles,list*crits,int options)
{
int kk= -1;
simplex_id f;
simplex_id ep;
SInt32 todo= -1;
SInt32 this;
struct grad12_struct*p;
SInt32 start= -1;
list*todo_list;
int tab[6]= {3,3,2,3,2,1};

hlist_clear(triangles);
if(crits!=NULL)list_clear(crits);
if((options&3)==2)list_initialize(&todo_list,sizeof(SInt32));

do
{
this= todo;
if(this>=0)
{
p= (struct grad12_struct*)hlist_entry(triangles,this);
f= hlist_get_key(triangles,this,0);
ep= r21(&f);
kk= p->flags&3;
todo= p->links[kk];
p->links[kk]= -1;
}
else ep= tau;
/*67:*/


{
simplex_id t,t1;
simplex_id vlist[3];
int link_count= 0;

t= t1= id2tetra_id(ep);
get_edge_vertices(ep,vlist);
vlist[2]= id2tetrahedron(t)->vertex_ids[tab[edge_index(ep)]];
f= vertices2trianglex(vlist,t);

do
{
if(id_is_null(f))abort_message("backgrad error 3");
if(triangle_is_in_K(f)&&(triangle_is_critical(&f)||
((!triangle_is_xdeadend(&f)||(options&3)==0)&&
triangle_is_paired_down(&f)&&!edges_equal(ep,r21(&f)))))
{
link_count++;
/*68:*/


{
struct grad12_struct r,*q;
SInt32 id;
Boolean flag;
int k;

f= best_face_id(f);

id= hlist_find_add(triangles,&f,&r,&flag);
q= (struct grad12_struct*)hlist_entry(triangles,id);
k= edge_in_triangle(ep,f);
if(this>=0)
p= (struct grad12_struct*)hlist_entry(triangles,this);
if(flag)
{
if(q->links[k]>=0)abort_message("backgrad error 1");
if(this<0)abort_message("backgrad error 2");
q->links[k]= p->links[kk];
p->links[kk]= id;
}
else
{
q->links[0]= q->links[1]= q->links[2]= -2;
if(this>=0)
{
q->links[k]= p->links[kk];
p->links[kk]= id;
}
else
{
q->links[k]= start;
start= id;
}
if(triangle_is_critical(&f))
{
q->flags= 3;
if(crits!=NULL)list_push(crits,&id);
if((options&3)==2)list_push(todo_list,&id);
}
else
{
q->flags= edge_in_triangle(r21(&f),f);
q->links[q->flags]= todo;
todo= id;
}
}
}

/*:68*/


}
f= next_face(vlist,f);
}while(t!=t1);

if(link_count==0&&(options&3)&&this>=0)
{
make_edge_xdeadend(&ep);
f= r12(&ep);
make_triangle_xdeadend(&f);
}
}

/*:67*/


}while(todo>=0);
if((options&3)==2)
/*69:*/


{
SInt32 id;
int k;
simplex_id e;
struct grad12_struct r,*q;
simplex_id f1;
Boolean flag;

while(!list_is_empty(todo_list))
{
list_pop(todo_list,&id);
p= (struct grad12_struct*)hlist_entry(triangles,id);
if(p->flags&4)continue;
p->flags|= 4;
kk= p->flags&3;
f= hlist_get_key(triangles,id,0);
for(k= 0;k<3;k++)
{
if(k==kk||p->links[k]<-1)continue;
e= get_edge(f,k);
if(edge_is_paired_up(&e)&&!edge_is_deadend(&e))
{
if(edge_is_xdeadend(&e))abort_message("back grad error 3");
f1= r12(&e);
id= hlist_find_add(triangles,&f1,&r,&flag);
q= (struct grad12_struct*)hlist_entry(triangles,id);
if(flag==0)
{
q->flags= edge_in_triangle(e,r12(&e));
printf("back grad puzzle\n");
}
if((q->flags&4)==0)list_push(todo_list,&id);
}
}
}
list_abandon(&todo_list);

for(id= 0;id<hlist_count(triangles);id++)
{
p= (struct grad12_struct*)hlist_entry(triangles,id);
if(p->flags&4)continue;
f= hlist_get_key(triangles,id,0);
make_triangle_xdeadend(&f);
e= r21(&f);
make_edge_xdeadend(&e);
}
}





/*:69*/


return start;
}

/*:66*/



/*:1*/
