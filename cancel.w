@i common.w

@* Cancel Routines.  
Sometimes a critical $m-1$ simplex and a critical $m$ simplex can be cancelled.
You can do this if there is exactly one gradient path between them.
A gradient path is a sequence of simplices 
$\sigma_i,\sigma_{i+1},\sigma_{i+2},\ldots ,\sigma_k$ so that
for $j$ even, $\sigma_j$ is an $m$ simplex,
and $\sigma_{j-1}$ is paired with $\sigma_j$, and $\sigma_{j+1}$
is a codimension one face of $\sigma_j$.
To cancel a pair of critical simplices $\sigma$ and $\tau$, 
you take the unique gradient path
$\sigma = \sigma_0,\ldots ,\sigma_{2k-1}=\tau$,
and you pair each $\sigma_{2i}$ with $\sigma_{2i+1}$
(rather than with $\sigma_{2i-1}$ as was done formerly).

@c
#include "Morse.h"
#include "globals.h"
@<Subroutines canceling a single pair of critical simplices@>@;
@<Subroutines canceling many pairs of critical simplices @>@;
@<Subroutines finding gradient paths@>@;

@*1 Local Cancellation.
 |LocalCancel| cancels simplices in the lower Star of a vertex |v|.
After doing it, you can show that you are left with a
minimal number of critical simplices.
For example, at a local maximum you would end up with only one
critical simplex, a 3 simplex.
Likewise at PL approximations of traditional smooth
Morse saddles of index $i$ you would end up with a single critical
simplex, an $i$ simplex.
The input parameter |n| is the lowest list index of 
all the critical 2 simplices in the lower star of |v|.
As it happens, when this is called all succeeding
critical 2 simplices will also be in the lower star of |v|.

@<Subroutines canceling many pairs of critical simplices @>+=
void LocalCancel(simplex_id v,UInt32 n)
{@/
	simplex_id f,s;  // triangles
	simplex_id e[2],ep[2]; // edges
	simplex_id tid[2],tidp[2];  // tetrahedra
	int j,i;
	Boolean in;@/
	
	list_read_init_at(crit[2],n);
	while (!id_is_null(f=id_list_read(crit[2])))  // run through the list of new critical 2 simplices
	{
		@<find the edges |e[0]| and |e[1]| of |f| containing |v|@>;
		@<find the ends |ep[i]| of the gradient paths starting at |e[i]|@>;
				
		if(!edges_equal(ep[0],ep[1]))  // can we cancel?
		{
			@<Cancel the |f| with the best of the two edges@>; 
			list_read_delete(crit[2]);
		}
		else
		{
			// We can't cancel with an edge, so see if we can cancel with a 3 simplex
			for(i=0;i<2;i++)
			{
				@<find the 3 simplex |tid[i]| connected to |coface(f,i)| by a gradient path@>;
			}
			if(tid[0]!=tid[1])
			{
				@<Cancel the |f| with the best of the two tetrahedra@>; 
				list_read_delete(crit[2]);
			}
		}
	}
}


@ @<find the edges |e[0]| and |e[1]| of |f| containing |v|@>=

e[0]=get_edge(f,1);
e[1]=get_edge(f,2);

@ @<find the ends |ep[i]| of the gradient paths starting at |e[i]|@>=
		
for(i=0;i<2;i++)	
{
	ep[i]=e[i];
	while(!edge_is_critical(ep+i))  
	{
		if(edge_is_paired_down(ep+i))
		{
			ep[i]=NULL_ID;  // runs into $B_1$ so no cancelation possible
			break;
		}
		s = r12(ep+i);  // get paired triangle
		ep[i] = other_edge(v,ep[i],s);
	}
}		

@ @<find the 3 simplex |tid[i]| connected to |coface(f,i)| by a gradient path@>=
tidp[i] = tid[i] = coface(f,i);
s = f;
in = false;
while (tetrahedron_is_in_K(tid[i]))  // while we are in K
{
	in = is_in_lower_Star(tid[i],v);
	if(!in)  break;  // see if exit lower Star
	if(tetrahedron_is_critical(tid[i])) break; // see if reached critical
	s = r32(tid[i]);
	tid[i] = other_coface(s,tid[i]);
}
if(!tetrahedron_is_in_K(tid[i]) || !in )		
	  tid[i]=NULL_ID;   

@ @<Cancel the |f| with the best of the two edges@>= 
if(id_is_null(ep[0])) LocalCancel12(v,ep[1],e[1],f);
else if(id_is_null(ep[1])) LocalCancel12(v,ep[0],e[0],f);
else if(min_value(ep[0],1) > min_value(ep[1],1)) 
	LocalCancel12(v,ep[0],e[0],f);
else LocalCancel12(v,ep[1],e[1],f);

@ @<Cancel the |f| with the best of the two tetrahedra@>= 
if (id_is_null(tid[0])) Cancel23(f,tidp[1],tid[1]);
else if (id_is_null(tid[1])) Cancel23(f,tidp[0],tid[0]);
else if(min_value(tid[1],3) > min_value(tid[0],3)) 
	Cancel23(f,tidp[0],tid[0]);
else Cancel23(f,tidp[1],tid[1]);  


@*1 Canceling pairs of Critical Simplices.  
The following routine cancels a critical 1 simplex $\sigma$ containing |v|
 with a critical 2 simplex $\tau$ containing |v|.
 It speeds up the ordinary Cancel12 by assuming that 
 the unique gradient path from $\tau$ to $\sigma$
 has all of its simplices containing |v|.
 The gradient path from $\tau$ to $\sigma$ goes through the face |sigmap| of $\tau$.

@<Subroutines canceling a single pair of critical simplices@>+=
void LocalCancel12(simplex_id v,simplex_id sigma,simplex_id sigmap,simplex_id tau)
{
	simplex_id u; // edge
	simplex_id w, wp;  // triangles
	
	u=sigmap;
	w=tau;
	unmake_triangle_critical(&w);
	while(!edge_is_critical(&u)) 
	{
		wp = r12(&u);
		pair12(&u,w,true);
		w=wp;
		u=other_edge(v,u,w);
	}
	if(!edges_equal(u,sigma)) abort_message("localcancel12 internal error");
	unmake_edge_critical(&u);
	pair12(&u,w,true);
}

@ The following routine cancels a critical 2 simplex $\sigma$ 
with a critical 3 simplex $\tau$. 
The 3 simplex |taup| contains $\sigma$ and
is such that  the gradient path from $\tau$ to $\sigma$
goes through |taup|.

@<Subroutines canceling a single pair of critical simplices@>+=
void Cancel23(simplex_id sigma,simplex_id taup,simplex_id tau)
{
	simplex_id s,sp;
	simplex_id t;

	s=sigma;
	t=taup;
	unmake_triangle_critical(&s);
	
	while(!tetrahedron_is_critical(t)) 
	{
		sp=r32(t);
		pair23(&s,t,true);
		s=sp;
		t=other_coface(s,t);
	}
	if(t!=tau) abort_message("cancel23 internal error");
	unmake_tetrahedron_critical(t);
	pair23(&s,t,true);
}

@ The following routine cancels a critical vertex |v| 
with a critical edge $\kappa$. 
The n is such that  the gradient path from |v| to $\kappa$
goes through |get_vertex(kappa,n)|.

@<Subroutines canceling a single pair of critical simplices@>+=


void Cancel01(simplex_id v,int n,simplex_id kappa)
{
	simplex_id u; // vertex
	simplex_id e,ep; // edges
	
	u = get_vertex(kappa,n);
	
	e = kappa;
	unmake_edge_critical(&e);
	
	while(!vertex_is_critical(u)) // while non critical
	{
		ep = r01(u);
		pair01(u,e,true);
		e = ep;
		u = other_vertex_in_edge(u,e);
	}
	if(u!=v) abort_message("cancel01 internal error");
	unmake_vertex_critical(v);
	pair01(v,e,true);
}


