#include "Morse.h"
#include "globals.h"

void init_glut_window(char *vfilename);
void begin_glut(void);

int global_persist;

void abort_message(char*s)
{
	fprintf(stderr,"Error: %s",s);
	exit(1);
}


int main (int argc, char ** argv) {
	long i,n[4];
	char c;
	float min=0,max=1,persist=.1;
	float scalex=1.0,scaley=1.0,scalez=1.0;
	UInt32 int_persist;
	FILE *dfv=NULL,*dft=NULL;
	char *vfilename=NULL;
	Boolean qdelaunay=false;
	Boolean optional_link_values=false;
	unsigned long t0,t0p,t1,t1p,t2,t2p,t3,t3p;
	
	glutInit(&argc, argv);
	
	// arguments: -v vertexfile -t tetrahedronfile
	//            -l minimumvalue -h maximumvalue -p persistence
	
	while (--argc > 0) {
    	if ((**(++argv)=='-'))  c = (*argv)[1];
    	else c = **argv;
    	if(--argc == 0) break;
    	argv++;
    	switch(c)
    	{
    		case 'v':
    			if(dfv!=NULL) abort_message("double vertex file\n");
    			dfv = fopen(*argv,"r");
    			vfilename = *argv;
    			break;
    		case 't':
    			if(dft!=NULL) abort_message("double tetrahedron file\n");
    			dft = fopen(*argv,"r");
    			break;
    		case 'l':
    			i = sscanf(*argv,"%f",&min);
    			if(i!=1) abort_message("bad -l argument\n");
    			break;
    		case 'h':
    			i = sscanf(*argv,"%f",&max);
    			if(i!=1) abort_message("bad -h argument\n");
    			break;
    		case 'p':
    			i = sscanf(*argv,"%f",&persist);
    			if(i!=1) abort_message("bad -p argument\n");
    			break;
    		case 'q':
    			if(dft!=NULL) abort_message("double tetrahedron file\n");
    			dft = fopen(*argv,"r");
    			qdelaunay=true;
    			break;
    		case 'x':
    			i = sscanf(*argv,"%f",&scalex);
    			if(i!=1) abort_message("bad -x argument\n");
    			break;
    		case 'y':
    			i = sscanf(*argv,"%f",&scaley);
    			if(i!=1) abort_message("bad -y argument\n");
    			break;
    		case 'z':
    			i = sscanf(*argv,"%f",&scalez);
    			if(i!=1) abort_message("bad -z argument\n");
    			break;
    		case 'o':
    			optional_link_values = true;
    			break;
    	}
	}
	
	if(dfv==NULL) abort_message("bad vertex file\n");
	if(dft==NULL) {dft = stdin; qdelaunay=true;}
	
t0 = clock();
	
	read_in_complex(dfv,dft,min,max,qdelaunay,scalex,scaley,scalez,optional_link_values);
t0p = clock();
//	complex_check(-1);
	clean_crit();
	for(i=0;i<4;i++) n[i]=list_count(crit[i]);
	printf("\n Number of critical simplices = %d, %d, %d, %d\n",n[0],n[1],n[2],n[3]);
	
t1 = clock();
	global_persist = int_persist = persist*65535./(max-min);
//	ExtractCancel1(int_persist);
t1p = clock();
//	complex_check(-1);
//	clean_crit();
//	for(i=0;i<4;i++) n[i]=list_count(crit[i]);
//	printf("\n Number of critical simplices = %d, %d, %d, %d\n",n[0],n[1],n[2],n[3]);
	
t2 = clock();
//	ExtractCancel3(int_persist);
t2p = clock();
//	complex_check(-1);
//	clean_crit();
//	for(i=0;i<4;i++) n[i]=list_count(crit[i]);
//	printf("\n Number of critical simplices = %d, %d, %d, %d\n",n[0],n[1],n[2],n[3]);
	
	
t3 = clock();
//	ExtractCancel2(int_persist);
t3p = clock();
//	complex_check(-1);
//	clean_crit();
//	for(i=0;i<4;i++) n[i]=list_count(crit[i]);
//	printf("\n Number of critical simplices = %d, %d, %d, %d\n",n[0],n[1],n[2],n[3]);

//printf("\nExtractRaw = %d, Cancel1 = %d, Cancel2 = %d, Cancel3 = %d\n",t0p-t0,t1p-t1,t3p-t3,t2p-t2);
	
	init_glut_window(vfilename);
 	begin_glut(); 
    return 0;
}


