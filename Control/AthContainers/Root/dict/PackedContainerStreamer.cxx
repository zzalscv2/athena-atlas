/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/Root/PackedContainerStreamer.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2014
 * @brief Streamer code for I/O with SG::PackedContainer.
 */


#include "AthContainers/PackedContainer.h"
#include "AthContainers/PackedConverter.h"
#include "AthContainers/ClassName.h"
#include "TClassStreamer.h"
#include "TMemberStreamer.h"
#include "TError.h"
#include "TROOT.h"
#include <cassert>


namespace SG {


/**
 * @brief Write a set of packed parameters to a buffer.
 * @param b Buffer to which to write.
 * @param param Parameters to write.
 */
inline
void writePackedParameters( TBuffer& b, const SG::PackedParameters& parms ) {
   b << parms.nbits();
   b << parms.flags();
   if (parms.isFloat()) {
      b << parms.nmantissa();
      if (parms.hasScale())
         b << parms.scale();
   }
}


/**
 * @brief Read a set of packed parameters from a buffer.
 * @param b Buffer from which to write.
 */
inline
SG::PackedParameters readPackedParameters( TBuffer& b ) {
   uint8_t nbits;
   b >> nbits;

   uint8_t flags;
   b >> flags;

   SG::PackedParameters parms (nbits, flags);
   if (parms.isFloat()) {
      uint8_t nmantissa;
      b >> nmantissa;
      parms.setNmantissa (nmantissa);
      if (parms.hasScale()) {
         float scale;
         b >> scale;
         parms.setScale (scale);
      }
   }
   return parms;
}


/**
 * @brief Streamer for reading/writing SG::PackedContainer instances.
 */
template <class T, class ALLOC>
class PackedContainerStreamer
  : public TClassStreamer
{
public:
  /**
   * @brief Constructor.
   */
  PackedContainerStreamer();


  /**
   * @brief Run the streamer.
   * @param b Buffer from/to which to read/write.
   * @param objp Object instance.
   */
  virtual void operator()(TBuffer &b, void *objp) override;


  /**
   * @brief Clone operation, required for MT.
   */
  virtual TClassStreamer* Generate() const override;


private:
  /// Name of the class we read/write (for error messages).
  std::string m_className;
};


/**
 * @brief constructor.
 */
template <class T, class ALLOC>
PackedContainerStreamer<T, ALLOC>::PackedContainerStreamer()
  : m_className( ClassName< SG::PackedContainer< T, ALLOC > >::name() )
{
}


/**
 * @brief Run the streamer.
 * @param b Buffer from/to which to read/write.
 * @param objp Object instance.
 */
template <class T, class ALLOC>
void PackedContainerStreamer<T, ALLOC>::operator() ( TBuffer& b, void* objp ) {
   SG::PackedContainer<T, ALLOC>* cont =
     reinterpret_cast<SG::PackedContainer<T, ALLOC>*> (objp);

   if (b.IsReading()) {
      UInt_t R__s, R__c;
      Version_t R__v = b.ReadVersion(&R__s, &R__c);
      if (R__v != 1) {
         Error ("PackedContainerStreamer",
                "Bad version %d for object of type %s (expected 1)",
                R__v, m_className.c_str());
      }

      cont->setParms (readPackedParameters (b));

      uint32_t nelt;
      b >> nelt;

      SG::PackedConverter cnv (cont->parms());
      cnv.read (nelt, *cont, b);
    
      b.CheckByteCount(R__s, R__c, m_className.c_str());
   }
   else {
      UInt_t R__c = b.Length();
      b.SetBufferOffset (R__c + sizeof(UInt_t));
      b << Version_t(1);
      writePackedParameters (b, cont->parms());

      uint32_t nelt = cont->size();
      b << nelt;

      SG::PackedConverter cnv (cont->parms());
      cnv.write (cont->size(), *cont, b);

      b.SetByteCount(R__c, kTRUE);
   }
}


/**
 * @brief Clone operation, required for MT.
 */
template <class T, class ALLOC>
TClassStreamer* PackedContainerStreamer<T, ALLOC>::Generate() const
{
  return new PackedContainerStreamer<T, ALLOC> (*this);
}


template <class T, class ALLOC>
struct InstallPackedContainerStreamer;


} // namespace SG


#define STREAMER1(TYPE, ALLOC)                                          \
  namespace ROOT { TGenericClassInfo* GenerateInitInstance(const SG::PackedContainer<TYPE, ALLOC>*);} \
  namespace SG {                                                        \
  template <> struct InstallPackedContainerStreamer<TYPE, ALLOC> {      \
    InstallPackedContainerStreamer() {                                  \
      ROOT::GenerateInitInstance((SG::PackedContainer<TYPE, ALLOC>*)nullptr)->AdoptStreamer (new PackedContainerStreamer<TYPE, ALLOC>) ; \
    } };                                                                \
  InstallPackedContainerStreamer<TYPE, ALLOC> _R__UNIQUE_(streamerInstance); \
  } class swallowSemicolon

#define STREAMER(TYPE) STREAMER1(TYPE, std::allocator<TYPE>)


STREAMER(char);
STREAMER(unsigned char);
STREAMER(short);
STREAMER(unsigned short);
STREAMER(int);
STREAMER(unsigned int);
STREAMER(float);
STREAMER(double);

STREAMER(std::vector<char>);
STREAMER(std::vector<unsigned char>);
STREAMER(std::vector<short>);
STREAMER(std::vector<unsigned short>);
STREAMER(std::vector<int>);
STREAMER(std::vector<unsigned int>);
STREAMER(std::vector<float>);
STREAMER(std::vector<double>);

STREAMER(std::vector<std::vector<char> >);
STREAMER(std::vector<std::vector<unsigned char> >);
STREAMER(std::vector<std::vector<short> >);
STREAMER(std::vector<std::vector<unsigned short> >);
STREAMER(std::vector<std::vector<int> >);
STREAMER(std::vector<std::vector<unsigned int> >);
STREAMER(std::vector<std::vector<float> >);
STREAMER(std::vector<std::vector<double> >);

#undef STREAMER
