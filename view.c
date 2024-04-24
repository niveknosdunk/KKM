#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Morse.h"
#include "globals.h"

extern int global_persist;

struct tri_tree
{
	SInt32 count; 
	simplex_id id;
	simplex_id vid[3];
	GLfloat normal[3];
};

struct tree12_struct
{
	GLfloat p[3];
	GLfloat r[3];
};

struct link_struct
{
	simplex_id vid[3];
	GLfloat normal[3];
};

list *tree12 = NULL;
list *display_tree = NULL;
list *link_list = NULL;


int menu;
int disp01=0;
int disp12=0;
int disp23=0;
int dispd=0;
int displ=0;
int disp_option=0;
int link_depth_indices[6];
int link_depth;

int cancel01;
int cancel12;
int cancel23;


int 		winWidth, winHeight;

GLfloat posx,posy,posz;  // view origin
GLfloat headx=0.0,heady=0.0,headz=1.0;  // view heading
GLfloat upx=0.0,upy=1.0,upz=0.0;  // view up direction
GLfloat sidex=1.0,sidey=0.0,sidez=0.0;  // view side direction
GLfloat mins[3],maxs[3];

GLfloat scale;
GLfloat scalecrit0;
GLfloat scalecrit1;
GLfloat scalecrit2;
GLfloat scalecrit3;
GLfloat scalemove;
GLfloat scalemove1;
GLfloat scale_epsilon;

int rotating=0;
int startx,starty;

GLfloat angle = 0;  

simplex_id selected_simplex=NULL_ID;



void display_crit0(GLenum mode);
void display_crit1(GLenum mode);
void display_crit2(GLenum mode);
void display_crit3(GLenum mode);
void barycenter(simplex_id t,GLfloat c[3]);

void
makeunit(GLfloat *x,GLfloat *y,GLfloat *z)
{
	double r;
	
	r = sqrt((*x)*(*x)+(*y)*(*y)+(*z)*(*z));
	(*x)/=r;
	(*y)/=r;
	(*z)/=r;
}

void
makeunitv(GLfloat *w)
{
	double r;
	
	r = sqrt((w[0])*(w[0])+(w[1])*(w[1])+(w[2])*(w[2]));
	w[0]/=r;
	w[1]/=r;
	w[2]/=r;
}

double
dot(GLfloat x,GLfloat y,GLfloat z,GLfloat a,GLfloat b,GLfloat c)
{
	return x*a+y*b+z*c;
}

double
dotv(GLfloat *w,GLfloat *c)
{
	return w[0]*c[0]+w[1]*c[1]+w[2]*c[2];
}

void
negatev(GLfloat *w)
{
	w[0]=-w[0];
	w[1]=-w[1];
	w[2]=-w[2];
}

double
distance(GLfloat wx,GLfloat wy,GLfloat wz,GLfloat cx,GLfloat cy,GLfloat cz)
{
	return sqrt((wx-cx)*(wx-cx)+(wy-cy)*(wy-cy)+(wz-cz)*(wz-cz));
}

double
distancev(GLfloat *w,GLfloat *c)
{
	return sqrt((w[0]-c[0])*(w[0]-c[0])+(w[1]-c[1])*(w[1]-c[1])+(w[2]-c[2])*(w[2]-c[2]));
}

void
cross(GLfloat x,GLfloat y,GLfloat z,GLfloat a,GLfloat b,GLfloat c,GLfloat *ax,GLfloat *ay,GLfloat *az)
{
	*ax = y*c-z*b;
	*ay = z*a-x*c;
	*az = x*b-y*a;
}


/* after modifying head, call this to make sure that
    head, up, and side are orthonormal  */
void
changehead(void)
{
	double a;
	makeunit(&headx,&heady,&headz);
	a = dot(headx,heady,headz,upx,upy,upz);
	upx -= a*headx;
	upy -= a*heady;
	upz -= a*headz;
	if(fabs(upx)<.01&&fabs(upy)<.01&&fabs(upz)<.01)
	{
		if(fabs(headx)>.2)
		{
			upx = heady;
			upy = -headx;
			upz = 0;
		}
		else
		{
			upx = 0;
			upy = -headz;
			upz = heady;
		}
	}
	makeunit(&upx,&upy,&upz);
	cross(headx,heady,headz,upx,upy,upz,&sidex,&sidey,&sidez);
}




void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    winWidth = width;
    winHeight = height;
	glMatrixMode(GL_PROJECTION);     
	glLoadIdentity(); 
    gluPerspective(40.0, (float)width/(float)height, .01*scale,5.0*scale);
    glutPostRedisplay();
}

void	setup_model_view(void)
{
    /* Perform scene rotations based on user mouse input. */
	GLfloat m[16];
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); 
	glRotatef(57.3*angle, 0.0, 0.0, 1.0);
	m[0] = sidex;
	m[1] = upx;
	m[2] = headx;
	m[3] = 0.0;
	m[4] = sidey;
	m[5] = upy;
	m[6] = heady;
	m[7] = 0.0;
	m[8] = sidez;
	m[9] = upz;
	m[10] = headz;
	m[11] = 0.0;
	m[12] = 0.0;
	m[13] = 0.0;
	m[14] = 0.0;
	m[15] = 1.0;
	glMultMatrixf(m);
	glTranslatef(-posx, -posy, -posz); 
}   

void abandon_lists(void)
{
	if(display_tree!=NULL) list_abandon(&display_tree);
	if(tree12!=NULL) list_abandon(&tree12);
	if(link_list!=NULL) list_abandon(&link_list);

}

 


void do_select(int x,int y)
{
	GLuint selectBuf[1024],*p;
	GLint hits;
	GLint viewport[4]; 
	GLuint bestz;
	int i;
	int dim;
	
	glGetIntegerv (GL_VIEWPORT, viewport);
	glSelectBuffer (1024, selectBuf);
	glRenderMode (GL_SELECT);
	glInitNames();
	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
	glLoadIdentity(); 
	gluPickMatrix ((GLdouble) x, (GLdouble) (viewport[3] - y), 5.0, 5.0, viewport); 
    gluPerspective(40.0, (float)winWidth/(float)winHeight, .01*scale,5.0*scale);
	setup_model_view(); 

	glPushName(0); 
	glPushName(0); 
	display_crit0(GL_SELECT);
	glPopName(); 
	glLoadName(1); 
	glPushName(0); 
	display_crit1(GL_SELECT);
	glPopName(); 
	glLoadName(2); 
	glPushName(0); 
	display_crit2(GL_SELECT);
	glPopName(); 
	glLoadName(3); 
	glPushName(0); 
	display_crit3(GL_SELECT);
	glMatrixMode (GL_PROJECTION);
	glPopMatrix ();
	glMatrixMode(GL_MODELVIEW); 
	glFlush ();
	
	hits = glRenderMode (GL_RENDER); 
	if(hits<0) abort_message("hit buffer overflow");
	else 
	{
		p = selectBuf;
		bestz = 0xffffffff;
		selected_simplex = NULL_ID;
		for (i = 0; i < hits; i++)
		{
			if((*p++)!=2) abort_message("bad hit buffer");
			if(bestz>*p)
			{
				bestz = *p++;
				p++;
				dim = *p++;
				selected_simplex = *p++;
			}
			else p+=4;
		}
	}
	abandon_lists();
    glutPostRedisplay();
}



