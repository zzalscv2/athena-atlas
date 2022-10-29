#! /usr/bin/env python
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Author : Benjamin Trocme (LPSC - Grenoble) - 2022
# Script used to generate to https://atlasdqm.web.cern.ch/atlasdqm/DeMo/ webpage
# used by teh DeMoDameon.exe script running on atlasdqm account
##################################################################

import sys, socket, pathlib, errno, re, subprocess

maindir = "/afs/cern.ch/user/a/atlasdqm/www/DeMo"
libdir = "/afs/cern.ch/user/a/atlasdqm/w1/DeMo"

global run3_yt

run3_yt = {"2022":["Tier0_2022","GRL_2022"]}
tag_menuName = {"Tier0_2022":"All",
                "GRL_2022":"GRL"}

global subsystems
subsystems = {"Inner detector":["Pixel","SCT","TRT"],
              "Calorimeters":["LAr","Tile"],
              "Muon":["RPC","MDT","TGC","CSC"],
              "Trigger/Lumi/Global":["Trig_L1","Trig_HLT","Lumi","Global"],
              "Forward": ["ALFA","LUCID","ZDC"],
              "Combined perf.":["IDGlobal","BTag","CaloCP","MuonCP"]} 
global subsystemName
subsystemName = {"Pixel":"Pixel",
                 "SCT":"SCT",
                 "TRT":"TRT",
                 "LAr":"LAr",
                 "Tile":"Tile",
                 "RPC":"RPC",
                 "MDT":"MDT",
                 "TGC":"TGC",
                 "CSC":"CSC",
                 "Trig_L1":"L1 trigger",
                 "Trig_HLT":"HLT trigger",
                 "Lumi":"Luminosity",
                 "Global":"Global",
                 "ALFA":"ALFA",
                 "LUCID":"LUCID",
                 "ZDC":"ZDC",
                 "IDGlobal":"ID global",
                 "BTag":"Btagging",
                 "CaloCP":"Calorimeter combined",
                 "MuonCP":"Muon combined"}

subsystemNotInRun2 = {}
subsystemNotInRun3 = ["CSC"]


################################################################
def addHeader(page):
    page.write('''<!doctype html>''')
    page.write('''<html lang=''>''')
    page.write('''<head>''')
    page.write('''   <meta charset='utf-8'>''')
    page.write('''   <meta http-equiv="X-UA-Compatible" content="IE=edge">''')
    page.write('''   <meta name="viewport" content="width=device-width, initial-scale=1">''')
    page.write('''   <link rel="stylesheet" href="styles.css">''')
    page.write('''   <script src="http://code.jquery.com/jquery-latest.min.js" type="text/javascript"></script>''')
    page.write('''   <script src="script.js"></script>''')
    page.write('''   <title>DeMo outputs</title>''')
    page.write('''</head> ''')
    page.write('''<body>''')

    page.write('''<div id='cssmenu'>''')
    page.write('''<ul>''')
    ### Home
    page.write('''   <li><a href='.'><span>Home</span></a></li>''')

    ### Weekly recap overview
    page.write('''   <li><a href='Weekly.html'><span>Weekly recap</span></a></li>''')

    ### Year stats for all systems
    page.write('''   <li class='active has-sub'><a><span>Year stats</span></a>''')
    page.write('''     <ul>''')
    for iYear in run3_yt.keys():
        for iTag in run3_yt[iYear]:
            page.write('''       <li><a href='YearStats-%s-%s.html'><span>%s overview - %s</span></a></li>'''%(iYear,iTag,iYear,tag_menuName[iTag]))
    #
    page.write('''       <li><a href='YearStats-2018-DQPaper_2018.html'><span>2018 overview - DQ Paper</span></a></li>''')
    page.write('''       <li><a href='YearStats-2017-DQPaper_2017.html'><span>2017 overview - DQ Paper</span></a></li>''')
    page.write('''       <li><a href='YearStats-2016-DQPaper_2016.html'><span>2016 overview - DQ Paper</span></a></li>''')
    page.write('''       <li><a href='YearStats-2015-DQPaper_2015.html'><span>2015 overview - DQ Paper</span></a></li>''')
    page.write('''     </ul>''')
    page.write('''   </li>''')
    
    for iSystem in subsystems.keys():
        page.write('''   <li class='active has-sub'><a><span>%s</span></a>'''%iSystem)
        page.write('''      <ul>''')

        for iSubSystem in subsystems[iSystem]:
             page.write('''<li class='has-sub'><a ><span>%s</span></a>'''%subsystemName[iSubSystem])
             page.write('''   <ul>''')
             if (iSubSystem not in subsystemNotInRun3):
                 page.write('''      <li><a href='%s-Weekly.html'><span>Weekly</span></a></li>'''%(iSubSystem))
                 for iYear in run3_yt.keys():
                     for iTag in run3_yt[iYear]:
                         page.write('''      <li><a href='%s-YearStats-%s-%s.html'><span>Year stats - %s - %s</span></a></li>'''%(iSubSystem,iYear,iTag,iYear,tag_menuName[iTag]))
                         page.write('''      <li><a href='%s-recapDefects-%s-%s.html'><span>Defect recap - %s - %s </span></a></li>'''%(iSubSystem,iYear,iTag,iYear,tag_menuName[iTag]))
             if (iSubSystem not in subsystemNotInRun2):
                 page.write('''      <li><a href='%s-YearStats-DQPaper_.html'><span>Year stats - Run2</span></a></li>'''%(iSubSystem))
                 page.write('''      <li class='last'><a href='%s-recapDefects-DQPaper_.html'><span>Defect recap - Run2</span></a></li>'''%(iSubSystem))
             page.write('''   </ul>''')
             page.write('''</li>''')
        page.write('''</ul>''')
        page.write('''</li>''')

    page.write('''</ul>''')
    page.write('''</div>''')

    page.write('''<br>''')

    return

