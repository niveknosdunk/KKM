#include <stdio.h>

// Newer version for compatibility with newer MorseExtract
//  The usual use will be to take a file vertex_file with one line for each
//    vertex of the following form:
//      x y z val
// where x, y, z are the coordinates of the vertex and val is the value of the function
//    at the point.  
//  If the vertex is not in the complex, then the line should start with the character 'x'.
//
//  After generating vertex_file, run the following:
//
//  enclose vertex_file
//
//  This will add four vertices to vertex_file which enclose the given vertices.
//  Next run this file and feed stdout to qdelaunay:
//
//  qhprep vertex_file | qdelaunay QJ -Fx i

int main (int argc, const char * argv[]) {
    
	int c,res;
	long n;
	float x,y,z,w;
	
	FILE *f;
	
	if(argc<2)
	{
		fprintf(stderr,"\nUsage: qhprep vertex_file | qdelaunay QJ -Fx i\n");
		exit(1);
	}
	
	f = fopen(argv[1],"r");
	
	for(n=0; ;n++)
	{
		c = fgetc(f);
		if(c!='x') ungetc(c,f);
		res = fscanf(f,"%f %f %f %f\n",&x,&y,&z,&w);
		if(res!=4) break;
	}

	printf("3\n%d\n",n);
	rewind(f);
	
	do
	{
		c = fgetc(f);
		if(c!='x') ungetc(c,f);
		res = fscanf(f,"%f %f %f %f\n",&x,&y,&z,&w);
		if(res!=4) break;
		printf("%f %f %f\n",x,y,z);
	} while(1);
	
    return 0;
}
