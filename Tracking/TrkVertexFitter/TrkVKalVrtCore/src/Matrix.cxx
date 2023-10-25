/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkVKalVrtCore/Matrix.h"
#include "TrkVKalVrtCore/TrkVKalUtils.h"
#include <cmath>
#include <exception>
#include <iostream>
#include <vector>
#include <memory>

namespace{
/*
 * Internal helper methods.
 * Used only internally for the
 * implementation of the external
 * functions.
 * Internal linkage symbols
 * not exported
 */

using namespace Trk;
double vkPythag(double a, double b) {
  double absa, absb;
  absa = std::abs(a);
  absb = std::abs(b);
  if (absa > absb)
    return absa * std::sqrt(1.0 + (absb / absa) * absb / absa);
  return (absb == 0.0 ? 0.0
                      : absb * std::sqrt(1.0 + (absa / absb) * absa / absb));
}

/* SCALE G MATRIX SO THAT IT'S ELEMENTS ARE ABOUT 1. OF THE ORDER OF */
/* MAGNITUDE. IN THE EQUATION A*X=B CHANGE THE UNITS SO */
/* A(I,J) -> A(I,J)*C(I)*C(J), B(I) -> B(I)*C(I), X(I) -> X(I)/C(I) */
/* MFIRST >= N */
/* Author: V.Kostyukhin */
/*-----------------------------------------------------*/
void scaleg(double g[], double scale[], long int N, long int mfirst) noexcept {
  if (N == 1)
    scale[0] = 1.;
  if (N <= 1)
    return;
#define g_ref(a_1, a_2) g[(a_2) * (mfirst) + (a_1)]
  for (int i = 0; i < N; i++) {
    if (g_ref(i, i) == 0.) {
      scale[i] = 1.;
      continue;
    }
    scale[i] = 1. / std::sqrt(std::abs(g_ref(i, i)));
    g_ref(i, i) = d_sign(1., g_ref(i, i));
  }

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      if (j == i)
        continue;
      g_ref(i, j) *= scale[i] * scale[j];
    }
  }
#undef g_ref
}

double checkMatrixInversion(const double mtx[], const double invmtx[],
                            int DIM) {
  int i, j, k;

  double maxDiff = 0.;
  for (i = 0; i < DIM; i++) {
    for (j = i; j < DIM; j++) {
      double mcheck = 0.;
      for (k = 0; k < DIM; k++)
        mcheck += mtx[k >= i ? k * (k + 1) / 2 + i : i * (i + 1) / 2 + k] *
                  invmtx[k >= j ? k * (k + 1) / 2 + j : j * (j + 1) / 2 + k];
      // r[i][j]=mcheck;
      if (i != j)
        maxDiff = (maxDiff > std::abs(mcheck)) ? maxDiff : std::abs(mcheck);
      if (i == j)
        maxDiff =
            (maxDiff > std::abs(1. - mcheck)) ? maxDiff : std::abs(1. - mcheck);
    }
  }
  return maxDiff;
}

// Invert packed symmetric matrix using SVD
//
int vkSInvSVD(const double *ci, long int DIM, double *co, double Chk) {
  int i, j, k;
  noinit_vector<double *> a(DIM + 1);
  noinit_vector<double> ab((DIM + 1) * (DIM + 1));
  noinit_vector<double *> v(DIM + 1);
  noinit_vector<double> vb((DIM + 1) * (DIM + 1));
  noinit_vector<double> w(DIM + 1);
  for (i = 0; i < DIM + 1; i++) {
    a[i] = ab.data() + i * (DIM + 1);
    v[i] = vb.data() + i * (DIM + 1);
  }
  std::vector<std::vector<double>> res;
  res.resize(DIM + 1, std::vector<double>(DIM + 1, 0.));

  for (i = 1; i <= DIM; i++) {
    for (j = i; j <= DIM; j++) {
      k = (j - 1) * j / 2 + i;
      a[i][j] = a[j][i] = ci[k - 1];
    }
  }

  vkSVDCmp(a.data(), DIM, DIM, w.data(), v.data());

  // Singular value limitation
  double svMax = 0.;
  for (k = 1; k <= DIM; k++)
    if (svMax < w[k])
      svMax = w[k];
  for (k = 1; k <= DIM; k++) { /*std::cout<<w[k]<<'\n';*/
    if (w[k] < 0. || std::abs(w[k] / svMax) < Chk)
      w[k] = 0.;
  }
  // Get inverse matrix
  for (i = 1; i <= DIM; i++) {
    for (j = 1; j <= DIM; j++) {
      double t = 0.;
      for (k = 1; k <= DIM; k++) {
        if (w[k] != 0)
          t += a[i][k] * v[j][k] / w[k];
      }
      res[i][j] = t;
    }
  }
  //
  // Get output matrix in symmetric way
  k = 0;
  for (i = 1; i <= DIM; i++)
    for (j = 1; j <= i; j++) {
      co[k] = res[j][i];
      k++;
    }

  return 0;
}

