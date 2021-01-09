#ifndef _NR_UTILS_H_
#define _NR_UTILS_H_
// Ingmar: I threw out a few unused things

#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

#if defined(__STDC__) || defined(ANSI) || defined(NRANSI) /* ANSI */

void nrerror(char error_text[]);

float *veCTor(long nl, long nh);
int *iveCTor(long nl, long nh);
unsigned char *cveCTor(long nl, long nh);
unsigned long *lveCTor(long nl, long nh);
double *dveCTor(long nl, long nh);
float **matrix(long nrl, long nrh, long ncl, long nch);
double **dmatrix(long nrl, long nrh, long ncl, long nch);
int **imatrix(long nrl, long nrh, long ncl, long nch);
float **submatrix(float **a, long oldrl, long oldrh, long oldcl, long oldch, long newrl, long newcl);
float **convert_matrix(float *a, long nrl, long nrh, long ncl, long nch);
float ***f3tensor(long nrl, long nrh, long ncl, long nch, long ndl, long ndh);
void free_veCTor(float *v, long nl, long nh);
void free_iveCTor(int *v, long nl, long nh);
void free_cveCTor(unsigned char *v, long nl, long nh);
void free_lveCTor(unsigned long *v, long nl, long nh);
void free_dveCTor(double *v, long nl, long nh);
void free_matrix(float **m, long nrl, long nrh, long ncl, long nch);
void free_dmatrix(double **m, long nrl, long nrh, long ncl, long nch);
void free_imatrix(int **m, long nrl, long nrh, long ncl, long nch);
void free_submatrix(float **b, long nrl, long nrh, long ncl, long nch);
void free_convert_matrix(float **b, long nrl, long nrh, long ncl, long nch);
void free_f3tensor(float ***t, long nrl, long nrh, long ncl, long nch, long ndl, long ndh);

#else /* ANSI */
/* traditional - K&R */

void nrerror();
float *veCTor();
float **matrix();
float **submatrix();
float **convert_matrix();
float ***f3tensor();
double *dveCTor();
double **dmatrix();
int *iveCTor();
int **imatrix();
unsigned char *cveCTor();
unsigned long *lveCTor();
void free_veCTor();
void free_dveCTor();
void free_iveCTor();
void free_cveCTor();
void free_lveCTor();
void free_matrix();
void free_submatrix();
void free_convert_matrix();
void free_dmatrix();
void free_imatrix();
void free_f3tensor();

#endif /* ANSI */

#endif /* _NR_UTILS_H_ */