void
mouse(int button, int state, int x, int y)
{
  /* Rotate the scene with the left mouse button. */
  if (button == GLUT_LEFT_BUTTON) {
    if (state == GLUT_DOWN) {
      rotating = 1;
      startx = x;
      starty = y;
    }
    if (state == GLUT_UP) {
      rotating = 0;
    }
  }
  else if (button == GLUT_MIDDLE_BUTTON)
  {
	if (state == GLUT_DOWN) do_select(x,y);
  }
}

void
motion(int x, int y)
{
	double mx;
	double my;
	double vx,vy,wx,wy,lv,lw,area,t,c,s,a,b;
  if (rotating) {
	mx = winWidth/2;
	my = winHeight/2;
	vx = startx-mx;
	vy = starty-my;
	wx = x-mx;
	wy = y-my;
	lv = sqrt(vx*vx+vy*vy);
	lw = sqrt(wx*wx+wy*wy);
	area = (vx*wy-vy*wx);
	angle = angle - ((lv+lw)/(mx+my))*area/(1+lv*lw);
	t = (lv-lw)/(lv+lw+1);
	c = t*cos(angle);
	s = -t*sin(angle);
	b = (c*(vx+wx)+s*(vy+wy))/mx;
	a = (s*(vx+wx)-c*(vy+wy))/my;
	headx -= a*upx+b*sidex;
	heady -= a*upy+b*sidey;
	headz -= a*upz+b*sidez;
	changehead();
    startx = x;
    starty = y;
    glutPostRedisplay();
  }
}

void
keyboard(unsigned char c, int x, int y)
{
	double a,b,d;
	double nx,ny,nz;
	GLfloat sel_coords[3];
	
  switch (c) {
  case 'Q':  
  case 'q':
  case 27:  /* Escape quits. */
    exit(0);
    break;
  case 'v':
  case 'V':
    posx+=headx*scalemove;
    posy+=heady*scalemove;
    posz+=headz*scalemove;
	break;
  case ' ':
    posx-=headx*scalemove;
    posy-=heady*scalemove;
    posz-=headz*scalemove;
	break;
  case 'b':
  case 'B':
	b = cos(angle)*.01;
	a = -sin(angle)*.01;
	headx += a*upx+b*sidex;
	heady += a*upy+b*sidey;
	headz += a*upz+b*sidez;
	a*=scalemove1;
	b*=scalemove1;
	posx += a*upx+b*sidex;
	posy += a*upy+b*sidey;
	posz += a*upz+b*sidez;
	changehead();
	break;
  case 'c':
  case 'C':
	b = -cos(angle)*.01;
	a = sin(angle)*.01;
	headx += a*upx+b*sidex;
	heady += a*upy+b*sidey;
	headz += a*upz+b*sidez;
	a*=scalemove1;
	b*=scalemove1;
	posx += a*upx+b*sidex;
	posy += a*upy+b*sidey;
	posz += a*upz+b*sidez;
	changehead();
	break;
  case 'g':
  case 'G':
	if(!id_is_null(selected_simplex))
	{
		barycenter(selected_simplex,sel_coords);
		d = distance(posx,posy,posz,sel_coords[0],sel_coords[1],sel_coords[2])+scale_epsilon;
		b = cos(angle)*.01;
		a = -sin(angle)*.01;
		headx += a*upx+b*sidex;
		heady += a*upy+b*sidey;
		headz += a*upz+b*sidez;
		a*=d;
		b*=d;
		posx += a*upx+b*sidex;
		posy += a*upy+b*sidey;
		posz += a*upz+b*sidez;
		
		
		
		headx = .95*headx - .051*(sel_coords[0]-posx)/d;
		heady = .95*heady - .05*(sel_coords[1]-posy)/d;
		headz= .95*headz - .05*(sel_coords[2]-posz)/d;
		changehead();
	}
	break;
 case 'f':
  case 'F':
	if(!id_is_null(selected_simplex))
	{
		barycenter(selected_simplex,sel_coords);
		d = distance(posx,posy,posz,sel_coords[0],sel_coords[1],sel_coords[2])+scale_epsilon;
		b = -cos(angle)*.01;
		a = sin(angle)*.01;
		headx += a*upx+b*sidex;
		heady += a*upy+b*sidey;
		headz += a*upz+b*sidez;
		a*=d;
		b*=d;
		posx += a*upx+b*sidex;
		posy += a*upy+b*sidey;
		posz += a*upz+b*sidez;
		
		
		
		headx = .95*headx - .05*(sel_coords[0]-posx)/d;
		heady = .95*heady - .05*(sel_coords[1]-posy)/d;
		headz= .95*headz - .05*(sel_coords[2]-posz)/d;
		changehead();
	}
	break;
  case 'a':
    posx+=.1*scale;
	break;
  case 's':
    posx-=.1*scale;
	break;
  case 'w':
    posy+=.1*scale;
	break;
  case 'z':
    posy-=.1*scale;
	break;
  case 'e':
    posz+=.1*scale;
	break;
  case 'x':
    posz-=.1*scale;
	break;
  }
    glutPostRedisplay();
}

void DrawStr(const char *str)
{
	GLint i = 0;
	
	if(!str) return;
        
	while(str[i])
	{
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, str[i]);
		i++;
	}
}



void
display_crit0(GLenum mode)
{
	GLfloat mat_emmission[] = { 0.25, .0, .5, 0.0 }; 
	GLfloat mat_emmission_sel[] = { 0.5, .5, 1.0, 0.0 }; 
	GLfloat *p;
	simplex_id v;

	glColor3f (0.5, 0.0, 1.0);  
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission);

	list_read_init(crit[0]);
	while (!id_is_null(v=id_list_read(crit[0])))  // run through all critical vertices
	{
		if(v == selected_simplex)
		{
			glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission_sel);
		}
		glPushMatrix(); 
		p = id2vertex(v)->coords;     
		glTranslatef(p[0],p[1],p[2]);   
		if (mode == GL_SELECT)  glLoadName (v); 
		glutSolidSphere(scalecrit0, 8, 8); 
		glPopMatrix(); 	
		if(v == selected_simplex)
		{
			glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission);
		}
	}
}

void
display_crit1(GLenum mode)
{
	GLfloat mat_emmission[] = { 0.0, .5, .25, 0.0 }; 
	GLfloat mat_emmission_sel[] = { 0.5, 1.0, .5, 0.0 }; 
	GLfloat *p,*q;
	simplex_id e;

	glColor3f (0.0, 1.0, .5); 
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission);
	
	list_read_init(crit[1]);
	while (!id_is_null(e=id_list_read(crit[1])))  // run through all critical edges
	{
		if(e == selected_simplex)
		{
			glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission_sel);
		}
		glPushMatrix();  
		p = id2vertex(get_vertex(e,0))->coords;     
		q = id2vertex(get_vertex(e,1))->coords;     
		glTranslatef((p[0]+q[0])/2,(p[1]+q[1])/2,(p[2]+q[2])/2);   
		if (mode == GL_SELECT)  glLoadName (e); 
		glutSolidSphere(scalecrit1, 8, 8); 
		glPopMatrix(); 	
		glBegin(GL_LINES); 
		{
			glVertex3fv(p); 
			glVertex3fv(q); 
		}
		glEnd(); 
		if(e == selected_simplex)
		{
			glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission);
		}
	}
}

