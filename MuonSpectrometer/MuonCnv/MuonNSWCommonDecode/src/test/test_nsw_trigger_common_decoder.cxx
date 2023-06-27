/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
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
#include "MuonNSWCommonDecode/MMTrigPacket.h"
#include "MuonNSWCommonDecode/NSWTriggerSTGL1AElink.h"
#include "MuonNSWCommonDecode/STGTPPackets.h"


#include <TFile.h>
#include <TTree.h>

struct Params
{
	bool print_only {false};
	uint32_t printout_level {0};
	uint32_t max_events {0};
	std::vector <std::string> file_names; //outfiles will just rename input ones
};

struct Statistics
{
	uint32_t nevents {0};
	uint32_t nevents_not_decoded {0};
	uint32_t nevents_sus_felix_stat {0};
	double total_decoding_time {0};
	double avg_decoding_time {0};
	double avg_fill_time {0};
	double avg_write_time {0};
	double total_write_time {0};
	double total_fill_time {0};
	//can be extended for any quick stat wanted
};

struct outBranches
{
	//each event has multple ROBs (e.g. multiple sectors)
	//each ROB then has multiple elinks
	//MML1A - comtemplating multiple elinks,
	//so the vector is on elinks
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
	std::vector<std::vector<uint32_t>> b_MML1A_trig_BCID = {} ;
	std::vector<std::vector<uint32_t>> b_MML1A_trig_dTheta = {} ;
	std::vector<std::vector<uint32_t>> b_MML1A_trig_phiBin = {} ;
	std::vector<std::vector<uint32_t>> b_MML1A_trig_rBin = {} ;
	std::vector<uint32_t> b_MML1A_trailer_CRC = {} ;
	//MMMon - comtemplating multiple elinks (even if only one possible in current design)
	//so the vector is on elinks
	std::vector<uint32_t> b_MMMon_ROD_sourceID = {} ;
	std::vector<uint32_t> b_MMMon_ROD_subdetID = {} ;
	std::vector<uint32_t> b_MMMon_ROD_moduleID = {} ;
	std::vector<uint32_t> b_MMMon_ROD_L1ID = {} ;
	std::vector<uint32_t> b_MMMon_ROD_n_words = {} ;
	std::vector<uint32_t> b_MMMon_head_fragID = {} ;
	std::vector<uint32_t> b_MMMon_head_sectID = {} ;
	std::vector<uint32_t> b_MMMon_head_EC = {} ;
	std::vector<uint32_t> b_MMMon_head_flags = {} ;
	std::vector<uint32_t> b_MMMon_head_BCID = {} ;
	std::vector<uint32_t> b_MMMon_head_orbit = {} ;
	std::vector<uint32_t> b_MMMon_head_spare = {} ;
	std::vector<uint32_t> b_MMMon_L1ID = {} ;
	std::vector<uint32_t> b_MMMon_head_coincBCID = {} ;
	std::vector<uint32_t> b_MMMon_head_regionCount = {} ;
	std::vector<uint32_t> b_MMMon_head_coincRegion = {} ;
	std::vector<uint32_t> b_MMMon_head_reserved = {} ;
	std::vector<std::vector<uint32_t>> b_MMMon_finder_streamID = {} ;
	std::vector<std::vector<uint32_t>> b_MMMon_finder_regionCount = {} ;
	std::vector<std::vector<uint32_t>> b_MMMon_finder_triggerID = {} ;
	std::vector<std::vector<uint32_t>> b_MMMon_finder_V1 = {} ;
	std::vector<std::vector<uint32_t>> b_MMMon_finder_V0 = {} ;
	std::vector<std::vector<uint32_t>> b_MMMon_finder_U1 = {} ;
	std::vector<std::vector<uint32_t>> b_MMMon_finder_U0 = {} ;
	std::vector<std::vector<uint32_t>> b_MMMon_finder_X3 = {} ;
	std::vector<std::vector<uint32_t>> b_MMMon_finder_X2 = {} ;
	std::vector<std::vector<uint32_t>> b_MMMon_finder_X1 = {} ;
	std::vector<std::vector<uint32_t>> b_MMMon_finder_X0 = {} ;
	std::vector<std::vector<uint32_t>> b_MMMon_fitter_streamID = {} ;
	std::vector<std::vector<uint32_t>> b_MMMon_fitter_regionCount = {} ;
	std::vector<std::vector<uint32_t>> b_MMMon_fitter_triggerID = {} ;
	std::vector<std::vector<uint32_t>> b_MMMon_fitter_filler = {} ;
	std::vector<std::vector<uint32_t>> b_MMMon_fitter_mxG = {} ;
	std::vector<std::vector<uint32_t>> b_MMMon_fitter_muG = {} ;
	std::vector<std::vector<uint32_t>> b_MMMon_fitter_mvG = {} ;
	std::vector<std::vector<uint32_t>> b_MMMon_fitter_mxL = {} ;
	std::vector<std::vector<uint32_t>> b_MMMon_fitter_mx_ROI = {} ;
	std::vector<std::vector<uint32_t>> b_MMMon_fitter_dTheta = {} ;
	std::vector<std::vector<uint32_t>> b_MMMon_fitter_zero = {} ;
	std::vector<std::vector<uint32_t>> b_MMMon_fitter_phiSign = {} ;
	std::vector<std::vector<uint32_t>> b_MMMon_fitter_phiBin = {} ;
	std::vector<std::vector<uint32_t>> b_MMMon_fitter_rBin = {} ;
	std::vector<uint32_t> b_MMMon_trailer_CRC = {} ;
	//PadL1A - comtemplating multiple elinks
	//so the vector is on elinks
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

