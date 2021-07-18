/*
  Copyright 2019-2021 The University of New Mexico

  This file is part of FIESTA.
  
  FIESTA is free software: you can redistribute it and/or modify it under the
  terms of the GNU Lesser General Public License as published by the Free
  Software Foundation, either version 3 of the License, or (at your option) any
  later version.
  
  FIESTA is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
  details.
  
  You should have received a copy of the GNU Lesser General Public License
  along with FIESTA.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CEQ_HPP
#define CEQ_HPP
#include <cstdio>
struct maxCvar2D {
  FS4D dvar;
  int v;
  Kokkos::View<double *> cd;

  maxCvar2D(FS4D dvar_, int v_, Kokkos::View<double *> cd_)
      : dvar(dvar_), v(v_), cd(cd_) {}

  KOKKOS_INLINE_FUNCTION
  void operator()(const int i, const int j, double &lmax) const {
    int ns = (int)cd(0);
    int nv = ns + 3;

    double s = abs(dvar(i, j, 0, nv + v));

    if (s > lmax)
      lmax = s;
  }
};

struct maxGradRho2D {
  FS3D gradRho;
  int v;

  maxGradRho2D(FS3D gradRho_, int v_) : gradRho(gradRho_), v(v_) {}

  KOKKOS_INLINE_FUNCTION
  void operator()(const int i, const int j, double &lmax) const {

    double s = abs(gradRho(i, j, v));

    if (s > lmax)
      lmax = s;
  }
};

struct maxWaveSpeed2D {
  FS4D var;
  FS2D p;
  FS2D rho;
  Kokkos::View<double *> cd;

  maxWaveSpeed2D(FS4D var_, FS2D p_, FS2D rho_, Kokkos::View<double *> cd_)
      : var(var_), p(p_), rho(rho_), cd(cd_) {}

  KOKKOS_INLINE_FUNCTION
  void operator()(const int i, const int j, double &lmax) const {
    int ns = (int)cd(0);
    double gamma, gammas, Rs;
    double Cp = 0;
    double Cv = 0;

    double a, s;

    for (int s = 0; s < ns; ++s) {
      gammas = cd(6 + 2 * s);
      Rs = cd(6 + 2 * s + 1);

      Cp = Cp + (var(i,j,0,3+s) / rho(i,j)) * (gammas * Rs / (gammas - 1));
      Cv = Cv + (var(i,j,0,3+s) / rho(i,j)) * (Rs / (gammas - 1));
    }

    gamma = Cp / Cv;

    a = sqrt(gamma * p(i, j) / rho(i, j));
    s = a + sqrt(var(i, j, 0, 0) * var(i, j, 0, 0) +
                 var(i, j, 0, 1) * var(i, j, 0, 1));

    if (s > lmax)
      lmax = s;
  }
};

struct calculateRhoGrad2D {
  FS4D var;
  FS3D vel;
  FS2D rho;
  FS3D gradRho;
  double dx,dy;

  calculateRhoGrad2D(FS4D var_, FS3D vel_, FS2D rho_, FS3D gradRho_, double dx_, double dy_)
      : var(var_), vel(vel_), rho(rho_), gradRho(gradRho_), dx(dx_), dy(dy_) {}

  // central difference scheme for 1st derivative in 2d on three index variable 
  KOKKOS_INLINE_FUNCTION
  double deriv23d(const FS3D &var, const int i, const int j, const int ih, const int jh, const int v, const double dx) const {
    return (var(i-2*ih,j-2*jh,v) - 8.0*var(i-ih,j-jh,v) + 8.0*var(i+ih,j+jh,v) - var(i+2*ih,j+2*jh,v)) / (12.0*dx);
  }

  // central difference scheme for 1st derivative in 2d on two index variable 
  KOKKOS_INLINE_FUNCTION
  double deriv22d(const FS2D &var, const int i, const int j, const int ih, const int jh, const double dx) const {
    return (var(i-2*ih,j-2*jh) - 8.0*var(i-ih,j-jh) + 8.0*var(i+ih,j+jh) - var(i+2*ih,j+2*jh)) / (12.0*dx);
  }

  // central difference scheme for 1st derivative of specific internal energy
  KOKKOS_INLINE_FUNCTION
  double deriv2de(const FS4D &v, const FS3D &u, const FS2D &r,
      const int i, const int j, const int ih, const int jh, const double dx) const {
    return (nrg(v,u,r,i-2*ih,j-2*jh) - 8.0*nrg(v,u,r,i-ih,j-jh) + 8.0*nrg(v,u,r,i+ih,j+jh) - nrg(v,u,r,i+2*ih,j+2*jh)) / (12.0*dx);
  }

  // convert total energy to specific internal energy (TE-KE)/rho
  KOKKOS_INLINE_FUNCTION
  double nrg(const FS4D &v, const FS3D &u, const FS2D &r, const int i, const int j) const {
    return v(i,j,0,2)/r(i,j)-0.5*(u(i,j,0)*u(i,j,0)+u(i,j,1)*u(i,j,1));
  }

  KOKKOS_INLINE_FUNCTION
  void operator()(const int i, const int j) const {

    double i1 = 0;
    double i2 = 0;

    double dxr = deriv22d(rho,i,j,1,0,dx);
    double dyr = deriv22d(rho,i,j,0,1,dy);

    double dxe = deriv2de(var,vel,rho,i,j,1,0,dx);
    double dye = deriv2de(var,vel,rho,i,j,0,1,dy);

    double dxu = deriv23d(vel,i,j,1,0,0,dx);
    double dyv = deriv23d(vel,i,j,0,1,1,dy);

    double n1 = dxr;
    double n2 = dyr;

    double rgrad = sqrt(dxr * dxr + dyr * dyr);
    double divu = dxu + dyv;

    double dnednr = (n1 * dxe + n2 * dye) * (n1 * dxr + n2 * dyr);

    // compression switch
    if (dnednr < 0.0)
      i1 = 1.0;
    else
      i1 = 0.0;

    // detect shock front (equation 5a)
    if (divu < 0.0) {
      i2 = 1.0;
    }

    gradRho(i, j, 0) = (1 - i1) * i2 * rgrad;
    gradRho(i, j, 1) = i1 * rgrad;
    gradRho(i, j, 2) = -i1 * dyr;
    gradRho(i, j, 3) = i1 * dxr;
  }
};

struct updateCeq2D {
  FS4D dvar;
  FS4D var;
  FS3D gradRho;
  double maxS, kap, eps;
  Kokkos::View<double *> cd;

  updateCeq2D(FS4D dvar_, FS4D var_, FS3D gradRho_, double maxS_,
              Kokkos::View<double *> cd_, double kap_, double eps_)
      : dvar(dvar_), var(var_), gradRho(gradRho_), maxS(maxS_), cd(cd_),
        kap(kap_), eps(eps_) {}

  KOKKOS_INLINE_FUNCTION
  void operator()(const int i, const int j) const {
    double dx = cd(1);
    double dy = cd(2);

    // calculate cequation variable indices based on number of species (c
    // variables come after species densities)
    int nv = (int)cd(0) + 3;
    int nc = nv;

    double dx_right, dx_left, dy_top, dy_bot;
    double lap;
    //double normalizer=1.0;

    // average cell size
    double dxmag = sqrt(dx * dx + dy * dy);

    for (int n = 0; n < 4; ++n) {
      dx_right = (var(i + 1, j, 0, nc + n) - var(i, j, 0, nc + n)) / dx;
      dx_left = (var(i, j, 0, nc + n) - var(i - 1, j, 0, nc + n)) / dx;

      dy_top = (var(i, j + 1, 0, nc + n) - var(i, j, 0, nc + n)) / dy;
      dy_bot = (var(i, j, 0, nc + n) - var(i, j - 1, 0, nc + n)) / dy;

      // update ceq right hand side
      lap = dx_right - dx_left + dy_top - dy_bot;

      dvar(i,j,0,nc+n) =
          (maxS / (eps * dxmag)) * (gradRho(i,j,n) - var(i,j,0,nc+n)) + kap * maxS * dxmag * lap;
    }
  }
};

struct computeCeqFlux2D {
  FS4D var;
  FS5D m; // m(face,direction of derivative, i, j, velocity component)
  FS2D r;
  FS3D v;
  double alpha;
  int nv;

  computeCeqFlux2D(FS4D var_, FS5D m_, FS3D v_, FS2D r_, double a_, int nv_)
      : var(var_), m(m_), v(v_), r(r_), alpha(a_), nv(nv_) {}

  KOKKOS_INLINE_FUNCTION
  double interpolate4d(const FS4D &var, const int i, const int j, const int ih, const int jh, const int v) const {
    return (var(i,j,0,v)+var(i+ih,j+jh,0,v))/2.0;
  }

  KOKKOS_INLINE_FUNCTION
  double interpolate2d(const FS2D &var, const int i, const int j, const int ih, const int jh) const {
    return ( var(i,j)+var(i+ih,j+jh) )/2.0;
  }

  KOKKOS_INLINE_FUNCTION
  void operator()(const int i, const int j) const {
    int ih; // ihat - i-direction unit vector
    int jh; // jhat - j-direction unit vector

    // Compute alph*rho*C*Ctau1*Ctau2 on each face for each direction
    for (int w=0;w<2;++w){
      for (int f=0;f<2;++f){
        for (int d=0;d<2;++d){
          // set direction unit vector
          ih=0; jh=0;
          if (f==0) ih=1;
          if (f==1) jh=1;

          m(f,d,i,j,w) =  interpolate2d(r,i,j,ih,jh);
          m(f,d,i,j,w) += interpolate4d(var,i,j,ih,jh,nv+1);
          m(f,d,i,j,w) += interpolate4d(var,i,j,ih,jh,nv+2);
          m(f,d,i,j,w) += interpolate4d(var,i,j,ih,jh,nv+3);
          m(f,d,i,j,w) += interpolate4d(var,i,j,ih,jh,nv+4);
          //m(f,d,i,j,w) =(r(i,j)+r(i+ih,j+jh))/2.0;
          //m(f,d,i,j,w)+=(var(i,j,0,nv+1)+var(i+ih,j+jh,0,nv+1))/2.0;
          //m(f,d,i,j,w)+=(var(i,j,0,nv+2)+var(i+ih,j+jh,0,nv+2))/2.0;
          //m(f,d,i,j,w)+=(var(i,j,0,nv+3)+var(i+ih,j+jh,0,nv+3))/2.0;
          m(f,d,i,j,w)*=alpha;
        }
      }
    }
  }
};

struct computeCeqFaces2D {
  FS5D m; // m(face,direction of derivative, i, j, velocity component)
  FS3D v;
  FS1D cd;

  computeCeqFaces2D(FS5D m_, FS3D v_, FS1D cd_) : m(m_), v(v_), cd(cd_) {}

  KOKKOS_INLINE_FUNCTION
  void operator()(const int i, const int j) const {
    double dx = cd(1);
    double dy = cd(2);

    // Compute dux and duy on each face for each velocity
    for (int w=0;w<2;++w){
      m(0,0,i,j,w) *= ( v(i+1,j,w)-v(i,j,w) )/dx;
      m(0,1,i,j,w) *= ( (v(i,j+1,w)+v(i+1,j+1,w))-(v(i,j-1,w)+v(i+1,j-1,w)) )/(4.0*dx);
      m(1,1,i,j,w) *= ( v(i,j+1,w)-v(i,j,w) )/dy;
      m(1,0,i,j,w) *= ( (v(i+1,j,w)+v(i+1,j+1,w))-(v(i-1,j,w)+v(i-1,j+1,w)) )/(4.0*dx);
    }
  }
};

struct applyCeq2D {
  FS5D m;
  FS4D dvar,varx;
  double dx,dy;

  applyCeq2D(FS4D dvar_, FS4D varx_, FS5D m_, double dx_, double dy_)
    : dvar(dvar_), varx(varx_), m(m_), dx(dx_), dy(dy_) {}

  KOKKOS_INLINE_FUNCTION
  void operator()(const int i, const int j) const {
    double diffu;

    // assembly artificial viscosity term
    for (int w=0;w<2;++w){
      diffu=0;
      diffu += (m(0,0,i+1,j,w)-m(0,0,i,j,w))/dx;
      diffu += (m(0,1,i+1,j,w)-m(0,1,i,j,w))/dx;
      diffu += (m(1,0,i,j+1,w)-m(1,0,i,j,w))/dy;
      diffu += (m(1,1,i,j+1,w)-m(1,1,i,j,w))/dy;

      dvar(i,j,0,w)+=diffu;
      varx(i,j,0,5+w)=diffu;
    }
  }
};
#endif