int vkcholInv(const double ci[], double co[], long int DIM) {
  int i, j, k;
  std::vector<double> p(DIM, 0);
  std::vector<std::vector<double>> a;
  a.resize(DIM, std::vector<double>(DIM, 0.));

  for (i = 0; i < DIM; i++)
    for (j = i; j < DIM; j++) {
      k = j * (j + 1) / 2 + i;
      a[i][j] = a[j][i] = ci[k];
    }

  double sum = 0;
  for (i = 0; i < DIM; i++) {
    for (j = i; j < DIM; j++) {
      sum = a[i][j];
      for (k = i - 1; k >= 0; k--)
        sum -= a[i][k] * a[j][k];
      if (i == j) {
        if (sum <= 0.0)
          return -1;
        p[i] = std::sqrt(sum);
      } else
        a[j][i] = sum / p[i];
    }
  }

  for (i = 0; i < DIM; i++) {
    a[i][i] = 1.0 / p[i];
    for (j = i + 1; j < DIM; j++) {
      sum = 0.0;
      for (k = i; k < j; k++)
        sum -= a[j][k] * a[k][i];
      a[j][i] = sum / p[j];
    }
  }

  for (i = 0; i < DIM; i++) {
    for (j = i; j < DIM; j++) {
      int ind = j * (j + 1) / 2 + i;
      co[ind] = 0.;
      for (k = 0; k < DIM; k++) {
        if (k < i || k < j)
          continue;
        co[ind] += a[k][i] * a[k][j];
      }
    }
  }

  return 0;
}

// Invert square matrix using SVD + solve linear equation if needed
[[maybe_unused]]
int vkInvSVD(const double *ci, long int DIM, double *co, double Chk,
             double *bvect = nullptr) {
  int i, j, k;
  noinit_vector<double *> a(DIM + 1);
  noinit_vector<double> ab((DIM + 1) * (DIM + 1));
  noinit_vector<double *> v(DIM + 1);
  noinit_vector<double> vb((DIM + 1) * (DIM + 1));
  noinit_vector<double> w(DIM + 1);
  for (i = 0; i < DIM + 1; i++) {
    a[i] = ab.data() + i * (DIM + 1);
    v[i] = vb.data() + i * (DIM + 1);
  }
  std::vector<std::vector<double>> res;
  res.resize(DIM + 1, std::vector<double>(DIM + 1, 0.));

  for (i = 1; i <= DIM; i++)
    for (j = i; j <= DIM; j++)
      a[i][j] = ci[(j - 1) * DIM + i - 1];

  vkSVDCmp(a.data(), DIM, DIM, w.data(), v.data());

  // Singular value limitation
  double svMax = 0.;
  for (k = 1; k <= DIM; k++)
    if (svMax < w[k])
      svMax = w[k];
  for (k = 1; k <= DIM; k++) { /*std::cout<<w[k]<<'\n';*/
    if (w[k] < 0. || std::abs(w[k] / svMax) < Chk)
      w[k] = 0.;
  }
  // Get inverse matrix
  for (i = 1; i <= DIM; i++) {
    for (j = 1; j <= DIM; j++) {
      double t = 0.;
      for (k = 1; k <= DIM; k++) {
        if (w[k] != 0)
          t += a[i][k] * v[j][k] / w[k];
      }
      res[i][j] = t;
    }
  }
  //
  // Get output matrix
  for (i = 1; i <= DIM; i++)
    for (j = 1; j <= DIM; j++) {
      co[(j - 1) * DIM + i - 1] = res[i][j];
    }

  if (bvect) {
    std::vector<double> rside(DIM + 1, 0.);
    for (i = 1; i <= DIM; i++)
      rside[i] = bvect[i - 1];
    //----
    for (i = 1; i <= DIM; i++) {
      double sum = 0.;
      for (j = 1; j <= DIM; j++)
        sum += res[i][j] * rside[j];
      bvect[i - 1] = sum;
    }
  }

  return 0;
}