@ Cancel a critical 1 simplex $\sigma$ with a critical 2 simplex $\kappa$.
It assumes that all the edges in the gradient path are in the list |grad_path|,
in order.
It pushes any 2 simplices with changed pairings to the stack |changed|.

@<Subroutines canceling a single pair of critical simplices@>+=
void Cancel12(simplex_id sigma,simplex_id kappa,list *grad_path,list *changed)
{
	simplex_id t,nextt; // triangles
	simplex_id e;  // edge

	t = kappa;
	unmake_triangle_critical(&t);
	
	do
	{
		e = id_list_pop(grad_path);
		if(edge_is_critical(&e)) break;
		nextt = r12(&e);
		pair12(&e,t,true);
		id_list_push(changed,t);
		t = nextt;
	} while(true);
	
	if(!edges_equal(e,sigma)) abort_message("cancel12 internal error");
	unmake_edge_critical(&e);
	pair12(&e,t,true);
	id_list_push(changed,t);
}

@ @(cancel.h@>=
void LocalCancel(simplex_id v,UInt32 n);
void LocalCancel12(simplex_id v,simplex_id sigma,simplex_id sigmap,simplex_id tau);
void Cancel01(simplex_id v,int n,simplex_id kappa);
void Cancel12(simplex_id sigma,simplex_id kappa,list *grad_path,list *changed);
void Cancel23(simplex_id sigma,simplex_id taup,simplex_id tau);
void ExtractCancel1(SInt32 p);
void ExtractCancel2(SInt32 p);
void ExtractCancel3(SInt32 p);
simplex_id FindGrad01(simplex_id u,SInt32 m);
simplex_id FindGrad23(simplex_id tau,SInt32 m);
simplex_id FindGrad23orientation(simplex_id sigma,int *orientation);
simplex_id FindGradPaths12(simplex_id sigma, SInt32 p, list *grad_path, int flags);
Boolean splitrejoin(simplex_id t,list *changed);
SInt32 find_all_grad12_paths(simplex_id sigma,hlist *edges,int options);
SInt32 find_all_backward_grad12_paths(simplex_id tau,hlist *triangles,list *crits,int options);

@*2 Canceling vertices and edges. 
Find all critical edges and vertices connected by a single gradient path.
Cancel the pair with smallest difference in value, as long as it's less than |p|.
Keep on doing this until you can't do any more.


@<Subroutines canceling many pairs of critical simplices @>=

void ExtractCancel1(SInt32 p)
{@/ 
	hlist *c0;  // list of vertices to cancel with
	hlist *c1;  // list of edges which can be cancelled with vertices
	struct ccrit1
	{
		simplex_id s;
		SInt32 c[2];
	} s;
	int i;
	simplex_id v[2];
	SInt32 thisk;
	SInt32 evalue;
	UInt8 persist_scaling;
	
	@<determine persistence scaling factor@>@;

	hlist_initialize(&c0,sizeof(SInt32),113,1,0);
	hlist_initialize(&c1,sizeof(struct ccrit1),1+(p>>persist_scaling),1,persist_scaling);

	@<Find all cancelable pairs and put them on lists |c0| and |c1|@>;
	
	while (!hlist_is_empty(c1))
	{@t\1@>@/ 
		hlist_min(c1,&thisk,&s);  // put best candidate in |s|, persistence in |thisk|
		@<let |v[i]| be the two vertices with which |s| can cancel@>@;
		i = @<index of best vertex of |s.s| to cancel@>;
		evalue = value(s.s);
		if (vertex_is_critical(v[i]) && thisk == evalue-vertex_value(v[i])) 
			@<cancel with the vertex@>@;
		else @<see if we can still cancel with some other vertex and replace |s| on |c1|@>@;@t\2@>@/ 
	}@/
	hlist_abandon(&c0);
	hlist_abandon(&c1);@t\2@>@/
}

@ |hlist_min| will only find the actual minimum if its hash function is one to one
(and order prerserving).
If |p| is large, this would lead to very large hash tables, so instead we scale back the 
persistence precision so the hash table size is $<8096$.
@<determine persistence scaling factor@>=
{
	UInt32 mask = 0xffffe000;
	
	if(p>65535) p = 65535;
	else if(p<0) p=0;
	
	for(persist_scaling=0;persist_scaling<4;persist_scaling++,mask<<=1) 
		if((p&mask)==0) break;
}

@ @<Find all cancelable pairs and put them on lists |c0| and |c1|@>=
{
	simplex_id e;

	list_read_init(crit[1]);
	while ((e=id_list_read(crit[1]))!=-1)  // run through all critical edges
	{
		if(!edge_is_critical(&e)) list_read_delete(crit[1]);
		else
		{
			evalue = value(e);
			
			v[0] = FindGrad01(get_vertex(e,0),evalue-p); // find the two gradient paths 
			v[1] = FindGrad01(get_vertex(e,1),evalue-p);  // descending from |e|
			if(v[0]!=v[1])
			{
				@<put |e| on the list |c1| and put |v[i]| on the list |c0|@>;
			}
		}
	}
}

@ @<put |e| on the list |c1| and put |v[i]| on the list |c0|@>=
{@t\1@>@/ 
	SInt32 link;
	SInt32 persist;

	link = -1;
	s.s=e;
	s.c[0] = (id_is_null(v[0]))? -1: hlist_find_add(c0,v,&link,NULL);
	s.c[1] = (id_is_null(v[1]))? -1: hlist_find_add(c0,v+1,&link,NULL);@/
	i = @<index of best vertex of |s.s| to cancel@>;
	persist = evalue - vertex_value(v[i]);
	hlist_add(c1,&persist ,&s);@t\2@>@/ 
}

@ @<let |v[i]| be the two vertices with which |s| can cancel@>=
v[0] = (s.c[0]<0)? NULL_ID : hlist_get_key(c0,s.c[0],0);
v[1] = (s.c[1]<0)? NULL_ID : hlist_get_key(c0,s.c[1],0);


@ @<cancel with the vertex@>=
{
	Cancel01(v[i],i,s.s);
	*((SInt32 *)hlist_entry(c0,s.c[i])) = s.c[1-i];  
	// now any gradient path to |v[i]| goes to other vertex |v[1-i]|
}
		
@ @<see if we can still cancel with some other vertex and replace |s| on |c1|@>= 
{@t\1@>@/ 
	SInt32 m;
	
	evalue = value(s.s);
	
	for(i=0;i<2;i++)
	{
		if(s.c[i]<0) continue;
		@<update |s.c[i]| to point to a critical vertex@>@;
	}
	
	
	if( s.c[1]!=s.c[0])
	{@t\1@>@/
		i = @<index of best vertex of |s.s| to cancel@>;
		m = evalue - vertex_value(v[i]);

		if( m < p)	hlist_add(c1,&m,&s);@t\2@>@/
	} @t\2@>@/
}

@ @<update |s.c[i]| to point to a critical vertex@>=
{
	SInt32 a;

	for(a = s.c[i]; a >= 0; )
	{
		v[i] = hlist_get_key(c0,a,0);
		if( vertex_is_critical(v[i])) break;
		a = *((SInt32 *)hlist_entry(c0,a));
	}
	if(a!=s.c[i])
	{
		*((SInt32 *)hlist_entry(c0,s.c[i])) = a;
	}
	if (vertex_value(v[i]) <= (evalue-p))
	{
		v[i] = NULL_ID;
		s.c[i] = -1;
	}
	else s.c[i] = a;
}

@ @<index of best vertex of |s.s| to cancel@>=
(id_is_null(v[0]))? 1:@t}$\par${@>
	(id_is_null(v[1]))? 0:@t}$\par${@>
		(vertex_value(v[0]) > vertex_value(v[1]))? 0: @t}$\par${@>
			(vertex_value(v[0]) < vertex_value(v[1]))? 1: @t}$\par${@>
				vertex_is_critical(v[0])?  1:0


@*2 Canceling triangles and tetrahedra.
 Find all critical triangles and tetrahedra connected by a single gradient path.
Cancel the pair with smallest difference in value, as long as it's less than |p|.
Keep on doing this until you can't do any more.
The code is essentially the same as that used to cancel edges and vertices.

@<Subroutines canceling many pairs of critical simplices @>=

void ExtractCancel3(SInt32 p)
{@/
	hlist *c3;  // list of tetrahedra to cancel
	hlist *c2;  // list of triangles which cancel with tetrahedra
	struct ccrit2
	{
		simplex_id s;
		SInt32 c[2];
	} r;
	int i;
	simplex_id t[2];
	SInt32 thisk;
	SInt32 fvalue,tvalue;
	UInt8 persist_scaling;

	@<determine persistence scaling factor@>@;

	hlist_initialize(&c3,sizeof(SInt32),113,1,0);
	hlist_initialize(&c2,sizeof(struct ccrit2),1+(p>>persist_scaling),1,persist_scaling);

	@<Find all cancelable pairs and put them on lists |c2| and |c3|@>;
	while (!hlist_is_empty(c2))
	{@t\1@>@/
		hlist_min(c2,&thisk,&r);  // put best candidate in |r|, persistence in |thisk|
		@<let |t[i]| be the two tetrahedra with which we can cancel@>@;
		i = @<index of best coface of |r.s| to cancel@>;
		fvalue = value(r.s);
		tvalue = value(t[i]);
		if (tetrahedron_is_critical(t[i]) && thisk == tvalue-fvalue)  @<cancel with the tetrahedron@>@;
		else @<see if we can still cancel with some other tetrahedron and replace on |c2|@>@;@t\2@>@/ 
	}@/
	hlist_abandon(&c2);
	hlist_abandon(&c3);@t\2@>@/
}

@ @<Find all cancelable pairs and put them on lists |c2| and |c3|@>=
{@t\1@>@/
	simplex_id s;

	list_read_init(crit[2]);
	while ((s = id_list_read(crit[2]))!=-1) // go through all critical triangles
	{@/
		if(!triangle_is_critical(&s)) list_read_delete(crit[2]);
		else
		{
			fvalue = value(s);
			// find the beginnings of the two gradient paths ending at |s|
			t[0] = FindGrad23(coface0(s),fvalue+p);
			t[1] = FindGrad23(coface1(s),fvalue+p);@/
			if(t[0]!=t[1])
				@<put |s| on |c2| and |t[0]| and |t[1]| on |c3|@>@;
		}
	}@t\2@>@/
}

@ @<put |s| on |c2| and |t[0]| and |t[1]| on |c3|@>=
{@t\1@>@/
	SInt32 link;
	SInt32 persist;
	
	link = -1;
	r.s=s;
	r.c[0] = (id_is_null(t[0]))? -1: hlist_find_add(c3,t,&link,NULL);
	r.c[1] = (id_is_null(t[1]))? -1: hlist_find_add(c3,t+1,&link,NULL);@/
	
	i = @<index of best coface of |r.s| to cancel@>;
	persist = value(t[i]) - fvalue;
	hlist_add(c2, &persist, &r);@t\2@>@/
}



@ @<let |t[i]| be the two tetrahedra with which we can cancel@>=
t[0] = (r.c[0]<0)? NULL_ID : hlist_get_key(c3,r.c[0],0);
t[1] = (r.c[1]<0)? NULL_ID : hlist_get_key(c3,r.c[1],0);



	
@ @<cancel with the tetrahedron@>=
{
	Cancel23(r.s,coface(r.s,i),t[i]);
	*((SInt32 *)hlist_entry(c3,r.c[i])) = r.c[1-i];  
}

@ @<see if we can still cancel with some other tetrahedron and replace on |c2|@>= 		
{@t\1@>@/
	SInt32 m;
	
	fvalue = value(r.s);
	
	for(i=0;i<2;i++)
	{
		if(r.c[i]<0) continue;
		@<update |r.c[i]| to point to a critical tetrahedron@>@;
	}
	
	
	if(r.c[1]!=r.c[0])
	{@t\1@>@/
		i = @<index of best coface of |r.s| to cancel@>;
		
		m = value(t[i]) - fvalue;
		
		if( m < p)	hlist_add(c2,&m,&r);@t\2@>@/
	}@t\2@>@/
}

@ @<update |r.c[i]| to point to a critical tetrahedron@>=
{
	SInt32 a;

	for(a = r.c[i]; a >= 0; )
	{
		t[i] = hlist_get_key(c3,a,0);
		if( tetrahedron_is_critical(t[i])) break;
		a = *((SInt32 *)hlist_entry(c3,a));
	}
	*((SInt32 *)hlist_entry(c3,r.c[i])) = a;
	if(value(t[i])>=fvalue+p)
	{
		t[i] = NULL_ID;
		r.c[i] = -1;
	}
	else r.c[i] = a;
}
	
@ @<index of best coface of |r.s| to cancel@>=
(id_is_null(t[0]))? 1:@t}$\par${@>
	(id_is_null(t[1]))? 0:@t}$\par${@>
		(value(t[0]) > value(t[1]))? 1: @t}$\par${@>
			(value(t[0]) < value(t[1]))? 0: @t}$\par${@>
				tetrahedron_is_critical(t[0])? 1: 0





@*2 Canceling edges and triangles.
 Find and cancel pairs of 1 and 2 simplices whose values differ by less than p.
This version speeds things up by not always canceling the pair with least persistence.
In particular, after one cancelation it may become possible for
some other pair of critical simplices to cancel which could not have been cancelled
before.  If this happens, this routine could be unaware until it has
cancelled all the pairs it knows about already.
Checking for this situation would slow down this routine dramatically.
In fact such occurrences appear to be quite rare.
For random functions on |K| with millions of simplices and persistence 1/13 of
the range of the values of vertices, it tends to happen at most once or twice.

@<Subroutines canceling many pairs of critical simplices @>=
void ExtractCancel2(SInt32 p)
{
	list *changed;  // triangles whose pairing was changed by Cancel12
	list *grad_path;  // edges in gradient path of a canceling pair
	hlist *goodpairs; // cancelable critical triangles
	UInt32 lastp;
	UInt8 persist_scaling;

	@<determine persistence scaling factor@>@;

	list_initialize(&changed,sizeof(simplex_id));
	list_initialize(&grad_path,sizeof(simplex_id));
	hlist_initialize(&goodpairs,sizeof(simplex_id),
				1+(p>>persist_scaling),1,persist_scaling);
	lastp = 0;
	do
	{
		@<fill list |goodpairs| with possible cancels@>@;
		if(hlist_is_empty(goodpairs)) break;
		@<cancel all pairs on |goodpairs|@>@;
	} while(true);
	list_abandon(&changed);
	list_abandon(&grad_path);
	hlist_abandon(&goodpairs);
}

@ @<fill list |goodpairs| with possible cancels@>=
{
	simplex_id f;
	simplex_id e;
	UInt32 persist;

	list_read_init(crit[2]);
	while (!id_is_null(f = id_list_read(crit[2]))) // go through all critical triangles
	{
		if(!triangle_is_critical(&f)) list_read_delete(crit[2]);
		else
		{
			e = FindGradPaths12(f,p,NULL,0);
			if(!id_is_null(e)) // if there is a gradient path from |f| to |e|
			{
				persist = value(f)-value(e);
				hlist_add(goodpairs,&persist,&f);
			}
		}
	}
}




@ @<cancel all pairs on |goodpairs|@>=
{
	simplex_id f;
	simplex_id e;
	UInt32 bp,thisp;

	while(!hlist_is_empty(goodpairs))
	{
		hlist_min(goodpairs,&bp,&f);
		e = FindGradPaths12(f,p,grad_path,0);
		if(id_is_null(e))  continue;  // no gradient paths from |t| found
		thisp = value(f)-value(e);
		if(thisp<=bp) 
		{
			list_clear(changed);
			Cancel12(e,f,grad_path,changed);  // cancel |e| and |t|
			@<fix up split-rejoin paths@>;

			if(thisp<lastp) printf("\n persistence out of order: %d > %d\n",lastp,thisp);

			lastp = thisp;
		}
		else hlist_add(goodpairs,&thisp,&f);
	}
}



	


@ After canceling a pair of one and two simplices,
there may be new pairs of gradient paths which differ only on the
boundary of a single tetrahedron.
This code modifies the simplex pairings to eliminate this happening.
It is not clear whether it is worth doing this.
Experiments show that this tends to allow cancellation of a few more pairs
of critical one and two simplices, but not many.
Experiments also show that very little time is spent in this code.
So I have left it in.
We have not yet attempted to prove that this fixing up process always
terminates, so to be careful I have put a limit of 10000 iterations which has so far never
been exceeded.

@<fix up split-rejoin paths@>=
{
	int i,j,k;
	simplex_id t;

	k = 0;
	while (!list_is_empty(changed) && k<10000)
	{
		f = id_list_pop(changed);
		if(id_is_null(f)) continue;
		for (i=0;i<2;i++)
		{
			t = coface(f,i);
			if( tetrahedron_is_in_K(t) && !tetrahedron_is_critical(t) )
			{
				if(splitrejoin(t,changed)) k++;
			}
		}
	}
	if (!list_is_empty(changed)) printf("May be infinite loop in split-rejoin");
}


@ Check if |t| is a bad tetrahedron where a gradient path could split and rejoin.
It returns -1 if not, otherwise it returns |i| so that |get_face(t,i)| is bad.
@d debug9 1
@<Subroutines finding gradient paths@>=
Boolean splitrejoin(simplex_id t,list *changed)
{
	int i,j;
	simplex_id e,ep;
	simplex_id s,sp;
	
	for(i=0;i<4;i++)
	{
		s = get_face(t,i);
		if(triangle_is_paired_down(&s))
		{
			e = r21(&s);
			for(j=0;j<3;j++)
			{
				ep = get_edge(s,j);
				if(!edges_equal(ep,e)
					&&(!edge_is_paired_up(&ep)||triangle_in_tetrahedron(r12(&ep),t)<0)) break;
			}
			if(j==3)
			{
				sp = r32(t); 
@q+9			if(!triangles_equal(r32(t),sp)) abort_message("Split-rejoin error"); @>
				pair23(&s,t,true);
				pair12(&e,sp,true);
				id_list_push(changed,sp);
				return true;
			}
		}
	}
	return false;
}




@*1 Finding Gradient Paths. Find the end of a gradient path starting at $u\in K_0$.
Return the critical vertex $v$ at the end of the path,
or return |NULL_ID| if $h(v)\le m$.

@<Subroutines finding gradient paths@>=
simplex_id FindGrad01(simplex_id u,SInt32 m)
{
	simplex_id v;
	simplex_id e;

	v=u;
	while(!vertex_is_critical(v) && vertex_value(v) > m)
	{
		e = r01(v);
		v = other_vertex_in_edge(v,e);
	}
	if (vertex_value(v) <= m) return NULL_ID;
	return v;
}

@ Find the start of a gradient path passing through a
tetrahedron $\tau$.
Return the critical 3 simplex |t| at the start of the path,
or return |NULL_ID| if the value of |t| is  $\ge m$ or if the path starts
at a boundary 2 simplex.  
Mark simplices deadend if we know the only gradient paths going to them
start at a boundary 2 simplex.
This is a lazy way of shortening future gradient path searches.


@<Subroutines finding gradient paths@>=
simplex_id FindGrad23(simplex_id tau,SInt32 m)

{
	simplex_id t;
	simplex_id s;
	
	if (id_is_null(tau)) return NULL_ID;  
	
	t=tau;
	while( tetrahedron_is_in_K(t) && value(t) < m )
	{
		if (tetrahedron_is_critical(t)) return t;  // reached critical t
		s = r32(t);
		if (triangle_is_deadend(&s)) 
		{
			make_tetrahedron_deadend(t);
			return NULL_ID;
		}
		t = other_coface(s,t);
		if (tetrahedron_is_deadend(t)) 
		{
			make_triangle_deadend(&s);
			return NULL_ID;
		}
	}
	if(!tetrahedron_is_in_K(t) && t!=tau) make_triangle_deadend(&s);
	return NULL_ID;
}

@ Find the starting critical tetrahedron of a gradient path ending at
the critical triangle $\sigma$ and going through the tetrahedron |id2tetra_id(sigma)|.
Return $\pm 1$ in |orientation| depending on the change in orientation along the path.
@<Subroutines finding gradient paths@>=
simplex_id FindGrad23orientation(simplex_id sigma,int *orientation)
{
	simplex_id t;
	simplex_id s;
	int orient;
	
	t=id2tetra_id(sigma);
	orient = face_index(sigma);
	
	while( tetrahedron_is_in_K(t) )
	{
		if (tetrahedron_is_critical(t)) 
		{
			if(orient&1) *orientation=-1; else *orientation=1;
			return t;  // reached critical t
		}
		s = r32(t);
		orient += face_index(s);
		if (triangle_is_deadend(&s)) 
		{
			make_tetrahedron_deadend(t);
			return NULL_ID;
		}
		s=other_face_id(s);
		t = id2tetra_id(s);
		if (tetrahedron_is_deadend(t)) 
		{
			make_triangle_deadend(&s);
			return NULL_ID;
		}
		orient++;
		orient += face_index(s);
	}
	return NULL_ID;
}

@ Find all of the gradient paths starting at $\sigma$ which descend less than $p$,
and return the best one.
If |grad_path| is not |NULL| return a list of the edges in the best gradient path. 
The |flags| parameter, if odd, is experimental code which
changes the behavior by returning if possible a critical edge 
which is connected to $\sigma$  by at least one gradient path 
and could possibly generate persistent homology.

For performance reasons, the lists used are declared static
so they do not have to be initialized each time the routine is called.
I found that without doing this this routine spent
much of its time calling |malloc|.
As a consequence this routine is not reentrant.
If this is ever a problem, just delete the words |static| below.

Note that here and elsewhere we are essentially just searching for paths in a graph,
in this case for nodes connected by exactly one path.
No doubt there are sophisticated and well-known algorithms for doing this
which would improve performance.
As an excercise the reader can improve on these naive implementations.

8/30/2005 Possible problem: perhaps there is a second path 
which dips down below sigvalue-p and thus is not detected?

@<Subroutines finding gradient paths@>=
simplex_id FindGradPaths12(simplex_id sigma, SInt32 p, list *grad_path, int flags)
{
	static hlist *graph;
	static list *to_do;
	static list *crits;
	static Boolean first = true;
	struct edge_graph
	{
		SInt32 up;  // first uplink
		SInt32 count; // number of uplinks
		simplex_id e;
	} r,*q;
	simplex_id e;
	simplex_id f;
	simplex_id vlist[2];
	SInt32 m;
	SInt32 sigvalue = value(sigma);
	
	@<initialize lists |graph|, |crits|, and |to_do|@>@;
	
	m = -1;  // indicate |NULL| uplink
	e = NULL_ID;
	f = sigma;
	@<put eligible edges of |f| on |graph|, |crits|, and |to_do|@>@;
	
	while (!list_is_empty(to_do))
	{
		list_pop(to_do,&m);
		q = (struct edge_graph *)hlist_entry(graph,m);
		e = q->e;
		f = r12(&e); // get the |f| which is paired with |e|
		@<put eligible edges of |f| on |graph|, |crits|, and |to_do|@>@;
	}
	
	if(flags&1) @<find a persistent critical edge |e| in |crits|@>@;
	else @<find the critical edge |e| with only one grad path so |value(e)| is maximized@>@;
	
	return e;
}

@ @<initialize lists |graph|, |crits|, and |to_do|@>=
if(first)
{
	hlist_initialize(&graph,sizeof(struct edge_graph),703,2,0);
	list_initialize(&crits,sizeof(SInt32));
	list_initialize(&to_do,sizeof(SInt32));
	first=false;
}
else
{
	hlist_clear(graph);
	list_clear(crits);
	list_clear(to_do);
}


@ @<put eligible edges of |f| on |graph|, |crits|, and |to_do|@>=
{
	int i;
	SInt32 n;
	Boolean flag;
	simplex_id ep;
	int livecount;
	
	livecount=0;
	
	for(i=0;i<3;i++)
	{
		ep = get_edge(f,i);
		if (!edges_equal(ep,e) && !edge_is_paired_downx(ep) && !edge_is_deadendx(&ep))
		{
			livecount++;
				
				
			// The following statement must be eliminated 
			// otherwise one might think a pair is cancellable
			// when in fact there is a second path between them
			// which dips down below the limit and later comes back up.
			// Eliminating gives a big performancs hit though.
			// Perhaps there is a way to determine a valid lower bound
			// to replace |sigvalue - p| and thus regain most of the performance.
			// As it is, we end up determining the whole descending disc.
			// |if (value(ep) <= sigvalue - p)  continue;|
			
			
			r.up = m;
			r.count = 0;
			r.e = ep;
			get_edge_vertices(ep,vlist);
			
			n = hlist_find_add(graph,vlist,&r,&flag);
			q = (struct edge_graph*)hlist_entry(graph,n);
			q->count++; 
			
			if(!flag)
			{
				// |ep| is newly added to the list |graph|
				if(edge_is_critical(&ep)) list_push(crits,&n);
				else list_push(to_do,&n);
				// The next line might change q->e to the optimal descriptor of |ep|
				q->e = ep; 
			}
		}
	}
	if(livecount == 0 && !id_is_null(e)) make_edge_deadend(&e);
}

@ @<find the critical edge |e| with only one grad path so |value(e)| is maximized@>=
{
	SInt32 best_value,val,bestm;
	
	bestm=-1;
	
	while(!list_is_empty(crits))
	{
		list_pop(crits,&m);
		q = (struct edge_graph*)hlist_entry(graph,m);
		val = value(q->e);
		
		if (val <= sigvalue - p)  continue;
		if(q->count==1 && (bestm<0 || val > best_value))
			@<replace |bestm| by |m| if there's just one path to |q->e|@>@;
	}
	if(bestm>=0)
	{
		q = (struct edge_graph*)hlist_entry(graph,bestm);
		e = q->e;
	}
	else e = NULL_ID;
	if(!id_is_null(e) && grad_path!=NULL) @<put gradient path to |e| on |grad_path|@>@;
	
}

@ @<replace |bestm| by |m| if there's just one path to |q->e|@>=
{
	
	while(q->up >= 0)
	{
		q = (struct edge_graph*)hlist_entry(graph,q->up);
		if(q->count > 1) break;
	}
	if(q->count==1)
	{
		best_value = val;
		bestm = m;
	}
}

@ @<put gradient path to |e| on |grad_path|@>=
{
	list_clear(grad_path);
	
	m = bestm;
	
	while(m >= 0)
	{
		q = (struct edge_graph*)hlist_entry(graph,m);
		id_list_push(grad_path,q->e);
		m = q->up;
	}
}


@ Find all gradient paths from the triangle |sigma| and return them encoded in |edges|.
If |options&3 == 0|, then no pruning is done,
if |options&3 == 1|, then lazy pruning is done,
if |options&3 == 2|, then full pruning is done.
Pruning deletes edges which are not connected to a critical edge
by some gradient path.
If |options&4| is set, then the count field in the edges data structure
will be filled in.
The ordered list |edges| is a list of structures |grad12_struct| below,
one for each edge connected to |sigma|, with 2 keys, the ids of
the edges vertices.
Each edge |e| is either critical or paired with a triangle |t|.
The fields |links| have the following meaning for an edge |e|:
One of the links we call the uplink, and the remaining links are side links.
If the edge is critical, the uplink is |link[0]|,
otherwise it is |links[i]| if $i$ is the index of |e| in |t|.
This uplink is the list index of an edge |ep| so that
|e| is contained in the triangle paired with |ep|,
in fact, the start of a linked list of such edges.
The links of this list are in the side links.
So for example, if the uplink of |e| points to |ep|
and |e| has index |j| in the triangle paired with |ep|,
then |links[j]| (of |ep|?) points to another edge |epp| paired with a 
triangle containing |e|, or is -1 if this is the last item on the list.
In case |e| is critical, then the side link |links[1]|
is the list index of another critical edge.

We could also look at |edges| as the vertices of
a directed graph $G$ without cycles.
There is a directed path in $G$ from $ep$ to $e$ if $e$ is in the
triangle paired with $ep$ and $e\ne ep$.
Then the uplink of $e$ is an $ep$ pointing to $e$,
and the other two are $epp$ pointing to an edge also pointed to by $e$.
It still sounds confusing but it works.
|flags&3| is the index of the edge in its paired triangle, or 3 if critical;
If complete pruning is requested (|options&3 == 2|) and
|flags&4| is set, then the edge is connected to a critical
edge by a gradient path, i.e., it cannot be pruned.

The return value of |find_all_grad12_paths| is the list index of a critical edge,
so that all critical edges are obtained by following the |links[1]|.
The |count| field gives the signed number of gradient paths
going through the edge (counting orientation).
If |flags&8| is set, then the count field in the edges data structure
is filled in.
If |flags&16| is set, then the count field in the edges data structure
cannot yet be filled in.

@(cancel.h@>=
struct grad12_struct
{
	SInt32 links[3];
	SInt32 fake;
	SInt32 count;
	SInt32 count2;
	simplex_id e;
	int flags;
};


@ @<Subroutines finding gradient paths@>=

void
check_grad12_struct(hlist *edges,int options)
{
	int i,k,kk,id;
	struct grad12_struct *p,*q;
	simplex_id e,ep,f,vlist[2];
	
	for(i=0;i<hlist_count(edges);i++)
	{
		p = (struct grad12_struct *)hlist_entry(edges,i);
		e = p->e;
		kk = p->flags&3;
		if(kk!=3)
		{
			if(!edge_is_paired_up(&e)) abort_message("check grad12 err 1");
			f = r12(&e);
			for(k=0;k<3;k++)
			{
				if(k==kk) continue;
				ep = get_edge(f,k); 
				if((options&3)!=0 && edge_is_deadend(&ep)) continue;
				@<check that |e| is a parent of |ep|@>@;
			}
		}

	}
}

@ @<check that |e| is a parent of |ep|@>=
{
	simplex_id epp;
	int j;
	
	get_edge_vertices(ep,vlist);
	id = hlist_find(edges,vlist);
	if(id<0)
	{
		if(!edge_is_deadend(&ep) && !edge_is_paired_down(&ep) ) 
			abort_message("check grad12 err 2");
	}
	else
	{
		q = (struct grad12_struct *)hlist_entry(edges,id);
		
		id = q->links[(q->flags&3)%3];
		
		while(id>=0)
		{
			if(id==i) break;
			q = (struct grad12_struct *)hlist_entry(edges,id);
			epp = q->e;
			
			j = edge_verts_in_triangle(vlist,r12(&epp));
			if(j<0) abort_message("check grad12 err 3");
			id = q->links[j];
		}
		if(id<0) abort_message("check grad12 err 4");
	}
}



@ @<Subroutines finding gradient paths@>=
SInt32
find_all_grad12_paths(simplex_id sigma,hlist *edges,int options)
{
	int kk = -1;
	simplex_id ep;
	SInt32 todo = -2;
	SInt32 this;
	SInt32 critid = -1;
	struct grad12_struct *p;
	simplex_id f;
	
	hlist_clear(edges);
	
	do
	{
		@<find an unprocessed triangle |f|@>@;
		@<find-add edges of |f| to |edges| if needed@>@;
	} while(todo>=0);
	
	check_grad12_struct(edges,options);
	
	if((options&3)==2)
		@<prune |edges|@>@;
		
	if(options&4)
		@<fill in count field@>@;
	
	if(options&8)
		@<fill in count2 field@>@;
	
	return critid;
}

@ If nonnegative, |todo| is the list index of an edge 
paired to an unprocessed triangle.
All these unprocessed triangles are in a list,
linked by either |links[0]| or |links[1]|.
@<find an unprocessed triangle |f|@>=
{
	this = todo;
	if(this>=0)
	{
		p = (struct grad12_struct *)hlist_entry(edges,this);
		ep = p->e;
		kk = p->flags&3;
		if(kk)
		{
			todo = p->links[0];
			p->links[0] = -1;
		}
		else
		{
			todo = p->links[1];
			p->links[1] = -1;
		}
		f = r12(&ep);
	}
	else	f = sigma;
}

@ @<find-add edges of |f| to |edges| if needed@>=
{
	int k;
	simplex_id e;
	int livecount = 0;
	
	for(k=0;k<3;k++)
	{
		if(k==kk) continue;
		e = get_edge(f,k);
		@<find-add |e| to |edges| if needed@>@;
	}	
	if(livecount == 0 && (options&3)!=0 && this>=0) 
	{
		make_triangle_deadend(&f);
		make_edge_deadend(&ep);
	}
}


@ If the edge |e| is paired up or critical 
we increment |livecount|, we point 
the uplink of |e| to |ep| (which has list index |this|) and
we set the sidelink of |ep| to the former uplink of |e|,
thus linking up the list.
If |e| is paired up but not on the list already, we link |e|
into the todo list.
On the other hand, if |e| is critical but not on the list already
we put it at the start of the linked list of critical edges.
@<find-add |e| to |edges| if needed@>=
{
	struct grad12_struct r,*q;
	simplex_id s,vlist[2];
	Boolean flag;
	int j;
	SInt32 id;
	
	// For efficiency, first do the following tests 
	// which don't require an optimal edge id.  
	if( ( (options&3)==0 || !edge_is_deadend(&e)) && !edge_is_paired_down(&e) )
	{
		livecount++;
		get_edge_vertices(e,vlist);
		r.fake=r.count=r.count2=0;
		id = hlist_find_add(edges,vlist,&r,&flag);
		q = (struct grad12_struct *)hlist_entry(edges,id);
		if(flag)  // already on list
		{
			p = (struct grad12_struct *)hlist_entry(edges,this);
			p->links[k] = q->links[(q->flags&3)%3];
			q->links[(q->flags&3)%3] = this;
		}
		else // newly added to list
		{
			// note the following test will change |e| to the optimal id of the edge
			//   as a side effect of the test.
			if(edge_is_paired_up(&e))
			{
				q->links[1] = q->links[2] = -1;
				s = r12(&e);
				q->e = e;
				q->flags = edge_in_triangle(e,s);
				if(q->flags<0) abort_message("help");
				q->links[q->flags] = this;
				if(q->flags) q->links[0] = todo;
				else q->links[1] = todo;
				todo = id;
			}
			else if(edge_is_critical(&e))
			{
				q->e = e;
				q->links[0] = this;
				q->links[1] = critid;
				q->flags = 3;
				critid = id;
			}
		}
	}
}

@ @<prune |edges|@>=
{
	SInt32 j;
	simplex_id e,s;
	list *todo_list;
	
	list_initialize(&todo_list,sizeof(SInt32));
	
	todo = critid;
	
	while(todo>=0)
	{
		@<mark all edges on critical paths to this critical edge@>@;
		p = (struct grad12_struct *)hlist_entry(edges,todo);
		todo = p->links[1];
	}
	
	list_abandon(&todo_list);

	for(j=0;j<hlist_count(edges);j++)
	{
		p = (struct grad12_struct *)hlist_entry(edges,j);
		if(p->flags & 4) continue;
		e = p->e;
		make_edge_deadend(&e);
		s = r12(&e);
		make_triangle_deadend(&s);
	}
}

@ @<mark all edges on critical paths to this critical edge@>=
{
	SInt32 next,id;
	struct grad12_struct *q;
	simplex_id vl[2];
	int i;
	
	list_clear(todo_list);
	list_push(todo_list,&todo);
	
	while(!list_is_empty(todo_list))
	{
		list_pop(todo_list,&next);
		p = (struct grad12_struct *)hlist_entry(edges,next);
		
		if(p->flags&4) continue;
		p->flags|=4;
		
		i = p->flags&3;
		id = p->links[i%3];
		
		e = p->e;
		vl[0] = hlist_get_key(edges,next,0);
		vl[1] = hlist_get_key(edges,next,1);
		
		while(id>=0)
		{
			q = (struct grad12_struct *)hlist_entry(edges,id);
			
			ep = q->e;
			
			if((q->flags&4) == 0)
			{
				list_push(todo_list,&id);
			}
			i = edge_verts_in_triangle(vl,r12(&ep));
			if(i<0) abort_message("error");
			id = q->links[i];
		}
	}
}


@ @<fill in count field@>=
{
	list *todo_list;
	SInt32 id = -1,*idp;
	struct grad12_struct *q;
	int error_check;
	
	list_initialize(&todo_list,sizeof(SInt32));
	list_push(todo_list,&id);
	
	while(!list_is_empty(todo_list))
	{
		list_read_init(todo_list);
		error_check = 1;
		while((idp = (SInt32 *)list_read(todo_list))!=NULL)
		{
			if(*idp<0)
			{
				list_read_delete(todo_list);
				kk = -1;
				f = sigma;
			}
			else
			{
				p = (struct grad12_struct *)hlist_entry(edges,*idp);
				if(p->flags&8) {list_read_delete(todo_list); continue;}
				//if(p->flags&24) continue;

				@<count if all incoming faces are counted@>@;
			}
				
			if(!id_is_null(f))
			{
				error_check = 0;
				@<add edges of |f| to |todo_list|@>@;
			}
		}
		if(error_check && !list_is_empty(todo_list)) 
		{  // debugging code follows
		
			
			@<backtrack uncounted ancestors@>@;
		
		
		
			// I don't understand it, sometimes the todo list
			// is not empty but will never be shortened.
			// here we check that nevertheless, all critical edges
			// have a valid count.
			//  If so, there must be some deadend edges unmarked I think.
			
			todo = critid;
			
			while(todo>=0)
			{
				p = (struct grad12_struct *)hlist_entry(edges,todo);
				if(p->flags&8==0) break;
				todo = p->links[1];
			}
		
			if(todo>=0) 
			{
				// If we reach here not all critical edges have a valid count.
				list_read_init(todo_list);
				while((idp = (SInt32 *)list_read(todo_list))!=NULL)
				{
					simplex_id vl[2];
					simplex_id e;
					int i;
					
					p = (struct grad12_struct *)hlist_entry(edges,*idp);
					ep = p->e;
					vl[0] = hlist_get_key(edges,*idp,0);
					vl[1] = hlist_get_key(edges,*idp,1);
					printf("%d %d ",*idp,p->flags);
					id = p->links[(p->flags&3)%3];
					while(id>=0)
					{
						q = (struct grad12_struct *)hlist_entry(edges,id);
						printf("%d-",q->flags);
						e = q->e;
						i = edge_verts_in_triangle(vl,r12(&e));
						id = q->links[i];
					}
					printf("\n");
				}			
				abort_message("count error");
			}
			else {printf("extra todo_list\n"); break;}
		}
	} 
	
	list_abandon(&todo_list);
}

@
@d edge_orient(i)  (((i)==1)?-1:1)
@<count if all incoming faces are counted@>=
{	
	simplex_id e;
	int i;
	simplex_id vl[2];
	SInt32 ct=0;
	
	ep = p->e;
	vl[0] = hlist_get_key(edges,*idp,0);
	vl[1] = hlist_get_key(edges,*idp,1);

	kk = p->flags&3;
	id = p->links[kk%3];
	
	while(id>=0)
	{
		q = (struct grad12_struct *)hlist_entry(edges,id);
		if((q->flags&8)==0) break;
		e = q->e;
		i = edge_verts_in_triangle(vl,r12(&e));
@q+9	if(i<0) abort_message("count error2");@>
		ct += edge_orient(i)*q->count;
		id = q->links[i];
	}
	
	if(id<0)
		@<finish counting incoming faces to |ep|@>@;
	else p->flags |= 16;
	
	if(id>=0 || edge_is_critical(&ep)) f = NULL_ID;
	else	f = r12(&ep);
}

@ @<add edges of |f| to |todo_list|@>=
{
	int k;
	simplex_id e;
	
	for(k=0;k<3;k++)
	{
		if(k==kk) continue;
		e = get_edge(f,k);
		@<add |e| to |todo_list|@>@;
	}	
}

@ @<add |e| to |todo_list|@>=
{
	int flag;
	SInt32 qid;
	simplex_id vll[2];
	
	if(!edge_is_paired_down(&e) && ( (options&3)==0 || !edge_is_deadend(&e) ))
	{
		get_edge_vertices(e,vll);
		qid = hlist_find(edges,vll);
@q+9	if(qid<0) abort_message("count error 2");@>
		q = (struct grad12_struct *)hlist_entry(edges,qid);
		if((q->flags & 32)==0)
		{
			list_read_insert(todo_list,&qid);
			q->flags |= 32;
		}
		q->flags &= 0xffffffef; // indicate count field perhaps can now be filled,
		// since another incoming triangle has been counted.
	}
}

@ @<finish counting incoming faces to |ep|@>=
{
	p->flags &= 0xffffffef;
	p->flags |= 8;
	list_read_delete(todo_list);

	if (id==-2)
	{
		// |sigma| is a coface of |ep| so count it
		ct += edge_orient(edge_in_triangle(ep,sigma));
	}
	if(edge_is_critical(&ep)) p->count = ct;
	else p->count = -ct * edge_orient(p->flags&3);
	if(p->count>1000 || p->count<-1000) abort_message("large count");
}

@ @<backtrack uncounted ancestors@>=
{
	simplex_id vl[2];
	simplex_id e;
	int i;
	
	printf("\nbacktrack: ");
	id = *(SInt32 *)list_entry(todo_list,0);
	
	while(id>=0)
	{
		p = (struct grad12_struct *)hlist_entry(edges,id);
		ep = p->e;
		vl[0] = hlist_get_key(edges,id,0);
		vl[1] = hlist_get_key(edges,id,1);
		printf(" %d %d,",id,p->flags);
		if(p->flags & 4096) break;
		p->flags |= 4096;
		id = p->links[(p->flags&3)%3];
		while(id>=0)
		{
			p = (struct grad12_struct *)hlist_entry(edges,id);
			if((p->flags&8)==0) break;
			e = p->e;
			i = edge_verts_in_triangle(vl,r12(&e));
			id = p->links[i];
		}
	}
	
	printf("\n");
	if(p->flags & 4096) abort_message("cyclic gradient field");
	
}

@ @<backtrack uncount2ed ancestors@>=
{
	simplex_id vl[2];
	simplex_id e;
	SInt32 id;
	
	printf("\nbacktrack2: ");
	id = *(SInt32 *)list_entry(todo_list,0);
	
	while(id>=0)
	{
		p = (struct grad12_struct *)hlist_entry(edges,id);
		ep = p->e;
		vl[0] = hlist_get_key(edges,id,0);
		vl[1] = hlist_get_key(edges,id,1);
		printf(" %d %d,",id,p->flags);
		id = p->links[(p->flags&3)%3];
		while(id>=0)
		{
			p = (struct grad12_struct *)hlist_entry(edges,id);
			if((p->flags&1024)==0) break;
			e = p->e;
			i = edge_verts_in_triangle(vl,r12(&e));
			id = p->links[i];
		}
	}
	printf("\n");
	if(p->flags & 4096) abort_message("cyclic gradient field");
	
}

@ An alternative version since something is screwy here.
set |flags&256| if an edge is added to todo list
set |flags&1024| if an edge has a valid count2 field
set |flags&2048| if an edge cannot yet be given a valid count2 field


@<fill in count2 field@>=
{
	list *todo_list;
	SInt32 *idp;
	int error_check;
	int i;
	
	list_initialize(&todo_list,sizeof(SInt32));
	
	// first push all critical edges to todo_list
	todo = critid;
	
	while(todo>=0)
	{
		p = (struct grad12_struct *)hlist_entry(edges,todo);
		p->flags |= 256;
		list_push(todo_list,&todo);
		todo = p->links[1];
	}
	
	// now push all ancestors of critical edges to todo_list 
	for(i=0;i<list_count(todo_list);i++)
	{
		todo = *(SInt32 *)list_entry(todo_list,i);
		p = (struct grad12_struct *)hlist_entry(edges,todo);
		@<add parents of |p| to |todo_list| if not there already@>@;
	}
	
	// repeatedly read through, counting those we can,
	// and eliminating them from |todo_list|
	while(!list_is_empty(todo_list))
	{
		list_read_init(todo_list);
		error_check = 1;
		while((idp = (SInt32 *)list_read(todo_list))!=NULL)
		{
			p = (struct grad12_struct *)hlist_entry(edges,*idp);
			if(p->flags&1024) {list_read_delete(todo_list); continue;}
			//if(p->flags&2048) continue;

			@<count2 if all incoming faces are count2ed@>@;

		}
		if(error_check && !list_is_empty(todo_list)) 
		{  
			@<backtrack uncount2ed ancestors@>@;
			abort_message("count2 error");
		}
	} 
	
	list_abandon(&todo_list);
}

@ @<count2 if all incoming faces are count2ed@>=
{	
	SInt32 id;
	SInt32 ct=0;
	struct grad12_struct *q;
	simplex_id e;
	simplex_id vl[2];
	
	ep = p->e;
	vl[0] = hlist_get_key(edges,*idp,0);
	vl[1] = hlist_get_key(edges,*idp,1);

	kk = p->flags&3;
	id = p->links[kk%3];
	
	while(id>=0)
	{
		q = (struct grad12_struct *)hlist_entry(edges,id);
		if((q->flags&1024)==0) break;
		e = q->e;
		i = edge_verts_in_triangle(vl,r12(&e));
@q+9	if(i<0) abort_message("count error2");@>
		ct += edge_orient(i)*q->count2;
		id = q->links[i];
	}
	
	if(id<0)
		@<finish count2 incoming faces to |ep|@>@;
	else p->flags |= 2048;
	
	if(id<0 && !edge_is_critical(&ep))
		@<mark children of |ep| as possible count2s@>@;
}

@ @<finish count2 incoming faces to |ep|@>=
{
	
	p->flags &= 0xfffff7ff;
	p->flags |= 1024;
	list_read_delete(todo_list);

	if (id==-2)
	{
		// one incoming face is |sigma|, so count it too
		ct += edge_orient(edge_in_triangle(ep,sigma));
	}
	if(edge_is_critical(&ep)) p->count2 = ct;
	else p->count2 = -ct * edge_orient(p->flags&3);
	
	error_check = 0;
}

@ @<mark children of |ep| as possible count2s@>=
{
	int k;

	f = r12(&ep);
	
	for(k=0;k<3;k++)
	{
		if(k==kk) continue;
		e = get_edge(f,k);
		@<mark |e| as possible count2@>@;
	}	
}

@ @<mark |e| as possible count2@>=
{
	SInt32 qid;
	simplex_id vll[2];
	
	if(!edge_is_paired_down(&e))
	{
		get_edge_vertices(e,vll);
		qid = hlist_find(edges,vll);
		if(qid>=0) 
		{
			q = (struct grad12_struct *)hlist_entry(edges,qid);
			q->flags &= 0xfffff7ff; // indicate count2 field perhaps can now be filled,
			// since another incoming triangle has been count2ed.
		}
		else if( (options&3)==0  || !edge_is_deadend(&e) )
			abort_message("count2 error 37");
	}
}


@ @<add parents of |p| to |todo_list| if not there already@>=
{
	SInt32 id;
	struct grad12_struct *q;
	simplex_id vl[2],e;

	kk = p->flags&3;
	id = p->links[kk%3];
	
	e = p->e;
	vl[0] = hlist_get_key(edges,todo,0);
	vl[1] = hlist_get_key(edges,todo,1);
	
	while(id>=0)
	{
		q = (struct grad12_struct *)hlist_entry(edges,id);
		
		ep = q->e;
		
		if((q->flags&256) == 0)
		{
			list_push(todo_list,&id);
			q->flags |= 256;
		}
		kk = edge_verts_in_triangle(vl,r12(&ep));
		if(kk<0) abort_message("error");
		id = q->links[kk];
	}
}

@ Find all gradient paths ending at the edge tau.
Now the hlist key is a triangle id but the entries are still grad12_structs.
The list |crits| is a list of indices of items in |triangles|
which are critical triangles.

@<Subroutines finding gradient paths@>=
SInt32
find_all_backward_grad12_paths(simplex_id tau,hlist *triangles,list *crits,int options)
{
	int kk = -1;
	simplex_id f;
	simplex_id ep;
	SInt32 todo = -1;
	SInt32 this;
	struct grad12_struct *p;
	SInt32 start = -1;
	list *todo_list;
	int tab[6] = {3,3,2,3,2,1};

	hlist_clear(triangles);
	if(crits!=NULL) list_clear(crits);
	if((options&3)==2) list_initialize(&todo_list,sizeof(SInt32));

	do
	{
		this = todo;
		if(this>=0)
		{
			p = (struct grad12_struct *)hlist_entry(triangles,this);
			f = hlist_get_key(triangles,this,0);
			ep = r21(&f);
			kk = p->flags&3;
			todo = p->links[kk];
			p->links[kk] = -1;
		}
		else ep = tau;
		@<find-add cofaces of |ep| to |triangles| if needed@>@;
	} while(todo>=0);
	if((options&3)==2)
		@<prune |triangles|@>@;
	return start;
}

@ @<find-add cofaces of |ep| to |triangles| if needed@>=
{
	simplex_id t,t1;
	simplex_id vlist[3];
	int link_count = 0;
	
	t = t1 = id2tetra_id(ep);
	get_edge_vertices(ep,vlist);
	vlist[2] = id2tetrahedron(t)->vertex_ids[tab[edge_index(ep)]];
	f = vertices2trianglex(vlist,t);
	
	do
	{
		if(id_is_null(f)) abort_message("backgrad error 3");
		if(triangle_is_in_K(f) && (triangle_is_critical(&f) ||
			((!triangle_is_xdeadend(&f) || (options&3)==0 ) &&
				triangle_is_paired_down(&f) && !edges_equal(ep,r21(&f)))))
		{
			link_count++;
			@<find-add |f| to |triangles|@>@;
		}
		f = next_face(vlist,f);
	} while(t!=t1);
	
	if(link_count==0 && (options&3) && this>=0)
	{
		make_edge_xdeadend(&ep);
		f = r12(&ep);
		make_triangle_xdeadend(&f);
	}
}

@ @<find-add |f| to |triangles|@>=
{
	struct grad12_struct r,*q;
	SInt32 id;
	Boolean flag;
	int k;
	
	f = best_face_id(f);
	
	id = hlist_find_add(triangles,&f,&r,&flag);
	q = (struct grad12_struct *)hlist_entry(triangles,id);
	k = edge_in_triangle(ep,f);
	if(this>=0)
		p = (struct grad12_struct *)hlist_entry(triangles,this);
	if(flag) // already on list
	{
		if(q->links[k]>=0) abort_message("backgrad error 1");
		if(this<0) abort_message("backgrad error 2");
		q->links[k] = p->links[kk];
		p->links[kk] = id;
	}
	else // newly added to list
	{
		q->links[0] = q->links[1] = q->links[2] = -2;
		if(this>=0) 
		{
			q->links[k] = p->links[kk];
			p->links[kk] = id;
		}
		else
		{
			q->links[k] = start;
			start = id;
		}
		if(triangle_is_critical(&f))
		{
			q->flags = 3;
			if(crits!=NULL) list_push(crits,&id);
			if((options&3)==2) list_push(todo_list,&id);
		}
		else
		{
			q->flags = edge_in_triangle(r21(&f),f);
			q->links[q->flags] = todo;
			todo = id;
		}
	}
}

@ @<prune |triangles|@>=
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
		p = (struct grad12_struct *)hlist_entry(triangles,id);
		if(p->flags&4) continue;
		p->flags|=4;
		kk = p->flags&3;
		f = hlist_get_key(triangles,id,0);
		for(k=0;k<3;k++)
		{
			if(k==kk || p->links[k] < -1) continue;
			e = get_edge(f,k);
			if(edge_is_paired_up(&e) && !edge_is_deadend(&e) )
			{
				if(edge_is_xdeadend(&e)) abort_message("back grad error 3");
				f1 = r12(&e);
				id = hlist_find_add(triangles,&f1,&r,&flag);
				q = (struct grad12_struct *)hlist_entry(triangles,id);
				if(flag==0) 
				{
					q->flags = edge_in_triangle(e,r12(&e));
					printf("back grad puzzle\n");
				}
				if((q->flags&4)==0) list_push(todo_list,&id);
			}
		}	
	}
	list_abandon(&todo_list);

	for(id=0;id<hlist_count(triangles);id++)
	{
		p = (struct grad12_struct *)hlist_entry(triangles,id);
		if(p->flags & 4) continue;
		f = hlist_get_key(triangles,id,0);
		make_triangle_xdeadend(&f); 
		e = r21(&f);
		make_edge_xdeadend(&e); 
	}
}





@ @<find a persistent critical edge |e| in |crits|@>=
{
	simplex_id ee;
	
	e = NULL_ID;
	while(!list_is_empty(crits))
	{
		list_pop(crits,&m);
		q = (struct edge_graph*)hlist_entry(graph,m);
		ee = q->e;
		if(false)
//		|if(edge_is_persistent(&ee))|  disable this for now
		{
			e = ee;
			break;
		}
	}
}
