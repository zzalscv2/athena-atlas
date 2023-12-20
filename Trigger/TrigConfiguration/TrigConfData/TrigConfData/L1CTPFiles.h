/*
   Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGCONFDATA_L1CTPFiles_H
#define TRIGCONFDATA_L1CTPFiles_H

#include "TrigConfData/ConstIter.h"
#include "TrigConfData/DataStructure.h"
#include "TrigConfData/L1Item.h"
#include "TrigConfData/L1Connector.h"
#include "TrigConfData/L1Board.h"
#include "TrigConfData/L1TopoAlgorithm.h"
#include "TrigConfData/L1Threshold.h"
#include "TrigConfData/L1ThrExtraInfo.h"
#include "TrigConfData/L1CTP.h"
#include "boost/property_tree/ptree.hpp"

#include <vector>
#include <map>

namespace TrigConf {

   /** 
    * @brief L1 menu configuration
    *
    * Provides access to menu name and ctpVersion and to the L1 items and thresholds
    */
   class L1CTPFiles final {
   public:

      using ptree = boost::property_tree::ptree;

      static const size_t CTPCORE_LUT_SIZE {725248};
      static const size_t CTPCORE_CAM_SIZE {55296};
      static const size_t CTPCORE_SMX_SIZE {0};
      
      static const size_t CTPIN_MONSEL_SIZE {124};
      static const size_t CTPIN_MONDEC_SIZE {4096};

      static const size_t CTPMON_MUX_OUTPUT_NUMBER {9};
      static const size_t CTPMON_ADDRESS_SELECTOR_NUMBER {24};
      static const size_t CTPMON_SELECTOR_SIZE = CTPMON_MUX_OUTPUT_NUMBER * CTPMON_ADDRESS_SELECTOR_NUMBER;
      static const size_t CTPMON_DECODER_SIZE {6656};
      static const size_t CTPMON_DMX_SIZE {160};

      enum MuctpiAccess { RoiMaskA, RoiMaskC, PtLutBarrel, PtLutEndcap };
      static const std::map<MuctpiAccess, std::string> s_keyMap;

      // Register the multiplexed inputs (phase) to the CTPCORE (direct electrical and from CTPIN)
      class CTPCoreInput {
      public:
         enum InputType { PIT, DIR, NONE };
         CTPCoreInput(size_t inputNumber, const std::string& name, size_t bit, size_t phase, InputType inputType) :
            m_inputNumber(inputNumber), m_name(name), m_bit(bit), m_phase(phase), m_inputType(inputType) {}
         size_t      inputNumber() const { return m_inputNumber; }
         std::string name() const { return m_name; }
         size_t      bit() const { return m_bit; }
         size_t      phase() const { return m_phase; }
         InputType   inputType() const { return m_inputType; }
      private:
         size_t      m_inputNumber;  // internal Trigger Menu Compiler index
         std::string m_name;         // trigger threshold name, a label for input to the CTP
         size_t      m_bit;          // number of multiplicity bits of the threshold (it can typically have multiplicity up to 7, so up to 3 bits)
         size_t      m_phase;        // double data rate flag, a parameter saying if the input comes on the rising or falling edge
         InputType   m_inputType;    // input tag, clarifying the origin or a group of the inputs
      };

      // Register the CTPCORE inputs to the CTPCORE switch matrix
      class CTPCoreCTPXInput {
      public:
         enum InputType { CTPX, NONE };
         CTPCoreCTPXInput(size_t inputNumber, const std::string& name, size_t bit, InputType inputType) :
            m_inputNumber(inputNumber), m_name(name), m_bit(bit), m_inputType(inputType) {}
         size_t      inputNumber() const { return m_inputNumber; }
         std::string name() const { return m_name; }
         size_t      bit() const { return m_bit; }
         InputType   inputType() const { return m_inputType; }
      private:
         size_t      m_inputNumber; // internal Trigger Menu Compiler index
         std::string m_name;        // trigger threshold name, a label for input to the CTP
         size_t      m_bit;         // number of multiplicity bits of the threshold (it can typically have multiplicity up to 7, so up to 3 bits)
         InputType   m_inputType;   // input tag, clarifying the origin or a group of the inputs
      };

     class CTPInCounter {
      public:
         CTPInCounter(const std::string& name, size_t slot, size_t cable, size_t number) :
            m_name(name), m_slot(slot), m_cable(cable), m_number(number) {}
         std::string name() const { return m_name; }
         size_t      slot() const { return m_slot; }
         size_t      cable() const { return m_cable; }
         size_t      number() const { return m_number; }
      private:
         std::string m_name;
         size_t      m_slot;
         size_t      m_cable;
         size_t      m_number;
      };

     class CTPMonCounter {
      public:
         CTPMonCounter(const std::string& name, size_t number) :
            m_name(name), m_number(number) {}
         std::string name() const { return m_name; }
         size_t      number() const { return m_number; }
      private:
         std::string m_name;
         size_t      m_number;
      };

      /** Constructor */
      L1CTPFiles();

      /** 
       * Accessors to the various CTP data
       **/
      bool hasCompleteCtpData() const;
      bool hasCompleteSmxData() const;
      bool hasCompleteMuctpiData() const;
      bool hasCompleteTmcData() const;

      const std::vector<uint32_t> & ctpcore_LUT() const;
      const std::vector<uint32_t> & ctpcore_CAM() const;
      const std::vector<uint32_t> & ctpcore_SMX() const;

      const std::vector<uint32_t> & ctpin_MonSelector_Slot7() const;
      const std::vector<uint32_t> & ctpin_MonSelector_Slot8() const;
      const std::vector<uint32_t> & ctpin_MonSelector_Slot9() const;
      const std::vector<uint32_t> & ctpin_MonDecoder_Slot7() const;
      const std::vector<uint32_t> & ctpin_MonDecoder_Slot8() const;
      const std::vector<uint32_t> & ctpin_MonDecoder_Slot9() const;

      const std::vector<uint32_t> & ctpmon_MonSelector() const;
      const std::vector<uint32_t> & ctpmon_MonDecoder() const;
      const std::vector<uint32_t> & ctpmon_DMX() const;

      const std::string & smx_Output() const;
      const std::string & smx_Vhdl_Slot7() const;
      const std::string & smx_Vhdl_Slot8() const;
      const std::string & smx_Vhdl_Slot9() const;
      const std::string & smx_Svfi_Slot7() const;
      const std::string & smx_Svfi_Slot8() const;
      const std::string & smx_Svfi_Slot9() const;

      const std::vector<uint32_t> & muctpiRoi(MuctpiAccess key) const;
      const std::vector<uint32_t> & muctpi_Extra_Ptlut(const std::string & sector) const;
      const std::vector<uint32_t> & muctpi_Nbits() const;

      const std::vector<TrigConf::L1CTPFiles::CTPCoreInput> & tmc_CtpcoreInputs() const;
      const std::vector<TrigConf::L1CTPFiles::CTPInCounter> & tmc_CtpinCounters() const;
      const std::vector<TrigConf::L1CTPFiles::CTPMonCounter> & tmc_CtpmonCounters() const;

      /**
       * Setters of the various CTP data
       */

      void set_HasCompleteCtpData(bool flag);
      void set_HasCompleteSmxData(bool flag);
      void set_HasCompleteMuctpiData(bool flag);
      void set_HasCompleteTmcData(bool flag);

      void set_Ctpcore_LUT(std::vector<uint32_t> data);
      void set_Ctpcore_CAM(std::vector<uint32_t> data);
      void set_Ctpcore_SMX(std::vector<uint32_t> data);

      void set_Ctpin_MonSelector_Slot7(std::vector<uint32_t> data);
      void set_Ctpin_MonSelector_Slot8(std::vector<uint32_t> data);
      void set_Ctpin_MonSelector_Slot9(std::vector<uint32_t> data);
      void set_Ctpin_MonDecoder_Slot7(std::vector<uint32_t> data);
      void set_Ctpin_MonDecoder_Slot8(std::vector<uint32_t> data);
      void set_Ctpin_MonDecoder_Slot9(std::vector<uint32_t> data);

      void set_Ctpmon_MonSelector(std::vector<uint32_t> data);
      void set_Ctpmon_MonDecoder(std::vector<uint32_t> data);
      void set_Ctpmon_DMX(std::vector<uint32_t> data);

      void set_Smx_Output(const std::string & data);
      void set_Smx_Vhdl_Slot7(const std::string & data);
      void set_Smx_Vhdl_Slot8(const std::string & data);
      void set_Smx_Vhdl_Slot9(const std::string & data);
      void set_Smx_Svfi_Slot7(const std::string & data);
      void set_Smx_Svfi_Slot8(const std::string & data);
      void set_Smx_Svfi_Slot9(const std::string & data);

      void set_Muctpi(MuctpiAccess key, std::vector<uint32_t> data);
      void set_Muctpi_Extra_Ptlut(const std::string & key, std::vector<uint32_t> data);
      void set_Muctpi_Nbits(std::vector<uint32_t> data);

      void set_Tmc_CtpcoreInputs(std::vector<TrigConf::L1CTPFiles::CTPCoreInput> data);
      void set_Tmc_CtpcoreCTPXInputs(std::vector<TrigConf::L1CTPFiles::CTPCoreCTPXInput> data);
      void set_Tmc_CtpinCounters(std::vector<TrigConf::L1CTPFiles::CTPInCounter> data);
      void set_Tmc_CtpmonCounters(std::vector<TrigConf::L1CTPFiles::CTPMonCounter> data);

      void set_Tmc_Data(DataStructure data);

      void print() const;

   private:

      bool m_hasCompleteCtpData {false};
      bool m_hasCompleteSmxData {false};
      bool m_hasCompleteMuctpiData {false};
      bool m_hasCompleteTmcData {false};


      /**
       * L1_CTP_FILES
       */
      std::vector<uint32_t> m_Ctpcore_LUT;
      std::vector<uint32_t> m_Ctpcore_CAM;
      std::vector<uint32_t> m_Ctpcore_SMX;

      std::vector<uint32_t> m_Ctpin_MonSelector_Slot7;
      std::vector<uint32_t> m_Ctpin_MonSelector_Slot8;
      std::vector<uint32_t> m_Ctpin_MonSelector_Slot9;
      std::vector<uint32_t> m_Ctpin_MonDecoder_Slot7;
      std::vector<uint32_t> m_Ctpin_MonDecoder_Slot8;
      std::vector<uint32_t> m_Ctpin_MonDecoder_Slot9;

      std::vector<uint32_t> m_Ctpmon_MonSelector;
      std::vector<uint32_t> m_Ctpmon_MonDecoder;
      std::vector<uint32_t> m_Ctpmon_DMX;

      /**
       * L1_SMX files
       */
      std::string m_Smx_Output {};
      std::string m_Smx_Vhdl_Slot7 {};
      std::string m_Smx_Vhdl_Slot8 {};
      std::string m_Smx_Vhdl_Slot9 {};
      std::string m_Smx_Svfi_Slot7 {};
      std::string m_Smx_Svfi_Slot8 {};
      std::string m_Smx_Svfi_Slot9 {};

      /**
       * L1 Muon files
       */
      std::map<std::string, std::vector<uint32_t>> m_muctpi;
      std::map<std::string, std::vector<uint32_t>> m_muctpi_Extra_Ptlut;
      std::vector<uint32_t> m_muctpi_Nbits;


      /**
       * L1 TMC output informaion
       */
      std::vector<TrigConf::L1CTPFiles::CTPCoreInput> m_Tmc_CtpcoreInputs;
      std::vector<TrigConf::L1CTPFiles::CTPCoreCTPXInput> m_Tmc_CtpcoreCTPXInputs;
      std::vector<TrigConf::L1CTPFiles::CTPInCounter> m_Tmc_CtpinCounters;
      std::vector<TrigConf::L1CTPFiles::CTPMonCounter> m_Tmc_CtpmonCounters;
      DataStructure m_Tmc;

   };
}

#endif
