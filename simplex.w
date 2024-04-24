@i common.w

@* Navigating Simplicial Complexes.
Only two types of simplices have explicit structures, vertices and tetrahedra.
Tetrahedra have many fields giving characteristics of their edges and  faces.

Simplices are referred to by a |simplex_id|.  
Each vertex has a unique id.
Each tetrahedron has a unique id.
Each triangle has two possible ids, one for each containing tetrahedron.
Each edge has many ids (on average about 6 ids), 
one for each containing tetrahedron.
Although edges and triangles have more than one id, but there is
one optimal id with which information about the simplex can be retrieved most quickly.
Consequently, many functions replace a given id with the optimal id.
@c
#include "Morse.h"
#include "globals.h"
#include "simplex.wh"
@<Simplex functions@>@;
@<Subroutines for constructing complexes@>@;

@ @(simplex.h@>+=
typedef SInt32 simplex_id;


@*1 Vertex structure.
First we have the vertex structure.
The field |h| is the the value of the vertex.
I expect 16 bit resolution of the function is good enough,
since there is deterministic tie breaking.
The field |tetra_id| is the id of a tetrahedron containing the simplex.
If the vertex is in $K$ and not critical, 
this tetrahedron contains the edge with which the vertex is paired.
The field |type| contains various bit flags with the following meanings:
\item{$\cdot$} Bit 0 is set if the vertex is in $K$.
\item{$\cdot$} Bit 1 is set if the vertex is critical.
\item{$\cdot$} If bit 1 is not set then bits 3,4,5 
	give the index of its paired edge in the tetrahedron |tetra_id|.

@(simplex.h@>+=

typedef struct vertexstruct
{
	UInt8 type;  
	SInt16 h;
	simplex_id tetra_id;
	GLfloat coords[3];
} vertex;

@*1 Tetrahedron structure.
Now we have the tetrahedron structure.
The field |vertex_ids| gives the ids of the four vertices.
These are in order of decreasing value.

The field |face_ids| gives the alternate ids of the four faces
of the tetrahedron, i.e., the ids based on the four neighboring
tetrahedra.  These are in an order consistent with |vertex_ids|,
so |face_ids[i]| is the face which doen't contain |vertex_ids[i]|.
Note however that when when first read in the fields |face_ids|
have a very different meaning which we explain later
in the section `Reading in a Complex'.

\smallskip
The field |flag| is used when reading in the complex. 
If |optional_link_values| is set then the lower order
2 bits give the index |i| of the vertex such that
the quotient 
|(value(vertex_ids[0])-value(vertex_ids[i]))/|length of edge between
|vertex_ids[0]| and |vertex_ids[i]|
is as small as possible.
The next 2 bits give the index with second smallest quotient and the 
third two give the index with the largest quotient.
If |optional_link_values| is not set then flag is set to
|0x39| which gives the indices defining the order of
the values of |vertex_ids[i]|, which are always in decreasing order.
Note that in either case, |vertex_ids[0]| is always the vertex
with highest value and |vertex_ids[1]| the vertex with next highest
value, and so on.


\smallskip
The field |types[0]| has bit fields with various info about the tetrahedron:  
\item{$\cdot$} Bit 0 is set if the tetrahedron is in $K$.
\item{$\cdot$} Bit 1 is set if the tetrahedron is critical.
\item{$\cdot$} Bits 3 and 4 give the index of the paired face if the tetrahedron is not critical. 
\item{$\cdot$} Bit 6 is set if it is known 
	that no gradient paths from the tetrahedron go to critical triangles.
\item{$\cdot$} Bit 7 is set if this tetrahedron has been processed.  
If not set many fields are untrustworthy. ? obsolete?
	
\smallskip
The bits in character |types[i]| have the folowing meaning for $1\le i\le 4$,
refering to the triangle face opposite the $i-1$-th vertex:
\item{$\cdot$} Bit 0 is set if the triangle is in $K$.
\item{$\cdot$} Bit 1 is set if the triangle is critical.
\item{$\cdot$} Bit 2 is set if the triangle is paired with an edge.
\item{$\cdot$} Bits 3-4 give the relative index of that edge if bit 2 is set. 
\item{$\cdot$} Bit 5 is set if the triangle is paired down and it is known
				that all gradient paths to the triangle do not come from critical triangles.
\item{$\cdot$} Bit 6 is set if the triangle is paired up and it is known 
	that all gradient paths from the triangle do not lead to critical triangles.
				Bit 6 is also set if the triangle is paired down and it is known
				that all gradient paths from the triangle do not lead to critical edges.
\item{$\cdot$} Bit 7 is set if this type character is valid.
A given triangle in $K$ is valid in exactly one tetrahedron,
	and if it is paired to a tetrahedron it is valid in that tetrahedron.
Bits 0 and 7 are correct in all two tetrahedra containing the triangle,
the others only if bit 7 is set.
Perhaps they should be valid in both tetrahedra.

\smallskip
The bits in character |types[i]| have the folowing meaning for $5\le i\le 10$,
refering to the $i-5$-th edge:
\item{$\cdot$} Bit 0 is set if the edge is in $K$.
\item{$\cdot$} Bit 1 is set if the edge is critical.
\item{$\cdot$} Bit 2 is set if the edge is paired with a vertex.
\item{$\cdot$} Bit 3 gives the relative index of that vertex if bit 2 is set. 
(the i so the vertex is |get_vertex(e,i)|.)
	Bits 3 and 4 give the index of the paired triangle if bits 1 and 2 are not set.
\item{$\cdot$} Bit 5 is set if the edge  is paired up and it is known 
	that all gradient paths to the edge do not come from critical triangles.
\item{$\cdot$} Bit 6 is set if the edge  is paired up and it is known 
	that all gradient paths from the edge do not lead to critical edges.
\item{$\cdot$} Bit 7 is set if this type character is valid.
A given edge in $K$ is valid in exactly one tetrahedron.
Bits 0 and 7 are correct in all tetrahedra containing the edge,
the others only if bit 7 is set.
However, bit 6, if set in some tetrahedron containing the edge, is valid.


@(simplex.h@>+=
typedef struct tetrahedronstruct
{
	UInt8 types[11]; 
	UInt8 flag; 
		
	simplex_id vertex_ids[4];
	simplex_id face_ids[4];
} tetrahedron;

@*1 Converting to and from a simplex id.
@(globals.c@>+=
#include "Morse.h"
int vtab[6][2] = {{0,1},{0,2},{0,3},{1,2},{1,3},{2,3}}; 
int etab[4][3] = {{10,9,8}, {10,7,6}, {9,7,5}, {8,6,5}}; 
int etab2[4][4] = {{0,5,6,7}, {5,0,8,9}, {6,8,0,10}, {7,9,10,0}};
int id_dim_tab[16] = {3,2,2,2,2,1,1,1,1,1,1,0,3,-2,-2,-1};
int id_val_tab[11] = {0,1,0,0,0,0,0,0,1,1,2};
int best_edge_tab[6] = {3,3,2,3,2,1};
UInt32 number_of_vertices, number_of_tetrahedra;
vertex *vertexlist;
tetrahedron *tlist;
list *crit[4];

@ @(globals.h@>+=
extern int vtab[6][2];
extern int etab[4][3]; 
extern int etab2[4][4];
extern int id_dim_tab[16];
extern int id_val_tab[11];
extern int best_edge_tab[6];
extern UInt32 number_of_vertices, number_of_tetrahedra;
extern vertex *vertexlist;
extern tetrahedron *tlist;
extern list *crit[4];

@ @(simplex.h@>+=
#define id2vertex(id)  (vertexlist+((id)>>4))
#define vertex_id(n) ((simplex_id)(((n)<<4) + 11))
#define id2tetrahedron(id) (tlist +((id)>>4))
#define id2tetra_id(id) ((id)&0xfffffff0)
#define tetrahedron_id(n) ((simplex_id)((n)<<4))
#define id_is_null(id) (((id)&15)==15)
#define NULL_ID 15
#define id_dimension(id) (id_dim_tab[(id)&15])
#define edge_index(id)  (((id)&15)-5)
#define face_index(id)  (((id)&15)-1)

void get_edge_vertices(simplex_id e,simplex_id *vl);
void get_triangle_vertices(simplex_id f, simplex_id  *vlist);
int triangle_in_tetrahedron(simplex_id f,simplex_id t);
list *vertex_star(simplex_id v);

@*1 Getting faces of a simplex.
@(simplex.h@>+=
#define get_vertex(e,i)  (id2tetrahedron(e)->vertex_ids[vtab[ edge_index(e)][i]])
#define get_tetrahedron_vertices(t,vlist)@/ \
	memcpy(vlist,id2tetrahedron(t)->vertex_ids,4*sizeof(simplex_id))
	
simplex_id other_vertex_in_edge(simplex_id u,simplex_id e);

@ @<Simplex functions@>+=
simplex_id other_vertex_in_edge(simplex_id u,simplex_id e)
{
	tetrahedron *t;
	simplex_id v;
	int j = edge_index(e);
	
	t = id2tetrahedron(e);
	v = t->vertex_ids[vtab[j][0]];
	if (u==v) v = t->vertex_ids[vtab[j][1]];
	return v;
}

void get_edge_vertices(simplex_id e,simplex_id *vl)
{
	tetrahedron *tp;
	int j = edge_index(e);
	
	tp = id2tetrahedron(e);
	vl[0] = tp->vertex_ids[vtab[j][0]];
	vl[1] = tp->vertex_ids[vtab[j][1]];
}

void get_triangle_vertices(simplex_id f,simplex_id  *vlist)
// return a list of the vertices of |f|
{
	int k = face_index(f);
	simplex_id *q = vlist;
	simplex_id *p = id2tetrahedron(f)->vertex_ids;
	
	if(k!=0) *q++ = *p;
	p++;
	if(k!=1) *q++ = *p;
	p++;
	if(k!=2) *q++ = *p;
	p++;
	if(k!=3) *q++ = *p;
}


@   |get_edge(f,i)| returns an id of the |i|-th edge of the triangle |f|.
|get_face(t,i)| returns an id of the i-th face of the terahedron |t|.
|get_edge_of_tetrahedron(s,i,j)| returns the edge whose vertices
are the i-th and j-th vertices of the tetrahedron containing |s|.
Usually |s| is a tetrahedron, but it could be an edge or triangle.

|coface(f,i)| returns a tetrahedron containing the triangle |f|.
|coface0(f) = coface(f,0)| and |coface1(f) = coface(f,1)|.

@(simplex.h@>+=
#define get_edge(f,i)  (id2tetra_id(f)+etab[face_index(f)][i])
#define get_face(t,i)  (id2tetra_id(t)+(i)+1)
#define get_edge_of_tetrahedron(s,i,j) (id2tetra_id(s)+etab2[i][j])
#define coface0(f) id2tetra_id(f)
#define coface1(f) id2tetra_id(id2tetrahedron(f)->face_ids[face_index(f)])
#define coface(f,i) (((i)==0)? coface0(f): coface1(f))
#define other_face_id(f) id2tetrahedron(f)->face_ids[face_index(f)]

simplex_id other_coface(simplex_id s,simplex_id t);
simplex_id other_edge(simplex_id v,simplex_id e,simplex_id t);


@ @<Simplex functions@>+=
simplex_id other_coface(simplex_id s,simplex_id t)
{
	if(id_dimension(s)!=2 || id_dimension(t)!=3) abort_message("internal coface");
	if(id2tetra_id(s) != t) return id2tetra_id(s);
	return id2tetra_id(id2tetrahedron(t)->face_ids[face_index(s)]);
}


@ Given a tetrahedron |t| and a list |fvl| of three vertices of |t|,
|vertices2triangle| and |vertices2trianglex| return the id of the corresponding triangle.
Use |vertices2trianglex| if the list of vertices |fvl|
might not be in order.
Likewise, |vertices2edge| returns the id of the edge corresponding to a list
of two vertices (in order).

@<Simplex functions@>+=

simplex_id vertices2triangle(simplex_id *fvl,simplex_id t)
{
	simplex_id *tvl = id2tetrahedron(t)->vertex_ids;
	
	if(fvl[0]==tvl[0] && fvl[1]==tvl[1] && fvl[2]==tvl[2]) return get_face(t,3);
	if(fvl[0]==tvl[0] && fvl[1]==tvl[1] && fvl[2]==tvl[3]) return get_face(t,2);
	if(fvl[0]==tvl[0] && fvl[1]==tvl[2] && fvl[2]==tvl[3]) return get_face(t,1);
	if(fvl[0]==tvl[1] && fvl[1]==tvl[2] && fvl[2]==tvl[3]) return get_face(t,0);
	return NULL_ID;
}

simplex_id vertices2trianglex(simplex_id *vl,simplex_id t)
{
	simplex_id *tvl = id2tetrahedron(t)->vertex_ids;
	simplex_id fvl[3];
	
	if(vl[0]>vl[1])
	{
		fvl[0]=vl[0];
		fvl[1]=vl[1];
	}
	else
	{
		fvl[0]=vl[1];
		fvl[1]=vl[0];
	}
	
	if(vl[2]<fvl[1]) fvl[2]=vl[2];
	else if(vl[2]<fvl[0])
	{
		fvl[2]=fvl[1];
		fvl[1]=vl[2];
	}
	else
	{
		fvl[2]=fvl[1];
		fvl[1]=fvl[0];
		fvl[0]=vl[2];
	}
	
	if(fvl[0]==tvl[0] && fvl[1]==tvl[1] && fvl[2]==tvl[2]) return get_face(t,3);
	if(fvl[0]==tvl[0] && fvl[1]==tvl[1] && fvl[2]==tvl[3]) return get_face(t,2);
	if(fvl[0]==tvl[0] && fvl[1]==tvl[2] && fvl[2]==tvl[3]) return get_face(t,1);
	if(fvl[0]==tvl[1] && fvl[1]==tvl[2] && fvl[2]==tvl[3]) return get_face(t,0);
	return NULL_ID;
}

simplex_id vertices2edge(simplex_id *fvl,simplex_id t)
{
	simplex_id *tvl = id2tetrahedron(t)->vertex_ids;
	int i,j;
	
	for(i=0;i<3;i++)
	{
		if(fvl[0]==tvl[i])
		{
			for(j=i+1;j<4;j++)
			{
				if(fvl[1]==tvl[j]) return get_edge_of_tetrahedron(t,i,j);
			}
		}
	}
	return NULL_ID;
}



@ These routines take a codimension one face of a simplex
and find the unique other codimension one face which also contains
the given codimension 2 face.


@<Simplex functions@>+=
simplex_id other_edge(simplex_id v,simplex_id e,simplex_id t)
// return the edge of the triangle t which contains v but is not e
{
	simplex_id evl[2],tvl[3];
	
	get_edge_vertices(e,evl);
	get_triangle_vertices(t,tvl);

	if(evl[0]==tvl[0] && evl[1]==tvl[1]) return (v==evl[0])? get_edge(t,1): get_edge(t,0);
	if(evl[0]==tvl[0] && evl[1]==tvl[2]) return (v==evl[0])? get_edge(t,2): get_edge(t,0);
	if(evl[0]==tvl[1] && evl[1]==tvl[2]) return (v==evl[0])? get_edge(t,2): get_edge(t,1);
	abort_message("edge not in triangle");	
}

simplex_id other_face(simplex_id e,simplex_id f,simplex_id t)
// Find the face of t which contains the edge e but is not f
{
	simplex_id evl[2],fvl[3];
	simplex_id *tvl = id2tetrahedron(t)->vertex_ids;
	int i,j;
	
	get_edge_vertices(e,evl);
	get_triangle_vertices(f,fvl);

	if(evl[0]==fvl[0] && evl[1]==fvl[1]) i=2;
	else if(evl[0]==fvl[0] && evl[1]==fvl[2]) i=1;
	else if(evl[0]==fvl[1] && evl[1]==fvl[2]) i=0;
	else abort_message("edge not in triangle");
	
	for(j=0;j<4;j++)
	{
		if(tvl[j]==fvl[i]) return get_face(t,j);
	}
	abort_message("triangle not in tetrahedron");	
}

@ Convert the dege id |e| to an id based on the tetrhedron |t|.
@<Simplex functions@>+=

simplex_id edge_id_in_tetrahedron(simplex_id e,simplex_id t)
{
	simplex_id evl[2];
	simplex_id *tvl = id2tetrahedron(t)->vertex_ids;
	
	get_edge_vertices(e,evl);
	
	if(evl[0]==tvl[0])
	{
		if(evl[1]==tvl[1]) return t+5;
		if(evl[1]==tvl[2]) return t+6;
		if(evl[1]==tvl[3]) return t+7;
		abort_message("internal error 23");
	}
	else if(evl[0]==tvl[1])
	{
		if(evl[1]==tvl[2]) return t+8;
		if(evl[1]==tvl[3]) return t+9;	
		abort_message("internal error 24");
	}
	return t + 10;
}


@ On input, |vert| is three vertices of the tetrahedron |t|.
(or |t| could be a face of some tetrahedron which is implied).
It must have |vert[0] > vert[1]|.
On output, |vert[2]| is changed to the vertex of |t| which is not in |vert|.
The return value is the unique neighboring tetrahedron |tp|
which contains the vertices |vert[0]|, |vert[1]|,
and the new |vert[2]|.
This allows you to successively travel the tetrahedra containing
a given edge.
This is sort of equivalent to |other_coface(other_face(e,f,t), t)|
where |e| is the edge with vertices |vert[0]| and |vert[1]|,
and |f| is the triangle with vertices |vert[i]|.
But it is a bit faster and returns a face id rather than
a tetrahedron id.
@<Simplex functions@>+=
simplex_id next_face(simplex_id *vert,simplex_id t)
{
	simplex_id *v = id2tetrahedron(t)->vertex_ids;
	int i,j;
	
	@<find |j,i| so |vert[2] = v[j]| and no |vert[k] = v[i]|@>@;
	
	vert[2] = v[i];
	return id2tetrahedron(t)->face_ids[j];
}

@ On input, |vert| is three vertices of a tetrahedron |t|
implied by the edge id |e|.
It must have |vert[0] > vert[1]| and the two vertices of |e|
are |vert[0]| and |vert[1]|
On output, |vert[2]| is changed to the vertex of |t| which is not in |vert|.
The return value is edge id of |e| based in the unique neighboring tetrahedron |tp|
which contains |e|,
and the new vertex |vert[2]|.
This allows you to successively travel the tetrahedra containing
a given edge.
@<Simplex functions@>+=
simplex_id next_edge(simplex_id *vert,simplex_id e)
{
	tetrahedron *tt = id2tetrahedron(e);
	simplex_id *v = tt->vertex_ids;
	simplex_id f;
	int i,j;
	
	@<find |j,i| so |vert[2] = v[j]| and no |vert[k] = v[i]|@>@;
	

	vert[2] = v[i];
	f = tt->face_ids[j];
	tt = id2tetrahedron(tt->face_ids[j]);
	v = tt->vertex_ids;
	
	if(v[0] == vert[0])
	{
		if(v[1] == vert[1]) return get_edge_of_tetrahedron(f,0,1);
		if(v[2] == vert[1]) return get_edge_of_tetrahedron(f,0,2);
		if(v[3] == vert[1]) return get_edge_of_tetrahedron(f,0,3);
		abort_message("next tetrahedron error 5");
	}
	if(v[1] == vert[0])
	{
		if(v[2] == vert[1]) return get_edge_of_tetrahedron(f,1,2);
		if(v[3] == vert[1]) return get_edge_of_tetrahedron(f,1,3);
		abort_message("next tetrahedron error 6");
	}
	if(v[2] != vert[0]) abort_message("next tetrahedron error 7");
	if(v[3] != vert[1]) abort_message("next tetrahedron error 8");
	return get_edge_of_tetrahedron(f,2,3);
}

@ @<find |j,i| so |vert[2] = v[j]| and no |vert[k] = v[i]|@>=
	if(v[0] == vert[0])
	{
		if(v[1] == vert[2])
		{
			j=1;
			if(v[2] == vert[1]) i=3; 
			else i=2;
		}
		else if(v[2] == vert[2])
		{
			j=2;
			if(v[1] == vert[1]) i=3; 
			else i=1;
		}
		else if(v[3] == vert[2])
		{
			j=3;
			if(v[2] == vert[1]) i=1; 
			else i=2;
		}
		else abort_message("next tetrahedron error 1");
	}
	else if(v[1] == vert[0])
	{
		if(v[0] == vert[2])
		{
			j=0;
			if(v[2] == vert[1]) i=3; 
			else i=2;
		}
		else if(v[2] == vert[2])
		{
			j=2;
			i = 0;
		}
		else if(v[3] == vert[2])
		{
			j=3;
			i=0;
		}
		else abort_message("next tetrahedron error 2");
	}
	else if(v[2] == vert[0])
	{
		if(v[0] == vert[2])
		{
			i=1;
			j=0;
		}
		else if(v[1] == vert[2])
		{
			i=0;
			j=1;
		}
		else abort_message("next tetrahedron error 3");
	}
	else abort_message("next tetrahedron error 4");


@ @<Simplex functions@>+=
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



@*1 Containment in the complex.
@(simplex.h@>+=
#define vertex_is_in_K(v) ((id2vertex(v)->type & 1)!=0)
#define edge_is_in_K(e) ((id2tetrahedron(e)->types[(e)&15] & 1)!=0)
#define triangle_is_in_K(f) ((id2tetrahedron(f)->types[(f)&15] & 1)!=0)
#define tetrahedron_is_in_K(t) ((id2tetrahedron(t)->types[0] & 1)!=0)


@*1 Simplex values.
The simplices are stored in order so you can compare values
just by comparing their ids.
@(simplex.h@>+=
#define vertex_value(v)  ((SInt32)id2vertex(v)->h)
#define vertex_compare(v,w) (v-w)
#define smallest_vertex_in_edge(e) get_vertex(e,1)
#define largest_vertex_in_edge(e) get_vertex(e,0)

@ @<Simplex functions@>+=

SInt32 value(simplex_id t)
{
	if((t&15)<11)
		return vertex_value(id2tetrahedron(t)->vertex_ids[id_val_tab[t&15]]);
	else if ((t&15)>11) abort_message("invalid id");
	return vertex_value(t);
}


@ Find the smallest or largest valued vertex in a list of vertices of length n
@<Simplex functions@>+=

int smallest_vertex(simplex_id *v,int n)
{
	int i,j;
	for(i=0,j=1;j<n;j++)
	{
		if(vertex_compare(v[j],v[i])<0) i=j;
	}
	return i;
}

int largest_vertex(simplex_id *v,int n)
{
	int i,j;
	for(i=0,j=1;j<n;j++)
	{
		if(vertex_compare(v[j],v[i])>0) i=j;
	}
	return i;
}

@ This routine tests whether the tetrahedron |t| is in the lower Star of the vertex |v|.
@(simplex.h@>+=
#define is_in_lower_Star(t,v) ((v) == id2tetrahedron(t)->vertex_ids[0])

Boolean edges_equal(simplex_id e0,simplex_id e1);
SInt32 min_value(simplex_id s,int d);


@ find the maximum value of the vertices of the simplex |s|
@<Simplex functions@>+=
SInt32 max_value(simplex_id s,int d)
{
	SInt32 m;
	tetrahedron *tp;
	
	if(id_is_null(s)) return -0x8000;  // smallest value
	
	switch(d)
	{
		case 3:
			tp = id2tetrahedron(s);
			m = value(tp->vertex_ids[0]);
			break;
		case 2:
			tp = id2tetrahedron(s);
			m = (face_index(s)==0)? value(tp->vertex_ids[1]): value(tp->vertex_ids[0]);
			break;
		case 1:
			m = value(get_vertex(s,0));
			break;
		default:
			abort_message("max value of simplex of unknown dimension");
	}
	return m;
}

@ find the minimum value of the vertices of the simplex |s|
@<Simplex functions@>+=
SInt32 min_value(simplex_id s,int d)
{
	SInt32 m;
	tetrahedron *tp;
	
	if(id_is_null(s)) return 0x7fff;  // largest value
	
	switch(d)
	{
		case 3:
			tp = id2tetrahedron(s);
			m = value(tp->vertex_ids[3]);
			break;
		case 2:
			tp = id2tetrahedron(s);
			m = (face_index(s)==3)? value(tp->vertex_ids[2]): value(tp->vertex_ids[3]);
			break;
		case 1:
			m = value(get_vertex(s,1));
			break;
		default:
			abort_message("max value of simplex of unknown dimension");
	}
	return m;
	
}

@ Put the distance between vertices |v0| and |v1| in |length|
and return |(value(v1)-value(v0))/length|.
@<Simplex functions@>+=
double alternate_vertex_value(simplex_id v0,simplex_id v1,double *length)
{
	GLfloat *u0,*u1,d,h;
	
	u0 = id2vertex(v0)->coords;
	u1 = id2vertex(v1)->coords;
	d = (u0[0]-u1[0])*(u0[0]-u1[0])+(u0[1]-u1[1])*(u0[1]-u1[1])+(u0[2]-u1[2])*(u0[2]-u1[2]);
	d = sqrt(d);
	if(length!=NULL) *length=d;
	h = value(v1)-value(v0);
	return h/d;
}

@ return the length of the cross product |(v1-v0)\times (v2-v0)|.
This is the sine of the angle formed by |v1,v0,v2| times the 
distance from |v0| to |v1| times the 
distance from |v0| to |v2|.
@<Simplex functions@>+=
double cross_length(simplex_id v0,simplex_id v1,simplex_id v2)
{
	GLfloat *u2,*u0,*u1,c[3];

	u2 = id2vertex(v2)->coords;
	u0 = id2vertex(v0)->coords;
	u1 = id2vertex(v1)->coords;
	
	c[0] = (u1[1]-u0[1])*(u2[2]-u0[2])-(u1[2]-u0[2])*(u2[1]-u0[1]);
	c[1] = (u1[2]-u0[2])*(u2[0]-u0[0])-(u1[0]-u0[0])*(u2[2]-u0[2]);
	c[2] = (u1[0]-u0[0])*(u2[1]-u0[1])-(u1[1]-u0[1])*(u2[0]-u0[0]);
	
	return sqrt(c[0]*c[0]+c[1]*c[1]+c[2]*c[2]);
}

@ return the alternate value of edge |e1| relative to edge |e0|.
@<Simplex functions@>+=

double alternate_edge_value_basic(simplex_id e0,simplex_id e1)
{
	double l0,l1,h0,h1,c;
	simplex_id v,v0,v1;
	
	v = get_vertex(e0,0);
	if(v!=get_vertex(e1,0)) abort_message("alternate_edge_value error");
	v0 = get_vertex(e0,1);
	v1 = get_vertex(e1,1);
	h0 = alternate_vertex_value(v,v0,&l0);
	h1 = alternate_vertex_value(v,v1,&l1);
	
	c = cross_length(v,v0,v1);
	
	return (h1-h0)*l0*l1/c;
}

@ A speedier version of the above.
Here |v1| is the lower vertex of the edge |e1| and the 
alternate vale and length of |e0| are a;ready known
so they do not need to be recalculated.
@<Simplex functions@>+=

double alternate_edge_value(simplex_id e0,simplex_id v1,double e0_value,double e0_length)
{
	double l1,h1,c;
	simplex_id v,v0;
	
	v = get_vertex(e0,0);
	v0 = get_vertex(e0,1);
	h1 = alternate_vertex_value(v,v1,&l1);
	
	c = cross_length(v,v0,v1);
	
	return (h1-e0_value)*e0_length*l1/c;
}



@*1 Deadend simplices.
@(simplex.h@>+=
Boolean edge_is_deadend(simplex_id *e);
Boolean edge_is_deadendx(simplex_id *e);
Boolean triangle_is_deadend(simplex_id *t);
Boolean tetrahedron_is_deadend(simplex_id t);
void make_edge_deadend(simplex_id *e);
void make_triangle_deadend(simplex_id *t);
void make_tetrahedron_deadend(simplex_id t);

#define edge_is_deadend(e) ((id2tetrahedron(*e)->types[(*e)&15] & 64)!=0)
#define edge_is_deadendx(e) edge_is_deadend(e)

@ @<Simplex functions@>+=




Boolean triangle_is_deadend(simplex_id *t)
{
	tetrahedron *tp;
	
	*t = best_face_id(*t);
	
	tp = id2tetrahedron(*t);
	return (tp->types[(*t)&15] & 64)!=0;
}

Boolean tetrahedron_is_deadend(simplex_id t)
{
	tetrahedron *tp;
	
	tp = id2tetrahedron(t);
	return (tp->types[0] & 64)!=0;
}

void make_edge_deadend(simplex_id *e)
{
	simplex_id vlist[3];
	simplex_id t,ee;
	
	t = id2tetra_id(*e);
	get_edge_vertices(*e,vlist);
	vlist[2] = id2tetrahedron(t)->vertex_ids[best_edge_tab[edge_index(*e)]];

	ee = *e;
	do
	{
		id2tetrahedron(ee)->types[ee&15] |= 64;
		ee = next_edge(vlist,ee);
	} while(ee!=*e);
}

void make_triangle_deadend(simplex_id *t)
{
	tetrahedron *tp;
	
	*t = best_face_id(*t);
	
	tp = id2tetrahedron(*t);
	tp->types[(*t)&15] |= 64;
}

void make_tetrahedron_deadend(simplex_id t)
{
	tetrahedron *tp;
	
	tp = id2tetrahedron(t);
	tp->types[0] |= 64;
}

Boolean edge_is_xdeadend(simplex_id *e)
{
	tetrahedron *tp;
	
	*e = best_edge_id(*e);
	
	tp = id2tetrahedron(*e);
	return (tp->types[(*e)&15] & 32)!=0;
}

Boolean triangle_is_xdeadend(simplex_id *t)
{
	tetrahedron *tp;
	
	*t = best_face_id(*t);
	
	tp = id2tetrahedron(*t);
	return (tp->types[(*t)&15] & 32)!=0;
}

void make_edge_xdeadend(simplex_id *e)
{
	tetrahedron *tp;
	
	tp = id2tetrahedron(*e);
	tp->types[(*e)&15] |= 32;

	*e = best_edge_id(*e);
	
	tp = id2tetrahedron(*e);
	tp->types[(*e)&15] |= 32;
}

void make_triangle_xdeadend(simplex_id *t)
{
	tetrahedron *tp;
	
	*t = best_face_id(*t);
	
	tp = id2tetrahedron(*t);
	tp->types[(*t)&15] |= 32;
}



@*1 face containment tests.
These routines test whether a given simplex is a codimension one face of another,
and if it is, returns the index of that face.
If it is not, then -1 is returned.
@(simplex.h@>+=
#define vertex_in_edge(v,e) (((v) == get_vertex(e,0))?0:(((v) == get_vertex(e,1))?1:-1))


@ @<Simplex functions@>+=
int edge_in_triangle(simplex_id e,simplex_id t)
{
	simplex_id evl[2],tvl[3];
	
	get_edge_vertices(e,evl);
	get_triangle_vertices(t,tvl);
	if(evl[0]==tvl[0] && evl[1]==tvl[1]) return 2;
	if(evl[0]==tvl[0] && evl[1]==tvl[2]) return 1;
	if(evl[0]==tvl[1] && evl[1]==tvl[2]) return 0;
	return -1;
}

int edge_verts_in_triangle(simplex_id *evl,simplex_id f)
{
	simplex_id *p = id2tetrahedron(f)->vertex_ids;
	
	switch(face_index(f))
	{
		case 0: 
			if(evl[0]==p[1] && evl[1]==p[2]) return 2;
			if(evl[0]==p[1] && evl[1]==p[3]) return 1;
			if(evl[0]==p[2] && evl[1]==p[3]) return 0;
			break;
		case 1: 
			if(evl[0]==p[0] && evl[1]==p[2]) return 2;
			if(evl[0]==p[0] && evl[1]==p[3]) return 1;
			if(evl[0]==p[2] && evl[1]==p[3]) return 0;
			break;
		case 2: 
			if(evl[0]==p[0] && evl[1]==p[1]) return 2;
			if(evl[0]==p[0] && evl[1]==p[3]) return 1;
			if(evl[0]==p[1] && evl[1]==p[3]) return 0;
			break;
		case 3: 
			if(evl[0]==p[0] && evl[1]==p[1]) return 2;
			if(evl[0]==p[0] && evl[1]==p[2]) return 1;
			if(evl[0]==p[1] && evl[1]==p[2]) return 0;
			break;
	}
	return -1;
}

int triangle_in_tetrahedron(simplex_id f,simplex_id t)
{
	simplex_id fvl[3];
	simplex_id *tvl = id2tetrahedron(t)->vertex_ids;
	
	if(t == id2tetra_id(f)) return(face_index(f));
	
	get_triangle_vertices(f,fvl);
		
	if(fvl[0]==tvl[0] && fvl[1]==tvl[1] && fvl[2]==tvl[2]) return 3;
	if(fvl[0]==tvl[0] && fvl[1]==tvl[1] && fvl[2]==tvl[3]) return 2;
	if(fvl[0]==tvl[0] && fvl[1]==tvl[2] && fvl[2]==tvl[3]) return 1;
	if(fvl[0]==tvl[1] && fvl[1]==tvl[2] && fvl[2]==tvl[3]) return 0;
	return -1;
}

@*1 Simplex equality tests.
These test whether two edges or triangles are equal.
They assume no two different edges have the same vertices.
If this were used on a CW complex where this might not be the case,
these would need to be changed to determine the optimum ids and compare them.

@<Simplex functions@>+=
Boolean edges_equal(simplex_id e0,simplex_id e1)
{
	simplex_id v0[2],v1[2];
	
	if(e0==e1) return true;
	if(id_is_null(e1) || id_is_null(e0)) return false;
	get_edge_vertices(e0,v0);
	get_edge_vertices(e1,v1);
	return (v0[0]==v1[0] && v0[1]==v1[1]);
}

Boolean triangles_equal(simplex_id f0,simplex_id f1)
{
	tetrahedron *tp;
	simplex_id f;
	int n;
	
	if (f0==f1) return true;
	if(id_is_null(f1) || id_is_null(f0)) return false;

	tp = id2tetrahedron(f1);
	f = tp->face_ids[face_index(f1)];
	return f0 == f;
}



@*1 Lists of simplices.
Often there are lists of simplex ids.
So the following are useful.
@(simplex.h@>+=
#define id_list_push(l,t) { simplex_id id_list_push_simplex = t; \
							(void)list_push(l,&id_list_push_simplex); }
simplex_id id_list_pop(list *l);
simplex_id id_list_read(list *l);

@ @<Simplex functions@>+=

simplex_id id_list_pop(list *l)
{
	simplex_id t;
	
	list_pop(l,&t);
	return t;
}

simplex_id id_list_read(list *l)
{
	simplex_id *r;
	r = (simplex_id *)list_read(l);
	if(r!=NULL) return *r;
	else return 0xffffffff; 
}




@*1 Critical simplices.
Here are various routines for critical simplices.
In |unmake_critical|, the 
formerly critical simplex is not actually removed from
the list of critical simplices.
So when running through |crit| you may need to 
check if the entries are in fact critical.
The routine |clean_crit| will delete all the noncritical entries from |crit|.
@(simplex.h@>+=
#define vertex_is_critical(v)  ( (id2vertex(v)->type & 2)!=0 )
#define unmake_vertex_critical(v)  ( id2vertex(v)->type &= 0x81 )
#define tetrahedron_is_critical(t)  ( (id2tetrahedron(t)->types[0] & 2)!=0 )
#define unmake_tetrahedron_critical(t)  ( id2tetrahedron(t)->types[0] &= 0x81 )

Boolean edge_is_critical(simplex_id *e);
Boolean triangle_is_critical(simplex_id *t);
void make_vertex_critical(simplex_id v);
void make_edge_critical(simplex_id *e);
void make_triangle_critical(simplex_id *f);
void make_tetrahedron_critical(simplex_id t);
void unmake_triangle_critical(simplex_id *f);
void unmake_edge_critical(simplex_id *e);
void clean_crit(void);


@ @<Simplex functions@>+=

Boolean edge_is_critical(simplex_id *e)
{
	tetrahedron *t;
	*e = best_edge_id(*e);
	t = id2tetrahedron(*e);
	return (id2tetrahedron(*e)->types[(*e)&15] & 2)!=0;
}

Boolean triangle_is_critical(simplex_id *t)
{
	tetrahedron *tt;
	*t = best_face_id(*t);
	tt = id2tetrahedron(*t);
	
	return (id2tetrahedron(*t)->types[(*t)&15] & 2)!=0;
}


@ @<Simplex functions@>+=
void make_vertex_critical(simplex_id v)
{
@q+ printf("Making 0 ");@>
@q+ print_simp(v);@>
@q+ printf(" critical\n");@>
	id2vertex(v)->type |= 2;
	id_list_push(crit[0],v);
}

void make_edge_critical(simplex_id *e)
{
@q+ printf("Making 1 ");@>
@q+ print_simp(*e);@>
@q+ printf(" critical\n");@>
	*e = best_edge_id(*e);
	id2tetrahedron(*e)->types[(*e)&15] |= 2;
	list_push(crit[1],e);
}

void make_triangle_critical(simplex_id *f)
{
@q+ printf("Making 2 ");@>
@q+ print_simp(*f);@>
@q+ printf(" critical\n");@>
	*f = best_face_id(*f);
	id2tetrahedron(*f)->types[(*f)&15] |= 2;
	list_push(crit[2],f);
}

void make_tetrahedron_critical(simplex_id t)
{
@q+ printf("Making 3 ");@>
@q+ print_simp(t);@>
@q+ printf(" critical\n");@>
	id2tetrahedron(t)->types[(t)&15] |= 2;
	id_list_push(crit[3],t);
}

@ @<Simplex functions@>+=

void unmake_edge_critical(simplex_id *e)
{
	*e = best_edge_id(*e);
	id2tetrahedron(*e)->types[(*e)&15] &= 0xe1;
}

void unmake_triangle_critical(simplex_id *f)
{
	*f = best_face_id(*f);
	id2tetrahedron(*f)->types[(*f)&15] &= 0x81;
}


void clean_crit(void)
{
	simplex_id *s;
	list *l;

	l = crit[0];
	list_read_init(l);
	while ((s = list_read(l))!= NULL)	
	{
		if(!vertex_is_critical(*s)) list_read_delete(l);
	}
	l = crit[1];
	list_read_init(l);
	while ((s = list_read(l))!= NULL)	
	{
		if(!edge_is_critical(s)) list_read_delete(l);
	}
	l = crit[2];
	list_read_init(l);
	while ((s = list_read(l))!= NULL)	
	{
		if(!triangle_is_critical(s)) list_read_delete(l);
	}
	l = crit[3];
	list_read_init(l);
	while ((s = list_read(l))!= NULL)	
	{
		if(!tetrahedron_is_critical(*s)) list_read_delete(l);
	}
}

void clean_crit_at(UInt32 at[4])
{
	simplex_id *s;
	list *l;

	l = crit[0];
	list_read_init_at(l,at[0]);
	while ((s = list_read(l))!= NULL)	
	{
		if(!vertex_is_critical(*s)) list_read_delete(l);
	}
	l = crit[1];
	list_read_init_at(l,at[1]);
	while ((s = list_read(l))!= NULL)	
	{
		if(!edge_is_critical(s)) list_read_delete(l);
	}
	l = crit[2];
	list_read_init_at(l,at[2]);
	while ((s = list_read(l))!= NULL)	
	{
		if(!triangle_is_critical(s)) list_read_delete(l);
	}
	l = crit[3];
	list_read_init_at(l,at[3]);
	while ((s = list_read(l))!= NULL)	
	{
		if(!tetrahedron_is_critical(*s)) list_read_delete(l);
	}
}

@*1 Making simplices valid.
A given triangle has two possible ids, and a given edge
has many possible ids, one for each tetrahedron which contains it.
In each case we pick one of these ids to be the preferred one.
We mark the corresponding tetrahedron valid to indicate this.
The tetrahedron with the valid id will correctly indicate
whether the edge or triangle is critical and if not, its paired simplex.
@(simplex.h@>+=
#define make_edge_valid(e) (id2tetrahedron(e)->types[(e)&15] |= 128)
#define make_triangle_valid(f) (id2tetrahedron(f)->types[(f)&15] |= 128)
#define edge_is_valid(e)  ((id2tetrahedron(e)->types[(e)&15] & 128)!=0)
#define triangle_is_valid(f)  ((id2tetrahedron(f)->types[(f)&15] & 128)!=0)


@ @<Simplex functions@>+=

void unmake_edge_valid(simplex_id *e)
{
	*e = best_edge_id(*e);
	id2tetrahedron(*e)->types[(*e)&15] &= 0x7f;
}

void unmake_triangle_valid(simplex_id *f)
{
	*f = best_face_id(*f);
	id2tetrahedron(*f)->types[(*f)&15] &= 0x7f;
}

// the following may be obsolete
void mark_tetrahedron_done(simplex_id v,simplex_id t) 
{
	tetrahedron *tp;
	int i;
	tp = id2tetrahedron(t);
	for(i=0;i<4;i++)
	{
		if (v==tp->vertex_ids[i])
		{
			tp->flag |= (1<<i);
			break;
		}
	}
}

Boolean tetrahedron_marked_done(simplex_id v,simplex_id t) 
{
	tetrahedron *tp;
	int i,j=0;
	tp = id2tetrahedron(t);
	for(i=0;i<4;i++)
	{
		if (v==tp->vertex_ids[i])
		{
			j = (tp->flag & (1<<i));
			break;
		}
	}
	return j!=0;
}

@*1 Finding optimal edge and triangle ids. 
@(simplex.h@>+=
simplex_id best_edge_idx(simplex_id e);
simplex_id best_face_id(simplex_id f);
#define best_edge_id(e) (edge_is_valid(e)?e:best_edge_idx(e))
#define best_face_id(f) (triangle_is_valid(f)?f:other_face_id(f))

@ @<Simplex functions@>+=
simplex_id best_edge_idx(simplex_id e)
{
	simplex_id vlist[3];
	simplex_id ee;

	get_edge_vertices(e,vlist);
	vlist[2] = id2tetrahedron(e)->vertex_ids[best_edge_tab[edge_index(e)]];
	
	for(ee = e;!edge_is_valid(ee);)
	{
		ee = next_edge(vlist,ee);
		if (ee==e) abort_message("internal error 25");
	}

	return ee;
}




@*1 Pairing simplices.  
The routines |..._is_paired_up| find whether the simplex |t| is paired with a
simplex of one higher dimension.
Likewise |..._is_paired_down| find whether the simplex |t| is paired with a
simplex of one lower dimension.
The routines |rij| return the dimension |j| simplex paired with the
dimension |i| simplex |t|.
The routines |pairij| pair up the i simplex |s0| and the j simplex |s1|.
@(simplex.h@>+=
#define vertex_is_paired_up(v)  ( (id2vertex(v)->type & 2)==0 )
#define tetrahedron_is_paired_down(t) ( (id2tetrahedron(t)->types[0] & 2)==0 )
#define r32(t) get_face(t,(id2tetrahedron(t)->types[0]>>3)&3) 
#define r01(v) ( id2vertex(v)->tetra_id + ((id2vertex(v)->type>>3)&7)+5 )

#define edge_is_paired_downx(e) (edge_is_valid(e)?(id2tetrahedron(e)->types[(e)&15]&6)==4:eipdx(e))

Boolean edge_is_paired_up(simplex_id *e);
Boolean triangle_is_paired_up(simplex_id *f);
Boolean edge_is_paired_down(simplex_id *e);
Boolean triangle_is_paired_down(simplex_id *f);
simplex_id r21(simplex_id *f);
simplex_id r10(simplex_id *e);
simplex_id r23(simplex_id *f);
simplex_id r12(simplex_id *e);
void pair01(simplex_id s0,simplex_id s1,Boolean flag);
void pair12(simplex_id *s0,simplex_id s1,Boolean flag);
void pair23(simplex_id *s0,simplex_id s1,Boolean flag);

@ @<Simplex functions@>+=

Boolean edge_is_paired_up(simplex_id *e)
{
	*e = best_edge_id(*e);
	return (id2tetrahedron(*e)->types[(*e)&15] & 6)==0;
}

Boolean triangle_is_paired_up(simplex_id *f)
{
	*f = best_face_id(*f);
	return (id2tetrahedron(*f)->types[(*f)&15] & 6)==0;
}

Boolean edge_is_paired_down(simplex_id *e)
{
	*e = best_edge_id(*e);
	return (id2tetrahedron(*e)->types[(*e)&15] & 6)==4;
}

Boolean eipdx(simplex_id e)
{
	simplex_id vl[2],e1;
	
	get_edge_vertices(e,vl);
	
	if(vertex_is_paired_up(vl[0]))
	{
		e1 = r01(vl[0]);
		if(vl[1] == get_vertex(e1,1)) return true;
	}
	if(vertex_is_paired_up(vl[1]))
	{
		e1 = r01(vl[1]);
		if(vl[0] == get_vertex(e1,0)) return true;
	}
	return false;
}

Boolean triangle_is_paired_down(simplex_id *f)
{
	*f = best_face_id(*f);
	return (id2tetrahedron(*f)->types[(*f)&15] & 6)==4;
}


simplex_id r21(simplex_id *f)
{
	*f = best_face_id(*f);
	return get_edge(*f,(id2tetrahedron(*f)->types[(*f)&15]>>3)&3);
}

simplex_id r10(simplex_id *e)
{
	*e = best_edge_id(*e);
	return get_vertex(*e,(id2tetrahedron(*e)->types[(*e)&15]>>3)&1);
}

simplex_id r23(simplex_id *f)
{
	*f = best_face_id(*f);
	return id2tetra_id(*f);
}

simplex_id r12(simplex_id *e)
{
	*e = best_edge_id(*e);
	return get_face(id2tetra_id(*e),(id2tetrahedron(*e)->types[(*e)&15]>>3)&3);
}



@ @<Simplex functions@>+=
void pair01(simplex_id s0,simplex_id s1,Boolean flag)
{
	simplex_id s;

@q+ printf("01 Pairing ");@>
@q+ print_simp(s0);@>
@q+ printf(" with ");@>
@q+ print_simp(s1);@>
@q+ printf("\n");@>

	
	if(flag)
	{
		s = s1;
		unmake_edge_valid(&s);
	}
	
	id2vertex(s0)->tetra_id = id2tetra_id(s1);
	id2vertex(s0)->type = (UInt8)(1+(edge_index(s1)<<3));
	id2tetrahedron(s1)->types[s1&15] = (UInt8)((s0==get_vertex(s1,0))? 5+128: 5+128+8);
} 

void pair12(simplex_id *s0,simplex_id s1,Boolean flag)
{
	int n;
	tetrahedron *tp;
	simplex_id s;


@q+ printf("12 Pairing ");@>
@q+ print_simp(*s0);@>
@q+ printf(" with ");@>
@q+ print_simp(s1);@>
@q+ printf("\n");@>

	if(flag)
	{
		s = s1;
		unmake_triangle_valid(&s);
		s = *s0;
		unmake_edge_valid(&s);
	}

	n = edge_in_triangle(*s0,s1);
	tp = id2tetrahedron(s1);
	
	tp->types[s1&15] = (UInt8)(5 + 128 + (n<<3));
	*s0 = get_edge(s1,n);
	tp->types[(*s0)&15] = (UInt8)(1 + 128 + (face_index(s1)<<3));
} 

void pair23(simplex_id *s0,simplex_id s1,Boolean flag)
{
	int n;
	tetrahedron *tp;
	simplex_id s;

@q+ printf("23 Pairing ");@>
@q+ print_simp(*s0);@>
@q+ printf(" with ");@>
@q+ print_simp(s1);@>
@q+ printf("\n");@>

	if(flag)
	{
		s = *s0;
		unmake_triangle_valid(&s);
	}
	n = triangle_in_tetrahedron(*s0,s1);
	tp = id2tetrahedron(s1);
	*s0 = get_face(s1,n);

	tp->types[0] = (UInt8)(1 + 128 + (n<<3));
	tp->types[(*s0)&15] = 1 + 128;
} 

@*1 Links of simplices. 

@ This creates a list of tetrahedra |simplex_id|s in the star of a vertex |v|.
The list has a vague ordering, first on the list are tetrahedra
in the lower star of |v|, next come tetrahedra for which |v| is
the second highest vertex (so some face(s) are in the lower star),
next come tetrahedra for which |v| is
the third highest vertex (so some edge(s) are in the lower star),
and last come tetrahedra for which |v| is the lowest vertex.

@<Simplex functions@>+=

list *vertex_star(simplex_id v)
{
	hlist *l;
	list *tl;
	int i;
	vertex *vv;
	
	hlist_initialize(&l,sizeof(tetrahedron *),4,2,0);
	list_initialize(&tl,sizeof(simplex_id));
	vv = id2vertex(v);
	if(id_is_null(vv->tetra_id))
		@<find a tetrahedron containing |v|@>@;
	@<put star of |v| on list |l|@>@;
	for(i=0;i<4;i++)
		@<push tetrahedra with i-th vertex |v| to |tl|@>@; 
	hlist_abandon(&l);
	return tl;
}

@ @<put star of |v| on list |l|@>=
{
	tetrahedron *tt;
	simplex_id t;
	SInt32 m[2];
	Boolean flag;

	list_push(tl,&(vv->tetra_id));
	while(!list_is_empty(tl))
	{
		list_pop(tl,m+1);
		tt = id2tetrahedron(m[1]);
		m[1] = id2tetra_id(m[1]);
		for(i=0;i<4;i++)
		{
			if(tt->vertex_ids[i]==v) {m[0] = i; break;}
		}
		hlist_find_add(l,m,&tt,&flag);
		if(!flag)
		{
			for(i=0;i<4;i++)
			{
				if(tt->vertex_ids[i]!=v) 
				{
					list_push(tl,tt->face_ids+i);
				}
			}
		}
	}
}

@ @<push tetrahedra with i-th vertex |v| to |tl|@>=
{
	SInt32 m[2];

	m[0]=i;
	
	while(hlist_sub_match(l,m,NULL,1))
	{
		list_push(tl,m+1);
	}
}

@ When the complex was read in, critical vertices did not
have their |tetra_id| field filled in.
This only becomes a problem now, so we remedy it.
We could also do this more efficiently-
when a tetrahedron is read in, set the |tetra_id| of its
lowest vertex to 15 + the id of the tetrahedron.
This will be interpreted as a |NULL_ID|,
but is easily converted to a valid id.

@<find a tetrahedron containing |v|@>=
{
	UInt32 j;
	tetrahedron *tt;
	
	for(j=0;j<number_of_tetrahedra;j++)
	{
		tt = id2tetrahedron( tetrahedron_id(j));
		if(tt->vertex_ids[3]==v)
		{
			vv->tetra_id = tetrahedron_id(j);
			break;
		}
	}
	if(j==number_of_tetrahedra) abort_message("internal error: tetra_id");
}

@* Checking Complexes. 
Check the discrete Morse function in the parts of $K$ in the 
lower stars of vertices $\ge w$ for consistency.
Form lists of all their edges and triangles and check that
each simplex is either critical or paired with a simplex
which is paired to it.

@<Subroutines for constructing complexes@>+=
void complex_check(simplex_id w)
{
	hlist *edges,*faces;
	SInt32 i,j,k;
	simplex_id v,e,f,t,vlist[4];
	
	hlist_initialize(&edges,sizeof(simplex_id),12713,2,0);
	hlist_initialize(&faces,sizeof(simplex_id),12713,3,0);
	
	for(i=(w>>4);i<number_of_vertices;i++)
	{
		v = vertex_id(i);
		if(vertex_is_in_K(v)) 
			@<check vertex@>@;
	}
	
	for(i=0;i<number_of_tetrahedra;i++)
	{
		t = tetrahedron_id(i);
		get_tetrahedron_vertices(t,vlist);
		if(vlist[0]<w) continue;
		for(j=0;j<4;j++)
		{
			f = get_face(t,j);
			if(triangle_is_in_K(f) && (j>0||vlist[1]>=w))
				@<check face@>@;
			for(k=j+1;k<4;k++)
			{
				e = get_edge_of_tetrahedron(t,j,k);
				if(edge_is_in_K(e)&&vlist[j]>=w)
					@<check edge@>@;
			}
		}
	}
	
	hlist_abandon(&edges);
	hlist_abandon(&faces);
}

@ @<check vertex@>=
{
	if(!vertex_is_critical(v))
	{
		e = r01(v);
		@<check valid edge@>@;
		@<check paired-down edge@>@;
	}
}

@ @<check edge@>=
{
	e = best_edge_id(e);
	@<check valid edge@>@;
	if(edge_is_paired_up(&e))
	{
		f = r12(&e);
		@<check valid face@>@;
		@<check paired-up edge@>@;
		@<check paired-down face@>@;
	}
	else if(edge_is_paired_down(&e))
	{
		v = r10(&e);
		@<check paired-down edge@>@;
	}
	else if(!edge_is_critical(&e)) abort_message("nothing edge");
}

@ @<check paired-down edge@>=
{
	if(!edge_is_paired_down(&e)) abort_message("should be paired-down edge");
	if(v!=r10(&e)) abort_message("nonsymmetric pairing");
}

@ @<check paired-up edge@>=
{
	if(!edge_is_paired_up(&e)) abort_message("should be paired-up edge");
	if(f!=r12(&e)) abort_message("nonsymmetric pairing");
}

@ @<check valid edge@>=
{
	Boolean flag;
	SInt32 index;
	
	if(id_dimension(e)!=1) abort_message("edge not an edge");
	if(!edge_is_valid(e)) abort_message("non-valid edge");
	get_edge_vertices(e,vlist);
	index = hlist_find_add(edges,vlist,&e,&flag);
	if(flag)
	{
		if(e!= *((simplex_id *)hlist_entry(edges,index))) abort_message("edge has two valid ids");
	}
}

@ @<check face@>=
{
	simplex_id t1;
	tetrahedron *ttt,*tttt;
	
	f = best_face_id(f);
	
	ttt = id2tetrahedron(f);
	tttt = id2tetrahedron(t);
	
	@<check valid face@>@;
	if(triangle_is_paired_up(&f))
	{
		t1 = r23(&f);
		@<check paired-down tetrahedron@>@;
		@<check paired-up face@>@;
	}
	else if(triangle_is_paired_down(&f))
	{
		e = r21(&f);
		@<check valid edge@>@;
		@<check paired-up edge@>@;
		@<check paired-down face@>@;
	}
	else if(!triangle_is_critical(&f)) abort_message("nothing triangle");
}

@ @<check paired-down face@>=
{
	if(!triangle_is_paired_down(&f)) abort_message("should be paired-down face");
	if(e!=r21(&f)) abort_message("nonsymmetric pairing");
}

@ @<check paired-up face@>=
{
	if(!triangle_is_paired_up(&f)) abort_message("should be paired-up face");
	if(t1!=r23(&f)) abort_message("nonsymmetric pairing");
}


@ @<check valid face@>=
{
	Boolean flag;
	SInt32 index;
	
	if(id_dimension(f)!=2) abort_message("face not a face");
	if(!triangle_is_valid(f)) abort_message("non-valid face");
	get_triangle_vertices(f,vlist);
	index = hlist_find_add(faces,vlist,&f,&flag);
	if(flag)
	{
		if(f!= *((simplex_id *)hlist_entry(faces,index))) abort_message("face has two valid ids");
	}
}

@ @<check paired-down tetrahedron@>=
{
	if(!tetrahedron_is_paired_down(t1)) abort_message("should be paired-down tetrahedron");
	if(f!=r32(t1)) abort_message("nonsymmetric pairing");
}



@* Constructing Complexes. 

@*1 Allocating vertices and tetrahedra.
The following routines allocate a new simplex.
The last parameter |flag| is  $=0$ if the simplex is not in |K|, 
and is $!=0$ if it is a vertex in |K| or
if it is a tetrahedron and you want it to be in |K| if and only if all its
vertices are in |K|.

@<Subroutines for constructing complexes@>+=
vertex *new_vertex(UInt32 n,SInt16 val,float x,float y,float z,Boolean flag)
{
	vertex *v;
	
	v = id2vertex( vertex_id(n));
	
	v->type=(UInt8)((flag)?1:0); // indicate in $K$ or not
	v->tetra_id=NULL_ID;
	v->h = val;
	v->coords[0]=x;
	v->coords[1]=y;
	v->coords[2]=z;
	
	return v;
}



@ @<Subroutines for constructing complexes@>+=
tetrahedron *new_tetrahedron(UInt32 n,UInt32 *vs,Boolean flag)
{
	tetrahedron *t;
	int i;
	Boolean flagp;
	
	t = id2tetrahedron( tetrahedron_id(n));
	
	t->flag = 0;
	
	flagp = flag;

	for(i=0;i<4;i++)
	{
		t->vertex_ids[i] = vertex_id(vs[i]);
		flagp = flagp && vertex_is_in_K(t->vertex_ids[i]);
		t->face_ids[i] = NULL_ID;
	}
	t->types[0]=(UInt8)((flagp)?1:0); 
	
	@<make sure vertices are in order@>@; 

	@<determine faces of |t| in K@>@;

	@<determine edges of |t| in K@>@;
	
	return t;
}

@ @<make sure vertices are in order@>=
{
	simplex_id temp;
	int j;
	
	for(j=0;j<3;j++)
	{
		for(i=j+1;i<4;i++)
		{
			if(vertex_compare(t->vertex_ids[j],t->vertex_ids[i]) < 0)
			{
				temp = t->vertex_ids[j];
				t->vertex_ids[j] = t->vertex_ids[i];
				t->vertex_ids[i] = temp;
			}
		}
	}
}

@ @<determine faces of |t| in K@>=
{
	int j;
	
	for(i=0;i<4;i++)
	{
		flagp = true;
		for(j=0;j<4;j++)
		{
			if(i==j) continue;
			flagp = flagp && vertex_is_in_K(t->vertex_ids[j]);
		}
		t->types[1+i] = (UInt8)((flagp)?1:0);
	}
}

@ @<determine edges of |t| in K@>=
{
	int j;
	
	for(i=0;i<3;i++)
	{
		for(j=i+1;j<4;j++)
		{
			flagp = vertex_is_in_K(t->vertex_ids[i]) && vertex_is_in_K(t->vertex_ids[j]);
			t->types[etab2[i][j]] = (UInt8)((flagp)?1:0);
		}
	}
}



@*1 Reading in a complex.
For now we suppose that the ambient complex is a manifold, later
versions could add in tetrahedra as needed to make the
complex a pseudo manifold if it is not already.
We also assume that the complex $K$ of interest is almost full,
in the sense that an edge or triangle is in $K$
if and only if all its vertices are in $K$.
It would be a trivial matter to read in a file of exceptions to this.
@(simplex.h@>+=
void read_in_complex(FILE *dfv,FILE *dft,double min,double max,Boolean qdelaunay,
							double scalex,double scaley,double scalez,
							Boolean optional_link_values);

@ @<Subroutines for constructing complexes@>+=
@<construction globals@>@;
void
read_in_complex(FILE *dfv,FILE *dft,double min,double max,Boolean qdelaunay,
							double scalex,double scaley,double scalez,
							Boolean optional_link_values)
{
	UInt32 i;
	UInt32 *translation; // permutation which sorts the vertices

	if(max<=min) abort_message("max is less than min");
	@<read in vertices@>@;
	@<read in tetrahedra@>@;
	free(translation);
	@<link tetrahedra and find an unoptimized discrete Morse function@>@;
}	

@ Read in and sort the vertices by value.
@<construction globals@>+=
list *vertex_blocks; // blocks of 256 vertices
int vertex_sort(const void *nn, const void *mm);

@ @<read in vertices@>=
{
	float actual_min,actual_max;
	UInt32 *translation1;
	
	list_initialize(&vertex_blocks,sizeof(float *));
	number_of_vertices=0;
	
	@<read vertices into |vertex_blocks|@>@;
	vertexlist = (vertex *)malloc( (number_of_vertices) * sizeof(vertex) );
	if (vertexlist == NULL ) abort_message("Out of memory");
	translation1 = (UInt32 *)malloc( (number_of_vertices) * sizeof(UInt32) );
	if (translation1 == NULL ) abort_message("Out of memory");
	for(i=0;i<number_of_vertices;i++) translation1[i]=i;
	@<sort vertices by permuting |translation1|@>@;
	for(i=0;i<number_of_vertices;i++)
		@<let |vertexlist[i]| be the |translation1[i]|-th vertex@>@;
	list_abandon(&vertex_blocks);
	translation = (UInt32 *)malloc( (number_of_vertices) * sizeof(UInt32) );
	if (translation == NULL ) abort_message("Out of memory");
	// invert the permutation |translation1| to |translation|
	for(i=0;i<number_of_vertices;i++) translation[translation1[i]]=i;
	free(translation1);
}

@ @<read vertices into |vertex_blocks|@>=
{
	float x,y,z,h;
	int n;
	int c;
	float *p;

	do
	{
		c = getc(dfv);
		if(c==EOF) break;
		if(c!='x') 
		{
			(void)ungetc(c,dfv);
			n = fscanf(dfv,"%f %f %f %f\n",&x,&y,&z,&h);
			if(n!=4) abort_message("bad vertex file");
			if(number_of_vertices==0) actual_min=actual_max=h;
			else if(h<actual_min) actual_min=h;
			else if(h>actual_max) actual_max=h;
			x/=scalex;
			y/=scaley;
			z/=scalez;
		}
		else
		{
			@<read to end of line@>@;	
			x = y = z = h = PLUS_INFINITY;
		}
		@<put vertex into |vertex_blocks|@>@;
		
		number_of_vertices++;
	}
	while(true);
}

@ @<read to end of line@>=
{
	do
	{
		c = getc(dfv);
	}
	while(c!=EOF && c!= '\n' && c!= '\r');
}

@ @<put vertex into |vertex_blocks|@>=
{
	if((number_of_vertices & 0xff)==0)
	{
		p = (float *)malloc(256*4*sizeof(float));
		if(p==NULL) abort_message("out of memory");
		list_push(vertex_blocks,&p);
	}
	*p++ = h;
	*p++ = x;
	*p++ = y;
	*p++ = z;
}

@ @<sort vertices by permuting |translation1|@>=
	qsort(translation1, number_of_vertices, sizeof(UInt32),vertex_sort);

@ @<Subroutines for constructing complexes@>+=
int vertex_sort(const void *nn, const void *mm)
{
	float *p,*q;
	UInt32 n,m;
	
	n = *(UInt32 *)nn;
	m = *(UInt32 *)mm;
	p = *(float **)list_entry(vertex_blocks,n>>8);
	q = *(float **)list_entry(vertex_blocks,m>>8);
	p+=((n&0xff)*4);
	q+=((m&0xff)*4);
	if(*p<*q) return -1;
	if(*p>*q) return 1;
	return 0;
}

@ @<let |vertexlist[i]| be the |translation1[i]|-th vertex@>=
{
	float *p;
	SInt16 val;
	
	p = *(float **)list_entry(vertex_blocks,translation1[i]>>8);
	p+=((translation1[i]&0xff)*4);
	
	if(*p<min) val = -32768;
	else if(*p>max) val = 32767;
	else val = (SInt32)(-32768.0+65535.0*(*p - min)/(max-min));
	
	new_vertex(i, val ,p[1],p[2],p[3], *p!=PLUS_INFINITY);
}



@ @<read in tetrahedra@>=
{
	@<first, read tetrahedra just to find how many there are@>@;
	tlist = (tetrahedron *)malloc( number_of_tetrahedra * sizeof(tetrahedron) );
	if (tlist == NULL ) abort_message("Out of memory");
	@<now read in tetrahedra for real@>@;
}

@ @<first, read tetrahedra just to find how many there are@>=
{
	long vi[4];
	int c;
	int n;
	
	if(qdelaunay) 
	{
		fscanf(dft,"%li\n",vi);
		number_of_tetrahedra = vi[0]+1;
	}
	else
	{
		number_of_tetrahedra = 0;
		
		do
		{
			@<read in one tetrahedron@>@;
			if(n!=4) break;
			number_of_tetrahedra++;
		} while (true);
		rewind(dft);
	}
}

@ @<now read in tetrahedra for real@>=
{
	long vi[4];
	int c;
	UInt32 vn[4];
	int j,n;
	tetrahedron *t;
	
	for(i=0;i<number_of_tetrahedra;i++)
	{
		@<read in one tetrahedron@>@;
		for(j=0;j<4;j++) vn[j] = translation[vi[j]];
		t = new_tetrahedron(i,vn,c!='x');
		@<pre-link the tetrahedron |t|@>@;
@q+1 {@>
@q+1 int j;@>
@q+1 print_simp(tetrahedron_id(i));@>
@q+1 printf(" ");@>
@q+1 for(j=0;j<11;j++) printf("%d",t->types[j]);@>
@q+1 printf("\n");@>
@q+1 }@>
	}
}

@ @<read in one tetrahedron@>=
{
	if(qdelaunay && i==0) 
	{
		// this is a tetrahedron at infinity, not in the input file
		vi[0] = number_of_vertices - 4;
		vi[1] = number_of_vertices - 3;
		vi[2] = number_of_vertices - 2;
		vi[3] = number_of_vertices - 1;
		c = 'x';
		n = 4;
	}
	else
	{
		c = getc(dft);
		if(c!='x') (void)ungetc(c,dft);
		n = fscanf(dft,"%li %li %li %li\n",vi,vi+1,vi+2,vi+3);
	}
}


@ When first read in, a tetrahedron's |face_ids| fields are set to very different
values from their eventual use, with very different meanings as follows:
For each edge with a nonempty lower star, there is one tetrahedron
in its lower star whose fields are as follows:
\item{} |face_ids[0]| is |NULL_ID| or the id of a tetrahedron with the same first vertex
but with a second vertex of lower value.
\item{} |face_ids[1]| is |NULL_ID| or the id of a tetrahedron with the same first vertex
but with a second vertex of higher value.
\item{} |face_ids[2]| is |NULL_ID| or the id of a tetrahedron 
with the same first and second vertex, i.e., another tetrahedron in the lower star
of the edge.  
\item{} |face_ids[3]| is ignored.

The first two give a binary tree of the 
locally three dimensional edges in the lower star
of the first vertex.  The third starts a linked list of the tetrahedra
in the lower star of that edge.
All the other tetrahedra in the lower star of the edge have fields as follows:
\item{} |face_ids[i]| is ignored for $i=0,1,3$. 
\item{} |face_ids[2]| is |NULL_ID| or the id of a tetrahedron 
with the same first and second vertex.

@<pre-link the tetrahedron |t|@>=
{
	simplex_id *tp = &(id2vertex(t->vertex_ids[0])->tetra_id);
	tetrahedron *tpt;
	SInt32 res;
	double best_val,tree_val;
	
	if(optional_link_values)
	{
		if(t->types[0]&1)
			@<set |t->flag| to optional vertex order@>@;
		else
		{
			// not in K
			t->flag = 0x39;
			best_val = PLUS_INFINITY;
		}
	}
	else 
		t->flag = 0x39;
	
	while(*tp != NULL_ID)
	{
		tpt = id2tetrahedron(*tp);
		if(tpt->vertex_ids[tpt->flag&3] == t->vertex_ids[t->flag&3])
			break;
		if(optional_link_values)
		{
			tree_val = alternate_vertex_value(t->vertex_ids[0],
					tpt->vertex_ids[tpt->flag&3],NULL);
			if (tree_val <  best_val) res = 1;
			else if (tree_val >  best_val) res = -1;
			else res = t->vertex_ids[t->flag&3] - tpt->vertex_ids[tpt->flag&3];
		}
		else
			res = vertex_compare(t->vertex_ids[1],tpt->vertex_ids[1]);
		if(res<0) tp = tpt->face_ids;
		else if(res>0) tp = tpt->face_ids+1;
		else abort_message("Huh?");
	}
	if(*tp != NULL_ID)
	{
		tp = tpt->face_ids+2;
		t->face_ids[2] = *tp;
	}
	*tp = tetrahedron_id(i);
}

@ @<set |t->flag| to optional vertex order@>=
{
	double vals[3];
	
	vals[0] = alternate_vertex_value(t->vertex_ids[0],t->vertex_ids[1],NULL);
	vals[1] = alternate_vertex_value(t->vertex_ids[0],t->vertex_ids[2],NULL);
	vals[2] = alternate_vertex_value(t->vertex_ids[0],t->vertex_ids[3],NULL);
	
	if(vals[0]>=vals[1])
	{
		if(vals[0]>=vals[2])
		{
			if(vals[1]>=vals[2]) t->flag = 0x39;
			else t->flag = 0x2d;
		}
		else t->flag = 0x27;
	}
	else
	{
		if(vals[1]>=vals[2])
		{
			if(vals[0]>=vals[2]) t->flag = 0x36;
			else t->flag = 0x1e;
		}
		else t->flag = 0x1b;
	}
	
	best_val = vals[(t->flag&3)-1];
}

@ This bit of code does two things at once.
It links up all the tetrahedra with neighboring tetrahedra
which share a face with it, and it puts a discrete Morse function
on the complex (a procedure called |ExtractRaw| in \cite{KKM}).
In particular it runs through all the vertices |v| in the complex,
first those not in |K| and then those in |K| in order of value,
largest value first.

For each vertex |v|, it links up each tetrahedron
in the lower star of |v| with any of its neighboring tetrahedra
which were previously encontered.
(The lower Star of |v| is all simplices for which |v|
is the vertex of maximal value.)
For any neighboring tetrahedron not yet encountered, the common face
is put on either the list |unglued_faces| (if the face does not contain |v|)
or |local_unglued_faces| (if the face contains |v| but not the second highest
vertex of the tetrahedron) or on |more_local_unglued_faces| 
(if the face contains both |v| and the second highest
vertex of the tetrahedron).

For each vertex |v|, if |v| is in |K|
it also finds an optimal discrete Morse function on the lower Star of |v|.
It then takes one critical edge |beste| in the lower Star of |v|,
makes it noncritical, and pairs it with |v|.
The choice of |beste| dependes on |optional_link_values|.
If |optional_link_values| is false,
it chooses |beste| so that its other vertex |bestv|
has value as small as possible.
If |optional_link_values| is true, 
it chooses |beste| so that the quantity 
(|value|(|v|) - |value|(|bestv|))/|length|(|beste|)
is as negative as possible.

Thus the discrete gradient flow from |v| follows the path of steepest descent
in some sense.
It is the genuine steepest edge if |optional_link_values| is true.
But if |optional_link_values| is false, it follows the edge which decreases
the function the most.
This may not be the steepest edge if it is relatively long.

The next cancelling step cancels all pairs of critical simplices it can
in the lower star so that in the end you get as few of them
as possible.  
Note that if the lower Star of |v| is empty, you make the 
vertex |v| critical, since |v| is a local minimum.

This also sets up lists |lone_faces|, |possible_lone_edges|, |local_done_edges|,
|local_lone_edges|, and |components|
which will be explained later.

@<construction globals@>=
	hlist *unglued_faces;  // triangles we have only seen in one tetrahedron
	hlist *local_unglued_faces;  // and those in the lower star of v
	hlist *lone_faces; // triangles isolated in the lower star of their maximal vertex
	hlist *local_lone_faces; // and those in the lower star of v
	hlist *possible_lone_edges; // edges perhaps isolated in the lower star of their maximal vertex
	struct star_component
	{
		simplex_id end_face[2]; // faces at boundary of lower link component
		simplex_id end_vert[2];  // lowest vertex in end face 
		SInt32 index[2];  // indices of |end_vert[2]| in |more_local_unglued_faces|
		simplex_id bestv;  // minimal vertex in lower link component
		double bestv_value; // value of mininal vertex
	};


@ @<link tetrahedra and find an unoptimized discrete Morse function@>=
{
	simplex_id v;
	simplex_id beste; // this will be the steepest edge from |v|
	simplex_id bestv;  // the other vertex of |bestv|
	hlist *more_local_unglued_faces;  // and those in the lower star of an edge
	hlist *local_done_edges;  // edges we have already processed
	hlist *local_lone_edges;  // edges perhaps isolated in the lower star of their maximal vertex
	list *components; // list of components of the deleted lower star of an edge.
	UInt32 critsave[4];  // save top of lists of critical points

	for(i=0;i<4;i++) list_initialize(crit+i,sizeof(simplex_id));
	hlist_initialize(&unglued_faces,sizeof(simplex_id),12713,3,0); //12713 prime
	hlist_initialize(&local_unglued_faces,sizeof(simplex_id),103,2,0); 
	hlist_initialize(&more_local_unglued_faces,sizeof(SInt32),13,1,0); 
	hlist_initialize(&lone_faces,sizeof(simplex_id),2713,3,0); //2713 prime
	hlist_initialize(&local_lone_faces,sizeof(simplex_id),103,2,0); 
	hlist_initialize(&local_done_edges,sizeof(simplex_id),103,1,0); 
	hlist_initialize(&local_lone_edges,sizeof(simplex_id),103,1,0); 
	hlist_initialize(&possible_lone_edges,sizeof(simplex_id),2713,2,0); //2713 prime
	list_initialize(&components,sizeof(struct star_component));

	for(i = number_of_vertices; i>0 ;)
	{
		i--;
		v = vertex_id(i);

@q+ printf("\n\nExtracting LowLk of vertex %ld at (%f,%f,%f) with value %d\n",@>
@q+ i,id2vertex(v)->coords[0],id2vertex(v)->coords[1],id2vertex(v)->coords[2],@>
@q+ id2vertex(v)->h);@>

		beste = NULL_ID;  
		bestv = NULL_ID; 
		hlist_clear(local_unglued_faces);
		hlist_clear(local_lone_faces);
		hlist_clear(local_done_edges);
		hlist_clear(local_lone_edges);
		critsave[0] = list_count(crit[0]);
		critsave[1] = list_count(crit[1]);
		critsave[2] = list_count(crit[2]);
		critsave[3] = list_count(crit[3]);
		
		@<extract and link up the lower star of |v|@>@;
		
		if(vertex_is_in_K(v))
		{
			if(id_is_null(beste)) 
			{
				make_vertex_critical(v);
			}
			else
			{
				pair01(v,beste,false);  // pair |v| with the steepest edge down
				LocalCancel(v,critsave[2]);  // cancel critical simplices in the lower Star of |v|
				clean_crit_at(critsave); // delete cancelled critical points
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


@ Here we link up the tetrahedra in the lower star of |v| and 
if $v\in K$ we find a discrete Morse function on the lower star of |v|.
We do this by taking each edge |e| in the lower Star of |v|,
linking up the tetrahedra in its lower star,
and (if |e|$\in K$) putting a discrete Morse function on its lower Star.
The lower Star of |e| is all simplices $\sigma$ of $K$ containing 
|e| so that the maximal and next--to--maximal vertices
of $\sigma$ are in |e|.
On entry the |face_ids| give a tree structure on the lower star of |v|,
whose root is in |vv->tetra_id|.
On exit each |face_ids[i]| will be the id of a face
shared with a neighboring tetrahedron
(if the neighboring tetrahedron was already encountered) or |NULL_ID| (if not).

There are three types of edges in the lower star of |v| which we must
account for.
The first type is those edges whose lower star is three dimensional.
We process those edges first, in order
(highest value of the other vertex first).
The next type of edge has two dimensional lower star.
We process these next in |@<add isolated triangles to the lower star of |v|@>|.
As we'll see later the triangles in the lower star of these edges
are all in the lists |lone_faces| or |local_lone_faces|.
Finally we have the edges with empty lower star.
These are on the lists |local_lone_edges| 
or |possible_lone_edges| and are processed last.

@<extract and link up the lower star of |v|@>=
/* |(simplex_id v,*simplex_id beste,*simplex_id bestv, 
		hlist *local_lone_edges,hlist *local_done_edges, Boolean optional_link_values,
		list *components,hlist *more_local_unglued_faces)|  */
{
	vertex *vv=id2vertex(v);
	simplex_id edge;
	tetrahedron *tt;
	simplex_id t;
	double beste_value;
	
	while(!id_is_null(vv->tetra_id))
	{
		@<make |edge| the highest remaining edge in the lower star of |v|@>@;
		
		@<extract the lower star of |edge|@>@;
	}
	@<add isolated triangles to the lower star of |v|@>@;
	@<add in any isolated edges of the lower star of |v|@>@;
}

@ @<make |edge| the highest remaining edge in the lower star of |v|@>=
{
	simplex_id *last;

	last = &vv->tetra_id;
	
	do
	{
		t = *last;
		tt = id2tetrahedron(t);
		if (id_is_null(tt->face_ids[1])) break;
		last = tt->face_ids + 1;
	} while(true);
	*last = tt->face_ids[0];
	
	edge = get_edge_of_tetrahedron(t,0,tt->flag&3);

}

@ We have an edge named |edge| whose lower star is three dimensional
and will link up all tetrahedra in its lower star.
If |edge| is in |K| we will also put an optimum
discrete Morse function on the lower star of |edge|.
|face_with_lowest_vertex| is the triangle in the lower star of |edge|
whose third vertex |lowest_vertex| has the lowest value.
This triangle will be paired with |edge|.

There are two possiblities for the lower star of |edge|.
It could be that every tetrahedron containing |edge| is in the lower star.
So the lower link is a circle.
In this case we make one of the tetrahedra containing 
|face_with_highest_vertex| critical.
There will be no other critical simplices in the lower star of |edge|.
The other possibility is that the lower link consists of a
number of intervals and points.
Each of the intervals will be represented by an entry in the list |components|.
Recall that an entry of |components| is a structure with fields
		|simplex_id end_face[2]|,
		|simplex_id end_vert[2]|, 
		|SInt32 index[2]|, 
		|simplex_id bestv|, and
		|double bestv_value|.
The field |end_vert[2]| is the two vertices at the ends of the interval.
The field |end_face[2]| is the corresponding triangles in the lower star,
	i.e., the joins of |end_vert[2]| with |edge|.
The field |index[2]| is the indices in the list |more_local_unglued_faces|
	of the vertices |end_vert[2]|. 
The field |bestv| is the minimal vertex in the lower link
and |bestv_value| is either the simplex_id itself,
or in the alternate case, the alternate value
|value(bestv)-value(v)/length(v,bestv)|.
The isolated points in the lower link correspond to triangles
in |local_unglued_faces| and |unglued_faces| whose highest edge is |edge|.

@<extract the lower star of |edge|@>=
/* |(tetrahedron *tt, simplex_id t, hlist *more_local_unglued_faces,
		list *components, simplex_id edge, Boolean optional_link_values,
		*simplex_id beste, *simplex_id bestv, *double beste_value,
		hlist *local_done_edges, hlist *local_lone_edges, simplex_id v)| */
{
	simplex_id next;
	simplex_id lowest_vertex = NULL_ID;
	simplex_id face_with_lowest_vertex = NULL_ID;
	double lowest_vertex_value;
	simplex_id highest_vertex = NULL_ID;
	simplex_id face_with_highest_vertex = NULL_ID;
	double highest_vertex_value;
	
	double edge_value,edge_length;
	
	if(optional_link_values)
		edge_value = alternate_vertex_value(v,get_vertex(edge,1),&edge_length);

@q+ printf("\nExtracting LowLk of ");@>
@q+ print_simp(edge);@>
@q+ printf("\n");@>

	hlist_clear(more_local_unglued_faces);
	list_clear(components);
	do
	{
		next = id2tetrahedron(t)->face_ids[2];

@q+ printf("Processing ");@>
@q+ print_simp(t);@>

		if(tetrahedron_is_in_K(t))
			@<add |t| to the lower star of |edge|@>@;
		else @<try linking up the faces of |t|@>@;
		t = next;
	} while(!id_is_null(t));
	if(edge_is_in_K(edge))
		@<put a discrete morse function on the lower star of |edge|@>@;
}

@ Here we take a tetrahedron in the lower star of |edge| and add it to
our representation of the lower star.
Perhaps it is easier to visualize what this does to the lower link.
Then we are adding an edge to the portion of the lower link
we have already discovered,
which is a collection of intervals.
One of three things could happen:
\item{1.} The new edge could intersect the portion of the link in both vertices.
Thus it either joins two intervals together or makes an interval into a circle.
\item{2.} The new edge could intersect in a single vertex, thus lengthening
one of the intervals.
\item{3.} The new edge could be disjoint from the portion of the link,
thus creating a new interval.

@<add |t| to the lower star of |edge|@>=
/* |(tetrahedron *tt, simplex_id t, hlist *more_local_unglued_faces,
		list *components, simplex_id next, Boolean optional_link_values,
		simplex_id edge, double edge_value, double edge_length,
		*double lowest_vertex_value, *double highest_vertex_value,
		*simplex_id lowest_vertex, *simplex_id face_with_lowest_vertex,
		*simplex_id highest_vertex, *simplex_id face_with_highest_vertex)| */
{
	SInt32 more_local_index[2];
	SInt32 comp_index[2];
	int vertex_index[2];
	simplex_id f;
	int j;
	
	
	tt = id2tetrahedron(t);
	
	vertex_index[0] = ((tt->flag)&12)>>2;
	vertex_index[1] = ((tt->flag)&48)>>4;
	
	tt->face_ids[0] = NULL_ID;
	tt->face_ids[1] = NULL_ID;
	tt->face_ids[2] = NULL_ID;
	tt->face_ids[3] = NULL_ID;
	
	more_local_index[0] = hlist_add_delete(more_local_unglued_faces,
				tt->vertex_ids+vertex_index[0],comp_index);
	more_local_index[1] = hlist_add_delete(more_local_unglued_faces,
				tt->vertex_ids+vertex_index[1],comp_index+1);

	if(more_local_index[0]<0 && more_local_index[1]<0)
		@<join two components together@>@;
	else if(more_local_index[0]>=0 && more_local_index[1]>=0)
		@<add in isolated component@>@;
	else
	{
		if(more_local_index[0]>=0) j=1;
		else j=0;
		@<add |t| to the end of component |comp_index[j]|@>@;
	}
}


@ This code either joins two intervals together or makes a single interval
into a circle.  
The last possibility could only occur if |t| is the very last tetrahedron
in the lower star and every tetrahedron containing |edge| is in the lower star
of |edge|.
In the first possibility, we combine the two components in the struct
for the first component and abandon the struct for the last component,
since I doubt whether the list of components would ever be long enough
in practice to worry about garbage collection.
The maximum number of items needed in the list |components| is the
number of tetrahedra in the lower link of |edge| which would typically be small.

@<join two components together@>=
/* |(list *components, SInt32 *comp_index, tetrahedron *tt, simplex_id t,
		int *vertex_index, Boolean optional_link_values, 
		hlist *more_local_unglued_faces, simplex_id next)| */
{
	struct star_component *comp[2];
	int j0,j1;
	simplex_id f0,f1;
	
@q+ printf(" -joining\n");@>

	comp[0] = (struct star_component *)list_entry(components,comp_index[0]);
	comp[1] = (struct star_component *)list_entry(components,comp_index[1]);
	
	if(comp[0]->end_vert[0] == tt->vertex_ids[vertex_index[0]]) j0 = 0; else j0 = 1;
	if(comp[1]->end_vert[0] == tt->vertex_ids[vertex_index[1]]) j1 = 0; else j1 = 1;
	
	f0 = get_face(t,vertex_index[0]);
	f1 = comp[1]->end_face[j1];
	
	glue_faces(f0,f1);
	
	f0 = get_face(t,vertex_index[1]);
	f1 = comp[0]->end_face[j0];
	
	glue_faces(f0,f1);
	
		
	if(comp_index[0]==comp_index[1])
	{
		// we are joining an interval into a circle.
		// so there should be no other simplices to add
		//  and no other components in the lower star.
		if(!id_is_null(next) || !hlist_is_empty(more_local_unglued_faces))
			abort_message("link of an edge is disconnected");
	}
	else
	{
		comp[0]->end_vert[j0] = comp[1]->end_vert[1-j1];
		comp[0]->end_face[j0] = comp[1]->end_face[1-j1];
		comp[0]->index[j0] = comp[1]->index[1-j1];
		
		
		if(comp[0]->bestv_value>comp[1]->bestv_value)
		{
			comp[0]->bestv = comp[1]->bestv;
			comp[0]->bestv_value = comp[1]->bestv_value;
		}
			
		*((SInt32 *)hlist_entry(more_local_unglued_faces,comp[0]->index[j0])) 
			= comp_index[0];
	}
}

@ This adds in a new component to the lower link of |edge|.
@<add in isolated component@>=
/* |(tetrahedron *tt, simplex_id t, SInt32 *more_local_index,
		*simplex_id lowest_vertex, *simplex_id face_with_lowest_vertex,
		*simplex_id highest_vertex, *simplex_id face_with_highest_vertex,
		*double lowest_vertex_value, *double highest_vertex_value,
		Boolean optional_link_values, int *vertex_index, int j,
		simplex_id edge, double edge_value, double edge_length,
		hlist *more_local_unglued_faces, list *components)| */
{
	struct star_component comp;
	double val[2];
	

@q+ printf(" -isolated\n");@>

	
	if(optional_link_values)
	{
		val[0] = alternate_edge_value(edge,tt->vertex_ids[vertex_index[0]],
				edge_value,edge_length);
		val[1] = alternate_edge_value(edge,tt->vertex_ids[vertex_index[1]],
				edge_value,edge_length);
	}
	else
	{
		val[0] = tt->vertex_ids[vertex_index[0]];
		val[1] = tt->vertex_ids[vertex_index[1]];
	}
	
	if(val[0] > val[1]) j=1; else j=0;
	
	
	
	comp.end_vert[0] = tt->vertex_ids[vertex_index[0]];
	comp.end_vert[1] = tt->vertex_ids[vertex_index[1]];
	comp.end_face[0] = get_face(t,vertex_index[1]);
	comp.end_face[1] = get_face(t,vertex_index[0]);
	comp.index[0] = more_local_index[0];
	comp.index[1] = more_local_index[1];
	
	comp.bestv = tt->vertex_ids[vertex_index[j]];
	
	comp.bestv_value = val[j];
	
	if(id_is_null(lowest_vertex) || lowest_vertex_value > val[j]) 
	{
		lowest_vertex = tt->vertex_ids[vertex_index[j]];
		face_with_lowest_vertex = get_face(t,vertex_index[1-j]);
		lowest_vertex_value = val[j];
	}
	
	
	
	if(id_is_null(highest_vertex) || highest_vertex_value < val[1-j])
	{
		highest_vertex_value = val[1-j];
		highest_vertex = tt->vertex_ids[vertex_index[1-j]];
		face_with_highest_vertex = get_face(t,vertex_index[j]);
	}
	
	*((SInt32 *)hlist_entry(more_local_unglued_faces,more_local_index[0])) 
		= (SInt32)list_count(components);
	*((SInt32 *)hlist_entry(more_local_unglued_faces,more_local_index[1]))
		= (SInt32)list_count(components);

	list_push(components,&comp);
}

@ Here the tetrahedron |t| intersects the already discovered portion of the link
in just a single face, |get_face(t,3-j)|.

@<add |t| to the end of component |comp_index[j]|@>=
/* |(list *components, SInt32 *comp_index, tetrahedron *tt, int j,
		simplex_id t, Boolean optional_link_values, int *vertex_index,
		*simplex_id face_with_lowest_vertex, *simplex_id lowest_vertex, 
		*simplex_id highest_vertex, *simplex_id face_with_highest_vertex,
		*double lowest_vertex_value, *double highest_vertex_value,
		simplex_id edge, double edge_value, double edge_length,
		hlist *more_local_unglued_faces, SInt32 *more_local_index)| */
{
	struct star_component *comp;
	int jj;
	simplex_id f0,f1;
	simplex_id new_vertex;
	simplex_id glued_vertex;
	double new_vertex_value;
	
@q+ printf(" -merging\n");@>

	glued_vertex = tt->vertex_ids[vertex_index[j]];
	new_vertex = tt->vertex_ids[vertex_index[1-j]];

	if(optional_link_values)
	{
		new_vertex_value = alternate_edge_value(edge,new_vertex,edge_value,edge_length);
	}
	else
		new_vertex_value = new_vertex;

	comp = (struct star_component *)list_entry(components,comp_index[j]);
	if(comp->end_vert[0] == glued_vertex) jj = 0; else jj = 1;
	

	f0 = get_face(t,vertex_index[1-j]);
	f1 = comp->end_face[jj];
	
	glue_faces(f0,f1);


	comp->end_vert[jj] = new_vertex;
	comp->end_face[jj] = get_face(t,vertex_index[j]);
	comp->index[jj] = more_local_index[1-j];

	if(comp->bestv_value > new_vertex_value) 
	{
		comp->bestv = new_vertex;
		comp->bestv_value = new_vertex_value;
		
		if(lowest_vertex_value > new_vertex_value) 
		{
			lowest_vertex = new_vertex;
			lowest_vertex_value = new_vertex_value;
			face_with_lowest_vertex = get_face(t,vertex_index[j]);
		}
	}
	
	
	if(highest_vertex_value < new_vertex_value) 
	{
		highest_vertex = new_vertex;
		highest_vertex_value = new_vertex_value;
		face_with_highest_vertex = get_face(t,vertex_index[j]);
	}

	*((SInt32 *)hlist_entry(more_local_unglued_faces,comp->index[jj])) = comp_index[j];
}



@ At this point we have linked up all the tetrahedra in the lower star of |edge|
to themselves.  We have also linked up all the tetrahedra not in |K| which
have highest edge equal to |edge|.

@<put a discrete morse function on the lower star of |edge|@>=
/* |(simplex_id t, *simplex_id lowest_vertex, hlist *more_local_unglued_faces,
		list *components, *simplex_id face_with_highest_vertex,
		tetrahedron *tt, *simplex_id beste, *simplex_id bestv,
		*double beste_value, *double lowest_vertex_value,
		simplex_id edge, double edge_value, double edge_length,
		simplex_id face_with_lowest_vertex, hlist *local_done_edges, simplex_id v,
		Boolean optional_link_values, hlist *local_lone_edges)| */
{
	simplex_id w;
	SInt32 index; 
	struct star_component *comp;
	simplex_id f,f0,f1,local_lowest_vertex;
	
	if(hlist_is_empty(more_local_unglued_faces) && !id_is_null(face_with_lowest_vertex))
		@<put a discrete morse function on a circular lower star@>@;
	else
	{	
		while(hlist_get(more_local_unglued_faces,&w,&index))
		{
			comp = (struct star_component *)list_entry(components,index);
			if(comp->end_vert[0] == w) w = comp->end_vert[1]; 
			else w = comp->end_vert[0];
			if(!hlist_find_delete(more_local_unglued_faces,&w,NULL))
				abort_message("internal error 46");
			f = comp->end_face[0];
			@<try linking up the end |f| of a component of the lower link@>@;
			f = comp->end_face[1];
			@<try linking up the end |f| of a component of the lower link@>@;
			
			local_lowest_vertex = comp->bestv;
			f0 = comp->end_face[0];
			f1 = comp->end_face[1];
			@<extract the interval@>@;
		}
		@<add isolated triangles to the lower star of |edge|@>@;
	}
	@<finish up discrete morse function on the lower star of |edge|@>@;
}


@ @<finish up discrete morse function on the lower star of |edge|@>=
/* |(*simplex_id beste,*simplex_id bestv,simplex_id edge,simplex_id w,
	Boolean optional_link_values, simplex_id v, *double beste_value,
	simplex_id face_with_lowest_vertex,hlist *local_done_edges)| */
{
	w = get_vertex(edge,1);

	if(id_is_null(face_with_lowest_vertex))
		@<finish up isolated |edge|@>@;
	else 
	{
		pair12(&edge,face_with_lowest_vertex,false);
	}
	// indicate this edge has been looked at
	hlist_find_add(local_done_edges,&w,&edge,NULL);
}

@ @<finish up isolated |edge|@>=
/* |(*simplex_id beste,*simplex_id bestv,simplex_id edge,
	*double beste_value,
	Boolean optional_link_values, simplex_id v, simplex_id w)| */
{
	double w_value;
	
	if(optional_link_values)
		w_value = alternate_vertex_value(v,w,NULL);
	else
		w_value = w;



	if(id_is_null(bestv))
	{
		bestv = w;
		beste = edge;
		beste_value = w_value;
	}
	else
	{
		if(beste_value>w_value)
		{
			make_edge_valid(beste);
			make_edge_critical(&beste);
			bestv = w;
			beste = edge;
			beste_value = w_value;
		}
		else
		{
			make_edge_valid(edge);
			make_edge_critical(&edge);
		}
	}
}


@ @<put a discrete morse function on a circular lower star@>=
/* |(simplex_id f, simplex_id t, simplex_id lowest_vertex,
		simplex_id f0, simplex_id f1, simplex_id local_lowest_vertex,
		simplex_id edge,
		Boolean optional_link_values, simplex_id face_with_highest_vertex)|  */
{
	int flag;
	
	//  We should really look at the other coface and pick the optimal one.
	t = id2tetra_id(face_with_highest_vertex);
	f = get_face(t,0);
	@<try linking up the 0 face |f|@>@;
	
	flag = id2tetrahedron(t)->flag;
	f = get_face(t,flag&3);
	@<try linking up the face |f|@>@;
	
	make_tetrahedron_critical(t);
	
	f0 = other_face_id(get_face(t,(flag>>2)&3));
	f1 = other_face_id(get_face(t,(flag>>4)&3));
	
	local_lowest_vertex = lowest_vertex;
	@<extract the interval@>@;
}



@ @<add isolated triangles to the lower star of |edge|@>=
/*  |(simplex_id f,*simplex_id lowest_vertex,simplex_id edge,
		*double lowest_vertex_value, double edge_value, double edge_length,
		*simplex_id face_with_lowest_vertex, Boolean optional_link_values,
		tetrahedron *tt, simplex_id v, hlist *local_lone_edges, hlist *local_done_edges)|
*/
{
	simplex_id vlist[3];
	
	get_edge_vertices(edge,vlist);


@q+ printf("Isolated triangles:\n");@>
	
	while(hlist_sub_match(lone_faces,vlist,&f,2))
		@<add |f| to the lower star of |edge|, etc@>@;
	while(hlist_sub_match(local_lone_faces,vlist+1,&f,1))
		@<add |f| to the lower star of |edge|, etc@>@;
}

@ @<add |f| to the lower star of |edge|, etc@>=
/*  |(simplex_id f,simplex_id *vlist,*simplex_id lowest_vertex,
		*double lowest_vertex_value,
		*simplex_id face_with_lowest_vertex, Boolean optional_link_values,
		simplex_id edge, double edge_value, double edge_length,
		tetrahedron *tt, simplex_id v, hlist *local_lone_edges, hlist *local_done_edges)|
*/
{
	simplex_id e;
	double new_vertex_value;

	if(triangle_is_in_K(f))
	{
	

@q+ printf("\t");@>
@q+ print_simp(f);@>
@q+ printf("\n");@>


		if(optional_link_values)
		{
			new_vertex_value = alternate_edge_value(edge,vlist[2],edge_value,edge_length);
		}
		else
			new_vertex_value = vlist[2];

		if(id_is_null(lowest_vertex))
		{
			lowest_vertex = vlist[2];
			face_with_lowest_vertex = f;
			lowest_vertex_value = new_vertex_value;
		}
		else
		{
			if(lowest_vertex_value > new_vertex_value)
			{
				make_triangle_valid(face_with_lowest_vertex);
				make_triangle_critical(&face_with_lowest_vertex);
				lowest_vertex = vlist[2];
				lowest_vertex_value = new_vertex_value;
				face_with_lowest_vertex = f;
			}
			else 
			{
				make_triangle_valid(f);
				make_triangle_critical(&f);
			}
		}
	}
	e = get_edge(f,1);
	if(get_vertex(e,1)!=vlist[2]) e = get_edge(f,2);
	if(get_vertex(e,1)!=vlist[2]) abort_message("internal 59");

	@<add |e| to |local_lone_edges|@>@;
	
	e = get_edge(f,0);
	@<add |e| to |possible_lone_edges|@>@;
}

@ @<add |e| to |local_lone_edges|@>=
/* |( tetrahedron *tt, simplex_id e, simplex_id v, 
		Boolean optional_link_values,
		hlist *local_lone_edges, hlist *local_done_edges)| */
{
	simplex_id vl[2];
	if(edge_is_in_K(e))
	{
		get_edge_vertices(e,vl);

@q+ printf("\tadding to local_lone_edges ");@>
@q+ print_simp(e);@>
@q+ printf("\n");@>

		if(hlist_find(local_done_edges,vl+1)<0)
			hlist_find_add(local_lone_edges,vl+1,&e,NULL);		
	}
}

@ @<add |e| to |possible_lone_edges|@>=
/* |( tetrahedron *tt, Boolean optional_link_values, simplex_id e)| */
{
	simplex_id vl[2], *last;
	if(edge_is_in_K(e))
	{
		get_edge_vertices(e,vl);
@q+ printf("\tadding to possible_lone_edges ");@>
@q+ print_simp(e);@>
@q+ printf("\n");@>

		hlist_find_add(possible_lone_edges,vl,&e,NULL);		
	}
}



@ @<add isolated triangles to the lower star of |v|@>=
/*  |(simplex_id v,simplex_id edge,hlist *local_done_edges, 
		Boolean optional_link_values, *double beste_value,
		tetrahedron *tt,*simplex_id beste,*simplex_id bestv, hlist *local_lone_edges)| */
{
	simplex_id vlist[3];
	simplex_id lowest_vertex;
	double lowest_vertex_value;
	simplex_id face_with_lowest_vertex;
	simplex_id f,w;
	
	double edge_value,edge_length;
	
	
	vlist[0] = v;
	

	if(!hlist_is_empty(local_unglued_faces)) abort_message("internal 34");
	if(hlist_sub_match(unglued_faces,vlist,NULL,1)) abort_message("internal 35");
	
	do 
	{
		if(hlist_sub_match(local_lone_faces,vlist+1,&f,0)) ;
		else if(!hlist_sub_match(lone_faces,vlist,&f,1)) break;
	
		if(hlist_find(local_done_edges,vlist+1)>=0)
			continue;

		edge = vertices2edge(vlist,id2tetra_id(f));
		

		if(optional_link_values)
			edge_value = alternate_vertex_value(v,get_vertex(edge,1),&edge_length);
		
		
@q+ printf("Extracting 2D Low Link of ");@>
@q+ print_simp(edge);@>
@q+ printf("\n");@>

		lowest_vertex = NULL_ID;
		face_with_lowest_vertex = NULL_ID;
		do
		{
			@<add |f| to the lower star of |edge|, etc@>@;
			if(!hlist_sub_match(lone_faces,vlist,&f,2)) 
			{
				if(!hlist_sub_match(local_lone_faces,vlist+1,&f,1)) break; 
			}
		} while(true);
				
		if(edge_is_in_K(edge))
			@<finish up discrete morse function on the lower star of |edge|@>@;
	} while(true);
}

@ Here we add any edges whose lower star is empty.
They will all be on either of the lists |local_lone_edges| 
or |possible_lone_edges|.  
But those lists may contain duplicates 
or edges whose lower star is not in fact empty,
so we test them against the list |local_done_edges| to make sure.

Let me see why these edges are on |local_lone_edges| 
or |possible_lone_edges|.
An edge is added to these lists in the code segment
|@<add |f| to the lower star of |edge|, etc@>|,
when |getedge(f,1)| is added to |local_lone_edges|
and |getedge(f,0)| is added to |possible_lone_edges|.
The code segment |@<add |f| to the lower star of |edge|, etc@>|
is executed for faces |f|
which appear on the lists
|lone_faces| and |local_lone_faces|.


@<add in any isolated edges of the lower star of |v|@>=
/* |(simplex_id v,*simplex_id beste,*simplex_id bestv,simplex_id edge,
		Boolean optional_link_values, *double beste_value,
		hlist *local_lone_edges,hlist *local_done_edges)|  */
{
	simplex_id w,vlist[2];
	
	vlist[0] = v;
	
	do
	{
		if(hlist_sub_match(local_lone_edges,vlist+1,&edge,0)) ;
		else if(!hlist_sub_match(possible_lone_edges,vlist,&edge,1)) break;
		
		if(!edge_is_in_K(edge) || hlist_find(local_done_edges,vlist+1)>=0)
			continue;

@q+ printf("Extracting 1D Low Link of ");@>
@q+ print_simp(edge);@>
@q+ printf("\n");@>

		w = vlist[1];
		@<finish up isolated |edge|@>@;
		// indicate this edge has been looked at
		hlist_find_add(local_done_edges,&w,&edge,NULL);
	
	} while(true);
}

@ @<try linking up the faces of |t|@>=
/* |(Boolean optional_link_values, simplex_id t)|  */
{
	simplex_id f;
	
@q+ printf(" -outside K\n");@>

	f = get_face(t,0);
	@<try linking up the 0 face |f|@>@;
	f = get_face(t,1);
	@<try linking up the face |f|@>@;
	f = get_face(t,2);
	@<try linking up the face |f|@>@;
	f = get_face(t,3);
	@<try linking up the face |f|@>@;
}

@ @<Simplex functions@>+=
void glue_faces(simplex_id f0,simplex_id f1)
{
	simplex_id v0[3];
	simplex_id v1[3];
	
	if(id2tetra_id(f0)==id2tetra_id(f1)) abort_message("err101");
	get_triangle_vertices(f0,v0);
	get_triangle_vertices(f1,v1);
	
	if(v0[0]!=v1[0])  abort_message("err102");
	if(v0[1]!=v1[1])  abort_message("err102");
	if(v0[2]!=v1[2])  abort_message("err102");
	
	glue_face(f0,f1);
	glue_face(f1,f0);
}


@ 
@d glue_face(f0,f1) (id2tetrahedron(f1)->face_ids[face_index(f1)] = f0)
@d glue_faces_old(f0,f1) (glue_face(f0,f1), glue_face(f1,f0))
@<try linking up the 0 face |f|@>=
// |(Boolean optional_link_values, simplex_id f)|
{
	simplex_id vl[3];
	simplex_id f2 = f;
	
	get_triangle_vertices(f,vl);
	
	if(hlist_add_delete(unglued_faces,vl,&f2) < 0)
	{
		if(face_index(f2)!=0) abort_message("internal error 68");
		glue_faces(f,f2);
		
		@<permute |vl| if needed@>@;
		
		hlist_add(lone_faces,vl,&f2);
	}
}

@ @<permute |vl| if needed@>=
{
	double val0,val1;
	simplex_id temp;

	if(optional_link_values)
	{
		val0 = alternate_vertex_value(vl[0],vl[1],NULL);
		val1 = alternate_vertex_value(vl[0],vl[2],NULL);
		
		if(val1 > val0)
		{
			temp = vl[2];
			vl[2] = vl[1];
			vl[1] = temp;
		}
	}
}

@ @<try linking up the face |f|@>=
/* |(Boolean optional_link_values, simplex_id f)|  */
{
	simplex_id vl[3];
	simplex_id f2 = f;
	
	get_triangle_vertices(f,vl);
	
	if(hlist_find_delete(unglued_faces,vl,&f2))
	{
		if(face_index(f2)!=0) abort_message("internal error 68");
		glue_faces(f,f2);
		
		@<permute |vl| if needed@>
		hlist_add(local_lone_faces,vl+1,&f2);
	}
	else if(hlist_add_delete(local_unglued_faces,vl+1,&f2) < 0)
	{
		glue_faces(f,f2);
		@<permute |vl| if needed@>
		hlist_add(local_lone_faces,vl+1,&f2);
	}
}

@ @<try linking up the end |f| of a component of the lower link@>=
// |(Boolean optional_link_values, simplex_id f)|
{
	simplex_id vl[3];
	simplex_id f2 = f;
	
	get_triangle_vertices(f,vl);
	
	if(hlist_find_delete(unglued_faces,vl,&f2))
	{
		if(face_index(f2)!=0) abort_message("internal error 68");
		glue_faces(f,f2);
	}
	else if(hlist_add_delete(local_unglued_faces,vl+1,&f2) < 0)
	{
		glue_faces(f,f2);
	}
	else abort_message("link faces: I don't think this should happen");
}


@ This takes an interval in the lower link of an edge whose minimum vertex
is |local_min| and whose ends are the triangles |f0| and |f1|
(after joining with the edge). It puts  a discrete Morse function on
the join of this interval with the edge.  
The vertex |global_min| is the minimum vertex in the whole lower link of the edge.

@<extract the interval@>=
{
	simplex_id vlist[3];

	get_triangle_vertices(f0,vlist);
	@<switch vertices if needed to make the first two in |edge|@>@;
	t = id2tetra_id(f0);
	f=f0;
	@<extract half the interval@>@;
	
	get_triangle_vertices(f1,vlist);
	@<switch vertices if needed to make the first two in |edge|@>@;
	t = id2tetra_id(f1);
	f=f1;
	@<extract half the interval@>@;
	if(local_lowest_vertex != lowest_vertex)
	{
		make_triangle_valid(f);
		make_triangle_critical(&f);
	}
}

@ @<switch vertices if needed to make the first two in |edge|@>=
if(vlist[2] == get_vertex(edge,1))
{
	vlist[2] = vlist[1];
	vlist[1] = get_vertex(edge,1);
}


@ @<extract half the interval@>=
while(vlist[2]!=local_lowest_vertex)
{
	pair23(&f,t,false);
	f = get_face(t,0);
	@<try linking up the 0 face |f|@>@;
	f = get_face(t,id2tetrahedron(t)->flag&3);
	@<try linking up the face |f|@>@;

	f = next_face(vlist,t);
	t = id2tetra_id(f);
}
