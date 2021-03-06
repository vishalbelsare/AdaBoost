#ifndef __RBFGRAD_H__
#define __RBFGRAD_H__

extern void 
rbfgrad(double *X, double *Y, double *O, double *mu, double *sigma, 
	double *w, double *H, double *Inp,
	double *h1_sigma_sigma, double *h2_sigma_sigma, 
	double *dif_h1_h1, double *dif_h2_h2, unsigned int a, 
	unsigned int b, unsigned int m, unsigned int l, 
	double *C_mu, double *C_sigma) ;

#endif 