	std::vector<uint32_t> b_STGL1A_ROD_sourceID = {} ;
	std::vector<uint32_t> b_STGL1A_ROD_subdetID = {} ;
	std::vector<uint32_t> b_STGL1A_ROD_moduleID = {} ;
	std::vector<uint32_t> b_STGL1A_ROD_L1ID = {} ;
	std::vector<uint32_t> b_STGL1A_ROD_n_words = {} ;
	std::vector<uint32_t> b_STGL1A_head_fragID = {} ;
	std::vector<uint32_t> b_STGL1A_head_sectID = {} ;
	std::vector<uint32_t> b_STGL1A_head_EC = {} ;
	std::vector<uint32_t> b_STGL1A_head_flags = {} ;
	std::vector<uint32_t> b_STGL1A_head_BCID = {} ;
	std::vector<uint32_t> b_STGL1A_head_orbit = {} ;
	std::vector<uint32_t> b_STGL1A_head_spare = {} ;
	std::vector<uint32_t> b_STGL1A_L1ID = {} ;
	std::vector<uint32_t> b_STGL1A_head_wdw_open = {} ;
	std::vector<uint32_t> b_STGL1A_head_l1a_req = {} ;
	std::vector<uint32_t> b_STGL1A_head_wdw_close = {} ;
	std::vector<uint32_t> b_STGL1A_head_overflowCount = {} ;
	std::vector<uint32_t> b_STGL1A_head_wdw_matching_engines_usage = {} ;
	std::vector<uint32_t> b_STGL1A_head_cfg_wdw_open_offset = {} ;
	std::vector<uint32_t> b_STGL1A_head_cfg_l1a_req_offset = {} ;
	std::vector<uint32_t> b_STGL1A_head_cfg_wdw_close_offset = {} ;
	std::vector<uint32_t> b_STGL1A_head_cfg_timeout = {} ;
	std::vector<uint32_t> b_STGL1A_head_link_const = {} ;
	std::vector<std::vector<uint32_t>> b_STGL1A_stream_head_nbits = {} ;
	std::vector<std::vector<uint32_t>> b_STGL1A_stream_head_nwords = {} ;
	std::vector<std::vector<uint32_t>> b_STGL1A_stream_head_fifo_size = {} ;
	std::vector<std::vector<uint32_t>> b_STGL1A_stream_head_streamID = {};

	std::vector<std::vector<uint32_t>> b_STGL1A_pad_coincidence_wedge = {};

	std::vector<std::vector<uint32_t>> b_STGL1A_pad_phiID_3 = {};
	std::vector<std::vector<uint32_t>> b_STGL1A_pad_phiID_2 = {};
	std::vector<std::vector<uint32_t>> b_STGL1A_pad_phiID_1 = {};
	std::vector<std::vector<uint32_t>> b_STGL1A_pad_phiID_0 = {};

	std::vector<std::vector<uint32_t>> b_STGL1A_pad_bandID_3 = {};
	std::vector<std::vector<uint32_t>> b_STGL1A_pad_bandID_2 = {};
	std::vector<std::vector<uint32_t>> b_STGL1A_pad_bandID_1 = {};
	std::vector<std::vector<uint32_t>> b_STGL1A_pad_bandID_0 = {};

	std::vector<std::vector<uint32_t>> b_STGL1A_pad_BCID = {};
	std::vector<std::vector<uint32_t>> b_STGL1A_pad_idleFlag = {};

	std::vector<std::vector<uint32_t>> b_STGL1A_merge_LUT_choiceSelection = {};
	std::vector<std::vector<uint32_t>> b_STGL1A_merge_nsw_segmentSelector = {};
	std::vector<std::vector<uint32_t>> b_STGL1A_merge_valid_segmentSelector = {};

    std::array<std::vector<std::vector<uint32_t>>, Muon::nsw::STGTPSegments::num_segments> b_STGL1A_merge_monitor_segments{};
    std::array<std::vector<std::vector<uint32_t>>, Muon::nsw::STGTPSegments::num_segments> b_STGL1A_merge_spare_segments{};
    std::array<std::vector<std::vector<uint32_t>>, Muon::nsw::STGTPSegments::num_segments> b_STGL1A_merge_lowRes_segments{};
    std::array<std::vector<std::vector<uint32_t>>, Muon::nsw::STGTPSegments::num_segments> b_STGL1A_merge_phiRes_segments{};
    std::array<std::vector<std::vector<uint32_t>>, Muon::nsw::STGTPSegments::num_segments> b_STGL1A_merge_dTheta_segments{};
    std::array<std::vector<std::vector<uint32_t>>, Muon::nsw::STGTPSegments::num_segments> b_STGL1A_merge_phiID_segments{};
    std::array<std::vector<std::vector<uint32_t>>, Muon::nsw::STGTPSegments::num_segments> b_STGL1A_merge_RIndex_segments{};

	std::vector<std::vector<uint32_t>> b_STGL1A_merge_BCID = {};
	std::vector<std::vector<uint32_t>> b_STGL1A_merge_sectorID = {};

	std::vector<uint32_t> b_STGL1A_trailer_CRC = {} ;

};

