/*5:*/
#line 143 "./temp.w"

#include "Morse.h"
int vtab[6][2]= {{0,1},{0,2},{0,3},{1,2},{1,3},{2,3}};
int etab[4][3]= {{10,9,8},{10,7,6},{9,7,5},{8,6,5}};
int etab2[4][4]= {{0,5,6,7},{5,0,8,9},{6,8,0,10},{7,9,10,0}};
int id_dim_tab[16]= {3,2,2,2,2,1,1,1,1,1,1,0,3,-2,-2,-1};
int id_val_tab[11]= {0,1,0,0,0,0,0,0,1,1,2};
int best_edge_tab[6]= {3,3,2,3,2,1};
UInt32 number_of_vertices,number_of_tetrahedra;
vertex*vertexlist;
tetrahedron*tlist;
list*crit[4];

/*:5*/
