#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# Module to collect JSON specific trigger configuration helpers
#
import pickle
import json

def create_joboptions_json(pkl_file, json_file, createDBJson = True):
   """Create the configuration JSON file from the properties in `pkl_file`
   and save it into `json_file`. If `createDBJson` then also create the
   JSON file for database upload at P1."""

   from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

   with open(pkl_file, "rb") as f:
      cfg = pickle.load(f)
      props = {}
      if isinstance(cfg, ComponentAccumulator):  # CA-based configuration
         app_props, msg_props, comp_props = cfg.gatherProps()
         props["ApplicationMgr"] = app_props
         props["MessageSvc"] = msg_props
         for comp, name, value in comp_props:
            props.setdefault(comp, {})[name] = value
      else:                                      # legacy job options
         svc = pickle.load(f)   # some specialized services (only in legacy config)
         props = cfg
         props.update(svc)      # merge the two dictionaries

      hlt_json = {'filetype' : 'joboptions'}
      hlt_json['properties'] = props

   # write JSON file
   with open(json_file, "w") as f:
      json.dump(hlt_json, f, indent=4, sort_keys=True, ensure_ascii=True)

   if createDBJson:
      # also create a configuration JSON file that can be uploaded
      # to the triggerdb for running at P1
      assert json_file[-5:] == ".json"
      db_file = json_file.replace(".json", ".db.json")

      modifyConfigForP1(json_file, db_file)


def modifyConfigForP1(json_file, db_file):
   """ modifies a number of job properties to run from the TriggerDB and writes out modified JSON
   """
   from AthenaCommon.Logging import logging
   log = logging.getLogger("JsonUtils")

   with open(json_file, 'r') as f:
      jocat = json.load(f)
      properties = jocat['properties']

      def mod(props, alg, prop, fnc):
         if alg not in props:
            log.warning("Asked to modify property of %s but it does not exist", alg)
            return

         origVal = props[alg].get(prop, "")
         props[alg][prop] = fnc(origVal)


      # L1 and HLT Config Svc must read from db
      mod( properties, "LVL1ConfigSvc", "InputType", lambda x : "DB" )
      mod( properties, "HLTConfigSvc", "InputType", lambda x : "DB" )
      mod( properties, "HLTPrescaleCondAlg", "Source", lambda x : "COOL" ) # prescales will be read from COOL online
      mod( properties, "HLTPrescaleCondAlg", "TriggerDB", lambda x : "JOSVC" ) # configuration will be taken from the JOSvc at P1
      # remove filenames to avoid duplicates
      mod( properties, "LVL1ConfigSvc", "HLTJsonFileName", lambda x : "None" )
      mod( properties, "LVL1ConfigSvc", "L1JsonFileName", lambda x : "None" )
      mod( properties, "TrigConf__BunchGroupCondAlg", "Filename", lambda x : "None" )
      mod( properties, "HLTConfigSvc", "HLTJsonFileName", lambda x : "None" )
      mod( properties, "HLTConfigSvc", "L1JsonFileName", lambda x : "None" )
      mod( properties, "HLTConfigSvc", "MonitoringJsonFileName", lambda x : "None" )
      mod( properties, "HLTPrescaleCondAlg", "Filename", lambda x : "None" )

   with open(db_file,'w') as f:
      json.dump(jocat, f, indent=4, sort_keys=True, ensure_ascii=True)


if __name__ == "__main__":
   import os, sys
   if len(sys.argv)!=2:
      print("Syntax: %s HLTJobOptions.[pkl,json]" % sys.argv[0].split("/")[-1])
      print("   .pkl: convert to JSON and DB JSON")
      print("  .json: convert to DB JSON")
      sys.exit(1)

   fname = sys.argv[1]
   ext = os.path.splitext(fname)[1]
   if ext == '.json':
      with open(fname) as fh:
         hlt_json = json.load( fh )
         properties = hlt_json["properties"]
         modifyConfigForP1(fname, fname.replace("json", "db.json"))
   elif ext == '.pkl':
      create_joboptions_json(fname, fname.replace("pkl","json"))
   else:
      print("Unkown file format")
      sys.exit(1)
