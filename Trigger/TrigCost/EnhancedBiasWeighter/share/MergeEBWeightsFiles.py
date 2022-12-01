#!/usr/bin/env python
#
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

from xml.dom import minidom
import re

from AthenaCommon.Logging import logging
log = logging.getLogger('MergeEBWeigtsFiles.py')

# Create a list of unique weights and a list of events with updated weights' ids
def mergeEBWeightsFiles(input_files):
    check_weight = []
    weightsList = []
    eventsList = []

    new_id=0
    log.debug("Processing file {0}".format(list(input_files.keys())[0]))
    weights = list(input_files.values())[0].getElementsByTagName("weight")
    for weight in weights:
        weightsList.append({"value":weight.getAttribute("value"), "unbiased":weight.getAttribute("unbiased"), "id":str(new_id)})
        check_weight.append({"value": weight.getAttribute("value"), "unbiased":weight.getAttribute("unbiased")})
        new_id+=1
    events = list(input_files.values())[0].getElementsByTagName("e")
    for event in events:
        eventsList.append({"n":event.getAttribute("n"), "w":event.getAttribute("w")})

    for fin_name, fin in list(input_files.items())[1:]:
        log.debug("Processing file {0}".format(fin_name))
        id_map = {} # mapping between old and new ids
        weights = fin.getElementsByTagName("weight")
        for weight in weights:
            element = {"value": weight.getAttribute("value"), "unbiased":weight.getAttribute("unbiased")}
            # If the weight is not already in the list, add it to the list, update the id and associate the new id to the old one through id_map
            # If the weight is already in the list, only associate the new id to the old one through id_map 
            if element not in check_weight:
                check_weight.append(element.copy())
                element["id"] = str(new_id)
                weightsList.append(element)
                id_map[weight.getAttribute("id")]=element["id"]
                new_id+=1
            else:
                id_map[weight.getAttribute("id")]=weightsList[check_weight.index(element)]["id"]
        events = fin.getElementsByTagName("e")
        # Loop on events to update the weights ids
        for event in events:
            eventsList.append({"n":event.getAttribute("n"), "w":id_map[event.getAttribute("w")]})
    return weightsList, eventsList

# Create weights node
def createWeightsNode(xmlDoc, xmlRoot, weights_list):
    weights_element = xmlDoc.createElement("weights")
    xmlRoot.appendChild(weights_element)
    for weight in weights_list:
        w=doc.createElement("weight")
        w.setAttribute("id", weight["id"])
        w.setAttribute("value", weight["value"])
        w.setAttribute("unbiased", weight["unbiased"])
        weights_element.appendChild(w)

# Create events node
def createEventsNode(xmlDoc, xmlRoot, events_list):
    events_element = xmlDoc.createElement("events")
    xmlRoot.appendChild(events_element)
    for event in events_list:
        e=doc.createElement("e")
        e.setAttribute("n", event["n"])
        e.setAttribute("w", event["w"])
        events_element.appendChild(e)

# Check that all the input files have the same run number    
def checkRunNumber(runNumber, input_files):
    for fin in input_files[1:]:
        rn = re.compile(r'\d+')
        rn = rn.findall(fin)
        if rn[-1] != runNumber[-1]:
            sys.exit("ERROR: run number should be the same for all input files. Exiting.")   

if __name__=='__main__':
    import sys
    from argparse import ArgumentParser
    parser = ArgumentParser()
    parser.add_argument('--inputfiles', nargs='*', help='Insert input xml files')
    parser.add_argument('--outputfile', default="EnhancedBiasWeights_[RUN_NUMBER]_merged.xml", help='Optional: insert output file name')
    parser.add_argument('--loglevel', type=int, default=3, help='Verbosity level: 1 - VERBOSE, 2 - DEBUG, 3 - INFO')
    args = parser.parse_args()

    log.setLevel(args.loglevel)
    
    input_files = args.inputfiles
    if input_files:
        if len(input_files) < 2:
            sys.exit("ERROR: insert at least 2 input files. Exiting.") 
    else:
        sys.exit("ERROR: insert at least 2 input files. Exiting.") 

    # parse run number and check that all the input files have the same run number
    runNumber = re.compile(r'\d+')
    runNumber = runNumber.findall(input_files[0])
    checkRunNumber(runNumber, input_files)
    
    parsed_files = {}
    for fin in input_files:
        parsed_files[fin] = minidom.parse(fin)
        
    # Create weights and events merged lists
    weights_list, events_list = mergeEBWeightsFiles(parsed_files)
          
    # XML document base
    doc = minidom.parseString("<run/>")
    root = doc.documentElement

    # Add weights and events nodes
    createWeightsNode(doc, root, weights_list)  
    createEventsNode(doc, root, events_list)
        
    # Write to file
    output_filename = args.outputfile
    if '[RUN_NUMBER]' in output_filename:
        output_filename = output_filename.replace('[RUN_NUMBER]', runNumber[-1])
    log.debug("Weights written to file {0}".format(output_filename))

    xml_str = root.toprettyxml(indent = "  ") 
    with open(output_filename, "w") as f:
        f.write(xml_str)  
