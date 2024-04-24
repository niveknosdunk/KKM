#define glue_face(f0,f1) (id2tetrahedron(f1) ->face_ids[face_index(f1) ]= f0) 
#define glue_faces_old(f0,f1) (glue_face(f0,f1) ,glue_face(f1,f0) ) 
/*1:*/
#line 16 "./simplex.w"

#include "Morse.h"
#include "globals.h"
/*9:*/
#line 193 "./simplex.w"

simplex_id other_vertex_in_edge(simplex_id u,simplex_id e)
{
tetrahedron*t;
simplex_id v;
int j= edge_index(e);

t= id2tetrahedron(e);
v= t->vertex_ids[vtab[j][0]];
if(u==v)v= t->vertex_ids[vtab[j][1]];
return v;
}

void get_edge_vertices(simplex_id e,simplex_id*vl)
{
tetrahedron*tp;
int j= edge_index(e);

tp= id2tetrahedron(e);
vl[0]= tp->vertex_ids[vtab[j][0]];
vl[1]= tp->vertex_ids[vtab[j][1]];
}

void get_triangle_vertices(simplex_id f,simplex_id*vlist)

{
int k= face_index(f);
simplex_id*q= vlist;
simplex_id*p= id2tetrahedron(f)->vertex_ids;

if(k!=0)*q++= *p;
p++;
if(k!=1)*q++= *p;
p++;
if(k!=2)*q++= *p;
p++;
if(k!=3)*q++= *p;
}


/*:9*//*11:*/
#line 255 "./simplex.w"

simplex_id other_coface(simplex_id s,simplex_id t)
{
if(id_dimension(s)!=2||id_dimension(t)!=3)abort_message("internal coface");
if(id2tetra_id(s)!=t)return id2tetra_id(s);
return id2tetra_id(id2tetrahedron(t)->face_ids[face_index(s)]);
}


/*:11*//*12:*/
#line 271 "./simplex.w"


simplex_id vertices2triangle(simplex_id*fvl,simplex_id t)
{
simplex_id*tvl= id2tetrahedron(t)->vertex_ids;

if(fvl[0]==tvl[0]&&fvl[1]==tvl[1]&&fvl[2]==tvl[2])return get_face(t,3);
if(fvl[0]==tvl[0]&&fvl[1]==tvl[1]&&fvl[2]==tvl[3])return get_face(t,2);
if(fvl[0]==tvl[0]&&fvl[1]==tvl[2]&&fvl[2]==tvl[3])return get_face(t,1);
if(fvl[0]==tvl[1]&&fvl[1]==tvl[2]&&fvl[2]==tvl[3])return get_face(t,0);
return NULL_ID;
}

simplex_id vertices2trianglex(simplex_id*vl,simplex_id t)
{
simplex_id*tvl= id2tetrahedron(t)->vertex_ids;
simplex_id fvl[3];

if(vl[0]> vl[1])
{
fvl[0]= vl[0];
fvl[1]= vl[1];
}
else
{
fvl[0]= vl[1];
fvl[1]= vl[0];
}

if(vl[2]<fvl[1])fvl[2]= vl[2];
else if(vl[2]<fvl[0])
{
fvl[2]= fvl[1];
fvl[1]= vl[2];
}
else
{
fvl[2]= fvl[1];
fvl[1]= fvl[0];
fvl[0]= vl[2];
}

if(fvl[0]==tvl[0]&&fvl[1]==tvl[1]&&fvl[2]==tvl[2])return get_face(t,3);
if(fvl[0]==tvl[0]&&fvl[1]==tvl[1]&&fvl[2]==tvl[3])return get_face(t,2);
if(fvl[0]==tvl[0]&&fvl[1]==tvl[2]&&fvl[2]==tvl[3])return get_face(t,1);
if(fvl[0]==tvl[1]&&fvl[1]==tvl[2]&&fvl[2]==tvl[3])return get_face(t,0);
return NULL_ID;
}

simplex_id vertices2edge(simplex_id*fvl,simplex_id t)
{
simplex_id*tvl= id2tetrahedron(t)->vertex_ids;
int i,j;

for(i= 0;i<3;i++)
{
if(fvl[0]==tvl[i])
{
for(j= i+1;j<4;j++)
{
if(fvl[1]==tvl[j])return get_edge_of_tetrahedron(t,i,j);
}
}
}
return NULL_ID;
}



/*:12*//*13:*/
#line 345 "./simplex.w"

simplex_id other_edge(simplex_id v,simplex_id e,simplex_id t)

{
simplex_id evl[2],tvl[3];

get_edge_vertices(e,evl);
get_triangle_vertices(t,tvl);

if(evl[0]==tvl[0]&&evl[1]==tvl[1])return(v==evl[0])?get_edge(t,1):get_edge(t,0);
if(evl[0]==tvl[0]&&evl[1]==tvl[2])return(v==evl[0])?get_edge(t,2):get_edge(t,0);
if(evl[0]==tvl[1]&&evl[1]==tvl[2])return(v==evl[0])?get_edge(t,2):get_edge(t,1);
abort_message("edge not in triangle");
}

simplex_id other_face(simplex_id e,simplex_id f,simplex_id t)

{
simplex_id evl[2],fvl[3];
simplex_id*tvl= id2tetrahedron(t)->vertex_ids;
int i,j;

get_edge_vertices(e,evl);
get_triangle_vertices(f,fvl);

if(evl[0]==fvl[0]&&evl[1]==fvl[1])i= 2;
else if(evl[0]==fvl[0]&&evl[1]==fvl[2])i= 1;
else if(evl[0]==fvl[1]&&evl[1]==fvl[2])i= 0;
else abort_message("edge not in triangle");

for(j= 0;j<4;j++)
{
if(tvl[j]==fvl[i])return get_face(t,j);
}
abort_message("triangle not in tetrahedron");
}

/*:13*//*14:*/
#line 383 "./simplex.w"


simplex_id edge_id_in_tetrahedron(simplex_id e,simplex_id t)
{
simplex_id evl[2];
simplex_id*tvl= id2tetrahedron(t)->vertex_ids;

get_edge_vertices(e,evl);

if(evl[0]==tvl[0])
{
if(evl[1]==tvl[1])return t+5;
if(evl[1]==tvl[2])return t+6;
if(evl[1]==tvl[3])return t+7;
abort_message("internal error 23");
}
else if(evl[0]==tvl[1])
{
if(evl[1]==tvl[2])return t+8;
if(evl[1]==tvl[3])return t+9;
abort_message("internal error 24");
}
return t+10;
}


/*:14*//*15:*/
#line 423 "./simplex.w"

simplex_id next_face(simplex_id*vert,simplex_id t)
{
simplex_id*v= id2tetrahedron(t)->vertex_ids;
int i,j;

/*17:*/
#line 479 "./simplex.w"

if(v[0]==vert[0])
{
if(v[1]==vert[2])
{
j= 1;
if(v[2]==vert[1])i= 3;
else i= 2;
}
else if(v[2]==vert[2])
{
j= 2;
if(v[1]==vert[1])i= 3;
else i= 1;
}
else if(v[3]==vert[2])
{
j= 3;
if(v[2]==vert[1])i= 1;
else i= 2;
}
else abort_message("next tetrahedron error 1");
}
else if(v[1]==vert[0])
{
if(v[0]==vert[2])
{
j= 0;
if(v[2]==vert[1])i= 3;
else i= 2;
}
else if(v[2]==vert[2])
{
j= 2;
i= 0;
}
else if(v[3]==vert[2])
{
j= 3;
i= 0;
}
else abort_message("next tetrahedron error 2");
}
else if(v[2]==vert[0])
{
if(v[0]==vert[2])
{
i= 1;
j= 0;
}
else if(v[1]==vert[2])
{
i= 0;
j= 1;
}
else abort_message("next tetrahedron error 3");
}
else abort_message("next tetrahedron error 4");


/*:17*/
#line 429 "./simplex.w"


vert[2]= v[i];
return id2tetrahedron(t)->face_ids[j];
}

/*:15*//*16:*/
#line 445 "./simplex.w"

simplex_id next_edge(simplex_id*vert,simplex_id e)
{
tetrahedron*tt= id2tetrahedron(e);
simplex_id*v= tt->vertex_ids;
simplex_id f;
int i,j;

/*17:*/
#line 479 "./simplex.w"

if(v[0]==vert[0])
{
if(v[1]==vert[2])
{
j= 1;
if(v[2]==vert[1])i= 3;
else i= 2;
}
else if(v[2]==vert[2])
{
j= 2;
if(v[1]==vert[1])i= 3;
else i= 1;
}
else if(v[3]==vert[2])
{
j= 3;
if(v[2]==vert[1])i= 1;
else i= 2;
}
else abort_message("next tetrahedron error 1");
}
else if(v[1]==vert[0])
{
if(v[0]==vert[2])
{
j= 0;
if(v[2]==vert[1])i= 3;
else i= 2;
}
else if(v[2]==vert[2])
{
j= 2;
i= 0;
}
else if(v[3]==vert[2])
{
j= 3;
i= 0;
}
else abort_message("next tetrahedron error 2");
}
else if(v[2]==vert[0])
{
if(v[0]==vert[2])
{
i= 1;
j= 0;
}
else if(v[1]==vert[2])
{
i= 0;
j= 1;
}
else abort_message("next tetrahedron error 3");
}
else abort_message("next tetrahedron error 4");


/*:17*/
#line 453 "./simplex.w"



vert[2]= v[i];
f= tt->face_ids[j];
tt= id2tetrahedron(tt->face_ids[j]);
v= tt->vertex_ids;

if(v[0]==vert[0])
{
if(v[1]==vert[1])return get_edge_of_tetrahedron(f,0,1);
if(v[2]==vert[1])return get_edge_of_tetrahedron(f,0,2);
if(v[3]==vert[1])return get_edge_of_tetrahedron(f,0,3);
abort_message("next tetrahedron error 5");
}
if(v[1]==vert[0])
{
if(v[2]==vert[1])return get_edge_of_tetrahedron(f,1,2);
if(v[3]==vert[1])return get_edge_of_tetrahedron(f,1,3);
abort_message("next tetrahedron error 6");
}
if(v[2]!=vert[0])abort_message("next tetrahedron error 7");
if(v[3]!=vert[1])abort_message("next tetrahedron error 8");
return get_edge_of_tetrahedron(f,2,3);
}

/*:16*//*18:*/
#line 539 "./simplex.w"

#ifdef debug
void print_simp(simplex_id s)
{
simplex_id vl[4];

switch(id_dimension(s))
{
case 0:
printf("vertex %ld",s>>4);
break;
case 1:
get_edge_vertices(s,vl);
printf("edge [%ld,%ld]",vl[0]>>4,vl[1]>>4);
break;
case 2:
get_triangle_vertices(s,vl);
printf("triangle [%ld,%ld,%ld]",vl[0]>>4,vl[1]>>4,vl[2]>>4);
break;
case 3:
get_tetrahedron_vertices(s,vl);
printf("tetrahedron [%ld,%ld,%ld,%ld]",vl[0]>>4,vl[1]>>4,vl[2]>>4,vl[3]>>4);
break;
default:
abort_message("Unknown simplex dimension");
}
}
#endif



/*:18*//*21:*/
#line 587 "./simplex.w"


SInt32 value(simplex_id t)
{
if((t&15)<11)
return vertex_value(id2tetrahedron(t)->vertex_ids[id_val_tab[t&15]]);
else if((t&15)> 11)abort_message("invalid id");
return vertex_value(t);
}


/*:21*//*22:*/
#line 599 "./simplex.w"


int smallest_vertex(simplex_id*v,int n)
{
int i,j;
for(i= 0,j= 1;j<n;j++)
{
if(vertex_compare(v[j],v[i])<0)i= j;
}
return i;
}

