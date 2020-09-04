#include "fiesta.hpp"
#ifndef FST_H
#define FST_H

#include "input.hpp"
#include "writer.hpp"

class fstWriter : public writer {

public:
  fstWriter(struct inputConfig, rk_func *f);
  ~fstWriter();
  void writeGrid(struct inputConfig cf, const FS4D gridD, const char *fname);
  void writeSPGrid(struct inputConfig cf, const FS4D gridD, const char *fname);

  void writeSolution(struct inputConfig cf, rk_func *f, int tdx, double time);
  void writeRestart(struct inputConfig cf, rk_func *f, int tdx, double time);

  void readSolution(struct inputConfig cf, FS4D &deviceG, FS4D &deviceV);

  template<typename T>
  void writeHDF(struct inputConfig cf, rk_func *f, int tdx,
                              double time, T* x, T* var, string name);

private:
  double *xdp;
  double *ydp;
  double *zdp;
  float *xsp;
  float *ysp;
  float *zsp;

  float* psp;
  double* pdp;
  int* pi;

  double *vdp;
  float *vsp;
  double *readV;
  FS4DH gridH;
  FS4DH varH;
};

#endif
