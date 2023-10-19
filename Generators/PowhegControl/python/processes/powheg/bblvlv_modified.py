# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import Logging

from ..powheg_RES import PowhegRES

## Get handle to Athena logging
logger = Logging.logging.getLogger("PowhegControl")


class bblvlv_modified(PowhegRES):
    """! Default Powheg configuration for top pair and Wt production including non-resonant and interference effects.
    This is the modified version that contains all dilepton decay channels in one sample.
    Create a configurable object with all applicable Powheg options.

    @author Andrej Saibel <Andrej.Saibel@cern.ch>
    """

    def __init__(self, base_directory, **kwargs):
        """! Constructor: all process options are set here.

        @param base_directory: path to PowhegBox code.
        @param kwargs          dictionary of arguments from Generate_tf.
        """
        super(bblvlv_modified, self).__init__(base_directory, "b_bbar_4l_modified", **kwargs)

        # This is a hacky fix that's needed at the moment...
        self.manually_set_openloops_paths()

        # Add parameter validation functions
        self.validation_functions.append("validate_decays")

        ## List of allowed decay modes
        self.allowed_decay_modes = ["b mu+ vmu b~ e- ve~", "b e+ ve b~ mu- vmu~", "b emu+ vemu b~ emu- vemu~",\
                                    "b tau+ vtau b~ e- ve~", "b e+ ve b~ tau- vtau~", \
                                    "b mu+ vmu b~ tau- vtau~", "b tau+ vtau b~ mu- vmu~", "b l+ vl b~ l- vl~"]

        # Add all keywords for this process, overriding defaults if required
        self.add_keyword("allrad", 1)
        self.add_keyword("alpha")
        self.add_keyword("bmass")
        self.add_keyword("bornktmin")
        self.add_keyword("bornonly")
        self.add_keyword("bornsuppfact")
        self.add_keyword("bornzerodamp")
        self.add_keyword("bottomthr")
        self.add_keyword("bottomthrpdf")
        self.add_keyword("btildeborn")
        self.add_keyword("btildecoll")
        self.add_keyword("btildereal")
        self.add_keyword("btildevirt")
        self.add_keyword("btlscalect")
        self.add_keyword("btlscalereal")
        self.add_keyword("charmthr")
        self.add_keyword("charmthrpdf")
        self.add_keyword("chklimseed")
        self.add_keyword("clobberlhe")
        self.add_keyword("colltest")
        self.add_keyword("complexGFermi")
        self.add_keyword("compress_lhe")
        self.add_keyword("compress_upb")
        self.add_keyword("compute_rwgt")
        self.add_keyword("doublefsr")
        self.add_keyword("enhancereg")
        self.add_keyword("evenmaxrat")
        self.add_keyword("ewscheme")
        self.add_keyword("facscfact", self.default_scales[0])
        self.add_keyword("fastbtlbound")
        self.add_keyword("fixedscale")
        self.add_keyword("flg_debug")
        self.add_keyword("foldcsi", 2)
        self.add_keyword("foldphi", 5)
        self.add_keyword("foldy", 5)
        self.add_keyword("for_reweighting")
        self.add_keyword("fullrwgt")
        self.add_keyword("hdamp", 172.5)
        self.add_keyword("hfact")
        self.add_keyword("hmass")
        self.add_keyword("hwidth")
        self.add_keyword("icsimax")
        self.add_keyword("ih1")
        self.add_keyword("ih2")
        self.add_keyword("itmx1")
        self.add_keyword("itmx1btl")
        self.add_keyword("itmx1btlbrn")
        self.add_keyword("itmx1rm")
        self.add_keyword("itmx2", 6)
        self.add_keyword("itmx2btl")
        self.add_keyword("itmx2btlbrn")
        self.add_keyword("itmx2rm")
        self.add_keyword("iupperfsr")
        self.add_keyword("iupperisr")
        self.add_keyword("iymax")
        self.add_keyword("lhans1", self.default_PDFs)
        self.add_keyword("lhans2", self.default_PDFs)
        self.add_keyword("lhrwgt_descr")
        self.add_keyword("lhrwgt_group_combine")
        self.add_keyword("lhrwgt_group_name")
        self.add_keyword("lhrwgt_id")
        self.add_keyword("LOevents")
        self.add_keyword("manyseeds")
        self.add_keyword("max_io_bufsize")
        self.add_keyword("maxseeds")
        self.add_keyword("minlo")
        self.add_keyword("mint_density_map")
        self.add_keyword("mintupbratlim")
        self.add_keyword("MSbarscheme")
        self.add_keyword("ncall1", 120000)
        self.add_keyword("ncall1btl")
        self.add_keyword("ncall1btlbrn")
        self.add_keyword("ncall1rm")
        self.add_keyword("ncall2", 180000)
        self.add_keyword("ncall2btl")
        self.add_keyword("ncall2btlbrn")
        self.add_keyword("ncall2rm")
        self.add_keyword("ncallfrominput")
        self.add_keyword("noevents")
        self.add_keyword("nores")
        self.add_keyword("novirtual")
        self.add_keyword("nubound", 100000)
        self.add_keyword("olpreset")
        self.add_keyword("olverbose")
        self.add_keyword("openloops-stability")
        self.add_keyword("openloopsreal")
        self.add_keyword("openloopsvirtual")
        self.add_keyword("par_2gsupp")
        self.add_keyword("par_diexp")
        self.add_keyword("par_dijexp")
        self.add_keyword("parallelstage")
        self.add_keyword("pdfreweight")
        self.add_keyword("ptsqmin")
        self.add_keyword("ptsupp")
        self.add_keyword("radregion")
        self.add_keyword("rand1")
        self.add_keyword("rand2")
        self.add_keyword("regridfix")
        self.add_keyword("renscfact", self.default_scales[1])
        self.add_keyword("rwl_add")
        self.add_keyword("rwl_file")
        self.add_keyword("rwl_format_rwgt")
        self.add_keyword("rwl_group_events")
        self.add_keyword("smartsig")
        self.add_keyword("softmismch")
        self.add_keyword("softonly")
        self.add_keyword("softtest")
        self.add_keyword("stage2init")
        self.add_keyword("storeinfo_rwgt")
        self.add_keyword("storemintupb")
        self.add_keyword("testplots")
        self.add_keyword("testsuda")
        self.add_keyword("tmass_phsp")
        self.add_keyword("tmass")
        self.add_keyword("twidth",-1)
        self.add_keyword("twidth_phsp",-1)
        self.add_keyword("ubexcess_correct")
        self.add_keyword("ubsigmadetails", -1)  # disable cross-section output to avoid Fortran crash
        self.add_keyword("use-old-grid")
        self.add_keyword("use-old-ubound")
        self.add_keyword("user_reshists_sep")
        self.add_keyword("verytinypars")
        self.add_keyword("virtonly")
        self.add_keyword("whichpwhgevent")
        self.add_keyword("withbtilde")
        self.add_keyword("withdamp", 1)
        self.add_keyword("withnegweights")
        self.add_keyword("withremnants")
        self.add_keyword("withsubtr")
        self.add_keyword("wmass")
        self.add_keyword("wwidth")
        self.add_keyword("xgriditeration")
        self.add_keyword("xupbound", 2)
        self.add_keyword("zerowidth")
        self.add_keyword("zmass")
        self.add_keyword("zwidth")
        self.add_keyword("channel", self.allowed_decay_modes[7], name="decay_mode")

    def validate_decays(self):
        """! Validate decay_mode keyword."""
        self.expose()  # convenience call to simplify syntax
        if self.decay_mode not in self.allowed_decay_modes:
            logger.warning("Decay mode {} not recognised!".format(self.decay_mode))
            raise ValueError("Decay mode {} not recognised!".format(self.decay_mode))
        # Calculate appropriate decay mode numbers

        __decay_mode_lookup = {"b mu+ vmu b~ e- ve~" : 0, "b e+ ve b~ mu- vmu~" : 1, "b emu+ vemu b~ emu- vemu~" : 2,\
                               "b tau+ vtau b~ e- ve~" : 3, "b e+ ve b~ tau- vtau~" : 4, \
                               "b mu+ vmu b~ tau- vtau~" : 5, "b tau+ vtau b~ mu- vmu~" : 6, "b l+ vl b~ l- vl~" : 7}
        self.parameters_by_keyword("channel")[0].value = __decay_mode_lookup[self.decay_mode]