int largest_vertex(simplex_id*v,int n)
{
int i,j;
for(i= 0,j= 1;j<n;j++)
{
if(vertex_compare(v[j],v[i])> 0)i= j;
}
return i;
}

/*:22*//*24:*/
#line 630 "./simplex.w"

SInt32 max_value(simplex_id s,int d)
{
SInt32 m;
tetrahedron*tp;

if(id_is_null(s))return-0x8000;

switch(d)
{
case 3:
tp= id2tetrahedron(s);
m= value(tp->vertex_ids[0]);
break;
case 2:
tp= id2tetrahedron(s);
m= (face_index(s)==0)?value(tp->vertex_ids[1]):value(tp->vertex_ids[0]);
break;
case 1:
m= value(get_vertex(s,0));
break;
default:
abort_message("max value of simplex of unknown dimension");
}
return m;
}

/*:24*//*25:*/
#line 658 "./simplex.w"

SInt32 min_value(simplex_id s,int d)
{
SInt32 m;
tetrahedron*tp;

if(id_is_null(s))return 0x7fff;

switch(d)
{
case 3:
tp= id2tetrahedron(s);
m= value(tp->vertex_ids[3]);
break;
case 2:
tp= id2tetrahedron(s);
m= (face_index(s)==3)?value(tp->vertex_ids[2]):value(tp->vertex_ids[3]);
break;
case 1:
m= value(get_vertex(s,1));
break;
default:
abort_message("max value of simplex of unknown dimension");
}
return m;

}

/*:25*//*26:*/
#line 688 "./simplex.w"

double alternate_vertex_value(simplex_id v0,simplex_id v1,double*length)
{
GLfloat*u0,*u1,d,h;

u0= id2vertex(v0)->coords;
u1= id2vertex(v1)->coords;
d= (u0[0]-u1[0])*(u0[0]-u1[0])+(u0[1]-u1[1])*(u0[1]-u1[1])+(u0[2]-u1[2])*(u0[2]-u1[2]);
d= sqrt(d);
if(length!=NULL)*length= d;
h= value(v1)-value(v0);
return h/d;
}

/*:26*//*27:*/
#line 706 "./simplex.w"

double cross_length(simplex_id v0,simplex_id v1,simplex_id v2)
{
GLfloat*u2,*u0,*u1,c[3];

u2= id2vertex(v2)->coords;
u0= id2vertex(v0)->coords;
u1= id2vertex(v1)->coords;

c[0]= (u1[1]-u0[1])*(u2[2]-u0[2])-(u1[2]-u0[2])*(u2[1]-u0[1]);
c[1]= (u1[2]-u0[2])*(u2[0]-u0[0])-(u1[0]-u0[0])*(u2[2]-u0[2]);
c[2]= (u1[0]-u0[0])*(u2[1]-u0[1])-(u1[1]-u0[1])*(u2[0]-u0[0]);

return sqrt(c[0]*c[0]+c[1]*c[1]+c[2]*c[2]);
}

/*:27*//*28:*/
#line 723 "./simplex.w"


double alternate_edge_value_basic(simplex_id e0,simplex_id e1)
{
double l0,l1,h0,h1,c;
simplex_id v,v0,v1;

v= get_vertex(e0,0);
if(v!=get_vertex(e1,0))abort_message("alternate_edge_value error");
v0= get_vertex(e0,1);
v1= get_vertex(e1,1);
h0= alternate_vertex_value(v,v0,&l0);
h1= alternate_vertex_value(v,v1,&l1);

c= cross_length(v,v0,v1);

return(h1-h0)*l0*l1/c;
}

/*:28*//*29:*/
#line 746 "./simplex.w"


double alternate_edge_value(simplex_id e0,simplex_id v1,double e0_value,double e0_length)
{
double l1,h1,c;
simplex_id v,v0;

v= get_vertex(e0,0);
v0= get_vertex(e0,1);
h1= alternate_vertex_value(v,v1,&l1);

c= cross_length(v,v0,v1);

return(h1-e0_value)*e0_length*l1/c;
}



/*:29*//*31:*/
#line 777 "./simplex.w"





Boolean triangle_is_deadend(simplex_id*t)
{
tetrahedron*tp;

*t= best_face_id(*t);

tp= id2tetrahedron(*t);
return(tp->types[(*t)&15]&64)!=0;
}

Boolean tetrahedron_is_deadend(simplex_id t)
{
tetrahedron*tp;

tp= id2tetrahedron(t);
return(tp->types[0]&64)!=0;
}

void make_edge_deadend(simplex_id*e)
{
simplex_id vlist[3];
simplex_id t,ee;

t= id2tetra_id(*e);
get_edge_vertices(*e,vlist);
vlist[2]= id2tetrahedron(t)->vertex_ids[best_edge_tab[edge_index(*e)]];

ee= *e;
do
{
id2tetrahedron(ee)->types[ee&15]|= 64;
ee= next_edge(vlist,ee);
}while(ee!=*e);
}

void make_triangle_deadend(simplex_id*t)
{
tetrahedron*tp;

*t= best_face_id(*t);

tp= id2tetrahedron(*t);
tp->types[(*t)&15]|= 64;
}

void make_tetrahedron_deadend(simplex_id t)
{
tetrahedron*tp;

tp= id2tetrahedron(t);
tp->types[0]|= 64;
}

Boolean edge_is_xdeadend(simplex_id*e)
{
tetrahedron*tp;

*e= best_edge_id(*e);

tp= id2tetrahedron(*e);
return(tp->types[(*e)&15]&32)!=0;
}

Boolean triangle_is_xdeadend(simplex_id*t)
{
tetrahedron*tp;

*t= best_face_id(*t);

tp= id2tetrahedron(*t);
return(tp->types[(*t)&15]&32)!=0;
}

void make_edge_xdeadend(simplex_id*e)
{
tetrahedron*tp;

tp= id2tetrahedron(*e);
tp->types[(*e)&15]|= 32;

*e= best_edge_id(*e);

tp= id2tetrahedron(*e);
tp->types[(*e)&15]|= 32;
}

void make_triangle_xdeadend(simplex_id*t)
{
tetrahedron*tp;

*t= best_face_id(*t);

tp= id2tetrahedron(*t);
tp->types[(*t)&15]|= 32;
}



/*:31*//*33:*/
#line 888 "./simplex.w"

int edge_in_triangle(simplex_id e,simplex_id t)
{
simplex_id evl[2],tvl[3];

get_edge_vertices(e,evl);
get_triangle_vertices(t,tvl);
if(evl[0]==tvl[0]&&evl[1]==tvl[1])return 2;
if(evl[0]==tvl[0]&&evl[1]==tvl[2])return 1;
if(evl[0]==tvl[1]&&evl[1]==tvl[2])return 0;
return-1;
}

int edge_verts_in_triangle(simplex_id*evl,simplex_id f)
{
simplex_id*p= id2tetrahedron(f)->vertex_ids;

switch(face_index(f))
{
case 0:
if(evl[0]==p[1]&&evl[1]==p[2])return 2;
if(evl[0]==p[1]&&evl[1]==p[3])return 1;
if(evl[0]==p[2]&&evl[1]==p[3])return 0;
break;
case 1:
if(evl[0]==p[0]&&evl[1]==p[2])return 2;
if(evl[0]==p[0]&&evl[1]==p[3])return 1;
if(evl[0]==p[2]&&evl[1]==p[3])return 0;
break;
case 2:
if(evl[0]==p[0]&&evl[1]==p[1])return 2;
if(evl[0]==p[0]&&evl[1]==p[3])return 1;
if(evl[0]==p[1]&&evl[1]==p[3])return 0;
break;
case 3:
if(evl[0]==p[0]&&evl[1]==p[1])return 2;
if(evl[0]==p[0]&&evl[1]==p[2])return 1;
if(evl[0]==p[1]&&evl[1]==p[2])return 0;
break;
}
return-1;
}

int triangle_in_tetrahedron(simplex_id f,simplex_id t)
{
simplex_id fvl[3];
simplex_id*tvl= id2tetrahedron(t)->vertex_ids;

if(t==id2tetra_id(f))return(face_index(f));

get_triangle_vertices(f,fvl);

if(fvl[0]==tvl[0]&&fvl[1]==tvl[1]&&fvl[2]==tvl[2])return 3;
if(fvl[0]==tvl[0]&&fvl[1]==tvl[1]&&fvl[2]==tvl[3])return 2;
if(fvl[0]==tvl[0]&&fvl[1]==tvl[2]&&fvl[2]==tvl[3])return 1;
if(fvl[0]==tvl[1]&&fvl[1]==tvl[2]&&fvl[2]==tvl[3])return 0;
return-1;
}

/*:33*//*34:*/
#line 953 "./simplex.w"

Boolean edges_equal(simplex_id e0,simplex_id e1)
{
simplex_id v0[2],v1[2];

if(e0==e1)return true;
if(id_is_null(e1)||id_is_null(e0))return false;
get_edge_vertices(e0,v0);
get_edge_vertices(e1,v1);
return(v0[0]==v1[0]&&v0[1]==v1[1]);
}

Boolean triangles_equal(simplex_id f0,simplex_id f1)
{
tetrahedron*tp;
simplex_id f;
int n;

if(f0==f1)return true;
if(id_is_null(f1)||id_is_null(f0))return false;

tp= id2tetrahedron(f1);
f= tp->face_ids[face_index(f1)];
return f0==f;
}



/*:34*//*36:*/
#line 990 "./simplex.w"


simplex_id id_list_pop(list*l)
{
simplex_id t;

list_pop(l,&t);
return t;
}

simplex_id id_list_read(list*l)
{
simplex_id*r;
r= (simplex_id*)list_read(l);
if(r!=NULL)return*r;
else return 0xffffffff;
}




/*:36*//*38:*/
#line 1036 "./simplex.w"


Boolean edge_is_critical(simplex_id*e)
{
tetrahedron*t;
*e= best_edge_id(*e);
t= id2tetrahedron(*e);
return(id2tetrahedron(*e)->types[(*e)&15]&2)!=0;
}

Boolean triangle_is_critical(simplex_id*t)
{
tetrahedron*tt;
*t= best_face_id(*t);
tt= id2tetrahedron(*t);

return(id2tetrahedron(*t)->types[(*t)&15]&2)!=0;
}


/*:38*//*39:*/
#line 1056 "./simplex.w"

void make_vertex_critical(simplex_id v)
{



id2vertex(v)->type|= 2;
id_list_push(crit[0],v);
}

void make_edge_critical(simplex_id*e)
{



*e= best_edge_id(*e);
id2tetrahedron(*e)->types[(*e)&15]|= 2;
list_push(crit[1],e);
}

void make_triangle_critical(simplex_id*f)
{



*f= best_face_id(*f);
id2tetrahedron(*f)->types[(*f)&15]|= 2;
list_push(crit[2],f);
}

void make_tetrahedron_critical(simplex_id t)
{



id2tetrahedron(t)->types[(t)&15]|= 2;
id_list_push(crit[3],t);
}

/*:39*//*40:*/
#line 1095 "./simplex.w"


void unmake_edge_critical(simplex_id*e)
{
*e= best_edge_id(*e);
id2tetrahedron(*e)->types[(*e)&15]&= 0xe1;
}

void unmake_triangle_critical(simplex_id*f)
{
*f= best_face_id(*f);
id2tetrahedron(*f)->types[(*f)&15]&= 0x81;
}


