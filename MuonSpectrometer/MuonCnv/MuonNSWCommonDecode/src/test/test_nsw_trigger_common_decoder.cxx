/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// STL include files

#include <vector>
#include <string>
#include <chrono>
#include <iomanip>

// TDAQ include files

#include "ers/SampleIssues.h"
#include "EventStorage/pickDataReader.h"
#include "eformat/eformat.h"

#include "MuonNSWCommonDecode/NSWTriggerCommonDecoder.h"
#include "MuonNSWCommonDecode/NSWTriggerElink.h"
#include "MuonNSWCommonDecode/NSWTriggerMML1AElink.h"
#include "MuonNSWCommonDecode/NSWTriggerMMMonElink.h"
#include "MuonNSWCommonDecode/NSWPadTriggerL1a.h"
#include "MuonNSWCommonDecode/NSWResourceId.h"
#include "MuonNSWCommonDecode/MMARTPacket.h"



#include <TFile.h>
#include <TTree.h>

struct Params
{
  bool print_only {false};
  unsigned int printout_level {0};
  unsigned int max_events {0};
  std::vector <std::string> file_names; //outfiles will just rename input ones
};

struct Statistics
{
  unsigned int nevents {0};
  double total_decoding_time {0};
  double avg_decoding_time {0};
  //can be extended for any quick stat wanted
};

struct outBranches
{
  //each event has multple ROBs (e.g. multiple sectors)
  //each ROB then has multiple elinks (so at least 2 layers of vector)
  //MML1A - comtemplating multiple elinks
  std::vector<uint32_t> b_MML1A_ROD_sourceID = {} ;
  std::vector<uint32_t> b_MML1A_ROD_subdetID = {} ;
  std::vector<uint32_t> b_MML1A_ROD_moduleID = {} ;
  std::vector<uint32_t> b_MML1A_ROD_L1ID = {} ;
  std::vector<uint32_t> b_MML1A_ROD_n_words = {} ;
  std::vector<uint32_t> b_MML1A_head_fragID = {} ;
  std::vector<uint32_t> b_MML1A_head_sectID = {} ;
  std::vector<uint32_t> b_MML1A_head_EC = {} ;
  std::vector<uint32_t> b_MML1A_head_flags = {} ;
  std::vector<uint32_t> b_MML1A_head_BCID = {} ;
  std::vector<uint32_t> b_MML1A_head_orbit = {} ;
  std::vector<uint32_t> b_MML1A_head_spare = {} ;
  std::vector<uint32_t> b_MML1A_L1ID = {} ;
  std::vector<uint32_t> b_MML1A_head_wdw_open = {} ;
  std::vector<uint32_t> b_MML1A_head_l1a_req = {} ;
  std::vector<uint32_t> b_MML1A_head_wdw_close = {} ;
  std::vector<uint32_t> b_MML1A_head_overflowCount = {} ;
  std::vector<uint32_t> b_MML1A_head_wdw_matching_engines_usage = {} ;
  std::vector<uint32_t> b_MML1A_head_cfg_wdw_open_offset = {} ;
  std::vector<uint32_t> b_MML1A_head_cfg_l1a_req_offset = {} ;
  std::vector<uint32_t> b_MML1A_head_cfg_wdw_close_offset = {} ;
  std::vector<uint32_t> b_MML1A_head_cfg_timeout = {} ;
  std::vector<uint32_t> b_MML1A_head_link_const = {} ;
  std::vector<std::vector<uint32_t>> b_MML1A_stream_head_nbits = {} ;
  std::vector<std::vector<uint32_t>> b_MML1A_stream_head_nwords = {} ;
  std::vector<std::vector<uint32_t>> b_MML1A_stream_head_fifo_size = {} ;
  std::vector<std::vector<uint32_t>> b_MML1A_stream_head_streamID = {} ;
  std::vector<std::vector<uint32_t>> b_MML1A_art_BCID = {} ;
  std::vector<std::vector<uint32_t>> b_MML1A_art_layers = {} ;
  std::vector<std::vector<uint32_t>> b_MML1A_art_channels = {} ;
  std::vector<uint32_t> b_MML1A_trailer_CRC = {} ;
  //PAD - comtemplating multiple elinks
  std::vector<uint32_t> b_PadL1A_ROD_sourceID = {} ;
  std::vector<uint32_t> b_PadL1A_ROD_subdetID = {} ;
  std::vector<uint32_t> b_PadL1A_ROD_moduleID = {} ;
  std::vector<uint32_t> b_PadL1A_ROD_L1ID = {} ;
  std::vector<uint32_t> b_PadL1A_ROD_n_words = {} ;
  std::vector<uint32_t> b_PadL1A_flags = {} ;
  std::vector<uint32_t> b_PadL1A_ec = {} ;
  std::vector<uint32_t> b_PadL1A_fragid = {} ;
  std::vector<uint32_t> b_PadL1A_secid = {} ;
  std::vector<uint32_t> b_PadL1A_spare = {} ;
  std::vector<uint32_t> b_PadL1A_orbit = {} ;
  std::vector<uint32_t> b_PadL1A_bcid = {} ;
  std::vector<uint32_t> b_PadL1A_l1id = {} ;
  std::vector<uint32_t> b_PadL1A_hit_n = {} ;
  std::vector<uint32_t> b_PadL1A_pfeb_n = {} ;
  std::vector<uint32_t> b_PadL1A_trigger_n = {} ;
  std::vector<uint32_t> b_PadL1A_bcid_n = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_hit_relbcid = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_hit_pfeb = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_hit_tdschannel = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_hit_vmmchannel = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_hit_vmm = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_hit_padchannel = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_pfeb_addr = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_pfeb_nchan = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_pfeb_disconnected = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_trigger_bandid = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_trigger_phiid = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_trigger_relbcid = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_bcid_rel = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_bcid_status = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_bcid_multzero = {} ;

};

