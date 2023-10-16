/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "HDF5Utils/Writer.h"

//-------------------------------------------------------------------------
// output data structure
struct out_t
{
  double dtype;
  float ftype;
  char ctype;
  short stype;
  int itype;
  long ltype;
  long long lltype;
  unsigned char uctype;
  unsigned short ustype;
  unsigned int uitype;
  unsigned long ultype;
  unsigned long long ulltype;
  bool btype;
};
using consumer_t = H5Utils::Consumers<const out_t&>;

consumer_t getConsumers() {
  consumer_t consumers;
  auto half = H5Utils::Compression::HALF_PRECISION;
  consumers.add("half" , [](const out_t& o) { return o.ftype; }, 0, half);
  consumers.add("dhalf", [](const out_t& o) { return o.dtype; }, 0, half);
#define ADD(NAME) consumers.add(#NAME, [](const out_t& o){ return o.NAME;}, 0)
  ADD(ftype);
  ADD(dtype);
  ADD(btype);
  ADD(ctype);
  ADD(stype);
  ADD(itype);
  ADD(ltype);
  ADD(lltype);
  ADD(uctype);
  ADD(ustype);
  ADD(uitype);
  ADD(ultype);
  ADD(ulltype);
#undef ADD
  return consumers;
}

//-------------------------------------------------------------------------
// outputs

std::vector<out_t> getOutputs(int offset, size_t length, float factor) {
  std::vector<out_t> outvec;
  for (size_t n = 0; n < length; n++) {
    out_t out;
    long long int shifted = n + offset;
    double factored = shifted*factor;
    out.dtype = factored;
    out.ftype = factored;
    out.ctype = shifted;
    out.stype = shifted;
    out.itype = shifted;
    out.ltype = shifted;
    out.lltype = shifted;
    out.uctype = shifted;
    out.ustype = shifted;
    out.uitype = shifted;
    out.ultype = shifted;
    out.ulltype = shifted;
    out.btype = n % 2;
    outvec.push_back(out);
  }
  return outvec;
}

template <size_t N>
auto nestOutputs(int offset, size_t length) {
  using ret_t = decltype(
    nestOutputs<N-1>(std::declval<int>(),std::declval<size_t>()));
  std::vector<ret_t> ret;
  for (size_t n = 0; n < length; n++) {
    ret.push_back(nestOutputs<N-1>(n + offset, length));
  }
  return ret;
}
template<>
auto nestOutputs<1>(int offset, size_t length) {
  return getOutputs(offset, length, 0.5);
}

//-------------------------------------------------------------------------
// main routine

int main(int, char*[]) {
  // make the output file
  H5::H5File out_file("output.h5", H5F_ACC_TRUNC);

  // scalar output
  using scalar_writer_t = H5Utils::Writer<0, consumer_t::input_type>;
  scalar_writer_t::configuration_type scalar_config;
  scalar_config.name = "scalar";
  consumer_t consumers = getConsumers();
  scalar_writer_t scalar(out_file, consumers, scalar_config);
  scalar.fill(getOutputs(1, 1, 0.5).at(0));

  // 1d output
  using d1_t = H5Utils::Writer<1, consumer_t::input_type>;
  d1_t::configuration_type d1_config;
  d1_config.name = "1d";
  d1_config.extent = {10};
  d1_config.chunks = {5};
  d1_t d1(out_file, consumers, d1_config);
  d1.fill(getOutputs(0, 10, 0.5));

  // 4d output
  using d4_t = H5Utils::Writer<4, consumer_t::input_type>;
  d4_t::configuration_type d4_config;
  d4_config.name = "4d";
  d4_config.extent = {2,3,4,5};
  d4_config.chunks = {1,2,1,2};
  d4_t d4(out_file, consumers, d4_config);
  d4.fill(nestOutputs<4>(0,3));

  return 0;
}

