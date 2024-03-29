#!/usr/bin env python
# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
from __future__ import print_function

# arguments : list of run

import subprocess as sp
import os 
# Return the path of the output of tier0 monitoring


def returnEosHistPath(run, stream, amiTag, tag="data16_13TeV"):
    prefix = {'express': 'express_', 'Egamma': 'physics_', 'CosmicCalo': 'physics_',
              'JetTauEtmiss': 'physics_', 'Main': 'physics_', 'ZeroBias': 'physics_', 'MinBias': 'physics_'}
    path = '/eos/atlas/atlastier0/rucio/'+tag + \
        '/'+prefix[stream]+stream+'/00'+str(run)+'/'
    listOfDirs = [ path+p for p in os.listdir(path) ]
    for iDir in listOfDirs:
        if ("HIST.%s" % (amiTag) in iDir):
            files = [ iDir+"/"+f for f in os.listdir(iDir) ]
            if len(files) == 0:
                return "FILE NOT FOUND"
            elif len(files) > 1:
                print("WARNING: pathExtract returning",len(files),"hist files, where one is expected in path",iDir)
            return files[0]
                
    return "FILE NOT FOUND"

# Return the path of the output of tier0 monitoring for a range of single LB (available only a couple of days after processing)


def returnEosHistPathLB(run, lb0, lb1, stream, amiTag, tag="data16_13TeV"):
    prefix = {'express': 'express_', 'Egamma': 'physics_', 'CosmicCalo': 'physics_',
              'JetTauEtmiss': 'physics_', 'Main': 'physics_', 'ZeroBias': 'physics_'}
    path = '/eos/atlas/atlastier0/tzero/prod/'+tag + \
        '/'+prefix[stream]+stream+'/00'+str(run)+'/'
    P = sp.Popen(['/usr/bin/eos', 'ls', path], stdout=sp.PIPE, stderr=sp.PIPE)
    p = P.communicate()[0].decode("utf-8")
    listOfFiles = p.split('\n')

    pathList = []
    for iFile in listOfFiles:
        if ("recon.HIST.%s" % (amiTag) in iFile and "LOG" not in iFile):
            path = '/eos/atlas/atlastier0/tzero/prod/'+tag+'/' + \
                prefix[stream]+stream+'/00'+str(run)+'/'+iFile
            P = sp.Popen(['/usr/bin/eos', 'ls', path],
                         stdout=sp.PIPE, stderr=sp.PIPE)
            p = P.communicate()[0].decode("utf-8")
            listOfFiles2 = p.split('\n')
            for iFile2 in listOfFiles2:
                if ("data" in iFile2):
                    ilb = int((iFile2.split("_lb")[1]).split("._")[0])
#               print(iFile2,ilb)
                    if (lb0 <= ilb and ilb <= lb1):
                        path = '/eos/atlas/atlastier0/tzero/prod/'+tag+'/' + \
                            prefix[stream]+stream+'/00' + \
                            str(run)+'/'+iFile+'/'+iFile2
                        pathList.append(path)

    if len(pathList) > 0:
        return pathList
    else:
        return "FILE NOT FOUND"

# Return the list of TAGs files on EOS


def returnEosTagPath(run, stream, amiTag="f", tag="data16_13TeV"):
    prefix = {'express': 'express_', 'Egamma': 'physics_', 'CosmicCalo': 'physics_',
              'JetTauEtmiss': 'physics_', 'Main': 'physics_', 'ZeroBias': 'physics_'}
    found = False
    listOfFiles = []
    path = '/eos/atlas/atlastier0/rucio/'+tag + \
        '/'+prefix[stream]+stream+'/00'+str(run)+'/'
    P = sp.Popen(['/usr/bin/eos', 'ls', path], stdout=sp.PIPE, stderr=sp.PIPE)
    p = P.communicate()
    if p[1] == '':
        files = p[0].decode("utf-8")
        files = files.split('\n')
        for f in files:
            dotAmiTag = ".%s" % (amiTag)
            if ('TAG' in f and dotAmiTag in f):
                path += f+'/'
                found = True
                break
    if not found:
        print('no TAG directory found in %s' % (path))
        return

    P = sp.Popen(['/usr/bin/eos', 'ls', path], stdout=sp.PIPE, stderr=sp.PIPE)
    p = P.communicate()
    if p[1] == '':
        files = p[0].decode("utf-8").split('\n')
        for iFile in files:
            if (len(iFile) > 0):
                pathFile = path+iFile
                listOfFiles.append(pathFile)
    return listOfFiles

