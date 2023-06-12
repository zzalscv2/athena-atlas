/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
// @file Base64Codec.cxx
// Header for base64 encoding/decoding functions
// @author Shaun Roe
// @date November 2019

#include "CoralBase/Blob.h"
#include "CxxUtils/base64.h"

#include <cstring> //memcpy



namespace IOVDbNamespace{
  std::string
  base64Encode(const coral::Blob & blob){
    //Blob::startingAddress returns a const void *, so cast to byte size
    const auto *const address = static_cast<const unsigned char *>(blob.startingAddress());
    const unsigned int nBytes = blob.size();
    return CxxUtils::base64_encode(address, nBytes);
  }

  coral::Blob
  base64Decode(const std::string & base64String){
    const auto &charVec = CxxUtils::base64_decode(base64String);
    coral::Blob blob(charVec.size());
    memcpy(blob.startingAddress(), charVec.data(), charVec.size());
    return blob;
  }
}