################################################################
def addFooter(page):
    page.write('''<div style="text-align:left" class="rectangle">''')
    page.write('''Original developer: Benjamin Trocm&eacute (LPSC Grenoble) - Maintenance: Benjamin Trocm&eacute (LPSC Grenoble)''')
    page.write('''</div></body>''')
    page.write('''<html>''')
    return

################################################################
def addHeaderWeekly(page):
    page.write('''<div style="text-align:left" class="rectangle">''')
    page.write('''Considered dataset: all ATLAS-ready runs taken or fully signed off in the last 7 days<br>''')
    page.write('''Plots updated daily at midnight<br>''')
    page.write('''Status meaning (based on UNCHECKED defect status):<br>''')
    page.write('''-EXPR.   : no assessment yet<br>''')
    page.write('''-BULK    : express assessment completed<br>''')
    page.write('''-DONE    : express and bulk assessment completed (but no final signoff - relevant only for LAr)<br>''')
    page.write('''-FINAL OK: assessment completed<br>''')
    page.write('''</div>''')
    return

################################################################
def addHeaderYearStatsRecap(page,year="",tag="",system=""):
    page.write('''<div style="text-align:left" class="rectangle">''')
    page.write('''<b>Dataset </b><br>''')

    if "Tier0" in tag: # Year tag named Tier0_[year] are usually devoted to monitor all runs
        page.write('''All runs with ATLAS ready considered <br>''')    
        page.write(''' Direct link to the <a href="RunList/all-%s.dat" target="_blank"> list </a> <br>'''%year)
        page.write('''Plots updated daily at midnight <br>''')
    if "GRL" in tag:
        page.write('''List of runs derived from the <a href="https://twiki.cern.ch/twiki/bin/view/AtlasProtected/GoodRunListsForAnalysisRun3" target="_blank">GRL twiki </a> <br>''')    
        page.write(''' Direct link to the <a href="RunList/grl-%s.dat" target="_blank"> list </a> <br>'''%year)


    # Scan the DeMoLib.py file to extract the monitored defects and the database tags
    DeMoLibFile = open("%s/DeMoLib.py"%libdir,"r")
    offLumiTag = ""
    offLumiAcctTag = ""
    defectTag = ""
    if (system != ""):
        listOfDefects = {"partIntol":"","partTol":"","globIntol":"","globTol":""}
        
    for line in DeMoLibFile:
        if (system != ""):
            defectSearch = "%s system"%system
            for iType in listOfDefects.keys():
                if re.search(defectSearch, line) and iType in line:
                    listOfDefects[iType] = (((line.split(" = ")[1]).split("# %s system"%system)[0]).replace(" ","")).replace(",",", ")
            
        if re.search('"OflLumi"',line):
            offLumiTag = ((line.split('"OflLumi":')[1]).split('"%s":"'%year)[1]).split('"')[0]
        if re.search('"OflLumiAcct"',line):
            offLumiAcctTag = ((line.split('"OflLumiAcct":')[1]).split('"%s":"'%year)[1]).split('"')[0]
        if re.search('yearTag\["defect"\]',line):
            defectTag = ((line.split(' = ')[1]).split('"%s":"'%tag)[1]).split('"')[0]

    if (system != ""):
        page.write("<hr>")
        page.write('''<b>Monitored defects </b><br>''')
        if len(listOfDefects["globIntol"])>2:
            page.write('''<b> Global    intolerable defects</b>: %s  <br>'''%listOfDefects["globIntol"])
        if len(listOfDefects["partIntol"])>2:
            page.write('''<b> Partition intolerable defects</b>: %s  <br>'''%listOfDefects["partIntol"])
        if len(listOfDefects["globTol"])>2:
            page.write('''<b> Global    tolerable defects  </b>: %s  <br>'''%listOfDefects["globTol"])
        if len(listOfDefects["partTol"])>2:
            page.write('''<b> Partition tolerable defects  </b>: %s  <br>'''%listOfDefects["partTol"])

    page.write("<hr>")
    page.write('''<b>Database tags </b><br>''')
    page.write('''Offline luminosity tag: %s<br>'''%offLumiTag)
    page.write('''Offline luminosity acct tag: %s<br>'''%offLumiAcctTag)
    page.write('''Defect tag: %s<br>'''%defectTag)

    # Look for remaining tokens
    p = subprocess.Popen("ls",stdout=subprocess.PIPE)
    (output, err) = p.communicate()
    remainingToken = ""
    for iLine in (output.decode('ascii').split("\n")):
        if (system != ""):
            if "%s-%s-%s.token"%(system,year,tag) in iLine:
                remainingToken = "%s<br>"%iLine
        else:
            if "%s-%s.token"%(year,tag) in iLine:
                remainingToken = remainingToken + "%s<br>"%iLine
    if remainingToken != "":
        page.write('''<hr><b>Remaining tokens</b>''')
        page.write('''<br>There are some remaining tokens in ~atlasdqm/w1/DeMo: %s <br> This may indicate a problem in processing. Please check the daemon ouputs...'''%remainingToken)


    page.write('''</div>''')
    return

