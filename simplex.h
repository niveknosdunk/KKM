/*2:*/
#line 23 "./temp.w"

typedef SInt32 simplex_id;


/*:2*//*3:*/
#line 41 "./temp.w"


typedef struct vertexstruct
{
UInt8 type;
SInt16 h;
simplex_id tetra_id;
GLfloat coords[3];
}vertex;

/*:3*//*4:*/
#line 132 "./temp.w"

typedef struct tetrahedronstruct
{
UInt8 types[11];
UInt8 flag;

simplex_id vertex_ids[4];
simplex_id face_ids[4];
}tetrahedron;

/*:4*//*7:*/
#line 168 "./temp.w"

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

void get_edge_vertices(simplex_id e,simplex_id*vl);
void get_triangle_vertices(simplex_id f,simplex_id*vlist);
int triangle_in_tetrahedron(simplex_id f,simplex_id t);
list*vertex_star(simplex_id v);

/*:7*//*8:*/
#line 186 "./temp.w"

#define get_vertex(e,i)  (id2tetrahedron(e)->vertex_ids[vtab[ edge_index(e)][i]])
#define get_tetrahedron_vertices(t,vlist) \
 memcpy(vlist,id2tetrahedron(t)->vertex_ids,4*sizeof(simplex_id))

simplex_id other_vertex_in_edge(simplex_id u,simplex_id e);

/*:8*//*10:*/
#line 242 "./temp.w"

#define get_edge(f,i)  (id2tetra_id(f)+etab[face_index(f)][i])
#define get_face(t,i)  (id2tetra_id(t)+(i)+1)
#define get_edge_of_tetrahedron(s,i,j) (id2tetra_id(s)+etab2[i][j])
#define coface0(f) id2tetra_id(f)
#define coface1(f) id2tetra_id(id2tetrahedron(f)->face_ids[face_index(f)])
#define coface(f,i) (((i)==0)? coface0(f): coface1(f))
#define other_face_id(f) id2tetrahedron(f)->face_ids[face_index(f)]

simplex_id other_coface(simplex_id s,simplex_id t);
simplex_id other_edge(simplex_id v,simplex_id e,simplex_id t);


/*:10*//*19:*/
#line 571 "./temp.w"

#define vertex_is_in_K(v) ((id2vertex(v)->type & 1)!=0)
#define edge_is_in_K(e) ((id2tetrahedron(e)->types[(e)&15] & 1)!=0)
#define triangle_is_in_K(f) ((id2tetrahedron(f)->types[(f)&15] & 1)!=0)
#define tetrahedron_is_in_K(t) ((id2tetrahedron(t)->types[0] & 1)!=0)


/*:19*//*20:*/
#line 581 "./temp.w"

#define vertex_value(v)  ((SInt32)id2vertex(v)->h)
#define vertex_compare(v,w) (v-w)
#define smallest_vertex_in_edge(e) get_vertex(e,1)
#define largest_vertex_in_edge(e) get_vertex(e,0)

/*:20*//*23:*/
#line 622 "./temp.w"

#define is_in_lower_Star(t,v) ((v) == id2tetrahedron(t)->vertex_ids[0])

Boolean edges_equal(simplex_id e0,simplex_id e1);
SInt32 min_value(simplex_id s,int d);


/*:23*//*30:*/
#line 765 "./temp.w"

Boolean edge_is_deadend(simplex_id*e);
Boolean edge_is_deadendx(simplex_id*e);
Boolean triangle_is_deadend(simplex_id*t);
Boolean tetrahedron_is_deadend(simplex_id t);
void make_edge_deadend(simplex_id*e);
void make_triangle_deadend(simplex_id*t);
void make_tetrahedron_deadend(simplex_id t);

#define edge_is_deadend(e) ((id2tetrahedron(*e)->types[(*e)&15] & 64)!=0)
#define edge_is_deadendx(e) edge_is_deadend(e)

/*:30*//*32:*/
#line 884 "./temp.w"

#define vertex_in_edge(v,e) (((v) == get_vertex(e,0))?0:(((v) == get_vertex(e,1))?1:-1))


/*:32*//*35:*/
#line 984 "./temp.w"

#define id_list_push(l,t) { simplex_id id_list_push_simplex =  t; \
       (void)list_push(l,&id_list_push_simplex); }
simplex_id id_list_pop(list*l);
simplex_id id_list_read(list*l);

/*:35*//*37:*/
#line 1019 "./temp.w"

#define vertex_is_critical(v)  ( (id2vertex(v)->type & 2)!=0 )
#define unmake_vertex_critical(v)  ( id2vertex(v)->type &=  0x81 )
#define tetrahedron_is_critical(t)  ( (id2tetrahedron(t)->types[0] & 2)!=0 )
#define unmake_tetrahedron_critical(t)  ( id2tetrahedron(t)->types[0] &=  0x81 )

Boolean edge_is_critical(simplex_id*e);
Boolean triangle_is_critical(simplex_id*t);
void make_vertex_critical(simplex_id v);
void make_edge_critical(simplex_id*e);
void make_triangle_critical(simplex_id*f);
void make_tetrahedron_critical(simplex_id t);
void unmake_triangle_critical(simplex_id*f);
void unmake_edge_critical(simplex_id*e);
void clean_crit(void);


/*:37*//*41:*/
#line 1203 "./temp.w"

#define make_edge_valid(e) (id2tetrahedron(e)->types[(e)&15] |=  128)
#define make_triangle_valid(f) (id2tetrahedron(f)->types[(f)&15] |=  128)
#define edge_is_valid(e)  ((id2tetrahedron(e)->types[(e)&15] & 128)!=0)
#define triangle_is_valid(f)  ((id2tetrahedron(f)->types[(f)&15] & 128)!=0)


/*:41*//*43:*/
#line 1257 "./temp.w"

simplex_id best_edge_idx(simplex_id e);
simplex_id best_face_id(simplex_id f);
#define best_edge_id(e) (edge_is_valid(e)?e:best_edge_idx(e))
#define best_face_id(f) (triangle_is_valid(f)?f:other_face_id(f))

/*:43*//*45:*/
#line 1292 "./temp.w"

#define vertex_is_paired_up(v)  ( (id2vertex(v)->type & 2)==0 )
#define tetrahedron_is_paired_down(t) ( (id2tetrahedron(t)->types[0] & 2)==0 )
#define r32(t) get_face(t,(id2tetrahedron(t)->types[0]>>3)&3)
#define r01(v) ( id2vertex(v)->tetra_id + ((id2vertex(v)->type>>3)&7)+5 )

#define edge_is_paired_downx(e) (edge_is_valid(e)?(id2tetrahedron(e)->types[(e)&15]&6)==4:eipdx(e))

Boolean edge_is_paired_up(simplex_id*e);
Boolean triangle_is_paired_up(simplex_id*f);
Boolean edge_is_paired_down(simplex_id*e);
Boolean triangle_is_paired_down(simplex_id*f);
simplex_id r21(simplex_id*f);
simplex_id r10(simplex_id*e);
simplex_id r23(simplex_id*f);
simplex_id r12(simplex_id*e);
void pair01(simplex_id s0,simplex_id s1,Boolean flag);
void pair12(simplex_id*s0,simplex_id s1,Boolean flag);
void pair23(simplex_id*s0,simplex_id s1,Boolean flag);

/*:45*//*70:*/
#line 1874 "./temp.w"

void read_in_complex(FILE*dfv,FILE*dft,double min,double max,Boolean qdelaunay,
double scalex,double scaley,double scalez,
Boolean optional_link_values);

/*:70*/