void
display_crit2(GLenum mode)
{
	GLfloat mat_emmission[] = { 0.5, .25, .0, 0.0 }; 
	GLfloat mat_emmission_sel[] = { 1.0, .5, .5, 0.0 }; 
	GLfloat *p,*q,*r,c[3];
	simplex_id f;
	simplex_id vl[3];

	glColor3f (1.0, .5, .0); 
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission);
	
	list_read_init(crit[2]);
	while (!id_is_null(f=id_list_read(crit[2])))  // run through all critical triangles
	{
		if(f == selected_simplex)
		{
			glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission_sel);
		}
		glPushMatrix(); 
		get_triangle_vertices(f,vl); 
		p = id2vertex(vl[0])->coords;     
		q = id2vertex(vl[1])->coords;     
		r = id2vertex(vl[2])->coords;  
		c[0] =  (p[0]+q[0]+r[0])/3;
		c[1] =  (p[1]+q[1]+r[1])/3;
		c[2] =  (p[2]+q[2]+r[2])/3;
		 
		glTranslatef(c[0],c[1],c[2]);  
		if (mode == GL_SELECT)  glLoadName (f); 
		glutSolidSphere(scalecrit2, 6, 6); 
		glPopMatrix(); 	
		glBegin(GL_LINES); 
		{
			glVertex3fv(c); 
			glVertex3f((p[0]+q[0])/2,(p[1]+q[1])/2,(p[2]+q[2])/2); 
			glVertex3fv(c); 
			glVertex3f((p[0]+r[0])/2,(p[1]+r[1])/2,(p[2]+r[2])/2); 
			glVertex3fv(c); 
			glVertex3f((r[0]+q[0])/2,(r[1]+q[1])/2,(r[2]+q[2])/2); 
		}
		glEnd(); 
		if(f == selected_simplex)
		{
			glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission);
		}
	}
}

void
display_crit3(GLenum mode)
{
	GLfloat mat_emmission[] = { 0.25, .0, .5, 0.0 }; 
	GLfloat mat_emmission_sel[] = { 0.5, .5, 1.0, 0.0 }; 
	GLfloat *p,*q,*r,*s,c[3];
	simplex_id t;
	simplex_id vl[4];

	glColor3f (1.0, .3, .2); 
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission);
	
	list_read_init(crit[3]);
	while (!id_is_null(t=id_list_read(crit[3])))  // run through all critical tetrahedra
	{
		if(t == selected_simplex)
		{
			glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission_sel);
		}
		glPushMatrix(); 
		get_tetrahedron_vertices(t,vl); 
		p = id2vertex(vl[0])->coords;     
		q = id2vertex(vl[1])->coords;     
		r = id2vertex(vl[2])->coords;  
		s = id2vertex(vl[3])->coords;  
		c[0] =  (p[0]+q[0]+r[0]+s[0])/4;
		c[1] =  (p[1]+q[1]+r[1]+s[1])/4;
		c[2] =  (p[2]+q[2]+r[2]+s[2])/4;
		 
		glTranslatef(c[0],c[1],c[2]);   
		if (mode == GL_SELECT)  glLoadName (t); 
		glutSolidSphere(scalecrit3, 4, 4); 
		glPopMatrix(); 	
		glBegin(GL_LINES); 
		{
			glVertex3fv(c); 
			glVertex3f((p[0]+q[0]+r[0])/3,(p[1]+q[1]+r[1])/3,(p[2]+q[2]+r[2])/3); 
			glVertex3fv(c); 
			glVertex3f((s[0]+q[0]+r[0])/3,(s[1]+q[1]+r[1])/3,(s[2]+q[2]+r[2])/3); 
			glVertex3fv(c); 
			glVertex3f((p[0]+s[0]+r[0])/3,(p[1]+s[1]+r[1])/3,(p[2]+s[2]+r[2])/3); 
			glVertex3fv(c); 
			glVertex3f((p[0]+q[0]+s[0])/3,(p[1]+q[1]+s[1])/3,(p[2]+q[2]+s[2])/3); 
		}
		glEnd(); 
		if(t == selected_simplex)
		{
			glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission);
		}
	}
}

void setup_tri_tree_entry(struct tri_tree *t,simplex_id f,int count)
{
	GLfloat *p[3];
	int i;

	t->id = f;
	get_triangle_vertices(f,t->vid);
	for(i=0;i<3;i++)
	{
		p[i] = id2vertex(t->vid[i])->coords;
	}
	cross(p[1][0]-p[0][0],p[1][1]-p[0][1],p[1][2]-p[0][2],p[2][0]-p[0][0],p[2][1]-p[0][1],p[2][2]-p[0][2],
				t->normal,t->normal+1,t->normal+2);
	makeunitv(t->normal);
	if(count<0)
	{
		t->count = -count;
		negatev(t->normal);
	}
	else t->count = count;
}

void setup_tri_tree(simplex_id f)
{
	hlist *edges;
	struct grad12_struct r;
	struct tri_tree tt;
	simplex_id vl[2];
	
	/* Note that f will not be displayed */
	
	hlist_initialize(&edges,sizeof(struct grad12_struct),113,2,0);

	if(disp_option) find_all_grad12_paths(f,edges,4+2);
	else find_all_grad12_paths(f,edges,4);

	list_initialize(&display_tree,sizeof(struct tri_tree));
	
	setup_tri_tree_entry(&tt,f,1);
	list_push(display_tree,&tt);
	
	while (hlist_get(edges,vl,&r))
	{
		if(edge_is_critical(&(r.e))) continue;
		if(edge_is_deadend(&(r.e))) continue;
		if(r.count==0) continue;
		setup_tri_tree_entry(&tt,r12(&(r.e)),r.count);
		list_push(display_tree,&tt);
	}
	hlist_abandon(&edges);
}


void display_descending_disc(simplex_id f)
{
	simplex_id e;
	long i;
	int flag;
	GLfloat *p;
	GLfloat mat_emmission[] = { 0.5, .5, .5, 0.0 }; 
	struct tri_tree *q;
	
	if(display_tree==NULL) setup_tri_tree(f);
	
	
	
	glBegin(GL_TRIANGLES);
	{
		for(i=0;i<list_count(display_tree);i++)
		{
			q = (struct tri_tree *)list_entry(display_tree,i);
			mat_emmission[0] =  .05*(q->count);
			glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission);
			glNormal3fv(q->normal);
			p = id2vertex(q->vid[0])->coords; 
			glVertex3fv(p);    
			p = id2vertex(q->vid[1])->coords; 
			glVertex3fv(p);    
			p = id2vertex(q->vid[2])->coords; 
			glVertex3fv(p);    
		}
	}
	glEnd(); 
}

