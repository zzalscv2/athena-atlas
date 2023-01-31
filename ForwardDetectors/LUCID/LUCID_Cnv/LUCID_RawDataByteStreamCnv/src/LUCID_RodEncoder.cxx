/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "LUCID_RawDataByteStreamCnv/LUCID_RodEncoder.h"

LUCID_RodEncoder::LUCID_RodEncoder() {}

LUCID_RodEncoder::~LUCID_RodEncoder() {}

void
LUCID_RodEncoder::encode(std::vector<uint32_t>& data_block,
                         LUCID_RodEncoder::Cache& cache,
                         MsgStream& log) const
{

  VDIGIT::iterator digit_it = cache.Digits.begin();
  VDIGIT::iterator digit_it_end = cache.Digits.end();

  uint32_t data_word0 = 0;
  uint32_t data_word1 = 0;
  uint32_t data_word2 = 0;
  uint32_t data_word3 = 0;

  cache.hitcounter0 = 0;
  cache.hitcounter1 = 0;
  cache.hitcounter2 = 0;
  cache.hitcounter3 = 0;

  for (; digit_it != digit_it_end; ++digit_it) {

    unsigned short tubeID = (*digit_it)->getTubeID();
    bool isHit = (*digit_it)->isHit();

    if (tubeID < 16) {
      data_word0 |= (isHit << (tubeID - 0));
      cache.hitcounter0 += isHit;
    } else if (tubeID < 20) {
      data_word2 |= (isHit << (tubeID - 16));
      cache.hitcounter2 += isHit;
    } else if (tubeID < 36) {
      data_word1 |= (isHit << (tubeID - 20));
      cache.hitcounter1 += isHit;
    } else if (tubeID < 40) {
      data_word3 |= (isHit << (tubeID - 36));
      cache.hitcounter3 += isHit;
    } else {
      log << MSG::ERROR << " Unknown tubeID: " << tubeID << endmsg;
    }
  }

  data_word0 |= (cache.hitcounter0 << 24);
  data_word1 |= (cache.hitcounter1 << 24);
  data_word2 |= (cache.hitcounter2 << 24);
  data_word3 |= (cache.hitcounter3 << 24);

  data_block.push_back(data_word0);
  data_block.push_back(data_word1);
  data_block.push_back(data_word2);
  data_block.push_back(data_word3);

  cache.Digits.clear();
}
