# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

import re

from .JetRecoCommon import getJetCalibDefaultString, jetChainParts, etaRangeAbbrev, jetRecoDictToString
from ..Menu.SignatureDicts import JetChainParts_Default
from TriggerMenuMT.HLT.Config.ControlFlow.HLTCFTools import NoHypoToolCreated


from TrigHLTJetHypo.TrigJetHypoToolConfig import trigJetHypoToolFromDict

# Extract preselection reco dictionary
# This is 1:1 based on the main (tracking/PF) jet collection
def getPreselRecoDict(reco,roiftf=False):

    # Define a fixed preselection dictionary for prototyping -- we may expand the options
    preselRecoDict = {
        'recoAlg':reco,
        'constitType':'tc',
        'clusterCalib':'em',
        'constitMod':'',
        'trkopt':'notrk',
        'ionopt':'noion',
    }
    ''' #Here you can set custom calibrations for large-R preselections. If you set to LCW you'll get an issue though, as the trigger expects the *same* topocluster collection to be used in the preselection and in the PFlow stage with tracking. Therefore this would need to be adapted, but it might not be so easy...
        
    if preselRecoDict['recoAlg']=='a10': #Setting LC calibrations for large-R jets
        preselRecoDict['clusterCalib']='lcw'
    '''
    preselRecoDict.update({'jetCalib':getJetCalibDefaultString(preselRecoDict) if preselRecoDict['recoAlg']=='a4' else 'nojcalib'}) #Adding default calibration for corresponding chain
    
    # Overwriting tracking option to roiftf tracking
    if roiftf: preselRecoDict['trkopt'] = 'roiftf'

    preselRecoDict['jetDefStr'] = jetRecoDictToString(preselRecoDict)

    return preselRecoDict

# Find the preselection definition in the chainParts
# Simply taken from the last chainPart
def extractPreselection(fullChainDict):
    jparts = jetChainParts(fullChainDict['chainParts'])
    return jparts[-1]['trkpresel']

# Calo jet preselection hypo tool generator
# Translate the preselection expression in the main jet chainDict into a temporary chainDict
# that is only seen by the standard hypo tool generator, and used to return a configured
# hypo tool for the preselection step
def caloPreselJetHypoToolFromDict(flags, mainChainDict):
    return _preselJetHypoToolFromDict(flags, mainChainDict)
    

def roiPreselJetHypoToolFromDict(flags, mainChainDict):
    return _preselJetHypoToolFromDict(flags, mainChainDict,doBJetSel=True)


