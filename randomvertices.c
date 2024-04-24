#include <stdio.h>
#include <stdlib.h>

float randf(void)
{
	int i;
	float x;
	
	i = random()&(0x7fff);
	i -= 0x4000;
	x = i;
	x/= 0x4000;
	return x;
}


int main
(int argc,const char*argv[]){
	int i,nv,seed;
	float x,y,z,h;
	
	if(argc<2)
	{
		nv = 1000;
	}
	else
	{
		sscanf(argv[1],"%d",&nv);
	}
	
	if(argc<3)
	{
		srandom(clock());
	}
	else
	{
		sscanf(argv[2],"%d",&seed);
		srandom(seed);
	}
	

	for(i=0;i<nv;i++)
	{
		x = randf();
		y = randf();
		z = randf();
		// h = x*y*z*(1-x*x-y*y-z*z);
		h = x*x*x-x+y*y*y-y+z*z*z-z;
		
		printf("%f %f %f %f\n",x,y,z,h);
	}


}