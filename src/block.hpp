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
#ifndef BLOCK_H
#define BLOCK_H

#include <string>
#include "rkfunction.hpp"
#include "mpi.h"
#include "kokkosTypes.hpp"
#include "hdf5.h"

using namespace std;

class blockWriter {
  public:
    blockWriter();
    //blockWriter(struct inputConfig&, rk_func*);
    //cf.ioblock = new blockWriter(cf,f,"bslice","./bslice",true,20,start,size,stride);
    blockWriter(struct inputConfig&,rk_func*,string,string,bool,size_t,vector<size_t>,vector<size_t>,vector<size_t>);
    //void writeHDF(struct inputConfig&, rk_func*, int, double, double*, double*, string);
    void write(struct inputConfig cf, rk_func *f, int tdx, double time);
    ~blockWriter();
    size_t frq();

  private:
    //struct inputConfig& cf;
    //rk_func* f;
      
    vector<size_t> lStart;  // local starting index
    vector<size_t> lEnd;    // local ending cell index
    vector<size_t> lEndG;   // local ending grid index
    vector<size_t> lExt;    // local extent
    vector<size_t> lExtG;   // local grid extent
    vector<size_t> lOffset; // local offset
    vector<size_t> gStart;  // local starting index
    vector<size_t> gEnd;    // local ending index
    vector<size_t> gExt;    // global extent
    vector<size_t> gExtG;   // global grid extent
    vector<size_t> stride; // slice stride

    size_t freq;    // block write frequency
    
    size_t lElems;
    size_t lElemsG;

    MPI_Comm sliceComm;
    int myColor;

    vector<float> varData;
    vector<float> gridData;

    FS4DH gridH;
    FS4DH varH;
    FS4DH varxH;

    bool slicePresent;
    int offsetDelta;

    string path;
    string name;
    int pad;
    bool avg;

    template <typename S>
    void write_h5(hid_t, string, int, vector<size_t>, vector<size_t>, vector<size_t>, vector<S>&);

    template <typename T>
    //void dataPack(int, int, size_t*, size_t*, size_t*, size_t*, vector<T>&, FS4DH&, int, bool);
    void dataPack(int ndim, int ng, const vector<size_t>& start, const vector<size_t>& end, const vector<size_t>& extent, const vector<size_t>& stride, vector<T>& dest, const FS4DH& source, const int vn, const bool average);
    template <typename H> hid_t getH5Type();
    //template <> hid_t getH5Type<float>();
    //template <> hid_t getH5Type<double>();
    //template <> hid_t getH5Type<int>();
    
    template <typename T, typename C>
    void reverseArray(int, T*, C*);

    hid_t openHDF5ForWrite(MPI_Comm, MPI_Info, string);
    void close_h5(hid_t);
};

#endif