void test_nsw_trigger_common_decoder_help (char *progname)
{
  std::cout << "Usage: " << progname
	    << " [-v] [-r] [-t] [-f] [-d <MMG/STG>] [-n events] [-h] file1, file2, ..." << std::endl;
  std::cout << "\t\t[-n events] maximum number of events to read (default = all)" << std::endl;
  std::cout << "\t\t[-p] only print raw fragments" << std::endl;
  std::cout << "\t\tMultiple [-v] options increase printout detail level" << std::endl;
}

int test_nsw_trigger_common_decoder_opt (int argc, char **argv, Params& params)
{
  int i;
  for (i=1; i < argc; ++i){
    if (argv[i][0] == '-'){
      switch (argv[i][1]){
        case 'v':
          ++params.printout_level;
          break;
        case 'p':
	        params.print_only = true;
	        break;
        case 'n':
          params.max_events = static_cast <unsigned int> (atoi (argv[++i]));
          break;
        case 'h':
	        test_nsw_trigger_common_decoder_help (argv[0]);
          return 1;
        default:
	        test_nsw_trigger_common_decoder_help (argv[0]);
	        return 1;
      }
    } else {
      std::string data_file_name (argv[i]);
      params.file_names.push_back (data_file_name);
    }
  }

  if (params.file_names.size () == 0)
  {
    test_nsw_trigger_common_decoder_help (argv[0]);
    std::cout << "\t\tNo input files provided" << std::endl;
    return 2;
  }

  return 0;
}