void clean_crit(void)
{
simplex_id*s;
list*l;

l= crit[0];
list_read_init(l);
while((s= list_read(l))!=NULL)
{
if(!vertex_is_critical(*s))list_read_delete(l);
}
l= crit[1];
list_read_init(l);
while((s= list_read(l))!=NULL)
{
if(!edge_is_critical(s))list_read_delete(l);
}
l= crit[2];
list_read_init(l);
while((s= list_read(l))!=NULL)
{
if(!triangle_is_critical(s))list_read_delete(l);
}
l= crit[3];
list_read_init(l);
while((s= list_read(l))!=NULL)
{
if(!tetrahedron_is_critical(*s))list_read_delete(l);
}
}

void clean_crit_at(UInt32 at[4])
{
simplex_id*s;
list*l;

l= crit[0];
list_read_init_at(l,at[0]);
while((s= list_read(l))!=NULL)
{
if(!vertex_is_critical(*s))list_read_delete(l);
}
l= crit[1];
list_read_init_at(l,at[1]);
while((s= list_read(l))!=NULL)
{
if(!edge_is_critical(s))list_read_delete(l);
}
l= crit[2];
list_read_init_at(l,at[2]);
while((s= list_read(l))!=NULL)
{
if(!triangle_is_critical(s))list_read_delete(l);
}
l= crit[3];
list_read_init_at(l,at[3]);
while((s= list_read(l))!=NULL)
{
if(!tetrahedron_is_critical(*s))list_read_delete(l);
}
}

/*:40*//*42:*/
#line 1186 "./simplex.w"


void unmake_edge_valid(simplex_id*e)
{
*e= best_edge_id(*e);
id2tetrahedron(*e)->types[(*e)&15]&= 0x7f;
}

void unmake_triangle_valid(simplex_id*f)
{
*f= best_face_id(*f);
id2tetrahedron(*f)->types[(*f)&15]&= 0x7f;
}


void mark_tetrahedron_done(simplex_id v,simplex_id t)
{
tetrahedron*tp;
int i;
tp= id2tetrahedron(t);
for(i= 0;i<4;i++)
{
if(v==tp->vertex_ids[i])
{
tp->flag|= (1<<i);
break;
}
}
}

Boolean tetrahedron_marked_done(simplex_id v,simplex_id t)
{
tetrahedron*tp;
int i,j= 0;
tp= id2tetrahedron(t);
for(i= 0;i<4;i++)
{
if(v==tp->vertex_ids[i])
{
j= (tp->flag&(1<<i));
break;
}
}
return j!=0;
}

/*:42*//*44:*/
#line 1239 "./simplex.w"

simplex_id best_edge_idx(simplex_id e)
{
simplex_id vlist[3];
simplex_id ee;

get_edge_vertices(e,vlist);
vlist[2]= id2tetrahedron(e)->vertex_ids[best_edge_tab[edge_index(e)]];

for(ee= e;!edge_is_valid(ee);)
{
ee= next_edge(vlist,ee);
if(ee==e)abort_message("internal error 25");
}

return ee;
}




/*:44*//*46:*/
#line 1288 "./simplex.w"


Boolean edge_is_paired_up(simplex_id*e)
{
*e= best_edge_id(*e);
return(id2tetrahedron(*e)->types[(*e)&15]&6)==0;
}

Boolean triangle_is_paired_up(simplex_id*f)
{
*f= best_face_id(*f);
return(id2tetrahedron(*f)->types[(*f)&15]&6)==0;
}

Boolean edge_is_paired_down(simplex_id*e)
{
*e= best_edge_id(*e);
return(id2tetrahedron(*e)->types[(*e)&15]&6)==4;
}

Boolean eipdx(simplex_id e)
{
simplex_id vl[2],e1;

get_edge_vertices(e,vl);

if(vertex_is_paired_up(vl[0]))
{
e1= r01(vl[0]);
if(vl[1]==get_vertex(e1,1))return true;
}
if(vertex_is_paired_up(vl[1]))
{
e1= r01(vl[1]);
if(vl[0]==get_vertex(e1,0))return true;
}
return false;
}

Boolean triangle_is_paired_down(simplex_id*f)
{
*f= best_face_id(*f);
return(id2tetrahedron(*f)->types[(*f)&15]&6)==4;
}


simplex_id r21(simplex_id*f)
{
*f= best_face_id(*f);
return get_edge(*f,(id2tetrahedron(*f)->types[(*f)&15]>>3)&3);
}

simplex_id r10(simplex_id*e)
{
*e= best_edge_id(*e);
return get_vertex(*e,(id2tetrahedron(*e)->types[(*e)&15]>>3)&1);
}

simplex_id r23(simplex_id*f)
{
*f= best_face_id(*f);
return id2tetra_id(*f);
}

simplex_id r12(simplex_id*e)
{
*e= best_edge_id(*e);
return get_face(id2tetra_id(*e),(id2tetrahedron(*e)->types[(*e)&15]>>3)&3);
}



/*:46*//*47:*/
#line 1360 "./simplex.w"

void pair01(simplex_id s0,simplex_id s1,Boolean flag)
{
simplex_id s;








if(flag)
{
s= s1;
unmake_edge_valid(&s);
}

id2vertex(s0)->tetra_id= id2tetra_id(s1);
id2vertex(s0)->type= (UInt8)(1+(edge_index(s1)<<3));
id2tetrahedron(s1)->types[s1&15]= (UInt8)((s0==get_vertex(s1,0))?5+128:5+128+8);
}

void pair12(simplex_id*s0,simplex_id s1,Boolean flag)
{
int n;
tetrahedron*tp;
simplex_id s;








if(flag)
{
s= s1;
unmake_triangle_valid(&s);
s= *s0;
unmake_edge_valid(&s);
}

n= edge_in_triangle(*s0,s1);
tp= id2tetrahedron(s1);

tp->types[s1&15]= (UInt8)(5+128+(n<<3));
*s0= get_edge(s1,n);
tp->types[(*s0)&15]= (UInt8)(1+128+(face_index(s1)<<3));
}

void pair23(simplex_id*s0,simplex_id s1,Boolean flag)
{
int n;
tetrahedron*tp;
simplex_id s;







if(flag)
{
s= *s0;
unmake_triangle_valid(&s);
}
n= triangle_in_tetrahedron(*s0,s1);
tp= id2tetrahedron(s1);
*s0= get_face(s1,n);

tp->types[0]= (UInt8)(1+128+(n<<3));
tp->types[(*s0)&15]= 1+128;
}

/*:47*//*49:*/
#line 1447 "./simplex.w"


list*vertex_star(simplex_id v)
{
hlist*l;
list*tl;
int i;
vertex*vv;

hlist_initialize(&l,sizeof(tetrahedron*),4,2,0);
list_initialize(&tl,sizeof(simplex_id));
vv= id2vertex(v);
if(id_is_null(vv->tetra_id))
/*52:*/
#line 1520 "./simplex.w"

{
UInt32 j;
tetrahedron*tt;

for(j= 0;j<number_of_tetrahedra;j++)
{
tt= id2tetrahedron(tetrahedron_id(j));
if(tt->vertex_ids[3]==v)
{
vv->tetra_id= tetrahedron_id(j);
break;
}
}
if(j==number_of_tetrahedra)abort_message("internal error: tetra_id");
}

/*:52*/
#line 1460 "./simplex.w"

/*50:*/
#line 1468 "./simplex.w"

{
tetrahedron*tt;
simplex_id t;
SInt32 m[2];
Boolean flag;

list_push(tl,&(vv->tetra_id));
while(!list_is_empty(tl))
{
list_pop(tl,m+1);
tt= id2tetrahedron(m[1]);
m[1]= id2tetra_id(m[1]);
for(i= 0;i<4;i++)
{
if(tt->vertex_ids[i]==v){m[0]= i;break;}
}
hlist_find_add(l,m,&tt,&flag);
if(!flag)
{
for(i= 0;i<4;i++)
{
if(tt->vertex_ids[i]!=v)
{
list_push(tl,tt->face_ids+i);
}
}
}
}
}

/*:50*/
#line 1461 "./simplex.w"

for(i= 0;i<4;i++)
/*51:*/
#line 1499 "./simplex.w"

{
SInt32 m[2];

m[0]= i;

while(hlist_sub_match(l,m,NULL,1))
{
list_push(tl,m+1);
}
}

/*:51*/
#line 1463 "./simplex.w"

hlist_abandon(&l);
return tl;
}

/*:49*//*106:*/
#line 3052 "./simplex.w"

void glue_faces(simplex_id f0,simplex_id f1)
{
simplex_id v0[3];
simplex_id v1[3];

if(id2tetra_id(f0)==id2tetra_id(f1))abort_message("err101");
get_triangle_vertices(f0,v0);
get_triangle_vertices(f1,v1);

if(v0[0]!=v1[0])abort_message("err102");
if(v0[1]!=v1[1])abort_message("err102");
if(v0[2]!=v1[2])abort_message("err102");

glue_face(f0,f1);
glue_face(f1,f0);
}


/*:106*/
#line 20 "./simplex.w"

/*53:*/
#line 1544 "./simplex.w"