################################################################
def addHeaderRun2RecapDefects(page):
    page.write('''<div style="text-align:left" class="rectangle">''')
    page.write('''Considered dataset: all "stable beam" runs signed off.''')
    page.write('''<br><a href='#2018' target='_self'> 2018 dataset </a>''')
    page.write('''<br><a href='#2017' target='_self'> 2017 dataset </a>''')
    page.write('''<br><a href='#2016' target='_self'> 2016 dataset </a>''')
    page.write('''<br><a href='#2015' target='_self'> 2015 dataset </a>''')
    page.write('''</div>''')
    return

#######################
# Main script

homepage = open("%s/index.html"%maindir,"w")
addHeader(homepage)
addFooter(homepage)
homepage.close()

weeklyYear = "2022"
weeklyTag = "Tier0_2022"

## Weekly page for all systems
weekly = open("%s/Weekly.html"%maindir,"w")
addHeader(weekly)
addHeaderWeekly(weekly)
for iSubSystem in subsystemName.keys():
    if (iSubSystem not in subsystemNotInRun3):
        weekly.write('''<div style="text-align:left" class="rectangle"> <b>%s</b>'''%subsystemName[iSubSystem])
        weekly.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/Weekly/summary-0.png" alt=""  width="750" content="no-cache"/></figure>'''%(iSubSystem,weeklyYear,weeklyTag))
        weekly.write('''<a> Link to the <a href="%s-Weekly.html" > weekly plots.</a>'''%(iSubSystem))
        weekly.write('''</div>''')
addFooter(weekly)
weekly.close()

