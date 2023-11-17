# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
import logging
def setupArgParser():
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument("--outFile", help="Output JSON file",
                                     default="NswUncerts.json")
    parser.add_argument("--scaleErrorMM", help="Scale the uncertainties of the MM",type=float, default = 5.)
    return parser

if __name__ == "__main__":
    args = setupArgParser().parse_args()
   
    dbEntries = []
    for stationName in [ "MMS", "MML"]:
        for stationEta in range(-2,3):
            if stationEta == 0: continue
            constant = args.scaleErrorMM * 0.074
            linear =  args.scaleErrorMM * 0.66
            quad =  args.scaleErrorMM * -0.15
            stripsPerBlob = 8192
            for stationPhi in range(1, 9):
                for multilayer in [1, 2]:
                    for gasGap in range(1, 5):
                        for author in [0, 65]:
                            for strip in range(1, 8192, stripsPerBlob):
                                calibEntry ="\n".join([  "   {",
                                                 "     \"station\"        : \"{name}\",".format(name = stationName),
                                                 "     \"eta\"            : {eta},".format(eta = stationEta),
                                                 "     \"phi\"            : {phi},".format(phi = stationPhi),
                                                 "     \"multilayer\"     : {ml},".format(ml = multilayer),
                                                 "     \"gasGap\"         : {gap},".format(gap = gasGap),
                                                 "     \"minStrip\"       : {minStrip},".format(minStrip=strip),
                                                 "     \"maxStrip\"       : {maxStrip},".format(maxStrip= (strip + stripsPerBlob-1)),
                                                 "     \"modelName\"      : \"thetaPolynomial\", ",
                                                 "     \"modelPars\"      : [{const}, {linear}, {quad}], ".format(const = constant,
                                                                                                                  linear = linear,
                                                                                                                  quad = quad),
                                                 "     \"clusterAuthor\"  : {author}".format(author= author),
                                                 "   }"])
                                logging.debug(calibEntry)
                                dbEntries+=[calibEntry]
    
    with open(args.outFile, 'w') as outStream:
        outStream.write("[\n")
        for num, dead in enumerate(dbEntries, 1):
            outStream.write(dead)
            if num != len(dbEntries): outStream.write(",")
            outStream.write("\n")
        outStream.write("]\n")