def _preselJetHypoToolFromDict(flags, mainChainDict, doBJetSel=False):

    preselChainDict = dict(mainChainDict)
    preselChainDict['chainParts']=[]
    trkpresel = extractPreselection(mainChainDict)

    # Get from the last chainPart in order to avoid to specify preselection for every leg
    #TODO: add protection for cases where the preselection is not specified in the last chainPart
    presel_matched = re.match(r'presel(?P<cut>\d?\d?[jacf](HT)?(Z)?[\d\D]+)', trkpresel)
    assert presel_matched is not None, "Impossible to match preselection pattern for self.trkpresel=\'{0}\'.".format(trkpresel)
    presel_cut_str = presel_matched.groupdict()['cut'] #This is the cut string you want to parse. For example 'presel2j50XXj40'
    
    preselCommonJetParts = dict(JetChainParts_Default)

    for ip,p in enumerate(presel_cut_str.split('XX')):
        hascalSel= bool(re.match(r'.*e\w?\d+', p))
        if not doBJetSel:  # Removing b-jet parts if b-jet presel is not requested (same for DIPZ as they use same input)
            p = re.sub(r'b\w?\d+|Z\d+', '', p)
        hasBjetSel = bool(re.match(r'.*b\w?\d+', p))
        hasDIPZsel = bool(re.match(r'.*Z', p))
        
        assert not ( (hasBjetSel or hasDIPZsel) and not doBJetSel), "Your jet preselection has a b-jet part but a calo-only preselection was requested instead. This should not be possible. Please investigate."
        
        pattern_to_test = r'(?P<mult>\d?\d?)(?P<region>[jacf])' # jet multiplicity and region
        pattern_to_test += r'(?P<cut>\d+)(?P<scenario>(Z(?P<dipzwp>\d+))?)(?P<prefilt>(MAXMULT\d+)?)' if hasDIPZsel else r'(?P<scenario>(HT)?)(?P<cut>\d+)' # scenario string # could be made more general
        pattern_to_test += r'b(?P<btagger>\D?)(?P<bwp>\d+)' if hasBjetSel else '' # b-tagging if needed
        pattern_to_test += r'e(?P<emf>\D?)(?P<emfc>\d+)' if hascalSel else ''
        matched = re.match(pattern_to_test, p)
        assert matched is not None, "Impossible to extract preselection cut for \'{0}\' substring. Please investigate.".format(p)
        cut_dict = matched.groupdict()

        if 'emfc' not in cut_dict.keys(): cut_dict['emfc'] = ''
        if 'bwp' not in cut_dict.keys(): cut_dict['bwp'] = ''
        if 'btagger' not in cut_dict.keys(): cut_dict['btagger'] = ''
        if 'dipzwp' not in cut_dict.keys(): cut_dict['dipzwp'] = ''
        if 'prefilt' not in cut_dict.keys(): cut_dict['prefilt'] = ''

        mult,region,scenario,cut,btagger,bwp,dipzwp,prefilt,emfc=cut_dict['mult'],cut_dict['region'],cut_dict['scenario'],cut_dict['cut'],cut_dict['btagger'], cut_dict['bwp'], cut_dict['dipzwp'], cut_dict['prefilt'],cut_dict['emfc']

        if mult=='': mult='1'
        etarange = etaRangeAbbrev[region]
        if scenario == "HT":
            hyposcenario=f'HT{cut}XX{etarange}'
            threshold='0'
            chainPartName=f'j0_{hyposcenario}'
        elif scenario == "Z%s"%str(dipzwp):
            hyposcenario=f'Z{dipzwp}XX{mult}j{cut}ptXX{etarange}'
            if prefilt != '': hyposcenario += f'_{prefilt}'
            threshold='0'
            chainPartName=f'j0_{hyposcenario}'
        else:
            hyposcenario='simple'
            threshold=cut
            chainPartName=f'{mult}j{cut}_{etarange}'
        
        if btagger == 'g':
            btagger = 'gnone'
        elif btagger == '':
            btagger = 'dips'

        tmpChainDict = dict(preselCommonJetParts)
        tmpChainDict.update(
            {'L1threshold': 'FSNOSEED',
            'chainPartName': chainPartName,
            'multiplicity': mult,
            'threshold': threshold,
            'etaRange':etarange,
            'jvt':'',
            'clrsel': emfc,
            'bsel': '' if bwp == '' else f'{bwp}b{btagger}',
            'chainPartIndex': ip,
            'hypoScenario': hyposcenario,
            }
        )
        preselChainDict['chainParts'] += [tmpChainDict]

    # We need to pad by the legs not in the preselection expression
    # otherwise the ComboHypo does not find the corresponding
    # legs in the DecisionObject and kills the event
    jetlegs = sum([p['signature'] in ["Jet","Bjet"] for p in mainChainDict['chainParts']])
    padding = jetlegs-len(preselChainDict['chainParts'])
    if padding>0:
        preselChainDict['chainParts'][-1]['chainPartName']+='_SHARED'
        preselChainDict['chainParts'][-1]['tboundary']='SHARED'
        dummyLegPart = dict(preselCommonJetParts)
        dummyLegPart.update(
            {'L1threshold': 'FSNOSEED',
                'chainPartName': 'j0_SHARED',
                'multiplicity': '1',
                'threshold': '0',
                'jvt':'',
                'tboundary': 'SHARED',
                }
        )
        preselChainDict['chainParts'] += [dict(dummyLegPart) for i in range(padding)]
        # Last one is not permitted to be shared as there is nothing following
        preselChainDict['chainParts'][-1]['chainPartName']='j0'
        preselChainDict['chainParts'][-1]['tboundary']=''
    
    # Update certain keys to be consistent with the main dict
    # Also set the chainPart indices correctly from the main dict
    # These should be index-parallel now, as we only receive jet chainParts
    for porig,ppresel in zip(mainChainDict['chainParts'],preselChainDict['chainParts']):
        for key in ['chainPartIndex','signature']:
            ppresel[key] = porig[key]
    
    assert(len(preselChainDict['chainParts'])==len(mainChainDict['chainParts']))
    try:
        return trigJetHypoToolFromDict(flags, preselChainDict)
    except NoHypoToolCreated as nohypo:
        raise nohypo # We only generate the hypo tool for the first jet leg
    except Exception as e:
        log.error("Failure with preselection for chain %s",mainChainDict['chainName'])
        raise e
