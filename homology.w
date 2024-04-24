@* Homology. 
These compute homology with $Z/pZ$ coefficients,
for $p$ a prime.
Since the coefficient is a field, this can just be done by
computing the ranks of the boundary maps.
@c
#include "Morse.h"
#include "globals.h"

@ Compute the rank of a matrix 
with coefficients in $Z/2Z$ by Gaussian elimination (on the transpose). 
The |matrix| is $m\times n$
and the $i,j$ entry is at |matrix+m*j+i|.
@c
int gauss2(unsigned char *matrix,int m,int n)
{
	unsigned char **perm;  // column permutations
	int i,j,k,l;
	unsigned char *p,*q;

	perm = (unsigned char **)malloc(n*sizeof(unsigned char *));
	for(i=0;i<n;i++) perm[i] = matrix+m*i;
	
	for(i=0,j=0;(i<n && j<m);i++,j++)
	{
		@<search for first nonzero entry l,k on or below,right of j,i@>@;
		@<if there are no such nonzero entries, break@>@;
		@<switch the columns k and i and let j = l@>@;
		@<Xor column |i| with column |>i| to zero entries right of |i,j|@>@;
	}
	free(perm);
	return i;
}

@ @<search for first nonzero entry l,k on or below,right of j,i@>=
for(;j<m;j++)
{
	for(k=i;k<n;k++)
	{
		if(perm[k][j]) break;
	}
	if(k<n) break;
}

@ @<if there are no such nonzero entries, break@>=
if(j==m) break;

@ Note in fact |l| is ficticious and |j=l| already.
@<switch the columns k and i and let j = l@>=
if(k!=i)
{
	p=perm[i];
	perm[i]=perm[k];
	perm[k]=p;
}

@ @<Xor column |i| with column |>i| to zero entries right of |i,j|@>=
for(k=i+1;k<n;k++)
{
	if(perm[k][j])
	{
		p = perm[i]+j;
		q = perm[k]+j;
		for(l=j;l<m;l++) *q++^=*p++;
	}
}


@ Compute the rank of a matrix with Z/prime Z entries.
It needs prime $< 256$.

@c
int gaussp(unsigned char *matrix,int m,int n,int prime)
{
	unsigned char **perm;  // column permutations
	unsigned char *mult_tab;  // multiplication and inverse table, mod prime
	int i,j,k,l,m1,m2;
	unsigned char *p,*q,*t1,*t2;
	int prime1 = prime + 1;

	perm = (unsigned char **)malloc(n*sizeof(unsigned char *));
	for(i=0;i<n;i++) perm[i] = matrix+m*i;
	
	mult_tab = (unsigned char *)malloc(prime*prime1*sizeof(unsigned char));
	for(i=0;i<prime;i++) 
	{
		for(j=i;j<prime;j++)
		{
			k = (i*j)%prime;
			mult_tab[prime1*i+j] = k;
			mult_tab[prime1*j+i] = k;
			if(k==1) mult_tab[prime1*i+prime] = j;
			if(k==1) mult_tab[prime1*j+prime] = i;
		}
	}

	
	for(i=0,j=0;(i<n && j<m);i++,j++)
	{
		@<search for first nonzero entry l,k on or below,right of j,i@>@;
		@<if there are no such nonzero entries, break@>@;
		@<switch the columns k and i and let j = l@>@;
		@<subtract multiple of column |i| from column |>i| to zero entries right of |i,j|@>@;
	}
	free(perm);
	free(mult_tab);
	return i;
}



@ 
@d mod_sub(a,b) (((a)>=(b))?(a)-(b):(a)-(b)+prime)
@<subtract multiple of column |i| from column |>i| to zero entries right of |i,j|@>=
for(k=i+1;k<n;k++)
{
	t1 = mult_tab + prime1*mult_tab[prime1*perm[i][j]+prime];
	if(perm[k][j])
	{
		p = perm[i]+j;
		q = perm[k]+j;
		m1 = t1[perm[k][j]];
		t2 = mult_tab + prime1*m1;
		for(l=j;l<m;l++) 
		{
			m2 = t2[*p++];
			*q = mod_sub(*q,m2);
			q++;
		}
	}
}







@
@d entry(i,j,k) ((k==2)?mat[k][n[3]*i+j]:mat[k][n[k]*j+i])
@c
void test_homology(int flag)
{
	int i,j,k,m,n[4],rank[3],betti[4];
	SInt32 *mat[3];  // arrays containing the three boundary matrices of the chain complex
	unsigned char *matp[3];  // arrays containing the three mod p boundary matrices
	simplex_id e;
	simplex_id f;
	SInt32 c,*p;  
	unsigned char odd_primes[11] = {3,5,7,11,13,17,19,23,29,31,37};
	
	clean_crit();
	for(i=0;i<4;i++) n[i]=list_count(crit[i]);
	
	for(i=0;i<3;i++)
	{
		mat[i] = (SInt32 *)malloc(n[i]*n[i+1]*sizeof(SInt32));
		matp[i] = (unsigned char *)malloc(n[i]*n[i+1]*sizeof(unsigned char));
	}
	
	j=0;
	list_read_init(crit[1]);
	while (!id_is_null(e=id_list_read(crit[1])))  // run through all critical edges
	{
		@<fill j-th column of |mat[0]| with number of gradient paths to each critical vertex@>@;
		j++;
	}

	j=0;
	list_read_init(crit[2]);
	while (!id_is_null(f=id_list_read(crit[2])))  // run through all critical faces
	{
		@<fill j-th column of |mat[1]| with number of gradient paths from |f| to each critical edge@>@;
		@<fill j-th column of |mat[2]| with number of grad paths to |f| from each critical tetra@>@;
		 // mat[2] will be transposed
		j++;
	}

	@<check that the boundary of the boundary is 0@>@;
	
	if(flag&4) @<calculate mod 2 Betti numbers@>@;
	
	m = flag>>3;
	for(j=0;j<11;j++)
	{
		if((m>>j)&1) @<calculate mod p Betti numbers@>@;
	}
}