# Return the list of LArNoise ntuple files on EOS


def returnEosLArNoisePath(run, stream, amiTag="f", tag="data16_13TeV"):
    prefix = {'express': 'express_', 'Egamma': 'physics_',
              'CosmicCalo': 'physics_', 'JetTauEtmiss': 'physics_', 'Main': 'physics_'}
    found = False
    listOfFiles = []
    path = '/eos/atlas/atlascerngroupdisk/det-larg/Tier0/perm/' + \
        tag+'/'+prefix[stream]+stream+'/00'+str(run)+'/'
    P = sp.Popen(['/usr/bin/eos', 'ls', path], stdout=sp.PIPE, stderr=sp.PIPE)
    p = P.communicate()
    if p[1] == '':
        files = p[0].decode("utf-8")
        files = files.split('\n')
        for f in files:
            dotAmiTag = ".%s" % (amiTag)
            if ('LARNOISE' in f and dotAmiTag in f):
                path += f+'/'
                found = True
                break
    if not found:
        print('no LARNOISE directory found in %s' % (path))
        return

    P = sp.Popen(['/usr/bin/eos', 'ls', path], stdout=sp.PIPE, stderr=sp.PIPE)
    p = P.communicate()
    if p[1] == '':
        files = p[0].decode("utf-8").split('\n')
        for iFile in files:
            if (len(iFile) > 0):
                pathFile = path+iFile
                listOfFiles.append(pathFile)
    return listOfFiles

# Return the list of ESDs files on EOS


def returnEosEsdPath(run, stream, amiTag="f", tag="data16_13TeV"):
    prefix = {'express': 'express_', 'Egamma': 'physics_',
              'CosmicCalo': 'physics_', 'JetTauEtmiss': 'physics_', 'Main': 'physics_'}
    found = False
    listOfFiles = []
    path = '/eos/atlas/atlastier0/rucio/'+tag + \
        '/'+prefix[stream]+stream+'/00'+str(run)+'/'
    P = sp.Popen(['/usr/bin/eos', 'ls', path], stdout=sp.PIPE, stderr=sp.PIPE)
    p = P.communicate()
    if p[1] == '':
        files = p[0].decode("utf-8")
        files = files.split('\n')
        for f in files:
            dotAmiTag = ".%s" % (amiTag)
            if ('.ESD' in f and dotAmiTag in f):
                path += f+'/'
                found = True
                break
    if not found:
        print('no ESD directory found in %s' % (path))
        return

    P = sp.Popen(['/usr/bin/eos', 'ls', path], stdout=sp.PIPE, stderr=sp.PIPE)
    p = P.communicate()
    if p[1] == '':
        files = p[0].decode("utf-8").split('\n')
        for iFile in files:
            if (len(iFile) > 0):
                pathFile = path+iFile
                listOfFiles.append(pathFile)
    return listOfFiles

# Return the list of all files stored on a local user directory


def returnFilesPath(directory=".", filterName=""):
    listOfFiles = []
    path = directory
    P = sp.Popen(['ls', path], stdout=sp.PIPE, stderr=sp.PIPE)
    p = P.communicate()
    found = False
    if p[1] == '':
        files = p[0].decode("utf-8")
        files = files.split('\n')
        for f in files:
            if filterName in f:
                pathFile = path + f
                listOfFiles.append(pathFile)
                found = True

    if not found:
        print('no file containing %f found in %s' % (filterName, path))
        return

    return listOfFiles