void complex_check(simplex_id w)
{
hlist*edges,*faces;
SInt32 i,j,k;
simplex_id v,e,f,t,vlist[4];

hlist_initialize(&edges,sizeof(simplex_id),12713,2,0);
hlist_initialize(&faces,sizeof(simplex_id),12713,3,0);

for(i= (w>>4);i<number_of_vertices;i++)
{
v= vertex_id(i);
if(vertex_is_in_K(v))
/*54:*/
#line 1584 "./simplex.w"

{
if(!vertex_is_critical(v))
{
e= r01(v);
/*58:*/
#line 1625 "./simplex.w"

{
Boolean flag;
SInt32 index;

if(id_dimension(e)!=1)abort_message("edge not an edge");
if(!edge_is_valid(e))abort_message("non-valid edge");
get_edge_vertices(e,vlist);
index= hlist_find_add(edges,vlist,&e,&flag);
if(flag)
{
if(e!=*((simplex_id*)hlist_entry(edges,index)))abort_message("edge has two valid ids");
}
}

/*:58*/
#line 1589 "./simplex.w"

/*56:*/
#line 1613 "./simplex.w"

{
if(!edge_is_paired_down(&e))abort_message("should be paired-down edge");
if(v!=r10(&e))abort_message("nonsymmetric pairing");
}

/*:56*/
#line 1590 "./simplex.w"

}
}

/*:54*/
#line 1558 "./simplex.w"

}

for(i= 0;i<number_of_tetrahedra;i++)
{
t= tetrahedron_id(i);
get_tetrahedron_vertices(t,vlist);
if(vlist[0]<w)continue;
for(j= 0;j<4;j++)
{
f= get_face(t,j);
if(triangle_is_in_K(f)&&(j> 0||vlist[1]>=w))
/*59:*/
#line 1640 "./simplex.w"

{
simplex_id t1;
tetrahedron*ttt,*tttt;

f= best_face_id(f);

ttt= id2tetrahedron(f);
tttt= id2tetrahedron(t);

/*62:*/
#line 1680 "./simplex.w"

{
Boolean flag;
SInt32 index;

if(id_dimension(f)!=2)abort_message("face not a face");
if(!triangle_is_valid(f))abort_message("non-valid face");
get_triangle_vertices(f,vlist);
index= hlist_find_add(faces,vlist,&f,&flag);
if(flag)
{
if(f!=*((simplex_id*)hlist_entry(faces,index)))abort_message("face has two valid ids");
}
}

/*:62*/
#line 1650 "./simplex.w"

if(triangle_is_paired_up(&f))
{
t1= r23(&f);
/*63:*/
#line 1695 "./simplex.w"

{
if(!tetrahedron_is_paired_down(t1))abort_message("should be paired-down tetrahedron");
if(f!=r32(t1))abort_message("nonsymmetric pairing");
}



/*:63*/
#line 1654 "./simplex.w"

/*61:*/
#line 1673 "./simplex.w"

{
if(!triangle_is_paired_up(&f))abort_message("should be paired-up face");
if(t1!=r23(&f))abort_message("nonsymmetric pairing");
}


/*:61*/
#line 1655 "./simplex.w"

}
else if(triangle_is_paired_down(&f))
{
e= r21(&f);
/*58:*/
#line 1625 "./simplex.w"

{
Boolean flag;
SInt32 index;

if(id_dimension(e)!=1)abort_message("edge not an edge");
if(!edge_is_valid(e))abort_message("non-valid edge");
get_edge_vertices(e,vlist);
index= hlist_find_add(edges,vlist,&e,&flag);
if(flag)
{
if(e!=*((simplex_id*)hlist_entry(edges,index)))abort_message("edge has two valid ids");
}
}

/*:58*/
#line 1660 "./simplex.w"

/*57:*/
#line 1619 "./simplex.w"

{
if(!edge_is_paired_up(&e))abort_message("should be paired-up edge");
if(f!=r12(&e))abort_message("nonsymmetric pairing");
}

/*:57*/
#line 1661 "./simplex.w"

/*60:*/
#line 1667 "./simplex.w"

{
if(!triangle_is_paired_down(&f))abort_message("should be paired-down face");
if(e!=r21(&f))abort_message("nonsymmetric pairing");
}

/*:60*/
#line 1662 "./simplex.w"

}
else if(!triangle_is_critical(&f))abort_message("nothing triangle");
}

/*:59*/
#line 1570 "./simplex.w"

for(k= j+1;k<4;k++)
{
e= get_edge_of_tetrahedron(t,j,k);
if(edge_is_in_K(e)&&vlist[j]>=w)
/*55:*/
#line 1594 "./simplex.w"

{
e= best_edge_id(e);
/*58:*/
#line 1625 "./simplex.w"

{
Boolean flag;
SInt32 index;

if(id_dimension(e)!=1)abort_message("edge not an edge");
if(!edge_is_valid(e))abort_message("non-valid edge");
get_edge_vertices(e,vlist);
index= hlist_find_add(edges,vlist,&e,&flag);
if(flag)
{
if(e!=*((simplex_id*)hlist_entry(edges,index)))abort_message("edge has two valid ids");
}
}

/*:58*/
#line 1597 "./simplex.w"

if(edge_is_paired_up(&e))
{
f= r12(&e);
/*62:*/
#line 1680 "./simplex.w"

{
Boolean flag;
SInt32 index;

if(id_dimension(f)!=2)abort_message("face not a face");
if(!triangle_is_valid(f))abort_message("non-valid face");
get_triangle_vertices(f,vlist);
index= hlist_find_add(faces,vlist,&f,&flag);
if(flag)
{
if(f!=*((simplex_id*)hlist_entry(faces,index)))abort_message("face has two valid ids");
}
}

/*:62*/
#line 1601 "./simplex.w"

/*57:*/
#line 1619 "./simplex.w"

{
if(!edge_is_paired_up(&e))abort_message("should be paired-up edge");
if(f!=r12(&e))abort_message("nonsymmetric pairing");
}

/*:57*/
#line 1602 "./simplex.w"

/*60:*/
#line 1667 "./simplex.w"

{
if(!triangle_is_paired_down(&f))abort_message("should be paired-down face");
if(e!=r21(&f))abort_message("nonsymmetric pairing");
}

/*:60*/
#line 1603 "./simplex.w"

}
else if(edge_is_paired_down(&e))
{
v= r10(&e);
/*56:*/
#line 1613 "./simplex.w"

{
if(!edge_is_paired_down(&e))abort_message("should be paired-down edge");
if(v!=r10(&e))abort_message("nonsymmetric pairing");
}

/*:56*/
#line 1608 "./simplex.w"

}
else if(!edge_is_critical(&e))abort_message("nothing edge");
}

/*:55*/
#line 1575 "./simplex.w"

}
}
}

hlist_abandon(&edges);
hlist_abandon(&faces);
}

/*:53*//*65:*/
#line 1712 "./simplex.w"

vertex*new_vertex(UInt32 n,SInt16 val,float x,float y,float z,Boolean flag)
{
vertex*v;

v= id2vertex(vertex_id(n));

v->type= (UInt8)((flag)?1:0);
v->tetra_id= NULL_ID;
v->h= val;
v->coords[0]= x;
v->coords[1]= y;
v->coords[2]= z;

return v;
}



/*:65*//*66:*/
#line 1731 "./simplex.w"

tetrahedron*new_tetrahedron(UInt32 n,UInt32*vs,Boolean flag)
{
tetrahedron*t;
int i;
Boolean flagp;

t= id2tetrahedron(tetrahedron_id(n));

t->flag= 0;

flagp= flag;

for(i= 0;i<4;i++)
{
t->vertex_ids[i]= vertex_id(vs[i]);
flagp= flagp&&vertex_is_in_K(t->vertex_ids[i]);
t->face_ids[i]= NULL_ID;
}
t->types[0]= (UInt8)((flagp)?1:0);

/*67:*/
#line 1761 "./simplex.w"

{
simplex_id temp;
int j;

for(j= 0;j<3;j++)
{
for(i= j+1;i<4;i++)
{
if(vertex_compare(t->vertex_ids[j],t->vertex_ids[i])<0)
{
temp= t->vertex_ids[j];
t->vertex_ids[j]= t->vertex_ids[i];
t->vertex_ids[i]= temp;
}
}
}
}

/*:67*/
#line 1752 "./simplex.w"


/*68:*/
#line 1780 "./simplex.w"

{
int j;

for(i= 0;i<4;i++)
{
flagp= true;
for(j= 0;j<4;j++)
{
if(i==j)continue;
flagp= flagp&&vertex_is_in_K(t->vertex_ids[j]);
}
t->types[1+i]= (UInt8)((flagp)?1:0);
}
}

/*:68*/
#line 1754 "./simplex.w"


/*69:*/
#line 1796 "./simplex.w"

{
int j;

for(i= 0;i<3;i++)
{
for(j= i+1;j<4;j++)
{
flagp= vertex_is_in_K(t->vertex_ids[i])&&vertex_is_in_K(t->vertex_ids[j]);
t->types[etab2[i][j]]= (UInt8)((flagp)?1:0);
}
}
}



/*:69*/
#line 1756 "./simplex.w"


return t;
}

/*:66*//*71:*/
#line 1825 "./simplex.w"

/*72:*/
#line 1843 "./simplex.w"

list*vertex_blocks;
int vertex_sort(const void*nn,const void*mm);

/*:72*//*86:*/
#line 2195 "./simplex.w"

hlist*unglued_faces;
hlist*local_unglued_faces;
hlist*lone_faces;
hlist*local_lone_faces;
hlist*possible_lone_edges;
struct star_component
{
simplex_id end_face[2];
simplex_id end_vert[2];
SInt32 index[2];
simplex_id bestv;
double bestv_value;
};


/*:86*/
#line 1826 "./simplex.w"