//  Solution of A*x = B via LU decomposition
//    A is DESTROYED!!!!
//   Solution X is returned in B
//-------------------------------------------------------------
int vkLUdcmp(double *a, long int n, int *indx) {
  int i, imax = 0, j, k;
  double big, dum, sum, temp;
  double dint = 1.0;
  double TINY = 1.e-30;
  // make indeces start from 1....
  a -= (n + 1);
  indx -= 1;

  std::vector<double> vv(n + 1, 0.);

  for (i = 1; i <= n; i++) {
    big = 0.0;
    for (j = 1; j <= n; j++) {
      if ((temp = std::abs(a[i * n + j])) > big)
        big = temp;
    }
    if (big == 0.0)
      return -1;
    vv[i] = 1.0 / big;
  }
  for (j = 1; j <= n; j++) {
    for (i = 1; i < j; i++) {
      sum = a[i * n + j];
      for (k = 1; k < i; k++)
        sum -= a[i * n + k] * a[k * n + j];
      a[i * n + j] = sum;
    }
    big = 0.0;
    for (i = j; i <= n; i++) {
      sum = a[i * n + j];
      for (k = 1; k < j; k++)
        sum -= a[i * n + k] * a[k * n + j];
      a[i * n + j] = sum;
      if ((dum = vv[i] * std::abs(sum)) >= big) {
        big = dum;
        imax = i;
      }
    }
    if (j != imax) {
      for (k = 1; k <= n; k++) {
        dum = a[imax * n + k];
        a[imax * n + k] = a[j * n + k];
        a[j * n + k] = dum;
      }
      dint = -dint;
      vv[imax] = vv[j];
    }
    indx[j] = imax;
    if (a[j * n + j] == 0.0)
      a[j * n + j] = TINY;
    if (j != n) {
      dum = 1.0 / (a[j * n + j]);
      for (i = j + 1; i <= n; i++)
        a[i * n + j] *= dum;
    }
  }
  return 0;
}
void vkLUbksb(const double *a, long int n, const int *indx, double *b) {
  int i, ii = 0, ip, j;
  double sum;
  // make indeces start from 1....
  a -= (n + 1);
  b -= 1;
  indx -= 1;

  for (i = 1; i <= n; i++) {
    ip = indx[i];
    sum = b[ip];
    b[ip] = b[i];
    if (ii)
      for (j = ii; j <= i - 1; j++)
        sum -= a[i * n + j] * b[j];
    else if (sum)
      ii = i;
    b[i] = sum;
  }
  for (i = n; i >= 1; i--) {
    sum = b[i];
    for (j = i + 1; j <= n; j++)
      sum -= a[i * n + j] * b[j];
    b[i] = sum / a[i * n + i];
  }
}

#define ROTATE(a, i, j, k, l)        \
  g = (a)[i][j];                     \
  h = (a)[k][l];                     \
  (a)[i][j] = g - s * (h + g * tau); \
  (a)[k][l] = h + s * (g - h * tau);

