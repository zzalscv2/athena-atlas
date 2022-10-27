
#ifndef L1CALO_BS_DECODER_RUN3_H
#define L1CALO_BS_DECODER_RUN3_H

#include <list>
#include <stdint.h>

#include "bytestreamDecoder/L1CaloRdoFexTob.h"
#include <memory>
#include <sstream>

class L1CaloRdoEfexTob;
class L1CaloRdoEfexTower;
class L1CaloRdoGfexTob;
class L1CaloRdoGfexTower;
class L1CaloRdoJfexTob;
class L1CaloRdoJfexTower;
class L1CaloRdoMuonTob;     // **FIXME** Different class for run 3?
class L1CaloRdoPh1TopoHit;
class L1CaloRdoRodInfo;

class L1CaloBsDecoderRun3
{
public:
   L1CaloBsDecoderRun3();

   /** Interface class for logging, can be overriden to e.g. log errors a different way
    * e.g. in offline software this can be replaced with a AthenaMonitoring histogramming
    *  Default is to log to cerr;
    */
   class Logging {
     public:
       virtual void err(const std::string& location, const std::string& title, const std::string& detail) const {
           std::cerr << "L1CaloBsDecoderRun3::" << location << " : " << title << " - " << detail << std::endl;
       }
   };

    void setVerbosity( bool verbosity );
    void setLogger( std::unique_ptr<Logging>&& logger ) { m_logger = std::move(logger); }


    void decodeEfexData( const uint32_t* beg, const uint32_t* end,
                        std::list<L1CaloRdoEfexTower>& dat,
                        std::list<L1CaloRdoRodInfo>::const_iterator rodInfo ) const;

   void decodeEfexTobs( const uint32_t* beg, const uint32_t* end,
                        std::list<L1CaloRdoEfexTob>& tob,
                        std::list<L1CaloRdoRodInfo>::const_iterator rodInfo ) const;

  private:
    void decodeOneEfexTob( const uint32_t word[], const uint32_t shelfNumber,
                           const uint32_t efexNumber, const uint32_t fpgaNumber,
                           const uint32_t errorMask,
                           const uint32_t numSlices, const uint32_t sliceNum,
                           L1CaloRdoFexTob::TobType tobType,
                           L1CaloRdoFexTob::TobSource tobSource,
                           std::list<L1CaloRdoEfexTob>& tob,
                           std::list<L1CaloRdoRodInfo>::const_iterator rodInfo ) const;

    uint32_t decodeEfexDataChan( const uint32_t payload[],
                                 const uint32_t efexNumber,
                                 const uint32_t shelfNumber,
                                 const uint32_t errorMask,
                                 std::list<L1CaloRdoEfexTower>& dat,
                                 std::list<L1CaloRdoRodInfo>::const_iterator rodInfo ) const;

    bool decodeEfexTobSlice( const uint32_t payload[], size_t& index,
                             const uint32_t efexNumber, const uint32_t shelfNumber,
                             const uint32_t numSlices, const uint32_t errorMask,
                             std::list<L1CaloRdoEfexTob>& tob,
                             std::list<L1CaloRdoRodInfo>::const_iterator rodInfo ) const;
  public:


#ifndef OFFLINE_DECODER
   void decodeJfexData( const uint32_t* beg, const uint32_t* end,
                        std::list<L1CaloRdoJfexTower>& dat,
                        std::list<L1CaloRdoRodInfo>::const_iterator rodInfo );

   void decodeJfexTobs( const uint32_t* beg, const uint32_t* end,
                        std::list<L1CaloRdoJfexTob>& tob,
                        std::list<L1CaloRdoRodInfo>::const_iterator rodInfo );

   void decodeGfexData( const uint32_t* beg, const uint32_t* end,
                        std::list<L1CaloRdoGfexTower>& dat,
                        std::list<L1CaloRdoRodInfo>::const_iterator rodInfo );

   void decodeGfexTobs( const uint32_t* beg, const uint32_t* end,
                        std::list<L1CaloRdoGfexTob>& dat,
                        std::list<L1CaloRdoRodInfo>::const_iterator rodInfo );

   void decodePh1TopoData( const uint32_t* beg, const uint32_t* end,
                           std::list<L1CaloRdoEfexTob>& etob,
                           std::list<L1CaloRdoJfexTob>& jtob,
                           std::list<L1CaloRdoGfexTob>& gtob,
                           std::list<L1CaloRdoMuonTob>& mtob,
                           std::list<L1CaloRdoRodInfo>::const_iterator rodInfo );

   void decodePh1TopoHits( const uint32_t* beg, const uint32_t* end,
                           std::list<L1CaloRdoPh1TopoHit>& hit,
                           std::list<L1CaloRdoRodInfo>::const_iterator rodInfo );

private:
   uint32_t decodeJfexDataChan( const uint32_t payload[],
                                const uint32_t jfexNumber,
                                const uint32_t fpgaNumber,
                                const uint32_t errorMask,
                                std::list<L1CaloRdoJfexTower>& dat,
                                std::list<L1CaloRdoRodInfo>::const_iterator rodInfo );

   bool decodeJfexTobSlice( const uint32_t payload[], size_t blockSize, size_t& index,
                            const uint32_t jfexNumber, const uint32_t fpgaNumber,
                            const uint32_t sliceNumber, const uint32_t numSlices,
                            const uint32_t errorMask,
                            std::list<L1CaloRdoJfexTob>& tob,
                            std::list<L1CaloRdoRodInfo>::const_iterator rodInfo );

   uint32_t decodeGfexDataChan( const uint32_t payload[],
                                const uint32_t fpgaNumber,
                                const uint32_t chanNumber,
                                const uint32_t errorMask,
                                std::list<L1CaloRdoGfexTower>& dat,
                                std::list<L1CaloRdoRodInfo>::const_iterator rodInfo );

   bool decodeGfexTobSlice( const uint32_t payload[], const uint32_t blockType,
                            const uint32_t sliceNumber, const uint32_t numSlices,
                            const uint32_t errorMask,
                            std::list<L1CaloRdoGfexTob>& tob,
                            std::list<L1CaloRdoRodInfo>::const_iterator rodInfo );
#endif // OFFLINE_DECODER

   bool checkFibreCRC( std::vector<uint32_t>& data ) const;

   int m_verbosity;
   std::unique_ptr<Logging> m_logger;
};

#endif