void
read_in_complex(FILE*dfv,FILE*dft,double min,double max,Boolean qdelaunay,
double scalex,double scaley,double scalez,
Boolean optional_link_values)
{
UInt32 i;
UInt32*translation;

if(max<=min)abort_message("max is less than min");
/*73:*/
#line 1847 "./simplex.w"

{
float actual_min,actual_max;
UInt32*translation1;

list_initialize(&vertex_blocks,sizeof(float*));
number_of_vertices= 0;

/*74:*/
#line 1872 "./simplex.w"

{
float x,y,z,h;
int n;
int c;
float*p;

do
{
c= getc(dfv);
if(c==EOF)break;
if(c!='x')
{
(void)ungetc(c,dfv);
n= fscanf(dfv,"%f %f %f %f\n",&x,&y,&z,&h);
if(n!=4)abort_message("bad vertex file");
if(number_of_vertices==0)actual_min= actual_max= h;
else if(h<actual_min)actual_min= h;
else if(h> actual_max)actual_max= h;
x/= scalex;
y/= scaley;
z/= scalez;
}
else
{
/*75:*/
#line 1907 "./simplex.w"

{
do
{
c= getc(dfv);
}
while(c!=EOF&&c!='\n'&&c!='\r');
}

/*:75*/
#line 1897 "./simplex.w"

x= y= z= h= PLUS_INFINITY;
}
/*76:*/
#line 1916 "./simplex.w"

{
if((number_of_vertices&0xff)==0)
{
p= (float*)malloc(256*4*sizeof(float));
if(p==NULL)abort_message("out of memory");
list_push(vertex_blocks,&p);
}
*p++= h;
*p++= x;
*p++= y;
*p++= z;
}

/*:76*/
#line 1900 "./simplex.w"


number_of_vertices++;
}
while(true);
}

/*:74*/
#line 1855 "./simplex.w"

vertexlist= (vertex*)malloc((number_of_vertices)*sizeof(vertex));
if(vertexlist==NULL)abort_message("Out of memory");
translation1= (UInt32*)malloc((number_of_vertices)*sizeof(UInt32));
if(translation1==NULL)abort_message("Out of memory");
for(i= 0;i<number_of_vertices;i++)translation1[i]= i;
/*77:*/
#line 1930 "./simplex.w"

qsort(translation1,number_of_vertices,sizeof(UInt32),vertex_sort);

/*:77*/
#line 1861 "./simplex.w"

for(i= 0;i<number_of_vertices;i++)
/*79:*/
#line 1950 "./simplex.w"

{
float*p;
SInt16 val;

p= *(float**)list_entry(vertex_blocks,translation1[i]>>8);
p+= ((translation1[i]&0xff)*4);

if(*p<min)val= -32768;
else if(*p> max)val= 32767;
else val= (SInt32)(-32768.0+65535.0*(*p-min)/(max-min));

new_vertex(i,val,p[1],p[2],p[3],*p!=PLUS_INFINITY);
}



/*:79*/
#line 1863 "./simplex.w"

list_abandon(&vertex_blocks);
translation= (UInt32*)malloc((number_of_vertices)*sizeof(UInt32));
if(translation==NULL)abort_message("Out of memory");

for(i= 0;i<number_of_vertices;i++)translation[translation1[i]]= i;
free(translation1);
}

/*:73*/
#line 1836 "./simplex.w"

/*80:*/
#line 1967 "./simplex.w"

{
/*81:*/
#line 1975 "./simplex.w"

{
long vi[4];
int c;
int n;

if(qdelaunay)
{
fscanf(dft,"%li\n",vi);
number_of_tetrahedra= vi[0]+1;
}
else
{
number_of_tetrahedra= 0;

do
{
/*83:*/
#line 2024 "./simplex.w"

{
if(qdelaunay&&i==0)
{

vi[0]= number_of_vertices-4;
vi[1]= number_of_vertices-3;
vi[2]= number_of_vertices-2;
vi[3]= number_of_vertices-1;
c= 'x';
n= 4;
}
else
{
c= getc(dft);
if(c!='x')(void)ungetc(c,dft);
n= fscanf(dft,"%li %li %li %li\n",vi,vi+1,vi+2,vi+3);
}
}


/*:83*/
#line 1992 "./simplex.w"

if(n!=4)break;
number_of_tetrahedra++;
}while(true);
rewind(dft);
}
}

/*:81*/
#line 1969 "./simplex.w"

tlist= (tetrahedron*)malloc(number_of_tetrahedra*sizeof(tetrahedron));
if(tlist==NULL)abort_message("Out of memory");
/*82:*/
#line 2000 "./simplex.w"

{
long vi[4];
int c;
UInt32 vn[4];
int j,n;
tetrahedron*t;

for(i= 0;i<number_of_tetrahedra;i++)
{
/*83:*/
#line 2024 "./simplex.w"

{
if(qdelaunay&&i==0)
{

vi[0]= number_of_vertices-4;
vi[1]= number_of_vertices-3;
vi[2]= number_of_vertices-2;
vi[3]= number_of_vertices-1;
c= 'x';
n= 4;
}
else
{
c= getc(dft);
if(c!='x')(void)ungetc(c,dft);
n= fscanf(dft,"%li %li %li %li\n",vi,vi+1,vi+2,vi+3);
}
}


/*:83*/
#line 2010 "./simplex.w"

for(j= 0;j<4;j++)vn[j]= translation[vi[j]];
t= new_tetrahedron(i,vn,c!='x');
/*84:*/
#line 2067 "./simplex.w"

{
simplex_id*tp= &(id2vertex(t->vertex_ids[0])->tetra_id);
tetrahedron*tpt;
SInt32 res;
double best_val,tree_val;

if(optional_link_values)
{
if(t->types[0]&1)
/*85:*/
#line 2115 "./simplex.w"

{
double vals[3];

vals[0]= alternate_vertex_value(t->vertex_ids[0],t->vertex_ids[1],NULL);
vals[1]= alternate_vertex_value(t->vertex_ids[0],t->vertex_ids[2],NULL);
vals[2]= alternate_vertex_value(t->vertex_ids[0],t->vertex_ids[3],NULL);

if(vals[0]>=vals[1])
{
if(vals[0]>=vals[2])
{
if(vals[1]>=vals[2])t->flag= 0x39;
else t->flag= 0x2d;
}
else t->flag= 0x27;
}
else
{
if(vals[1]>=vals[2])
{
if(vals[0]>=vals[2])t->flag= 0x36;
else t->flag= 0x1e;
}
else t->flag= 0x1b;
}

best_val= vals[(t->flag&3)-1];
}

/*:85*/
#line 2077 "./simplex.w"

else
{

t->flag= 0x39;
best_val= PLUS_INFINITY;
}
}
else
t->flag= 0x39;

while(*tp!=NULL_ID)
{
tpt= id2tetrahedron(*tp);
if(tpt->vertex_ids[tpt->flag&3]==t->vertex_ids[t->flag&3])
break;
if(optional_link_values)
{
tree_val= alternate_vertex_value(t->vertex_ids[0],
tpt->vertex_ids[tpt->flag&3],NULL);
if(tree_val<best_val)res= 1;
else if(tree_val> best_val)res= -1;
else res= t->vertex_ids[t->flag&3]-tpt->vertex_ids[tpt->flag&3];
}
else
res= vertex_compare(t->vertex_ids[1],tpt->vertex_ids[1]);
if(res<0)tp= tpt->face_ids;
else if(res> 0)tp= tpt->face_ids+1;
else abort_message("Huh?");
}
if(*tp!=NULL_ID)
{
tp= tpt->face_ids+2;
t->face_ids[2]= *tp;
}
*tp= tetrahedron_id(i);
}

/*:84*/
#line 2013 "./simplex.w"








}
}

/*:82*/
#line 1972 "./simplex.w"

}

/*:80*/
#line 1837 "./simplex.w"

free(translation);
/*87:*/
#line 2211 "./simplex.w"

{
simplex_id v;
simplex_id beste;
simplex_id bestv;
hlist*more_local_unglued_faces;
hlist*local_done_edges;
hlist*local_lone_edges;
list*components;
UInt32 critsave[4];

for(i= 0;i<4;i++)list_initialize(crit+i,sizeof(simplex_id));
hlist_initialize(&unglued_faces,sizeof(simplex_id),12713,3,0);
hlist_initialize(&local_unglued_faces,sizeof(simplex_id),103,2,0);
hlist_initialize(&more_local_unglued_faces,sizeof(SInt32),13,1,0);
hlist_initialize(&lone_faces,sizeof(simplex_id),2713,3,0);
hlist_initialize(&local_lone_faces,sizeof(simplex_id),103,2,0);
hlist_initialize(&local_done_edges,sizeof(simplex_id),103,1,0);
hlist_initialize(&local_lone_edges,sizeof(simplex_id),103,1,0);
hlist_initialize(&possible_lone_edges,sizeof(simplex_id),2713,2,0);
list_initialize(&components,sizeof(struct star_component));

for(i= number_of_vertices;i> 0;)
{
i--;
v= vertex_id(i);





beste= NULL_ID;
bestv= NULL_ID;
hlist_clear(local_unglued_faces);
hlist_clear(local_lone_faces);
hlist_clear(local_done_edges);
hlist_clear(local_lone_edges);
critsave[0]= list_count(crit[0]);
critsave[1]= list_count(crit[1]);
critsave[2]= list_count(crit[2]);
critsave[3]= list_count(crit[3]);

/*88:*/
#line 2308 "./simplex.w"




{
vertex*vv= id2vertex(v);
simplex_id edge;
tetrahedron*tt;
simplex_id t;
double beste_value;

while(!id_is_null(vv->tetra_id))
{
/*89:*/
#line 2329 "./simplex.w"

{
simplex_id*last;

last= &vv->tetra_id;

do
{
t= *last;
tt= id2tetrahedron(t);
if(id_is_null(tt->face_ids[1]))break;
last= tt->face_ids+1;
}while(true);
*last= tt->face_ids[0];

edge= get_edge_of_tetrahedron(t,0,tt->flag&3);

}

/*:89*/
#line 2321 "./simplex.w"


/*90:*/
#line 2383 "./simplex.w"





{
simplex_id next;
simplex_id lowest_vertex= NULL_ID;
simplex_id face_with_lowest_vertex= NULL_ID;
double lowest_vertex_value;
simplex_id highest_vertex= NULL_ID;
simplex_id face_with_highest_vertex= NULL_ID;
double highest_vertex_value;

double edge_value,edge_length;

if(optional_link_values)
edge_value= alternate_vertex_value(v,get_vertex(edge,1),&edge_length);





hlist_clear(more_local_unglued_faces);
list_clear(components);
do
{
next= id2tetrahedron(t)->face_ids[2];




if(tetrahedron_is_in_K(t))
/*91:*/
#line 2438 "./simplex.w"







{
SInt32 more_local_index[2];
SInt32 comp_index[2];
int vertex_index[2];
simplex_id f;
int j;


tt= id2tetrahedron(t);

vertex_index[0]= ((tt->flag)&12)>>2;
vertex_index[1]= ((tt->flag)&48)>>4;

tt->face_ids[0]= NULL_ID;
tt->face_ids[1]= NULL_ID;
tt->face_ids[2]= NULL_ID;
tt->face_ids[3]= NULL_ID;

more_local_index[0]= hlist_add_delete(more_local_unglued_faces,
tt->vertex_ids+vertex_index[0],comp_index);
more_local_index[1]= hlist_add_delete(more_local_unglued_faces,
tt->vertex_ids+vertex_index[1],comp_index+1);

if(more_local_index[0]<0&&more_local_index[1]<0)
/*92:*/
#line 2493 "./simplex.w"




{
struct star_component*comp[2];
int j0,j1;
simplex_id f0,f1;



comp[0]= (struct star_component*)list_entry(components,comp_index[0]);
comp[1]= (struct star_component*)list_entry(components,comp_index[1]);

if(comp[0]->end_vert[0]==tt->vertex_ids[vertex_index[0]])j0= 0;else j0= 1;
if(comp[1]->end_vert[0]==tt->vertex_ids[vertex_index[1]])j1= 0;else j1= 1;

f0= get_face(t,vertex_index[0]);
f1= comp[1]->end_face[j1];

glue_faces(f0,f1);

f0= get_face(t,vertex_index[1]);
f1= comp[0]->end_face[j0];

glue_faces(f0,f1);


if(comp_index[0]==comp_index[1])
{



if(!id_is_null(next)||!hlist_is_empty(more_local_unglued_faces))
abort_message("link of an edge is disconnected");
}
else
{
comp[0]->end_vert[j0]= comp[1]->end_vert[1-j1];
comp[0]->end_face[j0]= comp[1]->end_face[1-j1];
comp[0]->index[j0]= comp[1]->index[1-j1];


if(comp[0]->bestv_value> comp[1]->bestv_value)
{
comp[0]->bestv= comp[1]->bestv;
comp[0]->bestv_value= comp[1]->bestv_value;
}

*((SInt32*)hlist_entry(more_local_unglued_faces,comp[0]->index[j0]))
= comp_index[0];
}
}

/*:92*/
#line 2469 "./simplex.w"

else if(more_local_index[0]>=0&&more_local_index[1]>=0)
/*93:*/
#line 2548 "./simplex.w"








{
struct star_component comp;
double val[2];





if(optional_link_values)
{
val[0]= alternate_edge_value(edge,tt->vertex_ids[vertex_index[0]],
edge_value,edge_length);
val[1]= alternate_edge_value(edge,tt->vertex_ids[vertex_index[1]],
edge_value,edge_length);
}
else
{
val[0]= tt->vertex_ids[vertex_index[0]];
val[1]= tt->vertex_ids[vertex_index[1]];
}

if(val[0]> val[1])j= 1;else j= 0;



comp.end_vert[0]= tt->vertex_ids[vertex_index[0]];
comp.end_vert[1]= tt->vertex_ids[vertex_index[1]];
comp.end_face[0]= get_face(t,vertex_index[1]);
comp.end_face[1]= get_face(t,vertex_index[0]);
comp.index[0]= more_local_index[0];
comp.index[1]= more_local_index[1];

comp.bestv= tt->vertex_ids[vertex_index[j]];

comp.bestv_value= val[j];

if(id_is_null(lowest_vertex)||lowest_vertex_value> val[j])
{
lowest_vertex= tt->vertex_ids[vertex_index[j]];
face_with_lowest_vertex= get_face(t,vertex_index[1-j]);
lowest_vertex_value= val[j];
}



if(id_is_null(highest_vertex)||highest_vertex_value<val[1-j])
{
highest_vertex_value= val[1-j];
highest_vertex= tt->vertex_ids[vertex_index[1-j]];
face_with_highest_vertex= get_face(t,vertex_index[j]);
}

*((SInt32*)hlist_entry(more_local_unglued_faces,more_local_index[0]))
= (SInt32)list_count(components);
*((SInt32*)hlist_entry(more_local_unglued_faces,more_local_index[1]))
= (SInt32)list_count(components);

list_push(components,&comp);
}

/*:93*/
#line 2471 "./simplex.w"

else
{
if(more_local_index[0]>=0)j= 1;
else j= 0;
/*94:*/
#line 2619 "./simplex.w"








{
struct star_component*comp;
int jj;
simplex_id f0,f1;
simplex_id new_vertex;
simplex_id glued_vertex;
double new_vertex_value;



glued_vertex= tt->vertex_ids[vertex_index[j]];
new_vertex= tt->vertex_ids[vertex_index[1-j]];

if(optional_link_values)
{
new_vertex_value= alternate_edge_value(edge,new_vertex,edge_value,edge_length);
}
else
new_vertex_value= new_vertex;

comp= (struct star_component*)list_entry(components,comp_index[j]);
if(comp->end_vert[0]==glued_vertex)jj= 0;else jj= 1;


f0= get_face(t,vertex_index[1-j]);
f1= comp->end_face[jj];

glue_faces(f0,f1);


comp->end_vert[jj]= new_vertex;
comp->end_face[jj]= get_face(t,vertex_index[j]);
comp->index[jj]= more_local_index[1-j];

if(comp->bestv_value> new_vertex_value)
{
comp->bestv= new_vertex;
comp->bestv_value= new_vertex_value;

if(lowest_vertex_value> new_vertex_value)
{
lowest_vertex= new_vertex;
lowest_vertex_value= new_vertex_value;
face_with_lowest_vertex= get_face(t,vertex_index[j]);
}
}


if(highest_vertex_value<new_vertex_value)
{
highest_vertex= new_vertex;
highest_vertex_value= new_vertex_value;
face_with_highest_vertex= get_face(t,vertex_index[j]);
}

*((SInt32*)hlist_entry(more_local_unglued_faces,comp->index[jj]))= comp_index[j];
}



/*:94*/
#line 2476 "./simplex.w"

}
}


/*:91*/
#line 2416 "./simplex.w"

else/*105:*/
#line 3035 "./simplex.w"


{
simplex_id f;



f= get_face(t,0);
/*107:*/
#line 3074 "./simplex.w"


{
simplex_id vl[3];
simplex_id f2= f;

get_triangle_vertices(f,vl);

if(hlist_add_delete(unglued_faces,vl,&f2)<0)
{
if(face_index(f2)!=0)abort_message("internal error 68");
glue_faces(f,f2);

/*108:*/
#line 3093 "./simplex.w"

{
double val0,val1;
simplex_id temp;

if(optional_link_values)
{
val0= alternate_vertex_value(vl[0],vl[1],NULL);
val1= alternate_vertex_value(vl[0],vl[2],NULL);

if(val1> val0)
{
temp= vl[2];
vl[2]= vl[1];
vl[1]= temp;
}
}
}

/*:108*/
#line 3087 "./simplex.w"


hlist_add(lone_faces,vl,&f2);
}
}

/*:107*/
#line 3043 "./simplex.w"

f= get_face(t,1);
/*109:*/
#line 3112 "./simplex.w"


{
simplex_id vl[3];
simplex_id f2= f;

get_triangle_vertices(f,vl);

if(hlist_find_delete(unglued_faces,vl,&f2))
{
if(face_index(f2)!=0)abort_message("internal error 68");
glue_faces(f,f2);

/*108:*/
#line 3093 "./simplex.w"

{
double val0,val1;
simplex_id temp;

if(optional_link_values)
{
val0= alternate_vertex_value(vl[0],vl[1],NULL);
val1= alternate_vertex_value(vl[0],vl[2],NULL);

if(val1> val0)
{
temp= vl[2];
vl[2]= vl[1];
vl[1]= temp;
}
}
}

/*:108*/
#line 3125 "./simplex.w"

hlist_add(local_lone_faces,vl+1,&f2);
}
else if(hlist_add_delete(local_unglued_faces,vl+1,&f2)<0)
{
glue_faces(f,f2);
/*108:*/
#line 3093 "./simplex.w"

{
double val0,val1;
simplex_id temp;

if(optional_link_values)
{
val0= alternate_vertex_value(vl[0],vl[1],NULL);
val1= alternate_vertex_value(vl[0],vl[2],NULL);

if(val1> val0)
{
temp= vl[2];
vl[2]= vl[1];
vl[1]= temp;
}
}
}

/*:108*/
#line 3131 "./simplex.w"

hlist_add(local_lone_faces,vl+1,&f2);
}
}

/*:109*/
#line 3045 "./simplex.w"

f= get_face(t,2);
/*109:*/
#line 3112 "./simplex.w"


{
simplex_id vl[3];
simplex_id f2= f;

get_triangle_vertices(f,vl);

if(hlist_find_delete(unglued_faces,vl,&f2))
{
if(face_index(f2)!=0)abort_message("internal error 68");
glue_faces(f,f2);

/*108:*/
#line 3093 "./simplex.w"

{
double val0,val1;
simplex_id temp;

if(optional_link_values)
{
val0= alternate_vertex_value(vl[0],vl[1],NULL);
val1= alternate_vertex_value(vl[0],vl[2],NULL);

if(val1> val0)
{
temp= vl[2];
vl[2]= vl[1];
vl[1]= temp;
}
}
}

/*:108*/
#line 3125 "./simplex.w"

hlist_add(local_lone_faces,vl+1,&f2);
}
else if(hlist_add_delete(local_unglued_faces,vl+1,&f2)<0)
{
glue_faces(f,f2);
/*108:*/
#line 3093 "./simplex.w"

{
double val0,val1;
simplex_id temp;

if(optional_link_values)
{
val0= alternate_vertex_value(vl[0],vl[1],NULL);
val1= alternate_vertex_value(vl[0],vl[2],NULL);

if(val1> val0)
{
temp= vl[2];
vl[2]= vl[1];
vl[1]= temp;
}
}
}

/*:108*/
#line 3131 "./simplex.w"

hlist_add(local_lone_faces,vl+1,&f2);
}
}

/*:109*/
#line 3047 "./simplex.w"

f= get_face(t,3);
/*109:*/
#line 3112 "./simplex.w"


{
simplex_id vl[3];
simplex_id f2= f;

get_triangle_vertices(f,vl);

if(hlist_find_delete(unglued_faces,vl,&f2))
{
if(face_index(f2)!=0)abort_message("internal error 68");
glue_faces(f,f2);

/*108:*/
#line 3093 "./simplex.w"

{
double val0,val1;
simplex_id temp;

if(optional_link_values)
{
val0= alternate_vertex_value(vl[0],vl[1],NULL);
val1= alternate_vertex_value(vl[0],vl[2],NULL);

if(val1> val0)
{
temp= vl[2];
vl[2]= vl[1];
vl[1]= temp;
}
}
}

/*:108*/
#line 3125 "./simplex.w"

hlist_add(local_lone_faces,vl+1,&f2);
}
else if(hlist_add_delete(local_unglued_faces,vl+1,&f2)<0)
{
glue_faces(f,f2);
/*108:*/
#line 3093 "./simplex.w"

{
double val0,val1;
simplex_id temp;

if(optional_link_values)
{
val0= alternate_vertex_value(vl[0],vl[1],NULL);
val1= alternate_vertex_value(vl[0],vl[2],NULL);

if(val1> val0)
{
temp= vl[2];
vl[2]= vl[1];
vl[1]= temp;
}
}
}

/*:108*/
#line 3131 "./simplex.w"

hlist_add(local_lone_faces,vl+1,&f2);
}
}

/*:109*/
#line 3049 "./simplex.w"

}

/*:105*/
#line 2417 "./simplex.w"

t= next;
}while(!id_is_null(t));
if(edge_is_in_K(edge))
/*95:*/
#line 2691 "./simplex.w"








{
simplex_id w;
SInt32 index;
struct star_component*comp;
simplex_id f,f0,f1,local_lowest_vertex;

if(hlist_is_empty(more_local_unglued_faces)&&!id_is_null(face_with_lowest_vertex))
/*98:*/
#line 2788 "./simplex.w"





{
int flag;


t= id2tetra_id(face_with_highest_vertex);
f= get_face(t,0);
/*107:*/
#line 3074 "./simplex.w"


{
simplex_id vl[3];
simplex_id f2= f;

get_triangle_vertices(f,vl);

if(hlist_add_delete(unglued_faces,vl,&f2)<0)
{
if(face_index(f2)!=0)abort_message("internal error 68");
glue_faces(f,f2);

/*108:*/
#line 3093 "./simplex.w"

{
double val0,val1;
simplex_id temp;

if(optional_link_values)
{
val0= alternate_vertex_value(vl[0],vl[1],NULL);
val1= alternate_vertex_value(vl[0],vl[2],NULL);

if(val1> val0)
{
temp= vl[2];
vl[2]= vl[1];
vl[1]= temp;
}
}
}

/*:108*/
#line 3087 "./simplex.w"


hlist_add(lone_faces,vl,&f2);
}
}

/*:107*/
#line 2799 "./simplex.w"


flag= id2tetrahedron(t)->flag;
f= get_face(t,flag&3);
/*109:*/
#line 3112 "./simplex.w"


{
simplex_id vl[3];
simplex_id f2= f;

get_triangle_vertices(f,vl);

if(hlist_find_delete(unglued_faces,vl,&f2))
{
if(face_index(f2)!=0)abort_message("internal error 68");
glue_faces(f,f2);

/*108:*/
#line 3093 "./simplex.w"

{
double val0,val1;
simplex_id temp;

if(optional_link_values)
{
val0= alternate_vertex_value(vl[0],vl[1],NULL);
val1= alternate_vertex_value(vl[0],vl[2],NULL);

if(val1> val0)
{
temp= vl[2];
vl[2]= vl[1];
vl[1]= temp;
}
}
}

/*:108*/
#line 3125 "./simplex.w"

hlist_add(local_lone_faces,vl+1,&f2);
}
else if(hlist_add_delete(local_unglued_faces,vl+1,&f2)<0)
{
glue_faces(f,f2);
/*108:*/
#line 3093 "./simplex.w"

{
double val0,val1;
simplex_id temp;

if(optional_link_values)
{
val0= alternate_vertex_value(vl[0],vl[1],NULL);
val1= alternate_vertex_value(vl[0],vl[2],NULL);

if(val1> val0)
{
temp= vl[2];
vl[2]= vl[1];
vl[1]= temp;
}
}
}

/*:108*/
#line 3131 "./simplex.w"

hlist_add(local_lone_faces,vl+1,&f2);
}
}

/*:109*/
#line 2803 "./simplex.w"


make_tetrahedron_critical(t);

f0= other_face_id(get_face(t,(flag>>2)&3));
f1= other_face_id(get_face(t,(flag>>4)&3));

local_lowest_vertex= lowest_vertex;
/*111:*/
#line 3163 "./simplex.w"

{
simplex_id vlist[3];

get_triangle_vertices(f0,vlist);
/*112:*/
#line 3185 "./simplex.w"

if(vlist[2]==get_vertex(edge,1))
{
vlist[2]= vlist[1];
vlist[1]= get_vertex(edge,1);
}


/*:112*/
#line 3168 "./simplex.w"

t= id2tetra_id(f0);
f= f0;
/*113:*/
#line 3193 "./simplex.w"

while(vlist[2]!=local_lowest_vertex)
{
pair23(&f,t,false);
f= get_face(t,0);
/*107:*/
#line 3074 "./simplex.w"


{
simplex_id vl[3];
simplex_id f2= f;

get_triangle_vertices(f,vl);

if(hlist_add_delete(unglued_faces,vl,&f2)<0)
{
if(face_index(f2)!=0)abort_message("internal error 68");
glue_faces(f,f2);

/*108:*/
#line 3093 "./simplex.w"

{
double val0,val1;
simplex_id temp;

if(optional_link_values)
{
val0= alternate_vertex_value(vl[0],vl[1],NULL);
val1= alternate_vertex_value(vl[0],vl[2],NULL);

if(val1> val0)
{
temp= vl[2];
vl[2]= vl[1];
vl[1]= temp;
}
}
}

/*:108*/
#line 3087 "./simplex.w"


hlist_add(lone_faces,vl,&f2);
}
}

/*:107*/
#line 3198 "./simplex.w"

f= get_face(t,id2tetrahedron(t)->flag&3);
/*109:*/
#line 3112 "./simplex.w"


{
simplex_id vl[3];
simplex_id f2= f;

get_triangle_vertices(f,vl);

if(hlist_find_delete(unglued_faces,vl,&f2))
{
if(face_index(f2)!=0)abort_message("internal error 68");
glue_faces(f,f2);

/*108:*/
#line 3093 "./simplex.w"

{
double val0,val1;
simplex_id temp;

if(optional_link_values)
{
val0= alternate_vertex_value(vl[0],vl[1],NULL);
val1= alternate_vertex_value(vl[0],vl[2],NULL);

if(val1> val0)
{
temp= vl[2];
vl[2]= vl[1];
vl[1]= temp;
}
}
}

/*:108*/
#line 3125 "./simplex.w"

hlist_add(local_lone_faces,vl+1,&f2);
}
else if(hlist_add_delete(local_unglued_faces,vl+1,&f2)<0)
{
glue_faces(f,f2);
/*108:*/
#line 3093 "./simplex.w"

{
double val0,val1;
simplex_id temp;

if(optional_link_values)
{
val0= alternate_vertex_value(vl[0],vl[1],NULL);
val1= alternate_vertex_value(vl[0],vl[2],NULL);

if(val1> val0)
{
temp= vl[2];
vl[2]= vl[1];
vl[1]= temp;
}
}
}

/*:108*/
#line 3131 "./simplex.w"

hlist_add(local_lone_faces,vl+1,&f2);
}
}

/*:109*/
#line 3200 "./simplex.w"


f= next_face(vlist,t);
t= id2tetra_id(f);
}/*:113*/
#line 3171 "./simplex.w"


get_triangle_vertices(f1,vlist);
/*112:*/
#line 3185 "./simplex.w"

if(vlist[2]==get_vertex(edge,1))
{
vlist[2]= vlist[1];
vlist[1]= get_vertex(edge,1);
}


/*:112*/
#line 3174 "./simplex.w"

t= id2tetra_id(f1);
f= f1;
/*113:*/
#line 3193 "./simplex.w"

while(vlist[2]!=local_lowest_vertex)
{
pair23(&f,t,false);
f= get_face(t,0);
/*107:*/
#line 3074 "./simplex.w"


{
simplex_id vl[3];
simplex_id f2= f;

get_triangle_vertices(f,vl);

if(hlist_add_delete(unglued_faces,vl,&f2)<0)
{
if(face_index(f2)!=0)abort_message("internal error 68");
glue_faces(f,f2);

/*108:*/
#line 3093 "./simplex.w"

{
double val0,val1;
simplex_id temp;

if(optional_link_values)
{
val0= alternate_vertex_value(vl[0],vl[1],NULL);
val1= alternate_vertex_value(vl[0],vl[2],NULL);

if(val1> val0)
{
temp= vl[2];
vl[2]= vl[1];
vl[1]= temp;
}
}
}

/*:108*/
#line 3087 "./simplex.w"


hlist_add(lone_faces,vl,&f2);
}
}

/*:107*/
#line 3198 "./simplex.w"

f= get_face(t,id2tetrahedron(t)->flag&3);
/*109:*/
#line 3112 "./simplex.w"


{
simplex_id vl[3];
simplex_id f2= f;

get_triangle_vertices(f,vl);

if(hlist_find_delete(unglued_faces,vl,&f2))
{
if(face_index(f2)!=0)abort_message("internal error 68");
glue_faces(f,f2);

/*108:*/
#line 3093 "./simplex.w"

{
double val0,val1;
simplex_id temp;

if(optional_link_values)
{
val0= alternate_vertex_value(vl[0],vl[1],NULL);
val1= alternate_vertex_value(vl[0],vl[2],NULL);

if(val1> val0)
{
temp= vl[2];
vl[2]= vl[1];
vl[1]= temp;
}
}
}

/*:108*/
#line 3125 "./simplex.w"

hlist_add(local_lone_faces,vl+1,&f2);
}
else if(hlist_add_delete(local_unglued_faces,vl+1,&f2)<0)
{
glue_faces(f,f2);
/*108:*/
#line 3093 "./simplex.w"

{
double val0,val1;
simplex_id temp;

if(optional_link_values)
{
val0= alternate_vertex_value(vl[0],vl[1],NULL);
val1= alternate_vertex_value(vl[0],vl[2],NULL);

if(val1> val0)
{
temp= vl[2];
vl[2]= vl[1];
vl[1]= temp;
}
}
}

/*:108*/
#line 3131 "./simplex.w"

hlist_add(local_lone_faces,vl+1,&f2);
}
}

/*:109*/
#line 3200 "./simplex.w"


f= next_face(vlist,t);
t= id2tetra_id(f);
}/*:113*/
#line 3177 "./simplex.w"

if(local_lowest_vertex!=lowest_vertex)
{
make_triangle_valid(f);
make_triangle_critical(&f);
}
}

/*:111*/
#line 2811 "./simplex.w"

}



/*:98*/
#line 2706 "./simplex.w"

else
{
while(hlist_get(more_local_unglued_faces,&w,&index))
{
comp= (struct star_component*)list_entry(components,index);
if(comp->end_vert[0]==w)w= comp->end_vert[1];
else w= comp->end_vert[0];
if(!hlist_find_delete(more_local_unglued_faces,&w,NULL))
abort_message("internal error 46");
f= comp->end_face[0];
/*110:*/
#line 3136 "./simplex.w"


{
simplex_id vl[3];
simplex_id f2= f;

get_triangle_vertices(f,vl);

if(hlist_find_delete(unglued_faces,vl,&f2))
{
if(face_index(f2)!=0)abort_message("internal error 68");
glue_faces(f,f2);
}
else if(hlist_add_delete(local_unglued_faces,vl+1,&f2)<0)
{
glue_faces(f,f2);
}
else abort_message("link faces: I don't think this should happen");
}


/*:110*/
#line 2717 "./simplex.w"

f= comp->end_face[1];
/*110:*/
#line 3136 "./simplex.w"


{
simplex_id vl[3];
simplex_id f2= f;

get_triangle_vertices(f,vl);

if(hlist_find_delete(unglued_faces,vl,&f2))
{
if(face_index(f2)!=0)abort_message("internal error 68");
glue_faces(f,f2);
}
else if(hlist_add_delete(local_unglued_faces,vl+1,&f2)<0)
{
glue_faces(f,f2);
}
else abort_message("link faces: I don't think this should happen");
}


/*:110*/
#line 2719 "./simplex.w"


local_lowest_vertex= comp->bestv;
f0= comp->end_face[0];
f1= comp->end_face[1];
/*111:*/
#line 3163 "./simplex.w"

{
simplex_id vlist[3];

get_triangle_vertices(f0,vlist);
/*112:*/
#line 3185 "./simplex.w"

if(vlist[2]==get_vertex(edge,1))
{
vlist[2]= vlist[1];
vlist[1]= get_vertex(edge,1);
}


/*:112*/
#line 3168 "./simplex.w"

t= id2tetra_id(f0);
f= f0;
/*113:*/
#line 3193 "./simplex.w"

while(vlist[2]!=local_lowest_vertex)
{
pair23(&f,t,false);
f= get_face(t,0);
/*107:*/
#line 3074 "./simplex.w"


{
simplex_id vl[3];
simplex_id f2= f;

get_triangle_vertices(f,vl);

if(hlist_add_delete(unglued_faces,vl,&f2)<0)
{
if(face_index(f2)!=0)abort_message("internal error 68");
glue_faces(f,f2);

/*108:*/
#line 3093 "./simplex.w"

{
double val0,val1;
simplex_id temp;

if(optional_link_values)
{
val0= alternate_vertex_value(vl[0],vl[1],NULL);
val1= alternate_vertex_value(vl[0],vl[2],NULL);

if(val1> val0)
{
temp= vl[2];
vl[2]= vl[1];
vl[1]= temp;
}
}
}

/*:108*/
#line 3087 "./simplex.w"


hlist_add(lone_faces,vl,&f2);
}
}

/*:107*/
#line 3198 "./simplex.w"

f= get_face(t,id2tetrahedron(t)->flag&3);
/*109:*/
#line 3112 "./simplex.w"


{
simplex_id vl[3];
simplex_id f2= f;

get_triangle_vertices(f,vl);

if(hlist_find_delete(unglued_faces,vl,&f2))
{
if(face_index(f2)!=0)abort_message("internal error 68");
glue_faces(f,f2);

/*108:*/
#line 3093 "./simplex.w"

{
double val0,val1;
simplex_id temp;

if(optional_link_values)
{
val0= alternate_vertex_value(vl[0],vl[1],NULL);
val1= alternate_vertex_value(vl[0],vl[2],NULL);

if(val1> val0)
{
temp= vl[2];
vl[2]= vl[1];
vl[1]= temp;
}
}
}

/*:108*/
#line 3125 "./simplex.w"

hlist_add(local_lone_faces,vl+1,&f2);
}
else if(hlist_add_delete(local_unglued_faces,vl+1,&f2)<0)
{
glue_faces(f,f2);
/*108:*/
#line 3093 "./simplex.w"

{
double val0,val1;
simplex_id temp;

if(optional_link_values)
{
val0= alternate_vertex_value(vl[0],vl[1],NULL);
val1= alternate_vertex_value(vl[0],vl[2],NULL);

if(val1> val0)
{
temp= vl[2];
vl[2]= vl[1];
vl[1]= temp;
}
}
}

/*:108*/
#line 3131 "./simplex.w"

hlist_add(local_lone_faces,vl+1,&f2);
}
}

/*:109*/
#line 3200 "./simplex.w"


f= next_face(vlist,t);
t= id2tetra_id(f);
}/*:113*/
#line 3171 "./simplex.w"


get_triangle_vertices(f1,vlist);
/*112:*/
#line 3185 "./simplex.w"

if(vlist[2]==get_vertex(edge,1))
{
vlist[2]= vlist[1];
vlist[1]= get_vertex(edge,1);
}


/*:112*/
#line 3174 "./simplex.w"

t= id2tetra_id(f1);
f= f1;
/*113:*/
#line 3193 "./simplex.w"

while(vlist[2]!=local_lowest_vertex)
{
pair23(&f,t,false);
f= get_face(t,0);
/*107:*/
#line 3074 "./simplex.w"


{
simplex_id vl[3];
simplex_id f2= f;

get_triangle_vertices(f,vl);

if(hlist_add_delete(unglued_faces,vl,&f2)<0)
{
if(face_index(f2)!=0)abort_message("internal error 68");
glue_faces(f,f2);

/*108:*/
#line 3093 "./simplex.w"

{
double val0,val1;
simplex_id temp;

if(optional_link_values)
{
val0= alternate_vertex_value(vl[0],vl[1],NULL);
val1= alternate_vertex_value(vl[0],vl[2],NULL);

if(val1> val0)
{
temp= vl[2];
vl[2]= vl[1];
vl[1]= temp;
}
}
}

/*:108*/
#line 3087 "./simplex.w"


hlist_add(lone_faces,vl,&f2);
}
}

/*:107*/
#line 3198 "./simplex.w"

f= get_face(t,id2tetrahedron(t)->flag&3);
/*109:*/
#line 3112 "./simplex.w"


{
simplex_id vl[3];
simplex_id f2= f;

get_triangle_vertices(f,vl);

if(hlist_find_delete(unglued_faces,vl,&f2))
{
if(face_index(f2)!=0)abort_message("internal error 68");
glue_faces(f,f2);

/*108:*/
#line 3093 "./simplex.w"

{
double val0,val1;
simplex_id temp;

if(optional_link_values)
{
val0= alternate_vertex_value(vl[0],vl[1],NULL);
val1= alternate_vertex_value(vl[0],vl[2],NULL);

if(val1> val0)
{
temp= vl[2];
vl[2]= vl[1];
vl[1]= temp;
}
}
}

/*:108*/
#line 3125 "./simplex.w"

hlist_add(local_lone_faces,vl+1,&f2);
}
else if(hlist_add_delete(local_unglued_faces,vl+1,&f2)<0)
{
glue_faces(f,f2);
/*108:*/
#line 3093 "./simplex.w"

{
double val0,val1;
simplex_id temp;

if(optional_link_values)
{
val0= alternate_vertex_value(vl[0],vl[1],NULL);
val1= alternate_vertex_value(vl[0],vl[2],NULL);

if(val1> val0)
{
temp= vl[2];
vl[2]= vl[1];
vl[1]= temp;
}
}
}

/*:108*/
#line 3131 "./simplex.w"

hlist_add(local_lone_faces,vl+1,&f2);
}
}

/*:109*/
#line 3200 "./simplex.w"


f= next_face(vlist,t);
t= id2tetra_id(f);
}/*:113*/
#line 3177 "./simplex.w"

if(local_lowest_vertex!=lowest_vertex)
{
make_triangle_valid(f);
make_triangle_critical(&f);
}
}

/*:111*/
#line 2724 "./simplex.w"

}
/*99:*/
#line 2816 "./simplex.w"






{
simplex_id vlist[3];

get_edge_vertices(edge,vlist);




while(hlist_sub_match(lone_faces,vlist,&f,2))
/*100:*/
#line 2836 "./simplex.w"







{
simplex_id e;
double new_vertex_value;

if(triangle_is_in_K(f))
{







if(optional_link_values)
{
new_vertex_value= alternate_edge_value(edge,vlist[2],edge_value,edge_length);
}
else
new_vertex_value= vlist[2];

if(id_is_null(lowest_vertex))
{
lowest_vertex= vlist[2];
face_with_lowest_vertex= f;
lowest_vertex_value= new_vertex_value;
}
else
{
if(lowest_vertex_value> new_vertex_value)
{
make_triangle_valid(face_with_lowest_vertex);
make_triangle_critical(&face_with_lowest_vertex);
lowest_vertex= vlist[2];
lowest_vertex_value= new_vertex_value;
face_with_lowest_vertex= f;
}
else
{
make_triangle_valid(f);
make_triangle_critical(&f);
}
}
}
e= get_edge(f,1);
if(get_vertex(e,1)!=vlist[2])e= get_edge(f,2);
if(get_vertex(e,1)!=vlist[2])abort_message("internal 59");

/*101:*/
#line 2896 "./simplex.w"




{
simplex_id vl[2];
if(edge_is_in_K(e))
{
get_edge_vertices(e,vl);





if(hlist_find(local_done_edges,vl+1)<0)
hlist_find_add(local_lone_edges,vl+1,&e,NULL);
}
}

/*:101*/
#line 2890 "./simplex.w"


e= get_edge(f,0);
/*102:*/
#line 2915 "./simplex.w"


{
simplex_id vl[2],*last;
if(edge_is_in_K(e))
{
get_edge_vertices(e,vl);




hlist_find_add(possible_lone_edges,vl,&e,NULL);
}
}



/*:102*/
#line 2893 "./simplex.w"

}

/*:100*/
#line 2831 "./simplex.w"

while(hlist_sub_match(local_lone_faces,vlist+1,&f,1))
/*100:*/
#line 2836 "./simplex.w"







{
simplex_id e;
double new_vertex_value;

if(triangle_is_in_K(f))
{







if(optional_link_values)
{
new_vertex_value= alternate_edge_value(edge,vlist[2],edge_value,edge_length);
}
else
new_vertex_value= vlist[2];

if(id_is_null(lowest_vertex))
{
lowest_vertex= vlist[2];
face_with_lowest_vertex= f;
lowest_vertex_value= new_vertex_value;
}
else
{
if(lowest_vertex_value> new_vertex_value)
{
make_triangle_valid(face_with_lowest_vertex);
make_triangle_critical(&face_with_lowest_vertex);
lowest_vertex= vlist[2];
lowest_vertex_value= new_vertex_value;
face_with_lowest_vertex= f;
}
else
{
make_triangle_valid(f);
make_triangle_critical(&f);
}
}
}
e= get_edge(f,1);
if(get_vertex(e,1)!=vlist[2])e= get_edge(f,2);
if(get_vertex(e,1)!=vlist[2])abort_message("internal 59");

/*101:*/
#line 2896 "./simplex.w"




{
simplex_id vl[2];
if(edge_is_in_K(e))
{
get_edge_vertices(e,vl);





if(hlist_find(local_done_edges,vl+1)<0)
hlist_find_add(local_lone_edges,vl+1,&e,NULL);
}
}

/*:101*/
#line 2890 "./simplex.w"


e= get_edge(f,0);
/*102:*/
#line 2915 "./simplex.w"


{
simplex_id vl[2],*last;
if(edge_is_in_K(e))
{
get_edge_vertices(e,vl);




hlist_find_add(possible_lone_edges,vl,&e,NULL);
}
}



/*:102*/
#line 2893 "./simplex.w"

}

/*:100*/
#line 2833 "./simplex.w"

}

/*:99*/
#line 2726 "./simplex.w"

}
/*96:*/
#line 2732 "./simplex.w"




{
w= get_vertex(edge,1);

if(id_is_null(face_with_lowest_vertex))
/*97:*/
#line 2749 "./simplex.w"




{
double w_value;

if(optional_link_values)
w_value= alternate_vertex_value(v,w,NULL);
else
w_value= w;



if(id_is_null(bestv))
{
bestv= w;
beste= edge;
beste_value= w_value;
}
else
{
if(beste_value> w_value)
{
make_edge_valid(beste);
make_edge_critical(&beste);
bestv= w;
beste= edge;
beste_value= w_value;
}
else
{
make_edge_valid(edge);
make_edge_critical(&edge);
}
}
}


/*:97*/
#line 2740 "./simplex.w"

else
{
pair12(&edge,face_with_lowest_vertex,false);
}

hlist_find_add(local_done_edges,&w,&edge,NULL);
}

/*:96*/
#line 2728 "./simplex.w"

}


/*:95*/
#line 2421 "./simplex.w"

}

/*:90*/
#line 2323 "./simplex.w"

}
/*103:*/
#line 2932 "./simplex.w"




{
simplex_id vlist[3];
simplex_id lowest_vertex;
double lowest_vertex_value;
simplex_id face_with_lowest_vertex;
simplex_id f,w;

double edge_value,edge_length;


vlist[0]= v;


if(!hlist_is_empty(local_unglued_faces))abort_message("internal 34");
if(hlist_sub_match(unglued_faces,vlist,NULL,1))abort_message("internal 35");

do
{
if(hlist_sub_match(local_lone_faces,vlist+1,&f,0));
else if(!hlist_sub_match(lone_faces,vlist,&f,1))break;

if(hlist_find(local_done_edges,vlist+1)>=0)
continue;

edge= vertices2edge(vlist,id2tetra_id(f));


if(optional_link_values)
edge_value= alternate_vertex_value(v,get_vertex(edge,1),&edge_length);






lowest_vertex= NULL_ID;
face_with_lowest_vertex= NULL_ID;
do
{
/*100:*/
#line 2836 "./simplex.w"







{
simplex_id e;
double new_vertex_value;

if(triangle_is_in_K(f))
{







if(optional_link_values)
{
new_vertex_value= alternate_edge_value(edge,vlist[2],edge_value,edge_length);
}
else
new_vertex_value= vlist[2];

if(id_is_null(lowest_vertex))
{
lowest_vertex= vlist[2];
face_with_lowest_vertex= f;
lowest_vertex_value= new_vertex_value;
}
else
{
if(lowest_vertex_value> new_vertex_value)
{
make_triangle_valid(face_with_lowest_vertex);
make_triangle_critical(&face_with_lowest_vertex);
lowest_vertex= vlist[2];
lowest_vertex_value= new_vertex_value;
face_with_lowest_vertex= f;
}
else
{
make_triangle_valid(f);
make_triangle_critical(&f);
}
}
}
e= get_edge(f,1);
if(get_vertex(e,1)!=vlist[2])e= get_edge(f,2);
if(get_vertex(e,1)!=vlist[2])abort_message("internal 59");

/*101:*/
#line 2896 "./simplex.w"




{
simplex_id vl[2];
if(edge_is_in_K(e))
{
get_edge_vertices(e,vl);





if(hlist_find(local_done_edges,vl+1)<0)
hlist_find_add(local_lone_edges,vl+1,&e,NULL);
}
}

/*:101*/
#line 2890 "./simplex.w"


e= get_edge(f,0);
/*102:*/
#line 2915 "./simplex.w"


{
simplex_id vl[2],*last;
if(edge_is_in_K(e))
{
get_edge_vertices(e,vl);




hlist_find_add(possible_lone_edges,vl,&e,NULL);
}
}



/*:102*/
#line 2893 "./simplex.w"

}

/*:100*/
#line 2975 "./simplex.w"

if(!hlist_sub_match(lone_faces,vlist,&f,2))
{
if(!hlist_sub_match(local_lone_faces,vlist+1,&f,1))break;
}
}while(true);

if(edge_is_in_K(edge))
/*96:*/
#line 2732 "./simplex.w"




{
w= get_vertex(edge,1);

if(id_is_null(face_with_lowest_vertex))
/*97:*/
#line 2749 "./simplex.w"




{
double w_value;

if(optional_link_values)
w_value= alternate_vertex_value(v,w,NULL);
else
w_value= w;



if(id_is_null(bestv))
{
bestv= w;
beste= edge;
beste_value= w_value;
}
else
{
if(beste_value> w_value)
{
make_edge_valid(beste);
make_edge_critical(&beste);
bestv= w;
beste= edge;
beste_value= w_value;
}
else
{
make_edge_valid(edge);
make_edge_critical(&edge);
}
}
}


/*:97*/
#line 2740 "./simplex.w"

else
{
pair12(&edge,face_with_lowest_vertex,false);
}

hlist_find_add(local_done_edges,&w,&edge,NULL);
}

/*:96*/
#line 2983 "./simplex.w"

}while(true);
}

/*:103*/
#line 2325 "./simplex.w"

/*104:*/
#line 3006 "./simplex.w"




{
simplex_id w,vlist[2];

vlist[0]= v;

do
{
if(hlist_sub_match(local_lone_edges,vlist+1,&edge,0));
else if(!hlist_sub_match(possible_lone_edges,vlist,&edge,1))break;

if(!edge_is_in_K(edge)||hlist_find(local_done_edges,vlist+1)>=0)
continue;





w= vlist[1];
/*97:*/
#line 2749 "./simplex.w"




{
double w_value;

if(optional_link_values)
w_value= alternate_vertex_value(v,w,NULL);
else
w_value= w;



if(id_is_null(bestv))
{
bestv= w;
beste= edge;
beste_value= w_value;
}
else
{
if(beste_value> w_value)
{
make_edge_valid(beste);
make_edge_critical(&beste);
bestv= w;
beste= edge;
beste_value= w_value;
}
else
{
make_edge_valid(edge);
make_edge_critical(&edge);
}
}
}


/*:97*/
#line 3028 "./simplex.w"


hlist_find_add(local_done_edges,&w,&edge,NULL);

}while(true);
}

/*:104*/
#line 2326 "./simplex.w"

}

/*:88*/
#line 2253 "./simplex.w"


if(vertex_is_in_K(v))
{
if(id_is_null(beste))
{
make_vertex_critical(v);
}
else
{
pair01(v,beste,false);
LocalCancel(v,critsave[2]);
clean_crit_at(critsave);
}
}
}
hlist_abandon(&unglued_faces);
hlist_abandon(&local_unglued_faces);
hlist_abandon(&more_local_unglued_faces);
hlist_abandon(&lone_faces);
hlist_abandon(&local_lone_faces);
hlist_abandon(&local_done_edges);
hlist_abandon(&local_lone_edges);
hlist_abandon(&possible_lone_edges);
list_abandon(&components);
}


/*:87*/
#line 1839 "./simplex.w"

}

/*:71*//*78:*/
#line 1933 "./simplex.w"

int vertex_sort(const void*nn,const void*mm)
{
float*p,*q;
UInt32 n,m;

n= *(UInt32*)nn;
m= *(UInt32*)mm;
p= *(float**)list_entry(vertex_blocks,n>>8);
q= *(float**)list_entry(vertex_blocks,m>>8);
p+= ((n&0xff)*4);
q+= ((m&0xff)*4);
if(*p<*q)return-1;
if(*p> *q)return 1;
return 0;
}

/*:78*/
#line 21 "./simplex.w"


/*:1*/