void setup_tri_treex(simplex_id e)
{
	hlist *triangles;
	struct grad12_struct r;
	struct tri_tree tt;
	simplex_id f;
	simplex_id ep;
	simplex_id vl[3],vlp[2];
	GLfloat *p[3];
	long id,i;
	
	hlist_initialize(&triangles,sizeof(struct grad12_struct),113,1,0);

	if(disp_option) find_all_backward_grad12_paths(e,triangles,NULL,4+2);
	 find_all_backward_grad12_paths(e,triangles,NULL,4);

	list_initialize(&display_tree,sizeof(struct tri_tree));
	while (hlist_get(triangles,&f,&r))
	{
		if(triangle_is_xdeadend(&f)) continue;
		tt.id = f;
		tt.count = r.count;
		get_triangle_vertices(f,tt.vid);
		for(i=0;i<3;i++)
		{
			p[i] = id2vertex(tt.vid[i])->coords;
		}
		cross(p[1][0]-p[0][0],p[1][1]-p[0][1],p[1][2]-p[0][2],p[2][0]-p[0][0],p[2][1]-p[0][1],p[2][2]-p[0][2],
				tt.normal,tt.normal+1,tt.normal+2);
		makeunit(tt.normal,tt.normal+1,tt.normal+2);
		if(r.count<0)
		{
			tt.count = -r.count;
			for(i=0;i<3;i++)
			{
				tt.normal[i] = -tt.normal[i]; 
			}
		}
		list_push(display_tree,&tt);
	}
	hlist_abandon(&triangles);
}


void display_ascending_disc(simplex_id e)
{
	SInt32 i;
	int flag;
	GLfloat *p;
	GLfloat mat_emmission[] = { 0.5, .5, .5, 0.0 }; 
	struct tri_tree *q;
	
	if(display_tree==NULL) setup_tri_treex(e);
	
	
	
	glBegin(GL_TRIANGLES);
	{
		for(i=0;i<list_count(display_tree);i++)
		{
			q = (struct tri_tree *)list_entry(display_tree,i);
			mat_emmission[0] =  .05*(q->count);
			glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission);
			if(q->count != 0) glNormal3fv(q->normal);
			p = id2vertex(q->vid[0])->coords; 
			glVertex3fv(p);    
			p = id2vertex(q->vid[1])->coords; 
			glVertex3fv(p);    
			p = id2vertex(q->vid[2])->coords; 
			glVertex3fv(p);    
		}
	}
	glEnd(); 
}


void barycenter(simplex_id t,GLfloat c[3])
{
	simplex_id vl[4];
	GLfloat *p,*q,*r,*s;
	
	switch(id_dimension(t))
	{
		case 0:
			p = id2vertex(t)->coords;     
			c[0] =  p[0];
			c[1] =  p[1];
			c[2] =  p[2];
			break;
		case 1:
			get_edge_vertices(t,vl); 
			p = id2vertex(vl[0])->coords;     
			q = id2vertex(vl[1])->coords;     
			c[0] =  (p[0]+q[0])/2;
			c[1] =  (p[1]+q[1])/2;
			c[2] =  (p[2]+q[2])/2;
			break;
		case 2:
			get_triangle_vertices(t,vl); 
			p = id2vertex(vl[0])->coords;     
			q = id2vertex(vl[1])->coords;     
			r = id2vertex(vl[2])->coords;  
			c[0] =  (p[0]+q[0]+r[0])/3;
			c[1] =  (p[1]+q[1]+r[1])/3;
			c[2] =  (p[2]+q[2]+r[2])/3;
			break;
		case 3:
			get_tetrahedron_vertices(t,vl); 
			p = id2vertex(vl[0])->coords;     
			q = id2vertex(vl[1])->coords;     
			r = id2vertex(vl[2])->coords;  
			s = id2vertex(vl[3])->coords;  
			c[0] =  (p[0]+q[0]+r[0]+s[0])/4;
			c[1] =  (p[1]+q[1]+r[1]+s[1])/4;
			c[2] =  (p[2]+q[2]+r[2]+s[2])/4;
			break;
	}
}



void edge_barycenter(simplex_id e,GLfloat x[3])
{
	simplex_id vl[2];
	GLfloat *p,*q;
	
	get_edge_vertices(e,vl);
	p = id2vertex(vl[0])->coords;     
	q = id2vertex(vl[1])->coords;     
	x[0] = (p[0]+q[0])/2;
	x[1] = (p[1]+q[1])/2;
	x[2] = (p[2]+q[2])/2;
}


void add_paths(hlist *edges,SInt32 crit_id)
{
	list *todo;
	SInt32 next;
	int i;
	SInt32 id;
	simplex_id e,ep;
	simplex_id f;
	simplex_id vl[2];
	struct tree12_struct r;
	struct grad12_struct *p,*q;
	
	list_initialize(&todo,sizeof(SInt32));
	list_push(todo,&crit_id);
	
	while(!list_is_empty(todo))
	{
		list_pop(todo,&next);
		p = (struct grad12_struct *)hlist_entry(edges,next);
		
		i = p->flags&3;
		if(i==3) id = p->links[0];
		else id = p->links[i];
		
		e = p->e;
		get_edge_vertices(e,vl);
		edge_barycenter(e,r.p);
		
		while(id>=0)
		{
			q = (struct grad12_struct *)hlist_entry(edges,id);
			i = q->flags&3;
			ep = q->e;
			edge_barycenter(ep,r.r);
			list_push(tree12,&r);
			if((q->flags&64)==0)
			{
				list_push(todo,&id);
				q->flags|=64;
			}
			i = edge_verts_in_triangle(vl,r12(&ep));
			if(i<0) abort_message("error");
			id = q->links[i];
		}
	}
}


void setup_tree12(simplex_id f)
{
	hlist *edges;
	SInt32 crit_id;
	struct grad12_struct *p;
	
	hlist_initialize(&edges,sizeof(struct grad12_struct),113,2,0);
	list_initialize(&tree12,sizeof(struct tree12_struct));
	
	
	if(disp_option) crit_id = find_all_grad12_paths(f,edges,2);
	else  crit_id = find_all_grad12_paths(f,edges,0);
	
	while(crit_id>=0)
	{
		add_paths(edges,crit_id);
		p = (struct grad12_struct *)hlist_entry(edges,crit_id);
		crit_id = p->links[1];
	}
	hlist_abandon(&edges);
}

void display_grads_from_f(simplex_id f)
{
	struct tree12_struct *q;
	SInt32 i;
	
	if(tree12==NULL) setup_tree12(f);
	
	glBegin(GL_LINES);
	{
		for(i=0;i<list_count(tree12);i++)
		{
			q = (struct tree12_struct *)list_entry(tree12,i);
			glVertex3fv(q->p);    
			glVertex3fv(q->r);    
		}
	}
	glEnd(); 
}