## Weekly page per system
for iSubSystem in subsystemName.keys():
    if (iSubSystem not in subsystemNotInRun3):
        systemWeekly = open("%s/%s-Weekly.html"%(maindir,iSubSystem),"w")
        addHeader(systemWeekly)
        addHeaderWeekly(systemWeekly)
        systemWeekly.write('''<div style="text-align:left" class="rectangle"><b>%s</b> - <a href="YearStats-%s/%s/%s/daemon-weekly.out" target="_blank"> Daemon output </a>'''%(subsystemName[iSubSystem],iSubSystem,weeklyYear,weeklyTag))
        systemWeekly.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/Weekly/defects--Run--%s.png" alt=""  width="750" content="no-cache" /></figure>'''%(iSubSystem,weeklyYear,weeklyTag,weeklyTag))
        if iSubSystem == "LAr" :
            systemWeekly.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/Weekly/veto--Run--%s.png" alt=""  width="750" content="no-cache" /></figure>'''%(iSubSystem,weeklyYear,weeklyTag,weeklyTag))
            systemWeekly.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/Weekly/noiseBurst_veto_evol.png" alt=""  width="750" content="no-cache" /></figure>'''%(iSubSystem,weeklyYear,weeklyTag))
        systemWeekly.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/Weekly/summary-0.png" alt=""  width="750" content="no-cache" /></figure>'''%(iSubSystem,weeklyYear,weeklyTag))
        addFooter(systemWeekly)
        systemWeekly.close()

## YearStats page for all systems
yearStats = {}
for iYear in run3_yt.keys():
    for iTag in run3_yt[iYear]:
        yearTag = "%s_%s"%(iYear,iTag)
        yearStats[yearTag] = open("%s/YearStats-%s-%s.html"%(maindir,iYear,iTag),"w")
        addHeader(yearStats[yearTag])
        addHeaderYearStatsRecap(yearStats[yearTag],iYear,iTag)
        for iSubSystem in subsystemName.keys():
            if (iSubSystem not in subsystemNotInRun3):
                yearStats[yearTag].write('''<div style="text-align:left" class="rectangle"> <b>%s</b>'''%subsystemName[iSubSystem])
                yearStats[yearTag].write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/grl-defects.png" alt=""  width="750" content="no-cache" /></figure>'''%(iSubSystem,iYear,iTag))
                yearStats[yearTag].write('''</div>''')
        addFooter(yearStats[yearTag])
        yearStats[yearTag].close()
        
## YearStats page per system
yearStatsSystem = {}
for iYear in run3_yt.keys():
    for iTag in run3_yt[iYear]:
        for iSubSystem in subsystemName.keys():
            if (iSubSystem not in subsystemNotInRun3):
                yearStatsSystem[yearTag] = open("%s/%s-YearStats-%s-%s.html"%(maindir,iSubSystem,iYear,iTag),"w")
                addHeader(yearStatsSystem[yearTag])
                addHeaderYearStatsRecap(yearStatsSystem[yearTag],iYear,iTag,iSubSystem)
                yearStatsSystem[yearTag].write('''<div style="text-align:left" class="rectangle"><b>%s - %s</b> - <a href="YearStats-%s/%s/%s/daemon-grl.out" target="_blank"> daemon output </a> - <a href="YearStats-%s/%s/%s/runs-notYetSignedOff.dat" target="_blank"> Runs not yet signed off </a>'''%(iYear,tag_menuName[iTag],iSubSystem,iYear,iTag,iSubSystem,iYear,iTag))
                yearStatsSystem[yearTag].write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/grl-defects.png" alt=""  width="750" content="no-cache" /></figure>'''%(iSubSystem,iYear,iTag))
                if iSubSystem == "LAr" :
                    yearStatsSystem[yearTag].write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/grl-veto.png" alt=""  width="750" content="no-cache" /></figure>'''%(iSubSystem,iYear,iTag))
                yearStatsSystem[yearTag].write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/grl-lumi.png" alt=""  width="750" content="no-cache" /></figure>'''%(iSubSystem,iYear,iTag))
                yearStatsSystem[yearTag].write('''</div>''')
                addFooter(yearStatsSystem[yearTag])
                yearStatsSystem[yearTag].close()

