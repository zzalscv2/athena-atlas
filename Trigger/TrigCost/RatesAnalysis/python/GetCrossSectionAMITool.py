# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
import re

##
# Basic wrapper around pyAMI for getting MC dataset cross sections & maniuplating dataset name strings
##
class GetCrossSectionAMI:
  def __init__(self):
    self.log = logging.getLogger('GetCrossSectionAMI')

    try:
      import pyAMI.client
      import pyAMI.atlas.api as api
    except ImportError:
      self.log.fatal('Unable to import the pyAMI client. Did you run "lsetup pyami; voms-proxy-init -voms atlas"?')
      exit()

    self.ami_api = api
    self.ami_client = pyAMI.client.Client('atlas')
    self.ami_api.init()

  @property
  def crossSection(self):
    return self.xsec

  @property
  def filterEfficiency(self):
    return self.filtereff

  def getDatasetNameFromPath(self, path):
    import re
    datasetRegex = r'mc.._.*TeV\..*|valid.\..*' # mc??_????TeV.??? or valid?.???
    parts = path.split('/')
    for part in parts: # Do any of these parts look like a dataset name?
      if re.match(datasetRegex, part):
        return part

    self.log.error('Unable to figure out the MC dataset name from the path, you must supply it yourself as MCDatasetName')

  def cleanupDatasetName(self, dataset):
    dataset = re.sub('_tid[0-9]{8}_[0-9]{2}', '', dataset)
    dataset = re.sub('_sub[0-9]{10}', '', dataset)
    dataset = re.sub('/$', '', dataset)
    return dataset

  def convertDatasetNameToEVNT(self, dataset):
    dsid = dataset.split('.')[1]
    try:
      res_d = self.ami_api.get_dataset_prov(self.ami_client, dataset)
    except Exception as reason:
      self.log.fatal('Unable to query AMI. Do you have a grid certificate? "voms-proxy-init -voms atlas"')
      self.log.fatal('You can also supply the MCCrossSection and MCFilterEfficiency by hand for this sample')
      self.log.fatal(f'The reason for failure was: {reason}')
      exit()

    previous_nodes = [
      n for n in res_d['node'] 
      if 'logicalDatasetName' in n.keys() and n['logicalDatasetName'].split('.')[1] == dsid and 'EVNT' in n['logicalDatasetName']
    ]

    sorted_nodes = sorted(previous_nodes, key=lambda n: n['distance'])
    # We pick the latest one, just in case the cross section was modified (shouldn't have)
    evnt_dataset = sorted_nodes[0]['logicalDatasetName']

    if 'EVNT' in evnt_dataset:
      self.log.info('Found the original EVNT dataset using AMI provenance.')
      self.log.info(f'Went from: {dataset}')
      self.log.info(f'       to: {evnt_dataset}')
      return evnt_dataset
    else:
      self.log.fatal('Unable to get the original EVNT dataset from AMI provenance, you must supply it yourself as MCDatasetName')
      self.log.fatal('You can also supply the MCCrossSection and MCFilterEfficiency by hand for this sample')
      exit()

  def queryAmi(self, dataset):
    if 'EVNT' not in dataset:
      dataset = self.convertDatasetNameToEVNT(self.cleanupDatasetName(dataset))
    self.dataset = dataset
    self.log.info(f'Looking up details of {self.dataset} on AMI')

    # search for EVNT file
    fields = 'cross_section,generator_filter_efficienty' # Yes, "efficienty"... there's a typo in the field definition
    try:
      res_l = self.ami_api.list_datasets(self.ami_client, patterns=self.dataset, fields=fields)
    except Exception as reason:
      self.log.fatal('Unable to query AMI. Do you have a grid certificate? "voms-proxy-init -voms atlas"')
      self.log.fatal('You can also supply the MCCrossSection and MCFilterEfficiency by hand for this sample')
      self.log.fatal(f'The reason for failure was: {reason}')
      exit()

    if len(res_l) == 0:
      self.log.fatal(f'The dataset "{dataset}" was not found in AMI')
      self.log.fatal('You can also supply the MCCrossSection and MCFilterEfficiency by hand for this sample')
      exit()
    elif len(res_l) > 1:
      self.log.fatal(f'More than one dataset "{dataset}" was found in AMI.')
      exit()

    xsec = res_l[0]['cross_section']
    gfe = res_l[0]['generator_filter_efficienty']

    if xsec == 'NULL' or gfe == 'NULL':
      self.log.fatal(f'NULL cross section/filter efficiency values set on AMI for the dataset "{dataset}"')
      self.log.fatal('You can supply the MCCrossSection and MCFilterEfficiency by hand for this sample')
      exit()

    try:
      self.xsec = float(xsec)
      self.filtereff = float(gfe)
    except ValueError:
      self.log.fatal('Coudn\'t convert cross section/filter efficiency values to floats')
      self.log.fatal(f'    Cross Section: {xsec}')
      self.log.fatal(f'Filter Efficiency: {gfe}')
      self.log.fatal('You can supply the MCCrossSection and MCFilterEfficiency by hand for this sample')
      exit()

    if self.xsec and self.filtereff:
      self.log.info('################## ################## AMI Query Complete ################## ################## ')
      self.log.info(f'    Cross section: {self.xsec} nb')
      self.log.info(f'Filter efficiency: {self.filtereff}')
      self.log.info('You may want to supply these numbers directly in future as MCCrossSection and MCFilterEfficiency for speed')
      self.log.info('################## ################## ################## ################## ################### ')
    else:
      self.log.fatal(f'The cross section and/or filter efficiency values for the dataset "{dataset}" are set to 0 on AMI')
      self.log.fatal('You can supply the MCCrossSection and MCFilterEfficiency by hand for this sample')
      exit()