void setup_tree12x(simplex_id e)
{
	hlist *triangles;
	list *crits;
	SInt32 start;
	struct grad12_struct rp;
	simplex_id ep;
	simplex_id f;
	long id;
	int k,j;
	struct tree12_struct r;
	
	hlist_initialize(&triangles,sizeof(struct grad12_struct),103,1,0);
	list_initialize(&tree12,sizeof(struct tree12_struct));
	
	start = find_all_backward_grad12_paths(e,triangles,NULL,2);
	
	while(hlist_get(triangles,&f,&rp))
	{
		if((rp.flags&4)==0) continue;
		k = rp.flags&3;
		if(k == 3) continue;
		ep = get_edge(f,k);
		edge_barycenter(ep,r.r);
		
		for(j=0;j<3;j++)
		{
			if(k==j || rp.links[j] < -1) continue;
			ep = get_edge(f,j);
			edge_barycenter(ep,r.p);
			list_push(tree12,&r);
		}
	}
	hlist_abandon(&triangles);
}

void display_grads_to_e(simplex_id e)
{
	struct tree12_struct *q;
	SInt32 i;
	
	if(tree12==NULL) setup_tree12x(e);
	
	glBegin(GL_LINES);
	{
		for(i=0;i<list_count(tree12);i++)
		{
			q = (struct tree12_struct *)list_entry(tree12,i);
			glVertex3fv(q->p);    
			glVertex3fv(q->r);    
		}
	}
	glEnd(); 
}

void display_grad_to_t(simplex_id tau,simplex_id f)
{
	simplex_id t;
	GLfloat *p,*q,*r,*s,c[3];
	simplex_id vl[4];
	
	glBegin(GL_LINE_STRIP); 
	{
		t=tau;
		get_triangle_vertices(f,vl);
		p = id2vertex(vl[0])->coords;     
		q = id2vertex(vl[1])->coords;     
		r = id2vertex(vl[2])->coords;  
		c[0] =  (p[0]+q[0]+r[0])/3;
		c[1] =  (p[1]+q[1]+r[1])/3;
		c[2] =  (p[2]+q[2]+r[2])/3;
		glVertex3fv(c);
		
		while( tetrahedron_is_in_K(t) && !tetrahedron_is_critical(t))
		{
			get_tetrahedron_vertices(t,vl);
			p = id2vertex(vl[0])->coords;     
			q = id2vertex(vl[1])->coords;     
			r = id2vertex(vl[2])->coords;  
			s = id2vertex(vl[3])->coords;  
			c[0] =  (p[0]+q[0]+r[0]+s[0])/4;
			c[1] =  (p[1]+q[1]+r[1]+s[1])/4;
			c[2] =  (p[2]+q[2]+r[2]+s[2])/4;
			glVertex3fv(c);
			f = r32(t);
			if (triangle_is_deadend(&f)) break;
			t = other_coface(f,t);
			if (tetrahedron_is_deadend(t)) break;
			get_triangle_vertices(f,vl);
			p = id2vertex(vl[0])->coords;     
			q = id2vertex(vl[1])->coords;     
			r = id2vertex(vl[2])->coords;  
			c[0] =  (p[0]+q[0]+r[0])/3;
			c[1] =  (p[1]+q[1]+r[1])/3;
			c[2] =  (p[2]+q[2]+r[2])/3;
			glVertex3fv(c);
		}
	}
	glEnd(); 
}


void display_grad_from_t(simplex_id t)
{
	simplex_id f;
	
	list_read_init(crit[2]);
	while (!id_is_null(f=id_list_read(crit[2])))  // run through all critical triangles
	{
		if(t==FindGrad23(coface(f,0),40000)) display_grad_to_t(coface(f,0),f);
		if(t==FindGrad23(coface(f,1),40000)) display_grad_to_t(coface(f,1),f);
	}
}


void display_grad_from_v(simplex_id u)
{
	simplex_id v;
	simplex_id e;
	
	glBegin(GL_LINE_STRIP); 
	{
		v=u;
		glVertex3fv(id2vertex(v)->coords);
		while(!vertex_is_critical(v))
		{
			e = r01(v);
			v = other_vertex_in_edge(v,e);
			glVertex3fv(id2vertex(v)->coords);
		}
	}
	glEnd(); 
}

void display_grad_to_v(simplex_id v)
{
	simplex_id e;
	
	list_read_init(crit[1]);
	while (!id_is_null(e=id_list_read(crit[1])))  // run through all critical edges
	{
		if(v==FindGrad01(get_vertex(e,0),-40000)) display_grad_from_v(get_vertex(e,0));
		if(v==FindGrad01(get_vertex(e,1),-40000)) display_grad_from_v(get_vertex(e,1));
	}
}


/* 
	the following assumes that the star of v in K
	is a union of tetrahedra.
*/
void setup_link(simplex_id v)
{
	list *l;
	hlist *edges_in_minus_link,*other_edges,*lone_vertices_in_minus_link;
	struct link_struct link;
	simplex_id t,f,vid[4],evid[2];
	GLfloat *p[3],*q,d;
	int depth=0,i;
	
	l = vertex_star(v);
	list_initialize(&link_list,sizeof(struct link_struct));
	
	hlist_initialize(&edges_in_minus_link,sizeof(int),113,2,0);
	hlist_initialize(&other_edges,sizeof(int),113,2,0);
	hlist_initialize(&lone_vertices_in_minus_link,sizeof(int),113,1,0);
	
	q = id2vertex(v)->coords;
	
	list_read_init(l);
	while(!id_is_null(t=id_list_read(l)))
	{
		if(!tetrahedron_is_in_K(t)) continue;
		get_tetrahedron_vertices(t,vid);
		while(v!=vid[depth]) 
		{
			link_depth_indices[depth] = list_count(link_list);
			depth++; 
			if(depth>=4) abort_message("internal setup_link error");
		}
		f = get_face(t,depth);
		get_triangle_vertices(f,link.vid);
		for(i=0;i<3;i++)
		{
			p[i] = id2vertex(link.vid[i])->coords;
		}
		cross(p[1][0]-p[0][0],p[1][1]-p[0][1],p[1][2]-p[0][2],p[2][0]-p[0][0],p[2][1]-p[0][1],p[2][2]-p[0][2],
				link.normal,link.normal+1,link.normal+2);
		makeunitv(link.normal);
		d = dot(link.normal[0],link.normal[1],link.normal[2],q[0]-p[0][0],q[1]-p[0][1],q[2]-p[0][2]);
		if(d>0) negatev(link.normal);

		list_push(link_list,&link);
		
		evid[0]=link.vid[0];
		evid[1]=link.vid[2];
		if(depth==0)
		{
			hlist_find_add(edges_in_minus_link,link.vid,&depth,NULL);
			hlist_find_add(edges_in_minus_link,evid,&depth,NULL);
		}
		else
		{
			hlist_find_add(other_edges,link.vid,&depth,NULL);
			hlist_find_add(other_edges,evid,&depth,NULL);
		}
		if(depth<2)
			hlist_find_add(edges_in_minus_link,link.vid+1,&depth,NULL);
		else
			hlist_find_add(other_edges,link.vid+1,&depth,NULL);
			
		if(depth==2)
			hlist_find_add(lone_vertices_in_minus_link,link.vid+2,&depth,NULL);
	}
	link_depth = depth;
	while(depth<4)
	{
		link_depth_indices[depth] = list_count(link_list);
		depth++;
	}
	
	link.vid[2]=NULL_ID;
	while(!hlist_is_empty(edges_in_minus_link))
	{
		hlist_get(edges_in_minus_link,link.vid,&depth);
		list_push(link_list,&link);
	}
	link_depth_indices[4] = list_count(link_list);
	while(!hlist_is_empty(other_edges))
	{
		hlist_get(other_edges,link.vid,&depth);
		list_push(link_list,&link);
	}
	link_depth_indices[5] = list_count(link_list);
	
	link.vid[1]=NULL_ID;
	while(!hlist_is_empty(lone_vertices_in_minus_link))
	{
		hlist_get(lone_vertices_in_minus_link,link.vid,&depth);
		list_push(link_list,&link);
	}

	list_abandon(&l);
	hlist_abandon(&edges_in_minus_link);
	hlist_abandon(&other_edges);
	hlist_abandon(&lone_vertices_in_minus_link);
}