void test_nsw_trigger_common_decoder_help (char *progname)
{
	std::cout << "Usage: " << progname
		<< " [-h] [-n events] [-p] [-v] file1, file2, ..." << std::endl;
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
					params.max_events = static_cast <uint32_t> (strtol(argv[++i], NULL, 10));
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

int test_nsw_trigger_common_decoder_init_tree (TTree &outtree, outBranches &data)
{

	//creating always all branches... you never know which swROD modules are connected...
	//can change later if needed (but this is a test app so...)
	outtree.Branch( "MML1A_ROD_sourceID", &data.b_MML1A_ROD_sourceID);
	outtree.Branch( "MML1A_ROD_subdetID", &data.b_MML1A_ROD_subdetID);
	outtree.Branch( "MML1A_ROD_moduleID", &data.b_MML1A_ROD_moduleID);
	outtree.Branch( "MML1A_ROD_L1ID", &data.b_MML1A_ROD_L1ID);
	outtree.Branch( "MML1A_ROD_n_words", &data.b_MML1A_ROD_n_words);
	outtree.Branch( "MML1A_head_fragID", &data.b_MML1A_head_fragID);
	outtree.Branch( "MML1A_head_sectID", &data.b_MML1A_head_sectID);
	outtree.Branch( "MML1A_head_EC", &data.b_MML1A_head_EC);
	outtree.Branch( "MML1A_head_flags", &data.b_MML1A_head_flags);
	outtree.Branch( "MML1A_head_BCID", &data.b_MML1A_head_BCID);
	outtree.Branch( "MML1A_head_orbit", &data.b_MML1A_head_orbit);
	outtree.Branch( "MML1A_head_spare", &data.b_MML1A_head_spare);
	outtree.Branch( "MML1A_L1ID", &data.b_MML1A_L1ID);
	outtree.Branch( "MML1A_head_wdw_open", &data.b_MML1A_head_wdw_open);
	outtree.Branch( "MML1A_head_l1a_req", &data.b_MML1A_head_l1a_req);
	outtree.Branch( "MML1A_head_wdw_close", &data.b_MML1A_head_wdw_close);
	outtree.Branch( "MML1A_head_overflowCount", &data.b_MML1A_head_overflowCount);
	outtree.Branch( "MML1A_head_wdw_matching_engines_usage", &data.b_MML1A_head_wdw_matching_engines_usage);
	outtree.Branch( "MML1A_head_cfg_wdw_open_offset", &data.b_MML1A_head_cfg_wdw_open_offset);
	outtree.Branch( "MML1A_head_cfg_l1a_req_offset", &data.b_MML1A_head_cfg_l1a_req_offset);
	outtree.Branch( "MML1A_head_cfg_wdw_close_offset", &data.b_MML1A_head_cfg_wdw_close_offset);
	outtree.Branch( "MML1A_head_cfg_timeout", &data.b_MML1A_head_cfg_timeout);
	outtree.Branch( "MML1A_head_link_const", &data.b_MML1A_head_link_const);
	outtree.Branch( "MML1A_stream_head_nbits", &data.b_MML1A_stream_head_nbits);
	outtree.Branch( "MML1A_stream_head_nwords", &data.b_MML1A_stream_head_nwords);
	outtree.Branch( "MML1A_stream_head_fifo_size", &data.b_MML1A_stream_head_fifo_size);
	outtree.Branch( "MML1A_stream_head_streamID", &data.b_MML1A_stream_head_streamID);
	outtree.Branch( "MML1A_trailer_CRC", &data.b_MML1A_trailer_CRC);
	outtree.Branch( "MML1A_art_BCID", &data.b_MML1A_art_BCID);
	outtree.Branch( "MML1A_art_layers", &data.b_MML1A_art_layers);
	outtree.Branch( "MML1A_art_channels", &data.b_MML1A_art_channels);
	outtree.Branch( "MML1A_trig_BCID", &data.b_MML1A_trig_BCID);
	outtree.Branch( "MML1A_trig_dTheta", &data.b_MML1A_trig_dTheta);
	outtree.Branch( "MML1A_trig_phiBin", &data.b_MML1A_trig_phiBin);
	outtree.Branch( "MML1A_trig_rBin", &data.b_MML1A_trig_rBin);

	outtree.Branch("MMMon_ROD_sourceID", &data.b_MMMon_ROD_sourceID);
	outtree.Branch("MMMon_ROD_subdetID", &data.b_MMMon_ROD_subdetID);
	outtree.Branch("MMMon_ROD_moduleID", &data.b_MMMon_ROD_moduleID);
	outtree.Branch("MMMon_ROD_L1ID", &data.b_MMMon_ROD_L1ID);
	outtree.Branch("MMMon_ROD_n_words", &data.b_MMMon_ROD_n_words);
	outtree.Branch("MMMon_head_fragID", &data.b_MMMon_head_fragID);
	outtree.Branch("MMMon_head_sectID", &data.b_MMMon_head_sectID);
	outtree.Branch("MMMon_head_EC", &data.b_MMMon_head_EC);
	outtree.Branch("MMMon_head_flags", &data.b_MMMon_head_flags);
	outtree.Branch("MMMon_head_BCID", &data.b_MMMon_head_BCID);
	outtree.Branch("MMMon_head_orbit", &data.b_MMMon_head_orbit);
	outtree.Branch("MMMon_head_spare", &data.b_MMMon_head_spare);
	outtree.Branch("MMMon_L1ID", &data.b_MMMon_L1ID);
	outtree.Branch("MMMon_head_coincBCID", &data.b_MMMon_head_coincBCID);
	outtree.Branch("MMMon_head_regionCount", &data.b_MMMon_head_regionCount);
	outtree.Branch("MMMon_head_coincRegion", &data.b_MMMon_head_coincRegion);
	outtree.Branch("MMMon_head_reserved", &data.b_MMMon_head_reserved);
	outtree.Branch("MMMon_finder_streamID", &data.b_MMMon_finder_streamID);
	outtree.Branch("MMMon_finder_regionCount", &data.b_MMMon_finder_regionCount);
	outtree.Branch("MMMon_finder_triggerID", &data.b_MMMon_finder_triggerID);
	outtree.Branch("MMMon_finder_V1", &data.b_MMMon_finder_V1);
	outtree.Branch("MMMon_finder_V0", &data.b_MMMon_finder_V0);
	outtree.Branch("MMMon_finder_U1", &data.b_MMMon_finder_U1);
	outtree.Branch("MMMon_finder_U0", &data.b_MMMon_finder_U0);
	outtree.Branch("MMMon_finder_X3", &data.b_MMMon_finder_X3);
	outtree.Branch("MMMon_finder_X2", &data.b_MMMon_finder_X2);
	outtree.Branch("MMMon_finder_X1", &data.b_MMMon_finder_X1);
	outtree.Branch("MMMon_finder_X0", &data.b_MMMon_finder_X0);
	outtree.Branch("MMMon_fitter_streamID", &data.b_MMMon_fitter_streamID);
	outtree.Branch("MMMon_fitter_regionCount", &data.b_MMMon_fitter_regionCount);
	outtree.Branch("MMMon_fitter_triggerID", &data.b_MMMon_fitter_triggerID);
	outtree.Branch("MMMon_fitter_filler", &data.b_MMMon_fitter_filler);
	outtree.Branch("MMMon_fitter_mxG", &data.b_MMMon_fitter_mxG);
	outtree.Branch("MMMon_fitter_muG", &data.b_MMMon_fitter_muG);
	outtree.Branch("MMMon_fitter_mvG", &data.b_MMMon_fitter_mvG);
	outtree.Branch("MMMon_fitter_mxL", &data.b_MMMon_fitter_mxL);
	outtree.Branch("MMMon_fitter_mx_ROI", &data.b_MMMon_fitter_mx_ROI);
	outtree.Branch("MMMon_fitter_dTheta", &data.b_MMMon_fitter_dTheta);
	outtree.Branch("MMMon_fitter_zero", &data.b_MMMon_fitter_zero);
	outtree.Branch("MMMon_fitter_phiSign", &data.b_MMMon_fitter_phiSign);
	outtree.Branch("MMMon_fitter_phiBin", &data.b_MMMon_fitter_phiBin);
	outtree.Branch("MMMon_fitter_rBin", &data.b_MMMon_fitter_rBin);
	outtree.Branch("MMMon_trailer_CRC", &data.b_MMMon_trailer_CRC);

	outtree.Branch( "PadL1A_ROD_sourceID", &data.b_PadL1A_ROD_sourceID);
	outtree.Branch( "PadL1A_ROD_subdetID", &data.b_PadL1A_ROD_subdetID);
	outtree.Branch( "PadL1A_ROD_moduleID", &data.b_PadL1A_ROD_moduleID);
	outtree.Branch( "PadL1A_ROD_L1ID", &data.b_PadL1A_ROD_L1ID);
	outtree.Branch( "PadL1A_ROD_n_words", &data.b_PadL1A_ROD_n_words);
	outtree.Branch( "PadL1A_flags", &data.b_PadL1A_flags);
	outtree.Branch( "PadL1A_ec", &data.b_PadL1A_ec);
	outtree.Branch( "PadL1A_fragid", &data.b_PadL1A_fragid);
	outtree.Branch( "PadL1A_secid", &data.b_PadL1A_secid);
	outtree.Branch( "PadL1A_spare", &data.b_PadL1A_spare);
	outtree.Branch( "PadL1A_orbit", &data.b_PadL1A_orbit);
	outtree.Branch( "PadL1A_bcid", &data.b_PadL1A_bcid);
	outtree.Branch( "PadL1A_l1id", &data.b_PadL1A_l1id);
	outtree.Branch( "PadL1A_hit_n", &data.b_PadL1A_hit_n);
	outtree.Branch( "PadL1A_pfeb_n", &data.b_PadL1A_pfeb_n);
	outtree.Branch( "PadL1A_trigger_n", &data.b_PadL1A_trigger_n);
	outtree.Branch( "PadL1A_bcid_n", &data.b_PadL1A_bcid_n);
	outtree.Branch( "PadL1A_hit_relbcid", &data.b_PadL1A_hit_relbcid);
	outtree.Branch( "PadL1A_hit_pfeb", &data.b_PadL1A_hit_pfeb);
	outtree.Branch( "PadL1A_hit_tdschannel", &data.b_PadL1A_hit_tdschannel);
	outtree.Branch( "PadL1A_hit_vmmchannel", &data.b_PadL1A_hit_vmmchannel);
	outtree.Branch( "PadL1A_hit_vmm", &data.b_PadL1A_hit_vmm);
	outtree.Branch( "PadL1A_hit_padchannel", &data.b_PadL1A_hit_padchannel);
	outtree.Branch( "PadL1A_pfeb_addr", &data.b_PadL1A_pfeb_addr);
	outtree.Branch( "PadL1A_pfeb_nchan", &data.b_PadL1A_pfeb_nchan);
	outtree.Branch( "PadL1A_pfeb_disconnected", &data.b_PadL1A_pfeb_disconnected);
	outtree.Branch( "PadL1A_trigger_bandid", &data.b_PadL1A_trigger_bandid);
	outtree.Branch( "PadL1A_trigger_phiid", &data.b_PadL1A_trigger_phiid);
	outtree.Branch( "PadL1A_trigger_relbcid", &data.b_PadL1A_trigger_relbcid);
	outtree.Branch( "PadL1A_bcid_rel", &data.b_PadL1A_bcid_rel);
	outtree.Branch( "PadL1A_bcid_status", &data.b_PadL1A_bcid_status);
	outtree.Branch( "PadL1A_bcid_multzero", &data.b_PadL1A_bcid_multzero);

	outtree.Branch( "STGL1A_ROD_sourceID", &data.b_STGL1A_ROD_sourceID);
	outtree.Branch( "STGL1A_ROD_subdetID", &data.b_STGL1A_ROD_subdetID);
	outtree.Branch( "STGL1A_ROD_moduleID", &data.b_STGL1A_ROD_moduleID);
	outtree.Branch( "STGL1A_ROD_L1ID", &data.b_STGL1A_ROD_L1ID);
	outtree.Branch( "STGL1A_ROD_n_words", &data.b_STGL1A_ROD_n_words);
	outtree.Branch( "STGL1A_head_fragID", &data.b_STGL1A_head_fragID);
	outtree.Branch( "STGL1A_head_sectID", &data.b_STGL1A_head_sectID);
	outtree.Branch( "STGL1A_head_EC", &data.b_STGL1A_head_EC);
	outtree.Branch( "STGL1A_head_flags", &data.b_STGL1A_head_flags);
	outtree.Branch( "STGL1A_head_BCID", &data.b_STGL1A_head_BCID);
	outtree.Branch( "STGL1A_head_orbit", &data.b_STGL1A_head_orbit);
	outtree.Branch( "STGL1A_head_spare", &data.b_STGL1A_head_spare);
	outtree.Branch( "STGL1A_L1ID", &data.b_STGL1A_L1ID);
	outtree.Branch( "STGL1A_head_wdw_open", &data.b_STGL1A_head_wdw_open);
	outtree.Branch( "STGL1A_head_l1a_req", &data.b_STGL1A_head_l1a_req);
	outtree.Branch( "STGL1A_head_wdw_close", &data.b_STGL1A_head_wdw_close);
	outtree.Branch( "STGL1A_head_overflowCount", &data.b_STGL1A_head_overflowCount);
	outtree.Branch( "STGL1A_head_wdw_matching_engines_usage", &data.b_STGL1A_head_wdw_matching_engines_usage);
	outtree.Branch( "STGL1A_head_cfg_wdw_open_offset", &data.b_STGL1A_head_cfg_wdw_open_offset);
	outtree.Branch( "STGL1A_head_cfg_l1a_req_offset", &data.b_STGL1A_head_cfg_l1a_req_offset);
	outtree.Branch( "STGL1A_head_cfg_wdw_close_offset", &data.b_STGL1A_head_cfg_wdw_close_offset);
	outtree.Branch( "STGL1A_head_cfg_timeout", &data.b_STGL1A_head_cfg_timeout);
	outtree.Branch( "STGL1A_head_link_const", &data.b_STGL1A_head_link_const);
	outtree.Branch( "STGL1A_stream_head_nbits", &data.b_STGL1A_stream_head_nbits);
	outtree.Branch( "STGL1A_stream_head_nwords", &data.b_STGL1A_stream_head_nwords);
	outtree.Branch( "STGL1A_stream_head_fifo_size", &data.b_STGL1A_stream_head_fifo_size);
	outtree.Branch( "STGL1A_stream_head_streamID", &data.b_STGL1A_stream_head_streamID);

	outtree.Branch( "STGL1A_pad_coincidence_wedge", &data.b_STGL1A_pad_coincidence_wedge);
	outtree.Branch( "STGL1A_pad_phiID_3", &data.b_STGL1A_pad_phiID_3);
	outtree.Branch( "STGL1A_pad_phiID_2", &data.b_STGL1A_pad_phiID_2);
	outtree.Branch( "STGL1A_pad_phiID_1", &data.b_STGL1A_pad_phiID_1);
	outtree.Branch( "STGL1A_pad_phiID_0", &data.b_STGL1A_pad_phiID_0);

	outtree.Branch( "STGL1A_pad_bandID_3", &data.b_STGL1A_pad_bandID_3);
	outtree.Branch( "STGL1A_pad_bandID_2", &data.b_STGL1A_pad_bandID_2);
	outtree.Branch( "STGL1A_pad_bandID_1", &data.b_STGL1A_pad_bandID_1);
	outtree.Branch( "STGL1A_pad_bandID_0", &data.b_STGL1A_pad_bandID_0);

	outtree.Branch( "STGL1A_pad_BCID", &data.b_STGL1A_pad_BCID);
	outtree.Branch( "STGL1A_pad_idleFlag", &data.b_STGL1A_pad_idleFlag);

	outtree.Branch( "STGL1A_merge_LUT_choiceSelection", &data.b_STGL1A_merge_LUT_choiceSelection);
	outtree.Branch( "STGL1A_merge_nsw_segmentSelector", &data.b_STGL1A_merge_nsw_segmentSelector);
	outtree.Branch( "STGL1A_merge_valid_segmentSelector", &data.b_STGL1A_merge_valid_segmentSelector);

   for (std::size_t i=0; i < Muon::nsw::STGTPSegments::num_segments; ++i) {
     outtree.Branch( Muon::format("STGL1A_merge_monitor_segment{}", i).c_str(), &data.b_STGL1A_merge_monitor_segments.at(i));
     outtree.Branch( Muon::format("STGL1A_merge_spare_segment{}", i).c_str(), &data.b_STGL1A_merge_spare_segments.at(i));
     outtree.Branch( Muon::format("STGL1A_merge_lowRes_segment{}", i).c_str(), &data.b_STGL1A_merge_lowRes_segments.at(i));
     outtree.Branch( Muon::format("STGL1A_merge_phiRes_segment{}", i).c_str(), &data.b_STGL1A_merge_phiRes_segments.at(i));
     outtree.Branch( Muon::format("STGL1A_merge_dTheta_segment{}", i).c_str(), &data.b_STGL1A_merge_dTheta_segments.at(i));
     outtree.Branch( Muon::format("STGL1A_merge_phiID_segment{}", i).c_str(), &data.b_STGL1A_merge_phiID_segments.at(i));
     outtree.Branch( Muon::format("STGL1A_merge_RIndex_segment{}", i).c_str(), &data.b_STGL1A_merge_RIndex_segments.at(i));
   }

	outtree.Branch( "STGL1A_merge_BCID", &data.b_STGL1A_merge_BCID);
	outtree.Branch( "STGL1A_merge_sectorID", &data.b_STGL1A_merge_sectorID);

	return 0;
}

int test_nsw_trigger_common_decoder_end (Statistics &statistics)
{
	std::cout << "Total event processed             = " << statistics.nevents << std::endl;
	std::cout << "Total event not decoded           = " << statistics.nevents_not_decoded << std::endl;
	std::cout << "Total event with flx elink errors = " << statistics.nevents_sus_felix_stat << std::endl;
	std::cout << "Total fill time                   = " << statistics.total_fill_time << std::endl;
	std::cout << "Total write time                  = " << statistics.total_write_time << std::endl;
	std::cout << "Total decoding time (ms)          = " << statistics.total_decoding_time << std::endl;
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
		bool is_nsw = false, is_mmg = false, is_stg = false;
		bool is_tp = false, is_pt = false, is_l1a = false, is_mon = false; //is_l1a and is_mon are actuall

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
		if      ((m & 0xf0) == 0x10) {is_tp = true; is_l1a = true;}
		else if ((m & 0xf0) == 0x20) {is_pt = true; is_l1a = true;}
		else if ((m & 0xf0) == 0x50) {is_tp = true; is_mon = true;}


		if (params.printout_level > 1 && is_nsw){
			std::cout << "ROB source ID                = 0x" << std::hex << sid << std::dec << std::endl;
			std::cout << "ROB subdet ID                = 0x" << std::hex << s   << std::dec << std::endl;
			std::cout << "ROB module ID                = 0x" << std::hex << m   << std::dec << std::endl;
			//std::cout << "is_nsw                = " << (is_nsw?"Yes":"No") << std::endl;
			std::cout << "is_mmg                = " << (is_mmg?"Yes":"No") << std::endl;
			std::cout << "is_stg                = " << (is_stg?"Yes":"No") << std::endl;
			std::cout << "is_pt                 = " << (is_pt?"Yes":"No") << std::endl;
			std::cout << "is_tp                 = " << (is_tp?"Yes":"No") << std::endl;
			std::cout << "is_l1a                = " << (is_l1a?"Yes":"No") << std::endl;
			std::cout << "is_mon                = " << (is_mon?"Yes":"No") << std::endl;
		}


		if (is_nsw && (is_pt || is_tp)){
			if (params.printout_level > 0 || params.print_only){
				std::cout << "NSW Trigger fragment found: length " << r->rod_ndata () << std::endl;
			}

			const uint32_t *bs = r->rod_data ();

			// Print out raw fragment
			if (params.print_only){
				std::cout << "ROD Fragment size in words:" << std::endl;
				std::cout << "ROD Total: " << r->rod_fragment_size_word () << std::endl;
				std::cout << "ROD Header: " << r->rod_header_size_word () << std::endl;
				std::cout << "ROD Trailer: " << r->rod_trailer_size_word () << std::endl;
				std::cout << "ROD L1 ID: " << std::hex << r->rod_lvl1_id () << std::dec << std::endl;
				std::cout << "ROD Data words: " << r->rod_ndata () << std::endl;

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
				std::string robType = std::string(is_pt?"Pad":(is_mmg?"MM":"STG")) + std::string(is_l1a?"L1A":(is_mon?"Mon":""));
				std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now ();
				Muon::nsw::NSWTriggerCommonDecoder nsw_trigger_decoder (*r, robType);
				std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now ();
				unsigned int time_elapsed = std::chrono::duration_cast <std::chrono::microseconds> (end - begin).count ();
				float time_elapsed_ms = static_cast <float> (time_elapsed) / 1000;
				if (params.printout_level > 1){
					std::cout << "Time for decoding this event (ms): " << time_elapsed_ms << std::endl;
					std::cout << std::endl;
				}
				statistics.total_decoding_time += time_elapsed_ms;

				if (robType=="PadL1A"){
					for (size_t i = 0; i < nsw_trigger_decoder.get_elinks().size(); ++i) {
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
				}
				if (robType=="MML1A") {
					for (size_t i = 0; i < nsw_trigger_decoder.get_elinks().size(); ++i) {
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
						std::vector<uint32_t> tmp_art_BCIDs;
						std::vector<uint32_t> tmp_art_layers;
						std::vector<uint32_t> tmp_art_channels;
						for (auto art : arts){
							for (uint i = 0; i < art->channels().size(); ++i){
								//so that there's a timestamp per channel
								tmp_art_layers.push_back( art->channels()[i].first );
								tmp_art_channels.push_back( art->channels()[i].second );
								tmp_art_BCIDs.push_back( art->art_BCID() );
							}
						}
						data.b_MML1A_art_BCID.push_back( tmp_art_BCIDs );
						data.b_MML1A_art_layers.push_back( tmp_art_layers );
						data.b_MML1A_art_channels.push_back( tmp_art_channels );

						const std::vector<std::shared_ptr<Muon::nsw::MMTrigPacket>>& trigs = link->trig_packets();
						std::vector<uint32_t> tmp_trig_BCID;
						std::vector<uint32_t> tmp_trig_dTheta;
						std::vector<uint32_t> tmp_trig_phiBin;
						std::vector<uint32_t> tmp_trig_rBin;
						for (auto trig : trigs){
							tmp_trig_BCID.push_back( trig->trig_BCID() );
							tmp_trig_dTheta.push_back( trig->trig_dTheta() );
							tmp_trig_phiBin.push_back( trig->trig_phiBin() );
							tmp_trig_rBin.push_back( trig->trig_rBin() );
						}
						data.b_MML1A_trig_BCID.push_back( tmp_trig_BCID );
						data.b_MML1A_trig_dTheta.push_back( tmp_trig_dTheta );
						data.b_MML1A_trig_phiBin.push_back( tmp_trig_phiBin );
						data.b_MML1A_trig_rBin.push_back( tmp_trig_rBin );

						data.b_MML1A_trailer_CRC.push_back( link->trailer_CRC() );

					}
				}
				if (robType=="MMMon") {
					for (size_t i = 0; i < nsw_trigger_decoder.get_elinks().size(); ++i) {
						std::shared_ptr<Muon::nsw::NSWTriggerMMMonElink> link = std::dynamic_pointer_cast<Muon::nsw::NSWTriggerMMMonElink>(nsw_trigger_decoder.get_elinks()[i]);
						data.b_MMMon_ROD_sourceID.push_back( sid );
						data.b_MMMon_ROD_subdetID.push_back( s );
						data.b_MMMon_ROD_moduleID.push_back( m );
						data.b_MMMon_ROD_L1ID.push_back( r->rod_lvl1_id () );
						data.b_MMMon_ROD_n_words.push_back( r->rod_ndata () );
						data.b_MMMon_head_fragID.push_back( link->head_fragID() );
						data.b_MMMon_head_sectID.push_back( link->head_sectID() );
						data.b_MMMon_head_EC.push_back( link->head_EC() );
						data.b_MMMon_head_flags.push_back( link->head_flags() );
						data.b_MMMon_head_BCID.push_back( link->head_BCID() );
						data.b_MMMon_head_orbit.push_back( link->head_orbit() );
						data.b_MMMon_head_spare.push_back( link->head_spare() );
						data.b_MMMon_L1ID.push_back( link->L1ID() );
						data.b_MMMon_head_coincBCID.push_back( link->head_coincBCID() );
						data.b_MMMon_head_regionCount.push_back( link->head_regionCount() );
						data.b_MMMon_head_coincRegion.push_back( link->head_coincRegion() );
						data.b_MMMon_head_reserved.push_back( link->head_reserved() );
						data.b_MMMon_finder_streamID.push_back( link->finder_streamID() );
						data.b_MMMon_finder_regionCount.push_back( link->finder_regionCount() );
						data.b_MMMon_finder_triggerID.push_back( link->finder_triggerID() );
						data.b_MMMon_finder_V1.push_back( link->finder_V1() );
						data.b_MMMon_finder_V0.push_back( link->finder_V0() );
						data.b_MMMon_finder_U1.push_back( link->finder_U1() );
						data.b_MMMon_finder_U0.push_back( link->finder_U0() );
						data.b_MMMon_finder_X3.push_back( link->finder_X3() );
						data.b_MMMon_finder_X2.push_back( link->finder_X2() );
						data.b_MMMon_finder_X1.push_back( link->finder_X1() );
						data.b_MMMon_finder_X0.push_back( link->finder_X0() );
						data.b_MMMon_fitter_streamID.push_back( link->fitter_streamID() );
						data.b_MMMon_fitter_regionCount.push_back( link->fitter_regionCount() );
						data.b_MMMon_fitter_triggerID.push_back( link->fitter_triggerID() );
						data.b_MMMon_fitter_filler.push_back( link->fitter_filler() );
						data.b_MMMon_fitter_mxG.push_back( link->fitter_mxG() );
						data.b_MMMon_fitter_muG.push_back( link->fitter_muG() );
						data.b_MMMon_fitter_mvG.push_back( link->fitter_mvG() );
						data.b_MMMon_fitter_mxL.push_back( link->fitter_mxL() );
						data.b_MMMon_fitter_mx_ROI.push_back( link->fitter_mx_ROI() );
						data.b_MMMon_fitter_dTheta.push_back( link->fitter_dTheta() );
						data.b_MMMon_fitter_zero.push_back( link->fitter_zero() );
						data.b_MMMon_fitter_phiSign.push_back( link->fitter_phiSign() );
						data.b_MMMon_fitter_phiBin.push_back( link->fitter_phiBin() );
						data.b_MMMon_fitter_rBin.push_back( link->fitter_rBin() );
						data.b_MMMon_trailer_CRC.push_back( link->trailer_CRC() );
					}
				}


				if (robType=="STGL1A") {
					// resize vectors
					unsigned int n_elinks = nsw_trigger_decoder.get_elinks().size();
					data.b_STGL1A_pad_BCID.resize(n_elinks);
					data.b_STGL1A_pad_bandID_0.resize(n_elinks);
					data.b_STGL1A_pad_bandID_1.resize(n_elinks);
					data.b_STGL1A_pad_bandID_2.resize(n_elinks);
					data.b_STGL1A_pad_bandID_3.resize(n_elinks);
					data.b_STGL1A_pad_phiID_0.resize(n_elinks);
					data.b_STGL1A_pad_phiID_1.resize(n_elinks);
					data.b_STGL1A_pad_phiID_2.resize(n_elinks);
					data.b_STGL1A_pad_phiID_3.resize(n_elinks);
					data.b_STGL1A_pad_coincidence_wedge.resize(n_elinks);
					data.b_STGL1A_pad_idleFlag.resize(n_elinks);
					data.b_STGL1A_merge_LUT_choiceSelection.resize(n_elinks);
					data.b_STGL1A_merge_nsw_segmentSelector.resize(n_elinks);
					data.b_STGL1A_merge_valid_segmentSelector.resize(n_elinks);

					const auto resize_segment = [&n_elinks] (auto& segments) {
						for (auto& segment : segments) {
							segment.resize(n_elinks);
						}
					};
				
					resize_segment(data.b_STGL1A_merge_monitor_segments);
					resize_segment(data.b_STGL1A_merge_spare_segments);
					resize_segment(data.b_STGL1A_merge_lowRes_segments);
					resize_segment(data.b_STGL1A_merge_phiRes_segments);
					resize_segment(data.b_STGL1A_merge_dTheta_segments);
					resize_segment(data.b_STGL1A_merge_phiID_segments);
					resize_segment(data.b_STGL1A_merge_RIndex_segments);

					data.b_STGL1A_merge_BCID.resize(n_elinks);
					data.b_STGL1A_merge_sectorID.resize(n_elinks);

					for (size_t i = 0; i < nsw_trigger_decoder.get_elinks().size(); ++i) {

						std::shared_ptr<Muon::nsw::NSWTriggerSTGL1AElink> link = std::dynamic_pointer_cast<Muon::nsw::NSWTriggerSTGL1AElink>(nsw_trigger_decoder.get_elinks()[i]);
						data.b_STGL1A_ROD_sourceID.push_back( sid );
						data.b_STGL1A_ROD_subdetID.push_back( s );
						data.b_STGL1A_ROD_moduleID.push_back( m );
						data.b_STGL1A_ROD_L1ID.push_back( 0 );
						data.b_STGL1A_ROD_n_words.push_back( 0 );
						data.b_STGL1A_head_fragID.push_back( link->head_fragID() );
						data.b_STGL1A_head_sectID.push_back( link->head_sectID() );
						data.b_STGL1A_head_EC.push_back( link->head_EC() );
						data.b_STGL1A_head_flags.push_back( link->head_flags() );
						data.b_STGL1A_head_BCID.push_back( link->head_BCID() );
						data.b_STGL1A_head_orbit.push_back( link->head_orbit() );
						data.b_STGL1A_head_spare.push_back( link->head_spare() );
						data.b_STGL1A_L1ID.push_back( link->L1ID() );
						data.b_STGL1A_head_wdw_open.push_back( link->head_wdw_open() );
						data.b_STGL1A_head_l1a_req.push_back( link->head_l1a_req() );
						data.b_STGL1A_head_wdw_close.push_back( link->head_wdw_close() );
						data.b_STGL1A_head_overflowCount.push_back( link->head_overflowCount() );
						data.b_STGL1A_head_wdw_matching_engines_usage.push_back( link->head_wdw_matching_engines_usage() );
						data.b_STGL1A_head_cfg_wdw_open_offset.push_back( link->head_cfg_wdw_open_offset() );
						data.b_STGL1A_head_cfg_l1a_req_offset.push_back( link->head_cfg_l1a_req_offset() );
						data.b_STGL1A_head_cfg_wdw_close_offset.push_back( link->head_cfg_wdw_close_offset() );
						data.b_STGL1A_head_cfg_timeout.push_back( link->head_cfg_timeout() );
						data.b_STGL1A_head_link_const.push_back( link->head_link_const() );
						data.b_STGL1A_stream_head_nbits.push_back( link->stream_head_nbits() );
						data.b_STGL1A_stream_head_nwords.push_back( link->stream_head_nwords() );
						data.b_STGL1A_stream_head_fifo_size.push_back( link->stream_head_fifo_size() );
						data.b_STGL1A_stream_head_streamID.push_back( link->stream_head_streamID() );

						// pad block
						const auto& pad_packets = link-> pad_packets();

						for (auto packet : pad_packets)
						{
							data.b_STGL1A_pad_BCID[i].push_back(packet.BCID());
							data.b_STGL1A_pad_bandID_0[i].push_back(packet.BandID(0));
							data.b_STGL1A_pad_bandID_1[i].push_back(packet.BandID(1));
							data.b_STGL1A_pad_bandID_2[i].push_back(packet.BandID(2));
							data.b_STGL1A_pad_bandID_3[i].push_back(packet.BandID(3));

							data.b_STGL1A_pad_phiID_0[i].push_back(packet.PhiID(0));
							data.b_STGL1A_pad_phiID_1[i].push_back(packet.PhiID(1));
							data.b_STGL1A_pad_phiID_2[i].push_back(packet.PhiID(2));
							data.b_STGL1A_pad_coincidence_wedge[i].push_back(packet.CoincidenceWedge());
							data.b_STGL1A_pad_phiID_3[i].push_back(packet.PhiID(3));

							data.b_STGL1A_pad_idleFlag[i].push_back(packet.PadIdleFlag());

						}

						const auto& segment_packets = link-> segment_packet();


					        for (auto packet: segment_packets) {
							data.b_STGL1A_merge_LUT_choiceSelection[i].push_back(packet.LUT_ChoiceSelection());
							data.b_STGL1A_merge_nsw_segmentSelector[i].push_back(packet.NSW_SegmentSelector());
							data.b_STGL1A_merge_valid_segmentSelector[i].push_back(packet.ValidSegmentSelector());

							

							for (std::size_t i_seg=0; i_seg < Muon::nsw::STGTPSegments::num_segments; ++i_seg) {
								data.b_STGL1A_merge_monitor_segments.at(i_seg).at(i).push_back(packet.Segment(i_seg).monitor);
								data.b_STGL1A_merge_spare_segments.at(i_seg).at(i).push_back(packet.Segment(i_seg).spare);
								data.b_STGL1A_merge_lowRes_segments.at(i_seg).at(i).push_back(packet.Segment(i_seg).lowRes);
								data.b_STGL1A_merge_phiRes_segments.at(i_seg).at(i).push_back(packet.Segment(i_seg).phiRes);
								data.b_STGL1A_merge_dTheta_segments.at(i_seg).at(i).push_back(packet.Segment(i_seg).dTheta);
								data.b_STGL1A_merge_phiID_segments.at(i_seg).at(i).push_back(packet.Segment(i_seg).phiID);
								data.b_STGL1A_merge_RIndex_segments.at(i_seg).at(i).push_back(packet.Segment(i_seg).rIndex);
							}

							data.b_STGL1A_merge_BCID[i].push_back(packet.BCID());
							data.b_STGL1A_merge_sectorID[i].push_back(packet.SectorID());


						} // end of merge packets
					} // end of stgc elink loop
				} // end of stgc l1a block
			} //not print only
		}
	} 
	//end for loop on robs

	return 0;
}


int test_nsw_trigger_common_decoder_loop (Params &params, Statistics &statistics)
{
	outBranches data;

	bool anyBrokenFile = false;
	//int err;

	for (const std::string &filename : params.file_names)
	{
		TFile* outfile = nullptr;
		TTree* outtree = nullptr; //no need for a smart pointer for ttrees, given root behaviour (deleting ttree's when closing files)

		std::string data_file_name (filename);
		std::string out_file_name = data_file_name.substr(data_file_name.find_last_of("/\\") + 1) + ".decoded.root";

		std::cout << "Reading file " << data_file_name << std::endl;
		if (!params.print_only) {
			std::cout << "Saving here file " << out_file_name << std::endl;
			outfile = new TFile(out_file_name.c_str(), "recreate"); 
			outtree = new TTree("decoded_data", "decoded_data");
			test_nsw_trigger_common_decoder_init_tree (*outtree, data);
		}

		std::unique_ptr <DataReader> reader (pickDataReader (data_file_name));

		if (!reader || !reader->good ())
		{
			std::cout << "Cannot open this file properly; skipping to next one" << std::endl;
			anyBrokenFile = true;
		}
		else {
			while (!reader->endOfFile () && (params.max_events == 0 || statistics.nevents < params.max_events))
			{
				char *buf = nullptr;
				unsigned int size = 0;

				try
				{
					DRError err = reader->getData (size, &buf);

					if (err != EventStorage::DROK)
					{
						std::cout << "Cannot get data properly from file" << data_file_name.c_str () << "; skipping it!" << std::endl;
						anyBrokenFile = true;
						if (buf) delete [] buf;
						break;
					}

					eformat::read::FullEventFragment f ((unsigned int *) buf);
					f.check ();

					//reset branches
					data = outBranches();

					if ( test_nsw_trigger_common_decoder_event (f, data, params, statistics) )
					{
						std::cout << "Cannot decode properly event " << statistics.nevents << "; skipping it!" <<std::endl;
						if (buf) delete [] buf;
						continue;
					}


					if (!params.print_only) {
						std::chrono::steady_clock::time_point start= std::chrono::steady_clock::now ();
						outtree->Fill();
						std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now ();
						unsigned int time_elapsed = std::chrono::duration_cast <std::chrono::microseconds> (end - start).count ();
						statistics.total_fill_time += time_elapsed;
					}
					++statistics.nevents;
				}

				catch (std::runtime_error& error)
				{
					std::cout << "Exception!" << std::endl;
					std::cout << error.what() << std::endl;
					if (buf) delete [] buf;
					break;
				}

				if (buf) delete [] buf;
			}
		}

		if (!params.print_only) {
			std::chrono::steady_clock::time_point start= std::chrono::steady_clock::now ();
			outtree->Write();
			std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now ();
			unsigned int time_elapsed = std::chrono::duration_cast <std::chrono::microseconds> (end - start).count ();
			statistics.total_write_time += time_elapsed;
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