int test_nsw_trigger_common_decoder_init_tree (TTree* outtree, outBranches &data)
{
  outtree->Branch( "MML1A_ROD_sourceID", &data.b_MML1A_ROD_sourceID);
  outtree->Branch( "MML1A_ROD_subdetID", &data.b_MML1A_ROD_subdetID);
  outtree->Branch( "MML1A_ROD_moduleID", &data.b_MML1A_ROD_moduleID);
  outtree->Branch( "MML1A_ROD_L1ID", &data.b_MML1A_ROD_L1ID);
  outtree->Branch( "MML1A_ROD_n_words", &data.b_MML1A_ROD_n_words);
  outtree->Branch( "MML1A_head_fragID", &data.b_MML1A_head_fragID);
  outtree->Branch( "MML1A_head_sectID", &data.b_MML1A_head_sectID);
  outtree->Branch( "MML1A_head_EC", &data.b_MML1A_head_EC);
  outtree->Branch( "MML1A_head_flags", &data.b_MML1A_head_flags);
  outtree->Branch( "MML1A_head_BCID", &data.b_MML1A_head_BCID);
  outtree->Branch( "MML1A_head_orbit", &data.b_MML1A_head_orbit);
  outtree->Branch( "MML1A_head_spare", &data.b_MML1A_head_spare);
  outtree->Branch( "MML1A_L1ID", &data.b_MML1A_L1ID);
  outtree->Branch( "MML1A_head_wdw_open", &data.b_MML1A_head_wdw_open);
  outtree->Branch( "MML1A_head_l1a_req", &data.b_MML1A_head_l1a_req);
  outtree->Branch( "MML1A_head_wdw_close", &data.b_MML1A_head_wdw_close);
  outtree->Branch( "MML1A_head_overflowCount", &data.b_MML1A_head_overflowCount);
  outtree->Branch( "MML1A_head_wdw_matching_engines_usage", &data.b_MML1A_head_wdw_matching_engines_usage);
  outtree->Branch( "MML1A_head_cfg_wdw_open_offset", &data.b_MML1A_head_cfg_wdw_open_offset);
  outtree->Branch( "MML1A_head_cfg_l1a_req_offset", &data.b_MML1A_head_cfg_l1a_req_offset);
  outtree->Branch( "MML1A_head_cfg_wdw_close_offset", &data.b_MML1A_head_cfg_wdw_close_offset);
  outtree->Branch( "MML1A_head_cfg_timeout", &data.b_MML1A_head_cfg_timeout);
  outtree->Branch( "MML1A_head_link_const", &data.b_MML1A_head_link_const);
  outtree->Branch( "MML1A_stream_head_nbits", &data.b_MML1A_stream_head_nbits);
  outtree->Branch( "MML1A_stream_head_nwords", &data.b_MML1A_stream_head_nwords);
  outtree->Branch( "MML1A_stream_head_fifo_size", &data.b_MML1A_stream_head_fifo_size);
  outtree->Branch( "MML1A_stream_head_streamID", &data.b_MML1A_stream_head_streamID);
  outtree->Branch( "MML1A_trailer_CRC", &data.b_MML1A_trailer_CRC);
  outtree->Branch( "MML1A_art_BCID", &data.b_MML1A_art_BCID);
  outtree->Branch( "MML1A_art_layers", &data.b_MML1A_art_layers);
  outtree->Branch( "MML1A_art_channels", &data.b_MML1A_art_channels);
  outtree->Branch( "PadL1A_ROD_sourceID", &data.b_PadL1A_ROD_sourceID);
  outtree->Branch( "PadL1A_ROD_subdetID", &data.b_PadL1A_ROD_subdetID);
  outtree->Branch( "PadL1A_ROD_moduleID", &data.b_PadL1A_ROD_moduleID);
  outtree->Branch( "PadL1A_ROD_L1ID", &data.b_PadL1A_ROD_L1ID);
  outtree->Branch( "PadL1A_ROD_n_words", &data.b_PadL1A_ROD_n_words);
  outtree->Branch( "PadL1A_flags", &data.b_PadL1A_flags);
  outtree->Branch( "PadL1A_ec", &data.b_PadL1A_ec);
  outtree->Branch( "PadL1A_fragid", &data.b_PadL1A_fragid);
  outtree->Branch( "PadL1A_secid", &data.b_PadL1A_secid);
  outtree->Branch( "PadL1A_spare", &data.b_PadL1A_spare);
  outtree->Branch( "PadL1A_orbit", &data.b_PadL1A_orbit);
  outtree->Branch( "PadL1A_bcid", &data.b_PadL1A_bcid);
  outtree->Branch( "PadL1A_l1id", &data.b_PadL1A_l1id);
  outtree->Branch( "PadL1A_hit_n", &data.b_PadL1A_hit_n);
  outtree->Branch( "PadL1A_pfeb_n", &data.b_PadL1A_pfeb_n);
  outtree->Branch( "PadL1A_trigger_n", &data.b_PadL1A_trigger_n);
  outtree->Branch( "PadL1A_bcid_n", &data.b_PadL1A_bcid_n);
  outtree->Branch( "PadL1A_hit_relbcid", &data.b_PadL1A_hit_relbcid);
  outtree->Branch( "PadL1A_hit_pfeb", &data.b_PadL1A_hit_pfeb);
  outtree->Branch( "PadL1A_hit_tdschannel", &data.b_PadL1A_hit_tdschannel);
  outtree->Branch( "PadL1A_hit_vmmchannel", &data.b_PadL1A_hit_vmmchannel);
  outtree->Branch( "PadL1A_hit_vmm", &data.b_PadL1A_hit_vmm);
  outtree->Branch( "PadL1A_hit_padchannel", &data.b_PadL1A_hit_padchannel);
  outtree->Branch( "PadL1A_pfeb_addr", &data.b_PadL1A_pfeb_addr);
  outtree->Branch( "PadL1A_pfeb_nchan", &data.b_PadL1A_pfeb_nchan);
  outtree->Branch( "PadL1A_pfeb_disconnected", &data.b_PadL1A_pfeb_disconnected);
  outtree->Branch( "PadL1A_trigger_bandid", &data.b_PadL1A_trigger_bandid);
  outtree->Branch( "PadL1A_trigger_phiid", &data.b_PadL1A_trigger_phiid);
  outtree->Branch( "PadL1A_trigger_relbcid", &data.b_PadL1A_trigger_relbcid);
  outtree->Branch( "PadL1A_bcid_rel", &data.b_PadL1A_bcid_rel);
  outtree->Branch( "PadL1A_bcid_status", &data.b_PadL1A_bcid_status);
  outtree->Branch( "PadL1A_bcid_multzero", &data.b_PadL1A_bcid_multzero);

  return 0;
}

