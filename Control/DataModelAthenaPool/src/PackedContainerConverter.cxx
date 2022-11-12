/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file DataModelAthenaPool/src/PackedContainerConverter.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Dec, 2014
 * @brief Allow converting std::vector to SG::PackedContainer.
 */


#include "DataModelAthenaPool/PackedContainerConverter.h"
#include "AthContainers/PackedContainer.h"
#include "RootConversions/TConverterRegistry.h"
#include "TMemberStreamer.h"
#include "TClass.h"
#include "TROOT.h"
#include <vector>
#include <string>
#include <cassert>


namespace DataModelAthenaPool {


/**
 * @brief Move from one vector to another.
 * @param src Source vector.  Destroyed by the move.
 * @param dst Destination vector.
 *
 * If the types of @c src and @c dst are the same, then just do a move.
 * Otherwise, copy.
 */
template <class T, class ALLOC>
void vectorMove (std::vector<T, ALLOC>& src, std::vector<T, ALLOC>& dst)
{
  dst.swap (src);
}
template <class T, class U, class ALLOC>
void vectorMove (std::vector<T, ALLOC>& src, std::vector<U, ALLOC>& dst)
{
  dst.assign (src.begin(), src.end());
}


/**
 * @brief Converter from @c std::vector<T,ALLOC> to @c SG::PackedContainer<U,ALLOC>.
 */
template <class T, class U=T, class ALLOC=std::allocator<T> >
class PackedContainerConverter
  : public TMemberStreamer
{
public:
  /**
   * @brief Constructor.
   * @param tname The name of the vector element type T.
   * @param allocName The name of the allocator class, or null
   *                  to assume std::allocator.
   */
  PackedContainerConverter (const char* tname,
                            const char* allocName = nullptr);


  /**
   * @brief Run the streamer.
   * @param b Buffer from which to read.
   * @param pmember Pointer to the object into which to read.
   * @param size Number of instances to read.
   */
  virtual void operator() (TBuffer& b, void* pmember, Int_t size=0);


private:
  /// Hold the source class.
  TClass* m_cl;
};


/**
 * @brief Constructor.
 * @param tname The name of the vector element type T.
 * @param allocName The name of the allocator class, or null
 *                  to assume std::allocator.
 */
template <class T, class U, class ALLOC>
PackedContainerConverter<T, U, ALLOC>::PackedContainerConverter (const char* tname,
                                                                 const char* allocName /* = nullptr*/)
{
  std::string vname = "vector<";
  vname += tname;
  if (allocName != nullptr) {
    vname += ",";
    vname += allocName;
  }
  if (vname[vname.size()-1] == '>')
    vname += ' ';
  vname += '>';
  m_cl = gROOT->GetClass (vname.c_str());
}


/**
 * @brief Run the streamer.
 * @param b Buffer from which to read.
 * @param pmember Pointer to the object into which to read.
 * @param size Number of instances to read.
 */
template <class T, class U, class ALLOC>
void PackedContainerConverter<T, U, ALLOC>::operator() (TBuffer& b,
                                                        void* pmember, 
                                                        Int_t size /*=0*/)
{
  // This only works for reading!
  assert (b.IsReading());

  // The transient object.
  SG::PackedContainer<U, ALLOC>* obj =
    reinterpret_cast<SG::PackedContainer<U, ALLOC>*> (pmember);

  // We'll read into this object.
  std::vector<T, ALLOC> tmp;

  while (size-- > 0) {
    // Read into tmp and move data to *obj.
    tmp.clear();
    m_cl->Streamer (&tmp, b);
    vectorMove (tmp, *obj);
    ++obj;
  }
}


/**
 * @brief Install converters for supported instantiations.
 */
void installPackedContainerConverters()
{
#define CONVERTER(SRC, DST)                                     \
  do {                                                          \
    TConverterRegistry::Instance()->AddStreamerConverter        \
      ("vector<" #SRC ">", \
       "SG::PackedContainer<" #DST ",allocator<" #DST "> >",       \
       new PackedContainerConverter<SRC, DST> ( #SRC));         \
  } while (0)

#define CONVERTER1(T) CONVERTER(T,T)

#define CONVERTER2(T, ALLOC)                                    \
  do {                                                          \
    TConverterRegistry::Instance()->AddStreamerConverter        \
      ("vector<" #T "," #ALLOC "<" #T "> >",                    \
       "SG::PackedContainer<" #T "," #ALLOC "<" #T "> >",       \
       new PackedContainerConverter<T, T, ALLOC<T> > ( #T, #ALLOC "<" #T ">" )); \
  } while (0)

  
  CONVERTER1(char);
  CONVERTER1(unsigned char);
  CONVERTER1(short);
  CONVERTER1(unsigned short);
  CONVERTER1(int);
  CONVERTER1(unsigned int);
  CONVERTER1(float);
  CONVERTER1(double);

  CONVERTER1(std::vector<char>);
  CONVERTER1(std::vector<unsigned char>);
  CONVERTER1(std::vector<short>);
  CONVERTER1(std::vector<unsigned short>);
  CONVERTER1(std::vector<int>);
  CONVERTER1(std::vector<unsigned int>);
  CONVERTER1(std::vector<float>);
  CONVERTER1(std::vector<double>);

  CONVERTER1(std::vector<std::vector<char> >);
  CONVERTER1(std::vector<std::vector<unsigned char> >);
  CONVERTER1(std::vector<std::vector<short> >);
  CONVERTER1(std::vector<std::vector<unsigned short> >);
  CONVERTER1(std::vector<std::vector<int> >);
  CONVERTER1(std::vector<std::vector<unsigned int> >);
  CONVERTER1(std::vector<std::vector<float> >);
  CONVERTER1(std::vector<std::vector<double> >);
}


} // namespace DataModelAthenaPool
