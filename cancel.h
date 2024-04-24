/*12:*/
#line 245 "./temp.w"

void LocalCancel(simplex_id v,UInt32 n);
void LocalCancel12(simplex_id v,simplex_id sigma,simplex_id sigmap,simplex_id tau);
void Cancel01(simplex_id v,int n,simplex_id kappa);
void Cancel12(simplex_id sigma,simplex_id kappa,list*grad_path,list*changed);
void Cancel23(simplex_id sigma,simplex_id taup,simplex_id tau);
void ExtractCancel1(SInt32 p);
void ExtractCancel2(SInt32 p);
void ExtractCancel3(SInt32 p);
simplex_id FindGrad01(simplex_id u,SInt32 m);
simplex_id FindGrad23(simplex_id tau,SInt32 m);
simplex_id FindGrad23orientation(simplex_id sigma,int*orientation);
simplex_id FindGradPaths12(simplex_id sigma,SInt32 p,list*grad_path,int flags);
Boolean splitrejoin(simplex_id t,list*changed);
SInt32 find_all_grad12_paths(simplex_id sigma,hlist*edges,int options);
SInt32 find_all_backward_grad12_paths(simplex_id tau,hlist*triangles,list*crits,int options);

/*:12*//*44:*/
#line 1075 "./temp.w"

struct grad12_struct
{
SInt32 links[3];
SInt32 fake;
SInt32 count;
SInt32 count2;
simplex_id e;
int flags;
};


/*:44*/