int test_nsw_trigger_common_decoder_end (Statistics &statistics)
{
  std::cout << "Total event processed             = " << statistics.nevents << std::endl;
  return 0;
}

int test_nsw_trigger_common_decoder_event (const eformat::read::FullEventFragment &f, outBranches &data, Params &params, Statistics &statistics)
{
  std::vector <eformat::read::ROBFragment> robs;

  if (params.printout_level > 2)
    std::cout << "Entering fragment analysis" << std::endl;

  f.robs (robs);

  for (auto r = robs.begin (); r != robs.end (); ++r)
  {
    bool is_nsw = false, is_mmg = false, is_stg = false, is_tp = false, is_pt = false;

    // check fragment for errors
    try{ r->check (); }
    catch(...) { std::cout << "Something wrong with a fragment ROB" << std::endl; continue; }

    uint32_t sid = r->rob_source_id ();
    eformat::helper::SourceIdentifier source_id (sid);
    eformat::SubDetector s = source_id.subdetector_id ();
    uint16_t m = source_id.module_id ();

    if (s == eformat::MUON_MMEGA_ENDCAP_A_SIDE || s == eformat::MUON_MMEGA_ENDCAP_C_SIDE)
      is_nsw = is_mmg = true;
    else if (s == eformat::MUON_STGC_ENDCAP_A_SIDE  || s == eformat::MUON_STGC_ENDCAP_C_SIDE)
      is_nsw = is_stg = true;

    //int sector = m & 0xf;
    if ((m & 0xf0) == 0x10) {is_tp = true;}
    else if ((m & 0xf0) == 0x20) {is_pt = true;}


    if (params.printout_level > 1 && is_nsw){
      std::cout << "ROB source ID                = 0x" << std::hex << sid << std::dec << std::endl;
      std::cout << "ROB subdet ID                = 0x" << std::hex << s   << std::dec << std::endl;
      std::cout << "ROB module ID                = 0x" << std::hex << m   << std::dec << std::endl;
      //std::cout << "is_nsw                = " << (is_nsw?"Yes":"No") << std::endl;
      std::cout << "is_mmg                = " << (is_mmg?"Yes":"No") << std::endl;
      std::cout << "is_stg                = " << (is_stg?"Yes":"No") << std::endl;
      std::cout << "is_pt                 = " << (is_pt?"Yes":"No") << std::endl;
      std::cout << "is_tp                 = " << (is_tp?"Yes":"No") << std::endl;
    }


    if (is_nsw && (is_pt || is_tp)){

      //this has to be fixed to get TP data, filtering DAQ no! (need to add an offset)
      if (params.printout_level > 0 || params.print_only){
        std::cout << "NSW Trigger fragment found: length " << r->rod_ndata () << std::endl;
      }
	      // Print out raw fragment
	    if (params.print_only){
        std::cout << "ROD Fragment size in words:" << std::endl;
        std::cout << "ROD Total: " << r->rod_fragment_size_word () << std::endl;
        std::cout << "ROD Header: " << r->rod_header_size_word () << std::endl;
        std::cout << "ROD Trailer: " << r->rod_trailer_size_word () << std::endl;
        std::cout << "ROD L1 ID: " << std::hex << r->rod_lvl1_id () << std::dec << std::endl;
	      std::cout << "ROD Data words: " << r->rod_ndata () << std::endl;
      }

	     const uint32_t *bs = r->rod_data ();

       if (params.print_only){
         std::cout << "Printing raw data (ignoring any structure)" << std::endl;
         std::cout << std::hex;
         for (unsigned int i = 0; i < r->rod_ndata (); ++i){
           std::cout << " " << std::setfill('0') << std::setw(8) << bs[i];
           if (i % 4 == 3) std::cout << std::endl;
         }
         std::cout << std::dec;
         std::cout << std::endl;
       }

       if (!params.print_only){

         std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now ();
         Muon::nsw::NSWTriggerCommonDecoder nsw_trigger_decoder (*r, (is_pt?"PadL1A":(is_mmg?"MML1A":"STGL1A")) );
         //ignoring monitoring for now; rob ids not defined yet
         std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now ();
         unsigned int time_elapsed = std::chrono::duration_cast <std::chrono::microseconds> (end - begin).count ();
  	     float time_elapsed_ms = static_cast <float> (time_elapsed) / 1000;
         if (params.printout_level > 1){
           std::cout << "Time for decoding this event: " << time_elapsed_ms << std::endl;
           std::cout << std::endl;
         }
         statistics.total_decoding_time += time_elapsed_ms;

         if (is_pt){
           for (size_t i = 0; i < nsw_trigger_decoder.get_elinks().size(); i++) {
	     std::shared_ptr<Muon::nsw::NSWPadTriggerL1a> link = std::dynamic_pointer_cast<Muon::nsw::NSWPadTriggerL1a>(nsw_trigger_decoder.get_elinks()[i]);
             data.b_PadL1A_ROD_sourceID.push_back( sid );
             data.b_PadL1A_ROD_subdetID.push_back( s );
             data.b_PadL1A_ROD_moduleID.push_back( m );
             data.b_PadL1A_ROD_L1ID.push_back( r->rod_lvl1_id () );
             data.b_PadL1A_ROD_n_words.push_back( r->rod_ndata () );
             data.b_PadL1A_flags.push_back( link->getFlags() );
             data.b_PadL1A_ec.push_back( link->getEc() );
             data.b_PadL1A_fragid.push_back( link->getFragid() );
             data.b_PadL1A_secid.push_back( link->getSecid() );
             data.b_PadL1A_spare.push_back( link->getSpare() );
             data.b_PadL1A_orbit.push_back( link->getOrbit() );
             data.b_PadL1A_bcid.push_back( link->getBcid() );
             data.b_PadL1A_l1id.push_back( link->getL1id() );
             data.b_PadL1A_hit_n.push_back( link->getNumberOfHits() );
             data.b_PadL1A_pfeb_n.push_back( link->getNumberOfPfebs() );
             data.b_PadL1A_trigger_n.push_back( link->getNumberOfTriggers() );
             data.b_PadL1A_bcid_n.push_back( link->getNumberOfBcids() );
             data.b_PadL1A_hit_relbcid.push_back( link->getHitRelBcids() );
             data.b_PadL1A_hit_pfeb.push_back( link->getHitPfebs() );
             data.b_PadL1A_hit_tdschannel.push_back( link->getHitTdsChannels() );
             data.b_PadL1A_hit_vmmchannel.push_back( link->getHitVmmChannels() );
             data.b_PadL1A_hit_vmm.push_back( link->getHitVmms() );
             data.b_PadL1A_hit_padchannel.push_back( link->getHitPadChannels() );
             data.b_PadL1A_pfeb_addr.push_back( link->getPfebAddresses() );
             data.b_PadL1A_pfeb_nchan.push_back( link->getPfebNChannels() );
             data.b_PadL1A_pfeb_disconnected.push_back( link->getPfebDisconnecteds() );
             data.b_PadL1A_trigger_bandid.push_back( link->getTriggerBandIds() );
             data.b_PadL1A_trigger_phiid.push_back( link->getTriggerPhiIds() );
             data.b_PadL1A_trigger_relbcid.push_back( link->getTriggerRelBcids() );
             data.b_PadL1A_bcid_rel.push_back( link->getBcidRels() );
             data.b_PadL1A_bcid_status.push_back( link->getBcidStatuses() );
             data.b_PadL1A_bcid_multzero.push_back( link->getBcidMultZeros() );
           }
         } else if (is_mmg) {
           for (size_t i = 0; i < nsw_trigger_decoder.get_elinks().size(); i++) {
	     std::shared_ptr<Muon::nsw::NSWTriggerMML1AElink> link = std::dynamic_pointer_cast<Muon::nsw::NSWTriggerMML1AElink>(nsw_trigger_decoder.get_elinks()[i]);
             data.b_MML1A_ROD_sourceID.push_back( sid );
             data.b_MML1A_ROD_subdetID.push_back( s );
             data.b_MML1A_ROD_moduleID.push_back( m );
             data.b_MML1A_ROD_L1ID.push_back( r->rod_lvl1_id () );
             data.b_MML1A_ROD_n_words.push_back( r->rod_ndata () );
             data.b_MML1A_head_fragID.push_back( link->head_fragID() );
             data.b_MML1A_head_sectID.push_back( link->head_sectID() );
             data.b_MML1A_head_EC.push_back( link->head_EC() );
             data.b_MML1A_head_flags.push_back( link->head_flags() );
             data.b_MML1A_head_BCID.push_back( link->head_BCID() );
             data.b_MML1A_head_orbit.push_back( link->head_orbit() );
             data.b_MML1A_head_spare.push_back( link->head_spare() );
             data.b_MML1A_L1ID.push_back( link->L1ID() );
             data.b_MML1A_head_wdw_open.push_back( link->head_wdw_open() );
             data.b_MML1A_head_l1a_req.push_back( link->head_l1a_req() );
             data.b_MML1A_head_wdw_close.push_back( link->head_wdw_close() );
             data.b_MML1A_head_overflowCount.push_back( link->head_overflowCount() );
             data.b_MML1A_head_wdw_matching_engines_usage.push_back( link->head_wdw_matching_engines_usage() );
             data.b_MML1A_head_cfg_wdw_open_offset.push_back( link->head_cfg_wdw_open_offset() );
             data.b_MML1A_head_cfg_l1a_req_offset.push_back( link->head_cfg_l1a_req_offset() );
             data.b_MML1A_head_cfg_wdw_close_offset.push_back( link->head_cfg_wdw_close_offset() );
             data.b_MML1A_head_cfg_timeout.push_back( link->head_cfg_timeout() );
             data.b_MML1A_head_link_const.push_back( link->head_link_const() );
             data.b_MML1A_stream_head_nbits.push_back( link->stream_head_nbits() );
             data.b_MML1A_stream_head_nwords.push_back( link->stream_head_nwords() );
             data.b_MML1A_stream_head_fifo_size.push_back( link->stream_head_fifo_size() );
             data.b_MML1A_stream_head_streamID.push_back( link->stream_head_streamID() );

	     const std::vector<std::shared_ptr<Muon::nsw::MMARTPacket>>& arts = link->art_packets();
	     std::vector<uint32_t> tmp_BCIDs;
             std::vector<uint32_t> tmp_layers;
             std::vector<uint32_t> tmp_channels;

	     for (auto art : arts){
	       for (uint i = 0; i < art->channels().size(); i++){
		 //so that there's a timestamp per channel
		 tmp_layers.push_back( art->channels()[i].first );
		 tmp_channels.push_back( art->channels()[i].second );
		 tmp_BCIDs.push_back( art->art_BCID() );
	       }
	     }

             data.b_MML1A_art_BCID.push_back( tmp_BCIDs );
             data.b_MML1A_art_layers.push_back( tmp_layers );
             data.b_MML1A_art_channels.push_back( tmp_channels );

             data.b_MML1A_trailer_CRC.push_back( link->trailer_CRC() );
           }
         }
       }

     } //end if is_nsw
   } //end for on robs

   return 0;
}


