/* Gradient routine for rbf networks _with_ sample weightings
 * it supports the general form of rbf networks
 *   h^{[q](i)}=h^1_sigma(a,\sigma^{[q]})*
 *                 exp(norm(x^{(i)}-\mu^{[q]})^2/h^2_\sigma(\sigma^{[q]})) ;
 */ 

#include <math.h>
#include "mex.h"
#include <assert.h>
#include "matrix.h"
#include "rbfgrad_exp_w.h"

#ifndef MATLAB4
#define mexFuncConst const
#define Matrix mxArray
#else
#define mexFuncConst 
#define mxCreateDoubleMatrix mxCreateFull
#define mxREAL 0
#endif


void 
rbfgrad_exp_w(double *X, double *Y, double *O, double *mu, double *sigma, 
	  double *w, double *H, double *Inp, double *eta,
	  double *h1_sigma_sigma, double *h2_sigma_sigma, 
	  double *dif_h1_h1, double *dif_h2_h2, unsigned int a, 
	  unsigned int m, unsigned int l, 
	  double *C_mu, double *C_sigma, double *PatUE, double *PatLE)
{
  double *dif_oy_etha=(double *)calloc(l, sizeof(double)) ;
  double *H_h2_sigma=(double *)calloc(l, sizeof(double)) ;
  double *h_q__sigma_q=(double *)calloc(l, sizeof(double)) ;
  double *h_q__mu_q=(double *)calloc(l, sizeof(double)) ; 

  register unsigned int i, j, q, s ;
  for (i=0; i<l; i++)
    dif_oy_etha[i]=PatUE[i]-PatLE[i] ;
    /* now we have dif_oy_etha(i)= ...
       -(\eta_u^{(i)}*exp(  (f^{(i)}-{\hat y}^{(i)}) )-
         \eta_l^{(i)}*exp( -(f^{(i)}-{\hat y}^{(i)}) ) ) */

  /* loop for the centers*/
  for (q=0; q<m; q++)
  {
    double C__sigma_q ;

    /* compute as much as we can before entering a new for-level */
    for (i=0; i<l; i++)
    {
      H_h2_sigma[i]= 2*H[i+q*l] / h2_sigma_sigma[q] ;
      h_q__sigma_q[i]= (dif_h1_h1[q]+ h1_sigma_sigma[q]*Inp[i+q*l]*dif_h2_h2[q])*H[i+q*l] ;
    } ;

    /* gradient for \mu(:,q) */
    /*************************/
  
    /* loop for the input dimensions */
    for (s=0; s<a; s++)
    {
      double C__mu_q_s ;
	
      /* compute as much as we can before entering a new for-level */
      for (i=0; i<l; i++)
	h_q__mu_q[i]= (X[s+i*a]-mu[s+q*a])*H_h2_sigma[i] ;
        /* now we have h_q__mu_q(i)= ...
             2*(x_s^{(i)}-\mu_s^{[q]})*h^{[q](i)}/h^2_sigma(sigma^{[q]}) ;*/

      /* sum them up ... loop for the patterns */
      C__mu_q_s=0. ;
      for (i=0; i<l; i++)
	C__mu_q_s = C__mu_q_s + dif_oy_etha[i] * h_q__mu_q[i] ;
        /* now we have sum*w(q)= ...
	   \sum_{i=1}^l - dif_oy_etha(i) *...
                        \frac{\partial}{\partial \mu_s^{[q]}} f^{(i)} */
      C__mu_q_s *= w[q] ; 
      C_mu[s+q*a]=C__mu_q_s ;
        /* now we have \sum_{i=1}^l - dif_oy_etha(i) * ...
                        \frac{\partial}{\partial \mu_s^{[q]}} f^{(i)} */
    } ;
  
    /* gradient for \sigma(q) */
    /**************************/
    
    /* sum them up ... loop for the patterns */
    C__sigma_q=0. ;
    for (i=0; i<l; i++)
      C__sigma_q = C__sigma_q + dif_oy_etha[i] * h_q__sigma_q[i] ;
      /* now we have sum*w(q)= ...
           \sum_{i=1}^l -  dif_oy_etha(i) * \frac{\partial}{\partial \sigma^{[q]}} f^{(i)} */
    
    C__sigma_q *= w[q] ; 
  
    C_sigma[q]=C__sigma_q ;
    /* now we have C_sigma(q)=  ...
           \sum_{i=1}^l - dif_oy_etha(i) * \frac{\partial}{\partial \sigma^{[q]}} f^{(i)} */
  } ;
  free(dif_oy_etha) ;
  free(H_h2_sigma) ;
  free(h_q__sigma_q) ;
  free(h_q__mu_q) ;
} 