## RecapDefect page per system
recapDefectSystem = {}
for iYear in run3_yt.keys():
    for iTag in run3_yt[iYear]:
        for iSubSystem in subsystemName.keys():
            if (iSubSystem not in subsystemNotInRun3):
                recapDefectSystem[yearTag] = open("%s/%s-recapDefects-%s-%s.html"%(maindir,iSubSystem,iYear,iTag),"w")
                addHeader(recapDefectSystem[yearTag])
                addHeaderYearStatsRecap(recapDefectSystem[yearTag],iYear,iTag,iSubSystem)
                readFile = open("YearStats-%s/%s/%s/recapDefects.html"%(iSubSystem,iYear,iTag))
                for line in readFile:
                    htmlSplitted = line.split("https")
                    
                    if len(htmlSplitted)>1: # Url found -> Direct links added
                        urlAlreadyReplaced = []
                        for iUrl in range(1,len(htmlSplitted)):
                            url = "https%s"%(htmlSplitted[iUrl].split(" ")[0])
                            if (url not in urlAlreadyReplaced):
                                line = line.replace(url,'''<a href="%s" target="_blank"> %s </a>'''%(url,url))
                            urlAlreadyReplaced.append(url)

                    recapDefectSystem[yearTag].write(line)                    
                addFooter(recapDefectSystem[yearTag])
                recapDefectSystem[yearTag].close()

## Run 2 Year stats for all systems
yearStats = {}
for iYear in ["2015","2016","2017","2018"]:
    yearStats[iYear] = open("%s/YearStats-%s-DQPaper_%s.html"%(maindir,iYear,iYear),"w")
    addHeader(yearStats[iYear])
    for iSubSystem in subsystemName.keys():
        if (iSubSystem not in subsystemNotInRun2):
            yearStats[iYear].write('''<div style="text-align:left" class="rectangle"> <b>%s</b>'''%iSubSystem)
            yearStats[iYear].write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/DQPaper_%s/grl-defects.png" alt=""  width="750" content="no-cache" /></figure>'''%(iSubSystem,iYear,iYear))
            if iSubSystem == "LAr":
                yearStats[iYear].write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/DQPaper_%s/grl-veto.png" alt=""  width="750" content="no-cache" /></figure>'''%(iSubSystem,iYear,iYear))
            yearStats[iYear].write('''</div>''')
    addFooter(yearStats[iYear])
    yearStats[iYear].close()

## Run 2 YearStats page per system
for iSubSystem in subsystemName.keys():
    if (iSubSystem not in subsystemNotInRun3):
        yearStatsSystem = open("%s/%s-YearStats-DQPaper_.html"%(maindir,iSubSystem),"w")
        addHeader(yearStatsSystem)
        for iYear in ["2015","2016","2017","2018"]:
            yearStatsSystem.write('''<div style="text-align:left" class="rectangle"><b>%s</b>'''%(iYear))
            yearStatsSystem.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-LAr/%s/DQPaper_%s/grl-defects.png" alt=""  width="750" content="no-cache" /></figure>'''%(iYear,iYear))
            if iSubSystem == "LAr":
                yearStatsSystem.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-LAr/%s/DQPaper_%s/grl-veto.png" alt=""  width="750" content="no-cache" /></figure>'''%(iYear,iYear))
            yearStatsSystem.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-LAr/%s/DQPaper_%s/grl-lumi.png" alt=""  width="750" content="no-cache" /></figure>'''%(iYear,iYear))
            yearStatsSystem.write('''</div>''')
        addFooter(yearStatsSystem)
        yearStatsSystem.close()

## Run 2 RecapDefect page per system
for iSubSystem in subsystemName.keys():
    if (iSubSystem not in subsystemNotInRun2):
        recapDefectSystem = open("%s/%s-recapDefects-DQPaper_.html"%(maindir,iSubSystem),"w")
        addHeader(recapDefectSystem)
        addHeaderRun2RecapDefects(recapDefectSystem)
        for iYear in ["2015","2016","2017","2018"]:
            recapDefectSystem.write('''<hr>''')
            recapDefectSystem.write('''<a id='%s'> %s dataset </a>'''%(iYear,iYear))
            readFile = open("YearStats-%s/%s/DQPaper_%s/recapDefects.html"%(iSubSystem,iYear,iYear))
            for line in readFile:
                recapDefectSystem.write(line)                    
        addFooter(recapDefectSystem)
        recapDefectSystem.close()
