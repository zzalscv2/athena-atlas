# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from collections import namedtuple
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

from ..Base.TopoAlgos import EMMultiplicityAlgo, TauMultiplicityAlgo, JetMultiplicityAlgo, XEMultiplicityAlgo, LArSaturationAlgo
from ..Base.TopoAlgorithms import AlgType, AlgCategory

class TopoAlgoDefMultiplicity(object):
    """
    Defines the TopoAlgorithms that calculate multiplicities for L1Calo thresholds
    The thresholds have to be explicitly defined here.
    """
    @staticmethod
    def registerTopoAlgos(tm):

        emThresholds_3bits = [
            'eEM5', 'eEM7', 'eEM9', 'eEM10L', 
        ]
        emThresholds_2bits = [
            'eEM12L', 'eEM15', 'eEM18',  'eEM18L', 'eEM18M', 'eEM22M',
            'eEM24L', 
            #ATR-26979, replace original eEMSPARE1 with eEM1, replace original eEMSPARE2 with eEM2, decrement other eEMSPARE thresholds
            'eEM1', 'eEM2',
            # spares
            'eEMSPARE1',
        ]
        emVarThresholds_2bits = [
            'eEM24VM',  'eEM26',  'eEM26L', 'eEM26M', 'eEM26T', 'eEM28M',
            # spares
            'eEMSPARE2', 'eEMSPARE3', 'eEMSPARE4', 'eEMSPARE5',
        ]

        for em in emThresholds_3bits:
            alg = EMMultiplicityAlgo( name = em,
                                      threshold = em,
                                      nbits = 3, classtype='eEmMultiplicity')
            tm.registerTopoAlgo(alg)

        for em in emThresholds_2bits:
            alg = EMMultiplicityAlgo( name = em,
                                      threshold = em,
                                      nbits = 2, classtype='eEmMultiplicity')
            tm.registerTopoAlgo(alg)

        for em in emVarThresholds_2bits:
            alg = EMMultiplicityAlgo( name = em,
                                      threshold = em,
                                      nbits = 2, classtype='eEmVarMultiplicity')
            tm.registerTopoAlgo(alg)

        emThresholds_2bits = [ 
            'jEM20', 'jEM20M', 
  
            #spares
            'jEMSPARE1', 
        ]
        for em in emThresholds_2bits:
            alg = EMMultiplicityAlgo( name = em,
                                      threshold = em, 
                                      nbits = 2, classtype='jEmMultiplicity')
            tm.registerTopoAlgo(alg)
                
        etauThresholds_3bits = [ 
            'eTAU12', 'eTAU20',    

            # spares
            'eTAUSPARE1', 
        ]
        jtauThresholds_3bits = [ 
            'jTAU20'
        ]        
        ctauThresholds_3bits = [ 
            'cTAU12M', 'cTAU20M', 

            #spares
            'cTAUSPARE1',
        ]
        etauThresholds_2bits = [ 
            'eTAU20L', 'eTAU20M', 'eTAU30', 'eTAU35', 'eTAU40HM', 'eTAU60', 'eTAU80', 'eTAU140', 
 
            #spares
            'eTAUSPARE2', 'eTAUSPARE3', 'eTAUSPARE4', 'eTAUSPARE5', 'eTAUSPARE6', 'eTAUSPARE7',
        ]
        jtauThresholds_2bits = [ 
            'jTAU30', 'jTAU30M',

            #spares
            'jTAUSPARE1',
        ]
        ctauThresholds_2bits = [ 
            'cTAU30M', 'cTAU35M',  

            # spares
            'cTAUSPARE2',
        ]

        for tau in etauThresholds_3bits:
            alg = TauMultiplicityAlgo( name = tau,
                                       threshold = tau, 
                                       nbits = 3, classtype='eTauMultiplicity')
            tm.registerTopoAlgo(alg)

        for tau in jtauThresholds_3bits:
            alg = TauMultiplicityAlgo( name = tau,
                                       threshold = tau, 
                                       nbits = 3, classtype='jTauMultiplicity')
            tm.registerTopoAlgo(alg)

        for tau in ctauThresholds_3bits:
            alg = TauMultiplicityAlgo( name = tau,
                                       threshold = tau, 
                                       nbits = 3, classtype='cTauMultiplicity')
            tm.registerTopoAlgo(alg)

        for tau in etauThresholds_2bits:
            alg = TauMultiplicityAlgo( name = tau,
                                       threshold = tau, 
                                       nbits = 2, classtype='eTauMultiplicity')
            tm.registerTopoAlgo(alg)

        for tau in jtauThresholds_2bits:
            alg = TauMultiplicityAlgo( name = tau,
                                       threshold = tau, 
                                       nbits = 2, classtype='jTauMultiplicity')
            tm.registerTopoAlgo(alg)

        for tau in ctauThresholds_2bits:
            alg = TauMultiplicityAlgo( name = tau,
                                       threshold = tau, 
                                       nbits = 2, classtype='cTauMultiplicity')
            tm.registerTopoAlgo(alg)

        jJThresholds_3bits = [ 
            'jJ20', 'jJ30', 'jJ30p0ETA25', 'jJ40', 'jJ40p0ETA25', 'jJ50', 'jJ55', 'jJ55p0ETA23', 'jJ60',

            # spares
            'jJSPARE1', 'jJSPARE2',
        ]
        jJThresholds_2bits = [ 
            'jJ70p0ETA23', 'jJ80', 'jJ80p0ETA25', 'jJ85p0ETA21',
            'jJ90', 'jJ125',
            'jJ140', 'jJ160', 'jJ180', 'jJ500',

            'jJ15p31ETA49', 'jJ20p31ETA49',
            'jJ40p31ETA49', 'jJ50p31ETA49', 'jJ60p31ETA49', 'jJ90p31ETA49', 'jJ125p31ETA49',

            # spares
            'jJSPARE3', 'jJSPARE4',
        ]

        for jJet in jJThresholds_3bits:
            alg = JetMultiplicityAlgo( name = jJet,
                                       threshold = jJet,
                                       nbits = 3, classtype='jJetMultiplicity')
            tm.registerTopoAlgo(alg)

        for jJet in jJThresholds_2bits:
            alg = JetMultiplicityAlgo( name = jJet,
                                       threshold = jJet,
                                       nbits = 2, classtype='jJetMultiplicity')
            tm.registerTopoAlgo(alg)

        jLJThresholds_2bits = [ 
            # jLJ thresholds for commissioning
            'jLJ80', 'jLJ120', 'jLJ140', 'jLJ180',

            # jLJ thresholds for production
            'jLJ60', 'jLJ100', 'jLJ160', 'jLJ200',
        ]

        for jLJet in jLJThresholds_2bits:
            alg = JetMultiplicityAlgo( name = jLJet, 
                                       threshold = jLJet,
                                       nbits = 2, classtype='jLJetMultiplicity')
            tm.registerTopoAlgo(alg)

        gJThresholds_3bits = [ 'gJ20p0ETA25', 'gJ20p25ETA49', 'gJSPARE1',]
        gJThresholds_2bits = [ 'gJ50p0ETA25', 'gJ100p0ETA25', 'gJ400p0ETA25']

        for gJet in gJThresholds_3bits:
            alg = JetMultiplicityAlgo( name = gJet,
                                       threshold = gJet,
                                       nbits = 3, classtype='gJetMultiplicity')
            tm.registerTopoAlgo(alg)

        for gJet in gJThresholds_2bits:
            alg = JetMultiplicityAlgo( name = gJet,
                                       threshold = gJet, 
                                       nbits = 2, classtype='gJetMultiplicity')
            tm.registerTopoAlgo(alg)

        gLJThresholds_2bits = [ 
            'gLJ80p0ETA25', 'gLJ100p0ETA25', 'gLJ140p0ETA25', 'gLJ160p0ETA25', 

            # spares
            'gLJSPARE1', 'gLJSPARE2', 'gLJSPARE3', 'gLJSPARE4',
        ]

        for gLJet in gLJThresholds_2bits:
            alg = JetMultiplicityAlgo( name = gLJet,
                                       threshold = gLJet, 
                                       nbits = 2, classtype='gLJetMultiplicity')
            tm.registerTopoAlgo(alg)

        XEThresholds = [ 
            'gXEJWOJ70', 'gXEJWOJ80', 'gXEJWOJ100',
            'gXERHO70', 'gXERHO100', 
            'gXENC70', 'gXENC100',

            'jXE70', 'jXE80', 'jXE100', 'jXE110', 'jXE500',

            'jXEC100', 'jTE200', 'jTEC200', 'jTEFWD100', 'jTEFWDA100', 'jTEFWDC100', 
            'gTE200',

            #additional jTE thresholds needed for 2023 heavy ion runs
            'jTE3','jTE4','jTE5', 'jTE10', 'jTE20','jTE50',
            'jTE100', 'jTE600', 'jTE1500', 'jTE3000',
            'jTEFWDA1', 'jTEFWDC1', 'jTEFWDA5', 'jTEFWDC5',

            'gMHT500',

            'jXEPerf100',

            #spares (for any energy thresholds)
            #replace jXESPARE16 - jXESPARE27 with heavy ion jTE threhsolds
            'jXESPARE1', 'jXESPARE2', 'jXESPARE3', 'jXESPARE4',
            'jXESPARE5', 'jXESPARE6', 'jXESPARE7', 'jXESPARE8', 'jXESPARE9',
            'jXESPARE10', 'jXESPARE11', 'jXESPARE12', 'jXESPARE13', 
            'jXESPARE14',
            'jXESPARE15',

        ]

        for XE in XEThresholds:
            alg = XEMultiplicityAlgo( name = XE, 
                                      threshold = XE,
                                      nbits = 1)
            tm.registerTopoAlgo(alg)

        tm.registerTopoAlgo(LArSaturationAlgo())

    @staticmethod
    def checkMultAlgoFWconstraints(l1menu):
        """
        List of the constraints in terms of multiplicity algorithms to make sure the menu fits
        in the Topo1 FW
        """

        multLimits = namedtuple('ML', "thrtype, conn, nbit, startbit, endbit")
        multiplicities = [
           multLimits( thrtype='eEM',  conn='Topo1Opt0', nbit=3, startbit=0,  endbit=11),
           multLimits( thrtype='eEM',  conn='Topo1Opt0', nbit=2, startbit=24, endbit=43),
           multLimits( thrtype='eEMV', conn='Topo1Opt0', nbit=2, startbit=44, endbit=63),
           multLimits( thrtype='eTAU', conn='Topo1Opt1', nbit=3, startbit=0,  endbit=8 ),
           multLimits( thrtype='eTAU', conn='Topo1Opt1', nbit=2, startbit=12, endbit=39),
           multLimits( thrtype='gLJ',  conn='Topo1Opt1', nbit=2, startbit=44, endbit=59),
           multLimits( thrtype='gJ',   conn='Topo1Opt1', nbit=3, startbit=62, endbit=70),
           multLimits( thrtype='gJ',   conn='Topo1Opt1', nbit=2, startbit=74, endbit=79),

           multLimits( thrtype='jJ',   conn='Topo1Opt2', nbit=3, startbit=0,  endbit=32),
           multLimits( thrtype='jJ',   conn='Topo1Opt2', nbit=2, startbit=36, endbit=73),
           multLimits( thrtype='jLJ',  conn='Topo1Opt2', nbit=2, startbit=78, endbit=93),
           multLimits( thrtype='jTAU', conn='Topo1Opt3', nbit=3, startbit=0,  endbit=2 ),
           multLimits( thrtype='jTAU', conn='Topo1Opt3', nbit=2, startbit=6,  endbit=11),
           multLimits( thrtype='cTAU', conn='Topo1Opt3', nbit=3, startbit=14, endbit=19),
           multLimits( thrtype='cTAU', conn='Topo1Opt3', nbit=2, startbit=23, endbit=28),
           multLimits( thrtype='jEM',  conn='Topo1Opt3', nbit=2, startbit=31, endbit=36),
           multLimits( thrtype='LArSaturation', conn='Topo1Opt3', nbit=1, startbit=37, endbit=37),
           multLimits( thrtype='EN',   conn='Topo1Opt3', nbit=1, startbit=39, endbit=86),
        ]

        for conn in l1menu.connectors:
            if 'Topo1' not in conn.name or conn.legacy:
                continue
            for tl in conn.triggerLines:
                if 'Perf' in tl.name:
                    continue
                tl_name = 'Mult_'+tl.name
                algo = l1menu.topoAlgos.topoAlgos[AlgCategory.MULTI][AlgType.MULT][tl_name]
                goodAlgo = False
                for ml in multiplicities:
                    thrtype = algo.input
                    if 'LArSaturation' in algo.name:
                        thrtype = 'LArSaturation'
                    elif 'XE' in algo.input or 'TE' in algo.input or 'MHT' in algo.input:
                        thrtype = 'EN'
                    if 'eEmVar' in algo.classtype:
                        thrtype = 'eEMV'
                    if conn.name==ml.conn and thrtype==ml.thrtype and algo.nbits==ml.nbit and tl.startbit>=ml.startbit and (tl.startbit+tl.nbits-1)<=ml.endbit:
                        goodAlgo = True
                if not goodAlgo:
                    raise RuntimeError("The multiplicity algorithm %s with startbit %i does not fit with Topo1 and CTP FW. If this is intended, please correct the multiplicity constraints and communicate the new menu to the L1TOPO and CTP groups." % (algo.name, tl.startbit) )