int vkjacobi(double **a, int n, double d[], double **v) {
  bool getEVect = true;
  if (v == nullptr)
    getEVect = false;
  int j, iq, ip, i;
  double tresh, theta, tau, t, sm, s, h, g, c;

  std::vector<double> b(n + 1, 0.);
  std::vector<double> z(n + 1, 0.);

  if (getEVect) {
    for (ip = 1; ip <= n; ip++) {
      for (iq = 1; iq <= n; iq++)
        v[ip][iq] = 0.0;
      v[ip][ip] = 1.0;
    }
  }
  for (ip = 1; ip <= n; ip++) {
    b[ip] = d[ip] = a[ip][ip];
  }
  int nrot = 0;
  for (i = 1; i <= 1000; i++) {
    sm = 0.0;
    for (ip = 1; ip <= n - 1; ip++) {
      for (iq = ip + 1; iq <= n; iq++)
        sm += std::abs(a[ip][iq]);
    }
    if (sm == 0.0)
      return nrot;
    if (i < 4)
      tresh = 0.2 * sm / (n * n);
    else
      tresh = 0.0;
    for (ip = 1; ip <= n - 1; ip++) {
      for (iq = ip + 1; iq <= n; iq++) {
        g = 100.0 * std::abs(a[ip][iq]);
        if (i > 4 && (std::abs(d[ip]) + g) == std::abs(d[ip]) &&
            (std::abs(d[iq]) + g) == std::abs(d[iq]))
          a[ip][iq] = 0.0;
        else if (std::abs(a[ip][iq]) > tresh) {
          h = d[iq] - d[ip];
          if ((std::abs(h) + g) == std::abs(h))
            t = (a[ip][iq]) / h;
          else {
            theta = 0.5 * h / (a[ip][iq]);
            t = 1.0 / (std::abs(theta) + std::sqrt(1.0 + theta * theta));
            if (theta < 0.0)
              t = -t;
          }
          c = 1.0 / std::sqrt(1 + t * t);
          s = t * c;
          tau = s / (1.0 + c);
          h = t * a[ip][iq];
          z[ip] -= h;
          z[iq] += h;
          d[ip] -= h;
          d[iq] += h;
          a[ip][iq] = 0.0;
          for (j = 1; j <= ip - 1; j++) {
            ROTATE(a, j, ip, j, iq)
          }
          for (j = ip + 1; j <= iq - 1; j++) {
            ROTATE(a, ip, j, j, iq)
          }
          for (j = iq + 1; j <= n; j++) {
            ROTATE(a, ip, j, iq, j)
          }
          if (getEVect) {
            for (j = 1; j <= n; j++) {
              ROTATE(v, j, ip, j, iq)
            }
          }
          ++nrot;
        }
      }
    }
    for (ip = 1; ip <= n; ip++) {
      b[ip] += z[ip];
      d[ip] = b[ip];
      z[ip] = 0.0;
    }
  }
  return 0;  // not enough sweeps
}
#undef ROTATE

// end of internal implementation
// methods
}  // namespace

