#include <stdio.h>

// Enclose a bunch of points with a tetrahedron
//  by adding four new vertices to a vertex file
// It also determines the maximum and minimum values of the vertices

int main (int argc, const char * argv[]) {
	FILE *f;
	long i,n,res;
	int c;
	float x,y,z,w;
	float minx,maxx,miny,maxy,minz,maxz,minw,maxw;
	
	if(argc<2)
	{
		fprintf(stderr,"\nUsage: enclose vertexfile\n");
		exit(1);
	}
	
	f = fopen(argv[1],"r+");
	
	for(n=0; ;n++)
	{
		c = fgetc(f);
		if(c!='x') ungetc(c,f);
		res = fscanf(f,"%f %f %f %f\n",&x,&y,&z,&w);
		if(res!=4) break;
		if(n==0)
		{
			minx=maxx=x;
			miny=maxy=y;
			minz=maxz=z;
			minw=maxw=w;
		}
		else
		{
			if(x<minx) minx=x;
			if(y<miny) miny=y;
			if(z<minz) minz=z;
			if(w<minw) minw=w;
			if(x>maxx) maxx=x;
			if(y>maxy) maxy=y;
			if(z>maxz) maxz=z;
			if(w>maxw) maxw=w;
		}
	} 
	if(res!=EOF)
	{
		fprintf(stderr,"\nError on line %li\n",n);
		exit(1);
	}
	printf("\n%f <= x <= %f, %f <= y <= %f, %f <= z <= %f",minx,maxx,miny,maxy,minz,maxz);

	minx = 1.01*minx-.01*maxx;
	maxx = 3*maxx-2*minx;
	miny = 1.01*miny-.01*maxy;
	maxy = 3*maxy-2*miny;
	minz = 1.01*minz-.01*maxz;
	maxz = 3*maxz-2*minz;
	fprintf(f,"x%f %f %f 0\n",minx,miny,minz);
	fprintf(f,"x%f %f %f 0\n",maxx,miny,minz);
	fprintf(f,"x%f %f %f 0\n",minx,maxy,minz);
	fprintf(f,"x%f %f %f 0\n",minx,miny,maxz);
	
	printf("\nmin = %f, max = %f\nTetrahedron:\n%li %li %li %li\n",minw,maxw,n,n+1,n+2,n+3);
	fclose(f);
	
return 0;

}