void display_link(simplex_id v)
{
	struct link_struct *q;
	int i;
	GLfloat mat_emmission[] = { 0.5, .1, .1, 0.0 }; 
	
	if(link_list==NULL) setup_link(v);
	
	
	
	/* display depth 0 triangles in link */
	/* Note it acted funny sometimes if I did a glMaterialfv inside a glBegin(GL_TRIANGLES); loop
		so I put this outside */
	glColor3f (1.0, 0.0, 0.0);  
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission);

	
	glBegin(GL_TRIANGLES);
	{
		for(i=0;i<link_depth_indices[0];i++)
		{
			q = (struct link_struct *)list_entry(link_list,i);
			glNormal3fv(q->normal);
			glVertex3fv(id2vertex(q->vid[0])->coords);    
			glVertex3fv(id2vertex(q->vid[1])->coords);    
			glVertex3fv(id2vertex(q->vid[2])->coords);    
		}
	}
	glEnd(); 

	if(link_depth>0 && displ>0)
	{
		glColor3f (0.1, 0.2, 0.2);
		mat_emmission[0] = mat_emmission[1] = mat_emmission[2] = .1;
		glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission);
		
		glBegin(GL_TRIANGLES);
		{
			for(;i<link_depth_indices[1];i++)
			{
				q = (struct link_struct *)list_entry(link_list,i);
				glNormal3fv(q->normal);
				glVertex3fv(id2vertex(q->vid[0])->coords);    
				glVertex3fv(id2vertex(q->vid[1])->coords);    
				glVertex3fv(id2vertex(q->vid[2])->coords);    
			}
		}
		glEnd(); 
	}

	if(link_depth>1 && displ>0)
	{
		glColor3f (0.1, 0.1, 0.2);
		mat_emmission[0] = mat_emmission[1] = mat_emmission[2] = .1;
		glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission);
		glBegin(GL_TRIANGLES);
		{
			for(;i<link_depth_indices[2];i++)
			{
				q = (struct link_struct *)list_entry(link_list,i);
				glNormal3fv(q->normal);
				glVertex3fv(id2vertex(q->vid[0])->coords);    
				glVertex3fv(id2vertex(q->vid[1])->coords);    
				glVertex3fv(id2vertex(q->vid[2])->coords);    
			}
		}
		glEnd(); 
	}
	
	if(link_depth>2 && displ>0)
	{
		glColor3f (0.1, 0.1, 0.1);
		mat_emmission[0] = mat_emmission[1] = mat_emmission[2] = .1;
		glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission);
		glBegin(GL_TRIANGLES);
		{
			for(;i<link_depth_indices[3];i++)
			{
				q = (struct link_struct *)list_entry(link_list,i);
				glNormal3fv(q->normal);
				glVertex3fv(id2vertex(q->vid[0])->coords);    
				glVertex3fv(id2vertex(q->vid[1])->coords);    
				glVertex3fv(id2vertex(q->vid[2])->coords);    
			}
		}
		glEnd(); 
	}

	/* display edges in lower link */

	glColor3f (1.0, 0.0, 0.0);  
	mat_emmission[0] = 1.0;
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission);
	glLineWidth(3.0);
	glBegin(GL_LINES);
	{
		for(i=link_depth_indices[3];i < link_depth_indices[4];i++)
		{
			q = (struct link_struct *)list_entry(link_list,i);
			glVertex3fv(id2vertex(q->vid[0])->coords);    
			glVertex3fv(id2vertex(q->vid[1])->coords);    
		}
	}
	glEnd(); 
	glLineWidth(1.0);

	/* display edges not in lower link */

	if(displ>0)
	{
		glColor3f (0.2, 0.2, 0.2);
		mat_emmission[0] = mat_emmission[1] = mat_emmission[2] = .1;
		glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission);
		
		glLineWidth(2.0);
		glBegin(GL_LINES);
		{
			for(i=link_depth_indices[4];i < link_depth_indices[5];i++)
			{
				q = (struct link_struct *)list_entry(link_list,i);
				glVertex3fv(id2vertex(q->vid[0])->coords);    
				glVertex3fv(id2vertex(q->vid[1])->coords);    
			}
		}
		glEnd(); 
		glLineWidth(1.0);
	}
	
	/* display vertices in lower link */
	
	if(link_depth>1)
	{
		glPointSize(3.0);
		glBegin(GL_POINTS);
		{
			for(i=link_depth_indices[5];i < list_count(link_list);i++)
			{
				q = (struct link_struct *)list_entry(link_list,i);
				glVertex3fv(id2vertex(q->vid[0])->coords);    
			}
		}
		glEnd(); 
		glPointSize(1.0);
	}
	
}

void realgrad01(void)
{
	GLfloat 	index0[3]={1/sqrt(3),1/sqrt(3),1/sqrt(3)};  
	GLfloat 	index1[3]={-1/sqrt(3),1/sqrt(3),1/sqrt(3)};  
	GLfloat mat_emmission[] = { 0.5, .1, .1, 0.0 }; 

	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission);
	glColor3f (1.0, 0.1, 0.1);


	glBegin(GL_LINES);
	{
		glVertex3fv(index0);    
		glVertex3fv(index1); 
		index1[0] = 1/sqrt(3);   
		index1[1] = -1/sqrt(3);   
		glVertex3fv(index0);    
		glVertex3fv(index1);    
		index1[1] = 1/sqrt(3);   
		index1[2] = -1/sqrt(3);   
		glVertex3fv(index0);    
		glVertex3fv(index1);    
	}
	glEnd(); 
}

void realgrad23(void)
{
	GLfloat 	index3[3]={-1/sqrt(3),-1/sqrt(3),-1/sqrt(3)};  
	GLfloat 	index2[3]={1/sqrt(3),-1/sqrt(3),-1/sqrt(3)};  
	GLfloat mat_emmission[] = { 0.5, .1, .1, 0.0 }; 

	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission);
	glColor3f (1.0, 0.1, 0.1);
	glBegin(GL_LINES);
	{
		glVertex3fv(index2);    
		glVertex3fv(index3); 
		index2[0] = -1/sqrt(3);   
		index2[1] = 1/sqrt(3);   
		glVertex3fv(index2);    
		glVertex3fv(index3); 
		index2[1] = -1/sqrt(3);   
		index2[2] = 1/sqrt(3);   
		glVertex3fv(index2);    
		glVertex3fv(index3); 
	}
	glEnd(); 
}