@ @<fill j-th column of |mat[0]| with number of gradient paths to each critical vertex@>=
{
	simplex_id v0, v1, w;
	
	v0 = FindGrad01(get_vertex(e,0),-100000);
	v1 = FindGrad01(get_vertex(e,1),-100000);
	p = mat[0]+n[0]*j;
	i=0;
	if(v0!=v1)
	{
		m=0;
		list_read_init(crit[0]);
		while(m<2)
		{
			w = id_list_read(crit[0]);
			if(id_is_null(w)) abort_message("missing critical vertex");
			if(w==v0) {*p=1; m++;}
			else if(w==v1) {*p=-1; m++;}
			else *p=0;
			p++;
			i++;
		}
	}
	for(;i<n[0];i++) *p++=0;
}

@ @<fill j-th column of |mat[2]| with number of grad paths to |f| from each critical tetra@>=
{
	simplex_id te0,te1,te;
	int or0,or1;
	
	te0 = FindGrad23orientation(f,&or0);
	te1 = FindGrad23orientation(other_face_id(f),&or1);
	p = mat[2]+n[3]*j;
	i=0;
	m=0;
	k=2;
	if(id_is_null(te0)) k--;
	if(id_is_null(te1)) k--;
	list_read_init(crit[3]);
	while(m<k)
	{
		te = id_list_read(crit[3]);
		if(id_is_null(te)) abort_message("missing critical tetrahedron");
		*p=0;
		if(te==te0) {*p+=or0; m++;}
		if(te==te1) {*p+=or1; m++;}
		p++;
		i++;
	}
	for(;i<n[3];i++) *p++=0;
}





@ @<fill j-th column of |mat[1]| with number of gradient paths from |f| to each critical edge@>=
{
	hlist *edges;
	SInt32 index;
	struct grad12_struct *q;
	simplex_id vlist[2];
	
	hlist_initialize(&edges,sizeof(struct grad12_struct),113,2,0);
	find_all_grad12_paths(f,edges,8+4+2);

	
	p = mat[1]+n[1]*j;
	list_read_init(crit[1]);

	while(!id_is_null(e=id_list_read(crit[1])))
	{
		get_edge_vertices(e,vlist);		
		index = hlist_find(edges,vlist);
		if(index<0) *p=0;
		else
		{
			q = (struct grad12_struct *)hlist_entry(edges,index);
			*p = q->count;
		}
		p++;
	}
	hlist_abandon(&edges);
}

@ @<check that the boundary of the boundary is 0@>=
for(i=0;i<n[1];i++)
{
	for(j=0;j<n[3];j++)
	{
		c=0;
		for(k=0;k<n[2];k++)
		{
			c+= (entry(i,k,1)*entry(k,j,2));
		}
		if(c!=0) abort_message("boundary of boundary is not 0");
	}
}

for(i=0;i<n[0];i++)
{
	for(j=0;j<n[2];j++)
	{
		c=0;
		for(k=0;k<n[1];k++)
		{
			c+= (entry(i,k,0)*entry(k,j,1));
		}
		if(c!=0) abort_message("boundary of boundary isn't 0");
	}
}

@ @<calculate mod 2 Betti numbers@>=
{
	for(i=0;i<n[0]*n[1];i++) matp[0][i] = mat[0][i]&1;
	for(i=0;i<n[1]*n[2];i++) matp[1][i] = mat[1][i]&1;
	for(i=0;i<n[2]*n[3];i++) matp[2][i] = mat[2][i]&1;
	
	rank[0] = gauss2(matp[0],n[0],n[1]);
	rank[1] = gauss2(matp[1],n[1],n[2]);
	rank[2] = gauss2(matp[2],n[3],n[2]);
	
	betti[3] = n[3] - rank[2];
	betti[2] = n[2] - rank[1] - rank[2];
	betti[1] = n[1] - rank[0] - rank[1];
	betti[0] = n[0] - rank[0];
	
	for(i=0;i<4;i++) printf("mod 2 betti number %d is %d\n",i,betti[i]);
}

@ @<calculate mod p Betti numbers@>=
{
	int prime = odd_primes[j];
	
	for(i=0;i<n[0]*n[1];i++) { c = mat[0][i]%prime; matp[0][i] = (c<0)?c+prime:c;}
	for(i=0;i<n[1]*n[2];i++) { c = mat[1][i]%prime; matp[1][i] = (c<0)?c+prime:c;}
	for(i=0;i<n[2]*n[3];i++) { c = mat[2][i]%prime; matp[2][i] = (c<0)?c+prime:c;}
	
	rank[0] = gaussp(matp[0],n[0],n[1],prime);
	rank[1] = gaussp(matp[1],n[1],n[2],prime);
	rank[2] = gaussp(matp[2],n[3],n[2],prime);
	
	betti[3] = n[3] - rank[2];
	betti[2] = n[2] - rank[1] - rank[2];
	betti[1] = n[1] - rank[0] - rank[1];
	betti[0] = n[0] - rank[0];
	
	for(i=0;i<4;i++) printf("mod %d betti number %d is %d\n",prime,i,betti[i]);
}