int test_nsw_trigger_common_decoder_loop (Params &params, Statistics &statistics)
{
  outBranches data;

  bool anyBrokenFile = false;
  //int err;

  for (const std::string &filename : params.file_names)
  {

    char *buf = nullptr;
    unsigned int size = 0;

    TFile* outfile = new TFile();
    TTree* outtree = new TTree();

    std::string data_file_name (filename);
    std::string out_file_name = data_file_name.substr(data_file_name.find_last_of("/\\") + 1) + ".decoded.root";

    std::cout << "Reading file " << data_file_name << std::endl;
    if (!params.print_only) {
      std::cout << "Saving here file " << out_file_name << std::endl;
      outfile = new TFile(out_file_name.c_str(), "recreate");
      outtree = new TTree("decoded_data", "decoded_data");
      test_nsw_trigger_common_decoder_init_tree (outtree, data);
    }

    std::unique_ptr <DataReader> reader (pickDataReader (data_file_name));

    if (!reader || !reader->good ())
    {
      std::cout << "Cannot open this file properly; skipping to next one" << std::endl;
      anyBrokenFile = true;
    }

    while (!reader->endOfFile () && (params.max_events == 0 || statistics.nevents < params.max_events))
    {
      try
      {
        DRError err = reader->getData (size, &buf);

        if (err != EventStorage::DROK)
        {
          std::cout << "Cannot get data properly from file" << data_file_name.c_str () << "; skipping it!" << std::endl;
          anyBrokenFile = true;
          if (buf) delete buf;
          break;
        }

        eformat::read::FullEventFragment f ((unsigned int *) buf);
        f.check ();

        //reset branches
        data = outBranches();

        if ( test_nsw_trigger_common_decoder_event (f, data, params, statistics) )
        {
          std::cout << "Cannot decode properly event " << statistics.nevents << "; skipping it!" <<std::endl;
          if (buf) delete buf;
          continue;
        }

        if (!params.print_only) {outtree->Fill();}
        ++statistics.nevents;
      }

      catch (std::runtime_error& error)
      {
        std::cout << "Exception!" << std::endl;
        std::cout << error.what() << std::endl;
        if (buf) delete buf;
        break;
      }

      if (buf) delete buf;
    }

    if (!params.print_only) {
      outfile->Write();
      outfile->Close();
    }
  }

  if (anyBrokenFile) return 3;
  return 0;
}

int main (int argc, char **argv)
{
  Params params;
  Statistics statistics;

  int err;

  if ( (err = test_nsw_trigger_common_decoder_opt (argc, argv, params)) )
    return err;

  if ( (err = test_nsw_trigger_common_decoder_loop (params, statistics)) )
    return err;

  if ( (err = test_nsw_trigger_common_decoder_end (statistics)) )
    return err;

  return err;
}
