/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// Implementation of RpcROD_Encoder class

#include "RpcROD_Encoder.h"

#include <algorithm>

#include "TrigT1RPChardware/MatrixReadOutStructure.h"
#include "TrigT1RPChardware/PadReadOutStructure.h"
#include "TrigT1RPChardware/RPCRODStructure.h"
#include "TrigT1RPChardware/RXReadOutStructure.h"

/** constructor
 */

RpcROD_Encoder::RpcROD_Encoder() : m_hid2re(nullptr) {}

/** destructor
 */
RpcROD_Encoder::~RpcROD_Encoder() = default;

/** add coincidence matrices
 */
void RpcROD_Encoder::set(const RPC_Hid2RESrcID* hid2re) {
    m_hid2re = hid2re;
}

/** add coincidence matrices
 */
void RpcROD_Encoder::add(const RpcPad* rc) {
    m_vRpcPad.push_back(rc);
}

/** clear vector
 */
void RpcROD_Encoder::clear() {
    m_vRpcPad.erase(m_vRpcPad.begin(), m_vRpcPad.end());
}

/** convert all RPC Pad in the current list to
  a vector of 32bit words
*/

void RpcROD_Encoder::fillROD(std::vector<uint32_t>& v) {
    if (m_vRpcPad.empty()) { return; }
    std::vector<unsigned short int> v16;

    RPCRODStructure rodReadout;
    RXReadOutStructure rxReadout;
    PadReadOutStructure padReadout;
    MatrixReadOutStructure matrixReadout;

    // first group the pads into their receivers

    typedef std::vector<const RpcPad*> receiver;
    std::map<int, receiver> mapReceiver;

    for (const auto& pPad : m_vRpcPad) {
        int sector = pPad->sector();
        mapReceiver[sector].push_back(pPad);
    }

    std::map<int, receiver>::iterator re = mapReceiver.begin();
    std::map<int, receiver>::iterator re_end = mapReceiver.end();

    // make the header of the ROD
    v.push_back(rodReadout.getHeaderMarker());
    v.push_back(rodReadout.getHeaderSize());
    v.push_back(rodReadout.getFormatVersion());
    uint16_t side = ((*re).first < 32) ? 0 : 1;
    uint16_t rodId = ((*re).first % 32) / 2;
    assert(rodId <= 15);

    v.push_back(rodReadout.getSourceID(side, rodId));
    v.push_back(0);  // Level1ID
    v.push_back(0);  // BunchCrossingID
    v.push_back(0);  // Level1TriggerType
    v.push_back(0);  // DetectorEventType

    // make the body of the ROD
    for (; re != re_end; ++re) {
        // make the header of the receiver
        uint16_t rxHeader[3];
        rxHeader[0] = 0;
        rxHeader[1] = ((*re).first % 32) % 2 << 3;
        rxHeader[2] = 0;

        uint16_t receiverHeader = rxReadout.makeHeader(rxHeader);

        v16.push_back(receiverHeader);

        // make the receiver body
        std::vector<const RpcPad*>::const_iterator it = (*re).second.begin();
        std::vector<const RpcPad*>::const_iterator it_end = (*re).second.end();

        for (; it != it_end; ++it) {
            const RpcPad* pad = (*it);

            // make the pad header
            unsigned short int padHeader[3];
            padHeader[0] = 0;
            padHeader[1] = pad->onlineId();
            padHeader[2] = pad->status();
            v16.push_back(padReadout.makeHeader(padHeader));
            // make the pad body:: cma header, sub-header, body, and footer
            RpcPad::const_iterator it_cma = pad->begin();
            RpcPad::const_iterator it_cma_end = pad->end();
            for (; it_cma != it_cma_end; ++it_cma) {
                const RpcCoinMatrix* cma = (*it_cma);
                unsigned short int cmaHeader[3];
                cmaHeader[0] = 0;
                cmaHeader[1] = cma->onlineId();
                cmaHeader[2] = cma->fel1Id();
                v16.push_back(matrixReadout.makeHeader(cmaHeader));
                v16.push_back(matrixReadout.makeSubHeader());
                RpcCoinMatrix::const_iterator it_fCh = cma->begin();
                RpcCoinMatrix::const_iterator it_fCh_end = cma->end();
                for (; it_fCh != it_fCh_end; ++it_fCh) {
                    const RpcFiredChannel* firedChannel = (*it_fCh);
                    unsigned short int body[5];
                    body[0] = 0;
                    body[1] = firedChannel->bcid();
                    body[2] = firedChannel->time();
                    body[3] = firedChannel->ijk();

                    if (body[3] < 7) {
                        body[4] = firedChannel->channel();
                    } else {
                        body[4] = firedChannel->thr() | ((firedChannel->ovl()) << 2);
                    }

                    v16.push_back(matrixReadout.makeBody(body));
                }
                v16.push_back(matrixReadout.makeFooter(cma->crc()));
            }

            // make the pad footer

            ubit16 errorCodes[6] = {0, 0, 0, 0, 0, 0};
            v16.push_back(padReadout.makeFooter(errorCodes));
        }
        // make the receiver footer
        v16.push_back(rxReadout.makeFooter((uint16_t)0));
    }  // end of receivers

    // if numbers of 16 words is odd, add 0 to make it even
    int n16words = v16.size();
    if (n16words % 2 == 1) {
        v16.push_back(0);
        n16words++;
    }
    packFragments(v16, v, n16words);

    // finally make the ROD footer
    v.push_back(0);  // NumberOfStatusElements
    v.push_back(0);  // NumberOfDataElements
    v.push_back(0);  }

void RpcROD_Encoder::packFragments(const std::vector<uint16_t>& v16, std::vector<uint32_t>& v, int n) const {
    // now merge 2 consecutive 16 bit words in 32 bit words
    const unsigned short int nWords = 2;
    unsigned short int position[nWords] = {0, 16};
    unsigned short int v16words[nWords] = {0, 0};
    for (int i = 0; i < n;) {
        v16words[i % nWords] = v16[i];
        v16words[(i + 1) % nWords] = v16[i + 1];
        v.push_back(set32bits(v16words, position, nWords));
        i += nWords;
    }
    }

uint32_t RpcROD_Encoder::set32bits(const unsigned short int* v16, const unsigned short int* pos, const unsigned short int n) {
    uint32_t v32 = 0;
    uint32_t p = 0, v = 0;

    for (ubit16 i = 0; i < n; i++) {
        v = (uint32_t)(*(v16 + i));
        p = (uint32_t)(*(pos + i));
        v32 = v32 | (v << p);
    }
    return v32;
}