void realgrad12(void)
{
	GLfloat 	index2a[3]={1/sqrt(3),-1/sqrt(3),-1/sqrt(3)};  
	GLfloat 	index2b[3]={-1/sqrt(3),1/sqrt(3),-1/sqrt(3)};  
	GLfloat 	index2c[3]={-1/sqrt(3),-1/sqrt(3),1/sqrt(3)};  
	GLfloat 	index1a[3]={-1/sqrt(3),1/sqrt(3),1/sqrt(3)}; 
	GLfloat 	index1b[3]={1/sqrt(3),-1/sqrt(3),1/sqrt(3)}; 
	GLfloat 	index1c[3]={1/sqrt(3),1/sqrt(3),-1/sqrt(3)}; 
	
	int i;
	 
	GLfloat mat_emmission[] = { 0.5, .1, .1, 0.0 }; 

	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission);
	glColor3f (1.0, 0.1, 0.1);
	glBegin(GL_LINES);
	{
		glVertex3fv(index2a);    
		glVertex3fv(index1b); 
		glVertex3fv(index2a);    
		glVertex3fv(index1c); 
		glVertex3fv(index2b);    
		glVertex3fv(index1a); 
		glVertex3fv(index2b);    
		glVertex3fv(index1c); 
		glVertex3fv(index2c);    
		glVertex3fv(index1a); 
		glVertex3fv(index2c);    
		glVertex3fv(index1b); 
	}
	glEnd(); 
}



void
display(void)
{
	int i;
	GLfloat c[3];
	vertex *vv;
	GLfloat mat_emmission[] = { 0.5, .5, .5, 0.0 }; 
	char num_str[128];
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 	
	glColor3f (1.0, 1.0, 1.0);  
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission);
	
	setup_model_view();

    glBegin(GL_POINTS); 
		for(i=0,vv=vertexlist;i<number_of_vertices;i++,vv++) 
		{
			if(vv->type&1)
				glVertex3fv(vv->coords); 
		}
	glEnd(); 
	
	display_crit0(GL_RENDER);
	display_crit1(GL_RENDER);
	display_crit2(GL_RENDER);
	display_crit3(GL_RENDER);

	if(disp01 && !id_is_null(selected_simplex))
	{
		glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission);
		if(id_dimension(selected_simplex)==0) 
		{
			display_grad_to_v(selected_simplex);
			if(disp_option) realgrad01();
		}
		else if(id_dimension(selected_simplex)==1) 
		{
			display_grad_from_v(get_vertex(selected_simplex,0));
			display_grad_from_v(get_vertex(selected_simplex,1));
			if(disp_option) realgrad01();
		}
	}
	
	if(disp12 && !id_is_null(selected_simplex))
	{
		glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission);
		if(id_dimension(selected_simplex)==2) 
		{
			display_grads_from_f(selected_simplex);
			if(disp_option) realgrad12();
		}
		else if(id_dimension(selected_simplex)==1) 
		{
			display_grads_to_e(selected_simplex);
			if(disp_option) realgrad12();
		}
	}
	
	
	if(disp23 && !id_is_null(selected_simplex))
	{
		glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission);
		if(id_dimension(selected_simplex)==3) 
		{
			display_grad_from_t(selected_simplex);
			if(disp_option) realgrad23();
		}
		else if(id_dimension(selected_simplex)==2) 
		{
			if(!id_is_null(FindGrad23(coface(selected_simplex,0),40000)))
				display_grad_to_t(coface(selected_simplex,0),selected_simplex);
			if(!id_is_null(FindGrad23(coface(selected_simplex,1),40000)))
				display_grad_to_t(coface(selected_simplex,1),selected_simplex);
			if(disp_option) realgrad23();
		}
	}
	
	if(dispd && !id_is_null(selected_simplex))
	{
		glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission);
		if(id_dimension(selected_simplex)==2) display_descending_disc(selected_simplex);
		else if(id_dimension(selected_simplex)==1) display_ascending_disc(selected_simplex);
	}
	
	if(displ && !id_is_null(selected_simplex))
	{
		glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission);
		if(id_dimension(selected_simplex)==0) display_link(selected_simplex);
		else if(id_dimension(selected_simplex)==1) display_link(get_vertex(selected_simplex,0));
		else  
		{
			simplex_id vid[4];
			
			if(id_dimension(selected_simplex)==2) get_triangle_vertices(selected_simplex,vid);
			else get_tetrahedron_vertices(selected_simplex,vid);
			display_link(vid[0]);
		}
	}
	
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glLoadIdentity();
		
	glMatrixMode (GL_PROJECTION);
	glPushMatrix();  
	glLoadIdentity();
	glOrtho(0, winWidth, 0, winHeight,-10.0,10.0);
	
	glRasterPos2f(5.0, 5.0);
	glColor3f(1.0, 1.0, 1.0);
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission);

	if(id_is_null(selected_simplex))
		sprintf(num_str, "%0.2f, (%f,%f,%f)", 57.3*angle, headx,heady,headz);
	else
	{
		barycenter(selected_simplex,c);
		sprintf(num_str,"(%f,%f,%f), value = %d",c[0],c[1],c[2],value(selected_simplex));
	}
	DrawStr(num_str);

	glPopMatrix(); 	
		
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	
	glFlush (); 
}