void
mexFunction(
    int nlhs,
    Matrix *plhs[],
    int nrhs,
    mexFuncConst Matrix *prhs[])
{
  double *X, *Y, *O, *mu, *sigma, *w, *H, *Inp, *eta,
    *h1_sigma_sigma, *h2_sigma_sigma, *dif_h1_h1, *dif_h2_h2 ;
  unsigned int a, b, m, l; 
  double *C_mu=NULL, *C_sigma=NULL, *PatUE, *PatLE ;

  if ((nrhs!=15) || (nlhs!=2))
  {
    mexErrMsgTxt("[C_mu, C_sigma]=rbfgrad_exp_w(X, Y, O, mu, sigma, w, H, Inp, eta, h1_sigma_sigma, h2_sigma_sigma, dif_h1_h1, dif_h2_h2, PatUE, PatLE)") ;
    return ;
  } ;

  X=mxGetPr(prhs[0]) ; assert(X!=0) ; 
  /*printf("(%i,%i)\n", mxGetM(prhs[0]), mxGetN(prhs[0])) ;*/
  Y=mxGetPr(prhs[1]) ; assert(Y!=0) ;
  /*printf("(%i,%i)\n", mxGetM(prhs[1]), mxGetN(prhs[1])) ;*/
  O=mxGetPr(prhs[2]) ; assert(O!=0) ;
  /*printf("(%i,%i)\n", mxGetM(prhs[2]), mxGetN(prhs[2])) ;*/
  mu=mxGetPr(prhs[3]) ; assert(mu!=0) ;
  /*printf("(%i,%i)\n", mxGetM(prhs[3]), mxGetN(prhs[3])) ;*/
  sigma=mxGetPr(prhs[4]) ; assert(sigma!=0) ;
  /*printf("(%i,%i)\n", mxGetM(prhs[4]), mxGetN(prhs[4])) ;*/
  w=mxGetPr(prhs[5]) ; assert(w!=0) ;
  /*printf("(%i,%i)\n", mxGetM(prhs[5]), mxGetN(prhs[5])) ;*/
  H=mxGetPr(prhs[6]) ; assert(H!=0) ;
  /*printf("(%i,%i)\n", mxGetM(prhs[6]), mxGetN(prhs[6])) ;*/
  Inp=mxGetPr(prhs[7]) ; assert(Inp!=0) ;
  /*printf("(%i,%i)\n", mxGetM(prhs[7]), mxGetN(prhs[7])) ;*/
  eta=mxGetPr(prhs[8]) ; assert(eta!=0) ;
  /*printf("(%i,%i)\n", mxGetM(prhs[8]), mxGetN(prhs[8])) ;*/
  h1_sigma_sigma=mxGetPr(prhs[9]) ; assert(h1_sigma_sigma!=0) ;
  /*printf("(%i,%i)\n", mxGetM(prhs[9]), mxGetN(prhs[9])) ;*/
  h2_sigma_sigma=mxGetPr(prhs[10]) ; assert(h2_sigma_sigma!=0) ;
  /*printf("(%i,%i)\n", mxGetM(prhs[10]), mxGetN(prhs[10])) ;*/
  dif_h1_h1=mxGetPr(prhs[11]) ; assert(dif_h1_h1!=0) ;
  /*printf("(%i,%i)\n", mxGetM(prhs[11]), mxGetN(prhs[11])) ;*/
  dif_h2_h2=mxGetPr(prhs[12]) ; assert(dif_h2_h2!=0) ;
  /*printf("(%i,%i)\n", mxGetM(prhs[12]), mxGetN(prhs[12])) ;*/
  PatUE=mxGetPr(prhs[13]) ; assert(PatUE) ;
  PatLE=mxGetPr(prhs[14]) ; assert(PatLE) ;
  /*printf("(%i,%i,%g,%g)\n", mxGetM(prhs[13]), mxGetN(prhs[13]),PatUE[0],PatUE[1]) ;*/
  /*printf("(%i,%i,%g,%g)\n", mxGetM(prhs[14]), mxGetN(prhs[14]), PatLE[0],PatLE[1]) ;*/

  a=mxGetM(prhs[0]) ;
  l=mxGetN(prhs[0]) ;
  m=mxGetM(prhs[5]) ;
  /*printf("%i, %i, %i", a,l,m) ;*/
  if ((mxGetN(prhs[1])!=l))
  {
    mexErrMsgTxt("Dimension error (arg 2).");
    return ;
  } ;
  if ((mxGetN(prhs[2])!=l) | (mxGetM(prhs[2])!=1))
  {
    mexErrMsgTxt("Dimension error (arg 3).");
    return ;
  } ;
  if ((mxGetM(prhs[3])!=a) | (mxGetN(prhs[3])!=m))
  {
    mexErrMsgTxt("Dimension error (arg 4).");
    return ;
  } ;
  if ((mxGetM(prhs[4])!=1) | (mxGetN(prhs[4])!=m))
  {
    mexErrMsgTxt("Dimension error (arg 5).");
    return ;
  } ;
  if (mxGetN(prhs[5])!=1)
  {
    mexErrMsgTxt("Dimension error (arg 6).");
    return ;
  } ;
  if ((mxGetM(prhs[6])!=l) | (mxGetN(prhs[6])!=m))
  {
    mexErrMsgTxt("Dimension error (arg 7).");
    return ;
  } ;
  if ((mxGetM(prhs[7])!=l) | (mxGetN(prhs[4])!=m))
  {
    mexErrMsgTxt("Dimension error (arg 8).");
    return ;
  } ;
  if ((mxGetM(prhs[8])!=2) & (mxGetN(prhs[8])!=2))
  {
    mexErrMsgTxt("Dimension error (arg 9).");
    return ;
  } ;
  if ((mxGetM(prhs[9])!=1) | (mxGetN(prhs[9])!=m))
  {
    mexErrMsgTxt("Dimension error (arg 10).");
    return ;
  } ;
  if ((mxGetM(prhs[10])!=1) | (mxGetN(prhs[10])!=m))
  {
    mexErrMsgTxt("Dimension error (arg 11).");
    return ;
  } ;
  if ((mxGetM(prhs[11])!=1) | (mxGetN(prhs[11])!=m))
  {
    mexErrMsgTxt("Dimension error (arg 12).");
    return ;
  } ;
  if ((mxGetM(prhs[12])!=1) | (mxGetN(prhs[12])!=m))
  {
    mexErrMsgTxt("Dimension error (arg 13).");
    return ;
  } ;

  plhs[0]=(mxArray*)mxCreateDoubleMatrix(a, m, mxREAL) ; 
  C_mu = mxGetPr(plhs[0]);
  plhs[1]=(mxArray*)mxCreateDoubleMatrix(1, m, mxREAL) ; 
  C_sigma = mxGetPr(plhs[1]);
    
  rbfgrad_exp_w(X, Y, O, mu, sigma, w, H, Inp, eta,
	        h1_sigma_sigma, h2_sigma_sigma, dif_h1_h1, 
	        dif_h2_h2, a, m, l, C_mu, C_sigma, PatUE, PatLE) ;
} 
               