namespace Trk {

double cfSmallEigenvalue(double *cov,long int n )
{
    if (n > 100 || n <= 1) return -1;
    double eig[100];
    vkGetEigVal(cov, eig, n);

    return eig[0];
}

int cfInv5(double *cov, double *wgt )
{
    double dest[25];
    long int  i, j, k, N;
/* -----------------------------------------*/
/*  Track covariance matrix inversion       */
/*   stable to small value of mom. error    */
/*   Author: V.Kostyukhin                   */
/* -----------------------------------------*/

/* -- RESTORE MATRIX 4x4 -- */
    N = 4;
    k = 0;
    for (i=0; i<N; ++i)  for (j=0; j<=i; ++j) {
          dest[ i*N + j ] = dest[ j*N + i ] = cov[k]; wgt[k]=0.; k++; }

    double X[4]={cov[10], cov[11], cov[12], cov[13]};

    for (i = 0; i < N; ++i) {
	for (j = 0; j < N; ++j) dest[i*N+j] -= X[i]*X[j]/cov[14];
    }
    long int jerr;
    dsinv(N, dest, N, &jerr);
    if(jerr) return jerr;

    double L[4]={0.,0.,0.,0.};
    for (i = 0; i < N; ++i) {
      for (j = 0; j < N; ++j) L[i] -= dest[i*N+j]*X[j]/cov[14];
    }
    double W=1.;  for (i = 0; i < N; ++i)  W -= X[i]*L[i];
    wgt[10]=L[0]; wgt[11]=L[1]; wgt[12]=L[2]; wgt[13]=L[3]; wgt[14] = W/cov[14];

    k=0;  for (i=0; i<N; i++) for (j=0; j<=i; j++) wgt[k++]=dest[i*N+j];

    return 0;
}


int cfdinv(double *cov, double *wgt, long int NI )
{
    long int n = std::abs(NI);
    if (n <= 1) return -1;
    long int  jerr=0;;

//---Robust inversion if COV is suspected ill-defined
     if (NI>0) {
       jerr=vkSInvSVD(cov,NI,wgt, 1.e-18);
       //--Check if inverted matrix is at approximately correct
       if(!jerr){  if(checkMatrixInversion(cov, wgt, n)>0.1) jerr=-2; }
       return jerr;
    }
//---If COV is believed good
    jerr=vkcholInv(cov, wgt, n);
    if(!jerr){  if(checkMatrixInversion(cov, wgt, n)>0.1) jerr=-2; }
    return jerr;
}


void  dsinv(long int n, double *a, long int DIM, long int *ifail) noexcept
{
    long int a_dim1, a_offset, i__1, i__2, i__3;
    long int i__, j, k, l;
    double s1, s31, s32, s33;
    long int jm1, jm2, jp1;


    /* Parameter adjustments */
    a_dim1 = DIM;
    a_offset = a_dim1 + 1;
    a -= a_offset;

    /* Function Body */
    jp1 = 0;
    *ifail = 0;
    i__1 = n;
    for (j = 1; j <= i__1; ++j) {
	if (a[j + j * a_dim1] <= 0.) {
	    goto L150;
	}
	a[j + j * a_dim1] = 1. / a[j + j * a_dim1];
	if (j == n) {
	    goto L199;
	}
	jp1 = j + 1;
	i__2 = n;
	for (l = jp1; l <= i__2; ++l) {
	    a[j + l * a_dim1] = a[j + j * a_dim1] * a[l + j * a_dim1];
	    s1 = -a[l + (j + 1) * a_dim1];
	    i__3 = j;
	    for (i__ = 1; i__ <= i__3; ++i__) {
		s1 = a[l + i__ * a_dim1] * a[i__ + (j + 1) * a_dim1] + s1;
	    }
	    a[l + (j + 1) * a_dim1] = -s1;
	}
    }
L150:
    *ifail = -1;
    return;
L199:

    if (n == 1) {
	goto L399;
    }
    a[(a_dim1 << 1) + 1] = -a[(a_dim1 << 1) + 1];
    a[a_dim1 + 2] = a[(a_dim1 << 1) + 1] * a[(a_dim1 << 1) + 2];
    if (n == 2) {
	goto L320;
    }
    i__1 = n;
    for (j = 3; j <= i__1; ++j) {
	jm2 = j - 2;
	i__2 = jm2;
	for (k = 1; k <= i__2; ++k) {
	    s31 = a[k + j * a_dim1];
	    i__3 = jm2;
	    for (i__ = k; i__ <= i__3; ++i__) {
		s31 = a[k + (i__ + 1) * a_dim1] * a[i__ + 1 + j * a_dim1] +
			s31;
	    }
	    a[k + j * a_dim1] = -s31;
	    a[j + k * a_dim1] = -s31 * a[j + j * a_dim1];
	}
	a[j - 1 + j * a_dim1] = -a[j - 1 + j * a_dim1];
	a[j + (j - 1) * a_dim1] = a[j - 1 + j * a_dim1] * a[j + j * a_dim1];
    }
L320:
    j = 1;
L323:
    s33 = a[j + j * a_dim1];
    if (j == n)	goto L325;
    jp1 = j + 1;
    i__1 = n;
    for (i__ = jp1; i__ <= i__1; ++i__) {
	s33 = a[j + i__ * a_dim1] * a[i__ + j * a_dim1] + s33;
    }
L325:
    a[j + j * a_dim1] = s33;
    jm1 = j;
    j = jp1;
    i__1 = jm1;
    for (k = 1; k <= i__1; ++k) {
	s32 = 0.;
	i__2 = n;
	for (i__ = j; i__ <= i__2; ++i__) {
	    s32 = a[k + i__ * a_dim1] * a[i__ + j * a_dim1] + s32;
	}
	a[k + j * a_dim1] = s32;
	a[j + k * a_dim1] = s32;
    }
    if (j < n)	goto L323;
L399:
    return;
}


//
//  Solve linear equation with LU decomposition.
//  Matrix (*a) left decomposed
//
int vkMSolve(double *a, double *b, long int n, double *ainv/*=nullptr*/)
{
   std::unique_ptr<int[]>  indx(new int[n+1]);
   std::unique_ptr<double[]> Scale(new double[n]);
   scaleg(a,Scale.get(),n,n);
   int ierr = vkLUdcmp( a, n, indx.get());
   if( ierr )return -1;
   for(int i=0;i<n;i++)b[i]*=Scale[i];
   vkLUbksb(a, n, indx.get(), b);
   for(int i=0;i<n;i++)b[i]*=Scale[i];
   if(ainv){  //Also invert matrix on request
     int i,j;
     std::unique_ptr<double[]> tmp(new double[n]);
     for(j=0; j<n; j++){
       for(i=0;i<n;i++)tmp[i]=0.;
       tmp[j]=1.;
       vkLUbksb(a, n, indx.get(), tmp.get());
       for(i=0;i<n;i++)ainv[i+j*n]=tmp[i];
     }
     for(i=0;i<n;i++) for(j=0;j<n;j++) ainv[i+j*n] *= Scale[i]*Scale[j];
   }
   return 0;
}


//
// SVD method
//

#define SIGN(a,b) ((b) >= 0.0 ? std::abs(a) : -std::abs(a))
void vkSVDCmp(double **a, int m, int n, double w[], double **v)
{
	int flag,i,its,j,jj,k,l=0, nm=0;
	double anorm,c,f,g,h,s,scale,x,y,z;

	std::vector<double> rv1(n+1,0.);
	g=scale=anorm=0.0;
	for (i=1;i<=n;i++) {
		l=i+1;
		rv1[i]=scale*g;
		g=s=scale=0.0;
		if (i <= m) {
			for (k=i;k<=m;k++) scale += std::abs(a[k][i]);
			if (scale) {
				for (k=i;k<=m;k++) {
					a[k][i] /= scale;
					s += a[k][i]*a[k][i];
				}
				f=a[i][i];
				g = -SIGN(std::sqrt(s),f);
				h=f*g-s;
				a[i][i]=f-g;
				for (j=l;j<=n;j++) {
					for (s=0.0,k=i;k<=m;k++) s += a[k][i]*a[k][j];
					f=s/h;
					for (k=i;k<=m;k++) a[k][j] += f*a[k][i];
				}
				for (k=i;k<=m;k++) a[k][i] *= scale;
			}
		}
		w[i]=scale *g;
		g=s=scale=0.0;
		if (i <= m && i != n) {
			for (k=l;k<=n;k++) scale += std::abs(a[i][k]);
			if (scale) {
				for (k=l;k<=n;k++) {
					a[i][k] /= scale;
					s += a[i][k]*a[i][k];
				}
				f=a[i][l];
				g = -SIGN(std::sqrt(s),f);
				h=f*g-s;
				a[i][l]=f-g;
				for (k=l;k<=n;k++) rv1[k]=a[i][k]/h;
				for (j=l;j<=m;j++) {
					for (s=0.0,k=l;k<=n;k++) s += a[j][k]*a[i][k];
					for (k=l;k<=n;k++) a[j][k] += s*rv1[k];
				}
				for (k=l;k<=n;k++) a[i][k] *= scale;
			}
		}
		double nanorm=std::abs(w[i])+std::abs(rv1[i]);
		if(anorm < nanorm) anorm = nanorm;
	}
	for (i=n;i>=1;i--) {
		if (i < n) {
			if (g) {
				for (j=l;j<=n;j++)
					v[j][i]=(a[i][j]/a[i][l])/g;
				for (j=l;j<=n;j++) {
					for (s=0.0,k=l;k<=n;k++) s += a[i][k]*v[k][j];
					for (k=l;k<=n;k++) v[k][j] += s*v[k][i];
				}
			}
			for (j=l;j<=n;j++) v[i][j]=v[j][i]=0.0;
		}
		v[i][i]=1.0;
		g=rv1[i];
		l=i;
	}
	for (i=(m<n?m:n);i>=1;i--) {
		l=i+1;
		g=w[i];
		for (j=l;j<=n;j++) a[i][j]=0.0;
		if (g) {
			g=1.0/g;
			for (j=l;j<=n;j++) {
				for (s=0.0,k=l;k<=m;k++) s += a[k][i]*a[k][j];
				f=(s/a[i][i])*g;
				for (k=i;k<=m;k++) a[k][j] += f*a[k][i];
			}
			for (j=i;j<=m;j++) a[j][i] *= g;
		} else for (j=i;j<=m;j++) a[j][i]=0.0;
		++a[i][i];
	}
	for (k=n;k>=1;k--) {
		for (its=1;its<=300;its++) {
			flag=1;
			for (l=k;l>=1;l--) {
				nm=l-1;
				if ( (std::abs(rv1[l])+anorm) == anorm) {
					flag=0;
					break;
				}
				if ( (std::abs(w[nm])+anorm) == anorm) break;
			}
			if (flag) {
				c=0.0;
				s=1.0;
				for (i=l;i<=k;i++) {
					f=s*rv1[i];
					rv1[i]=c*rv1[i];
					if ( (std::abs(f)+anorm) == anorm) break;
					g=w[i];
					h=vkPythag(f,g);
					w[i]=h;
					h=1.0/h;
					c=g*h;
					s = -f*h;
					for (j=1;j<=m;j++) {
						y=a[j][nm];
						z=a[j][i];
						a[j][nm]=y*c+z*s;
						a[j][i]=z*c-y*s;
					}
				}
			}
			z=w[k];
			if (l == k) {
				if (z < 0.0) {
					w[k] = -z;
					for (j=1;j<=n;j++) v[j][k] = -v[j][k];
				}
				break;
			}
			x=w[l];
			nm=k-1;
			y=w[nm];
			g=rv1[nm];
			h=rv1[k];
			f=((y-z)*(y+z)+(g-h)*(g+h))/(2.0*h*y);
			g=vkPythag(f,1.0);
			f=((x-z)*(x+z)+h*((y/(f+SIGN(g,f)))-h))/x;
			c=s=1.0;
			for (j=l;j<=nm;j++) {
				i=j+1;
				g=rv1[i];
				y=w[i];
				h=s*g;
				g=c*g;
				z=vkPythag(f,h);
				rv1[j]=z;
				c=f/z;
				s=h/z;
				f=x*c+g*s;
				g = g*c-x*s;
				h=y*s;
				y *= c;
				for (jj=1;jj<=n;jj++) {
					x=v[jj][j];
					z=v[jj][i];
					v[jj][j]=x*c+z*s;
					v[jj][i]=z*c-x*s;
				}
				z=vkPythag(f,h);
				w[j]=z;
				if (z) {
					z=1.0/z;
					c=f*z;
					s=h*z;
				}
				f=c*g+s*y;
				x=c*y-s*g;
				for (jj=1;jj<=m;jj++) {
					y=a[jj][j];
					z=a[jj][i];
					a[jj][j]=y*c+z*s;
					a[jj][i]=z*c-y*s;
				}
			}
			rv1[l]=0.0;
			rv1[k]=f;
			w[k]=x;
		}
	}
}

//-------------------------------------------
// ci - symmetric matrix in compact form
//-------------------------------------------
void vkGetEigVal(const double ci[], double d[], int n)
{
  int i,j,k; d--;
  noinit_vector<double*> a (n+1);
  noinit_vector<double> ab ((n+1)*(n+1));
  for(i=0; i<n+1; i++) a[i] = ab.data() + i*(n+1);

  for( i=1; i<=n; i++) {
    for( j=i; j<=n; j++) { k=(j-1)*j/2 + i; a[i][j]=a[j][i]=ci[k-1];}
  }

  vkjacobi(a.data(),n,d,nullptr);

  for (i=1;i<n;i++) {
     double p=d[k=i]; for (j=i+1;j<=n;j++) if (d[j] < p) p=d[k=j];
     if (k != i) { d[k]=d[i]; d[i]=p; }
  }
}

void vkGetEigVect(const double ci[], double d[], double vect[], int n)
{
  int i,j,k; d--;
  noinit_vector<double*> a (n+1);
  noinit_vector<double> ab ((n+1)*(n+1));
  noinit_vector<double*> v (n+1);
  noinit_vector<double> vb ((n+1)*(n+1));
  for(i=0; i<n+1; i++) a[i] = ab.data() + i*(n+1);
  for(i=0; i<n+1; i++) v[i] = vb.data() + i*(n+1);

  for( i=1; i<=n; i++) {
    for( j=i; j<=n; j++) { k=(j-1)*j/2 + i; a[i][j]=a[j][i]=ci[k-1];}
  }

  vkjacobi(a.data(),n,d,v.data());

  for (i=1;i<n;i++) {
     double p=d[k=i]; for (j=i+1;j<=n;j++) if (d[j] < p) p=d[k=j];
     if (k != i) { d[k]=d[i]; d[i]=p;
              for (j=1;j<=n;j++) { p=v[j][i]; v[j][i]=v[j][k]; v[j][k]=p;}
     }
  }

  k=0; for (i=1;i<=n;i++) for(j=1;j<=n;j++) vect[k++]=v[j][i];
}

}  //end of namespace Trk