void 
init (void)  
{ 
	GLfloat light_position[] = { 0.0, 0.0, 0.0, 0.0 }; 
	GLfloat light_ambient[] = { .3, .3, .3, 0.0 }; 
	GLfloat mat_emmission[] = { 0.3, .3, .3, 0.0 }; 
	SInt32 i,j;
	vertex *v;
	Boolean first=true;
	GLfloat w,z;
	GLfloat *p,*q,c[3],a[3];
	simplex_id s;

	cancel01 = cancel12 = cancel23 = global_persist;
	
	for(i=0;i<number_of_vertices;i++)
	{
		v = vertexlist+i;
		if((v->type & 1)==0) continue;
		p = v->coords;
		for(j=0;j<3;j++)
		{
			if(first) 
			{
				mins[j]=maxs[j]=p[j];
			}
			else if(v->coords[j]<mins[j]) mins[j]=p[j];
			else if(v->coords[j]>maxs[j]) maxs[j]=p[j];
		}
		first=false;
	}
	
	w = maxs[0]-mins[0];
	if(w<maxs[1]-mins[1]) w = maxs[1]-mins[1];
	if(w<maxs[2]-mins[2]) w = maxs[2]-mins[2];
	
	for(j=0;j<3;j++)
	{
		mins[j] = (mins[j]+maxs[j]-w)/2;
		maxs[j] = mins[j]+w;
		light_position[j] = maxs[j]+w/3;
	}
	scale = w;
	scalemove1 = w*.3;
	scalemove = w*.01;
	scale_epsilon = w*.00001;
	
	p = id2vertex(*((simplex_id *)list_entry(crit[0],0)))->coords;
	posx = p[0]+5*scalemove*headx;
	posy = p[1]+5*scalemove*heady;
	posz = p[2]+5*scalemove*headz;
	
	changehead();
	
	list_read_init(crit[1]);
	while (!id_is_null(s=id_list_read(crit[1])))  // run through all critical edges
	{
		p = id2vertex(get_vertex(s,0))->coords;     
		q = id2vertex(get_vertex(s,1))->coords;   
		z = distancev(p,q)*.8;
		if(z<w) w=z;  
	}
	
	list_read_init(crit[2]);
	while (!id_is_null(s=id_list_read(crit[2])))  // run through all critical faces
	{
		barycenter(s,c); 
		for(i=0;i<3;i++)
		{
			barycenter(get_edge(s,i),a); 
			z = distancev(a,c)*2.4;
			if(z<w) w=z; 
		} 
	}
	
	list_read_init(crit[3]);
	while (!id_is_null(s=id_list_read(crit[3])))  // run through all critical tetra
	{
		barycenter(s,c); 
		for(i=0;i<4;i++)
		{
			barycenter(get_face(s,i),a); 
			z = distancev(a,c)*3.2;
			if(z<w) w=z; 
		} 
	}
	
	
	scalecrit0 = w;
	scalecrit1 = w/2;
	scalecrit2 = w/3;
	scalecrit3 = w/4;
	
	
	glClearColor(0.0, 0.0, 0.0, 0.0); 
	glLightfv(GL_LIGHT0, GL_POSITION, light_position); 
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient); 
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emmission);
   	glShadeModel (GL_SMOOTH);  	
	glEnable(GL_LIGHTING);    
	glEnable(GL_LIGHT0); 	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_FOG);   
	{       
		GLfloat fogColor[4] = {0.0, 0.0, 0.0, 1.0};       
		glFogi (GL_FOG_MODE, GL_LINEAR);       
		glFogfv (GL_FOG_COLOR, fogColor);       
		glFogf (GL_FOG_DENSITY, 1.5/scale);       
		glHint (GL_FOG_HINT, GL_DONT_CARE);       
		glFogf (GL_FOG_START, .20*scale);       
		glFogf (GL_FOG_END, 3*scale);   
	}  
	glutPostRedisplay();
} 


void 
menu_hit(int item)
{
	int i;
	long n[4];
	
	switch(item)
	{
		case 1:
			if(disp01) 
			{
				disp01=0;
				glutChangeToMenuEntry(1,"Display 0-1 gradient paths", 1); 
			}
			else
			{
				disp01=1;
				glutChangeToMenuEntry(1,"Hide 0-1 gradient paths", 1); 
			}
			glutPostRedisplay();
			break;
			
		case 2:
			if(disp12) 
			{
				disp12=0;
				glutChangeToMenuEntry(2,"Display 1-2 gradient paths", 2); 
			}
			else
			{
				disp12=1;
				glutChangeToMenuEntry(2,"Hide 1-2 gradient paths", 2); 
			}
			glutPostRedisplay();
			break;
			
		case 3:
			if(disp23) 
			{
				disp23=0;
				glutChangeToMenuEntry(3,"Display 2-3 gradient paths", 3); 
			}
			else
			{
				disp23=1;
				glutChangeToMenuEntry(3,"Hide 2-3 gradient paths",3); 
			}
			glutPostRedisplay();
			break;
			
		case 4:
			if(dispd) 
			{
				dispd=0;
				glutChangeToMenuEntry(4,"Display descending 2 discs", 4); 
			}
			else
			{
				dispd=1;
				glutChangeToMenuEntry(4,"Hide descending 2 discs", 4); 
			}
			glutPostRedisplay();
			break;

		case 5:
			if(displ>0) 
			{
				displ=0;
				glutChangeToMenuEntry(5,"Display link", 5); 
			}
			else if(displ<0) 
			{
				displ=1;
				glutChangeToMenuEntry(5,"Hide link", 5); 
				glutChangeToMenuEntry(6,"Display lower link", 6); 
			}
			else
			{
				displ=1;
				glutChangeToMenuEntry(5,"Hide link", 5); 
			}
			glutPostRedisplay();
			break;

		case 6:
			if(displ<0) 
			{
				displ=0;
				glutChangeToMenuEntry(6,"Display lower link", 6); 
			}
			else if(displ>0) 
			{
				displ=-1;
				glutChangeToMenuEntry(5,"Display link", 5); 
				glutChangeToMenuEntry(6,"Hide lower link", 6); 
			}
			else
			{
				displ=-1;
				glutChangeToMenuEntry(6,"Hide lower link", 6); 
			}
			glutPostRedisplay();
			break;
		case 7:
			cancel01*=2;
			ExtractCancel1(cancel01);
			clean_crit();
			for(i=0;i<4;i++) n[i]=list_count(crit[i]);
			printf("\n Number of critical simplices = %d, %d, %d, %d\n",n[0],n[1],n[2],n[3]);
			abandon_lists();
			glutPostRedisplay();
			break;
		case 8:
			cancel12*=2;
			ExtractCancel2(cancel12);
			clean_crit();
			for(i=0;i<4;i++) n[i]=list_count(crit[i]);
			printf("\n Number of critical simplices = %d, %d, %d, %d\n",n[0],n[1],n[2],n[3]);
			abandon_lists();
			glutPostRedisplay();
			break;
		case 9:
			cancel23*=2;
			ExtractCancel3(cancel23);
			clean_crit();
			for(i=0;i<4;i++) n[i]=list_count(crit[i]);
			printf("\n Number of critical simplices = %d, %d, %d, %d\n",n[0],n[1],n[2],n[3]);
			abandon_lists();
			glutPostRedisplay();
			break;
		case 10:
			test_homology(0xffff);
			break;
			
		case 11:
			if(disp_option) 
			{
				disp_option=0;
				glutChangeToMenuEntry(11,"set option", 11); 
				//glFogi (GL_FOG_MODE, GL_LINEAR);
			}
			else
			{
				disp_option=1;
				glutChangeToMenuEntry(11,"reset option", 11); 
				//glFogi (GL_FOG_MODE,GL_EXP);
			}
			glutPostRedisplay();
			break;
	}
}


void init_glut_window(char *vfilename)
{
	glutInitWindowSize(winWidth = 512, winHeight = 512);
	glutInitDisplayMode (GLUT_DEPTH | GLUT_SINGLE | GLUT_RGB);
	
	glutCreateWindow(vfilename);
	
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMotionFunc(motion);
    glutMouseFunc(mouse);
    glutReshapeFunc(reshape);
	
	menu = glutCreateMenu(menu_hit);
	glutAddMenuEntry("Display 0-1 gradient paths", 1);
	glutAddMenuEntry("Display 1-2 gradient paths", 2);
	glutAddMenuEntry("Display 2-3 gradient paths", 3);
	glutAddMenuEntry("Display descending 2 discs", 4);
	glutAddMenuEntry("Display link", 5);
	glutAddMenuEntry("Display lower link", 6);
	glutAddMenuEntry("Cancel 0-1", 7);
	glutAddMenuEntry("Cancel 1-2", 8);
	glutAddMenuEntry("Cancel 2-3", 9);
	glutAddMenuEntry("Compute Z/pZ Betti numbers", 10);
	glutAddMenuEntry("set option", 11);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	
	init();

}



void
begin_glut()
{	
	glutMainLoop();
}
