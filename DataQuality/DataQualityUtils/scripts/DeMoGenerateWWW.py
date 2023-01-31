#! /usr/bin/env python
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Author : Benjamin Trocme (CNRS/IN2P3 - LPSC Grenoble) - 2022
#
# Script used to generate to https://atlasdqm.web.cern.ch/atlasdqm/DeMo/ webpage
#
# Documentation: https://twiki.cern.ch/twiki/bin/viewauth/Atlas/DataQualityDemo
#############################################################################################

import sys, socket, pathlib, errno, re, subprocess, os

maindir = "/afs/cern.ch/user/a/atlasdqm/www/DeMo"
libdir = os.path.dirname(__file__)

run2 = False
run3 = True

global run3_yt
run3_yt = {"2022":["Tier0_2022","GRL_2022"],
           "2023":["Tier0_2023","GRL_2023"]}

tag_menuName = {"Tier0_2022":"All",
                "GRL_2022":"GRL",
                "Tier0_2023":"All",
                "GRL_2023":"GRL"}

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
# When https is found in a character line, add a link in html
# If none, simply return the line unchanged.
def addLink(l):
    htmlSplitted = l.split("https")

    if len(htmlSplitted)>1: # Url found -> Direct link added
        urlAlreadyReplaced = []
        for iUrl in range(1,len(htmlSplitted)):
            url = "https%s"%(htmlSplitted[iUrl].split(" ")[0])
            if (url not in urlAlreadyReplaced):
                l = l.replace(url,'''<a href="%s" target="_blank"> %s </a>'''%(url,url))
            urlAlreadyReplaced.append(url)

    return l


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

    ### Weekly recap overview - Only in Run3
    if (run3):
        page.write('''   <li><a href='Weekly.html'><span>Weekly overview</span></a></li>''')

    ### Year stats for all systems
    page.write('''   <li class='active has-sub'><a><span>Overview</span></a>''')
    page.write('''     <ul>''')
    if (run3):
        for iYear in run3_yt.keys():
            for iTag in run3_yt[iYear]:
                page.write('''       <li><a href='YearStats-%s-%s.html'><span>Year stats - %s - %s</span></a></li>'''%(iYear,iTag,iYear,tag_menuName[iTag]))
                page.write('''       <li><a href='recapDefectsHighlights-%s-%s.html'><span>Defect highlights (beta) - %s - %s</span></a></li>'''%(iYear,iTag,iYear,tag_menuName[iTag]))

    if (run2):
        page.write('''       <li><a href='YearStats-2018-DQPaper_2018.html'><span>2018 overview - DQ Paper</span></a></li>''')
        page.write('''       <li><a href='YearStats-2017-DQPaper_2017.html'><span>2017 overview - DQ Paper</span></a></li>''')
        page.write('''       <li><a href='YearStats-2016-DQPaper_2016.html'><span>2016 overview - DQ Paper</span></a></li>''')
        page.write('''       <li><a href='YearStats-2015-DQPaper_2015.html'><span>2015 overview - DQ Paper</span></a></li>''')

    page.write('''     </ul>''')
    page.write('''   </li>''')
    
    for iSystem in subsystems.keys():
        page.write('''   <li class='active has-sub'><a><span>%s</span></a>'''%iSystem)
        page.write('''      <ul>''')

        for iSystem in subsystems[iSystem]:
            if ((run3) and (iSystem not in subsystemNotInRun3)) or ((run2) and (iSystem not in subsystemNotInRun2)):
                page.write('''<li class='has-sub'><a ><span>%s</span></a>'''%subsystemName[iSystem])
                page.write('''   <ul>''')
            else:
                continue

            if (run3) and (iSystem not in subsystemNotInRun3):
                page.write('''      <li><a href='%s-Weekly.html'><span>Weekly</span></a></li>'''%(iSystem))
                for iYear in run3_yt.keys():
                    for iTag in run3_yt[iYear]:
                        page.write('''      <li><a href='%s-YearStats-%s-%s.html'><span>Year stats - %s - %s</span></a></li>'''%(iSystem,iYear,iTag,iYear,tag_menuName[iTag]))
                        page.write('''      <li><a href='%s-recapDefects-%s-%s.html'><span>Defect recap - %s - %s </span></a></li>'''%(iSystem,iYear,iTag,iYear,tag_menuName[iTag]))
            if (run2) and (iSystem not in subsystemNotInRun2):
                page.write('''      <li><a href='%s-YearStats-DQPaper_.html'><span>Year stats - Run2</span></a></li>'''%(iSystem))
                page.write('''      <li class='last'><a href='%s-recapDefects-DQPaper_.html'><span>Defect recap - Run2</span></a></li>'''%(iSystem))
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
    page.write('''Link to the <a href="logs/DeMoCron.log"> cron job output </a> - Documentation available <a href="https://twiki.cern.ch/twiki/bin/view/Atlas/DataQualityDemo"> here </a> <br>''')
    page.write('''Original developer: Benjamin Trocm&eacute (LPSC Grenoble) - Maintenance: Benjamin Trocm&eacute (LPSC Grenoble)''')
    page.write('''</div></body>''')
    page.write('''<html>''')
    return

################################################################
def addHeaderWeekly(page):
    page.write('''<div style="text-align:left" class="rectangle">''')
    page.write('''Considered dataset: <br>''')
    page.write('''- all ATLAS-ready runs acquired in the last 7 days <br>''')
    page.write('''- all ATLAS-ready runs fully signed off in the last 7 days <br>''')
    page.write('''- all runs considered for the upcoming wednesday formal signoff (list available <a href="RunList/weekly.dat" target="_blank"> here </a>)<br>''')
    page.write('''Plots updated daily at midnight<br>''')
    page.write('''Status meaning (based on UNCHECKED defect status):<br>''')
    page.write('''-EXPR.   : no assessment yet<br>''')
    page.write('''-BULK    : express assessment completed<br>''')
    page.write('''-DONE    : express and bulk assessment completed (but no final signoff - relevant only for LAr)<br>''')
    page.write('''-FINAL OK: assessment completed<br>''')
    page.write('''The spade symbol after the status indicates that this run is supposed to be signed off at the upcoming DQ wednesday meeting <br>''')
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

################################################################
def createRun2LegacyPages():
    ## Run 2 Year stats for all systems
    for iYear in ["2015","2016","2017","2018"]:
        page = open("%s/YearStats-%s-DQPaper_%s.html"%(maindir,iYear,iYear),"w")
        addHeader(page)
        for iSystem in subsystemName.keys():
            if (iSystem not in subsystemNotInRun2):
                page.write('''<div style="text-align:left" class="rectangle"> <b>%s</b>'''%iSystem)
                page.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/DQPaper_%s/grl-defects.png" alt=""  width="750" content="no-cache" /></figure>'''%(iSystem,iYear,iYear))
                if iSystem == "LAr":
                    page.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/DQPaper_%s/grl-veto.png" alt=""  width="750" content="no-cache" /></figure>'''%(iSystem,iYear,iYear))
                page.write('''</div>''')
        addFooter(page)
        page.close()

    ## Run 2 YearStats page per system
    for iSystem in subsystemName.keys():
        if (iSystem not in subsystemNotInRun3):
            page = open("%s/%s-YearStats-DQPaper_.html"%(maindir,iSystem),"w")
            addHeader(page)
            for iYear in ["2015","2016","2017","2018"]:
                page.write('''<div style="text-align:left" class="rectangle"><b>%s</b>'''%(iYear))
                page.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-LAr/%s/DQPaper_%s/grl-defects.png" alt=""  width="750" content="no-cache" /></figure>'''%(iYear,iYear))
                if iSystem == "LAr":
                    page.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-LAr/%s/DQPaper_%s/grl-veto.png" alt=""  width="750" content="no-cache" /></figure>'''%(iYear,iYear))
                page.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-LAr/%s/DQPaper_%s/grl-lumi.png" alt=""  width="750" content="no-cache" /></figure>'''%(iYear,iYear))
                page.write('''</div>''')
            addFooter(page)
            page.close()

    ## Run 2 RecapDefect page per system
    for iSystem in subsystemName.keys():
        if (iSystem not in subsystemNotInRun2):
            page = open("%s/%s-recapDefects-DQPaper_.html"%(maindir,iSystem),"w")
            addHeader(page)
            addHeaderRun2RecapDefects(page)
            for iYear in ["2015","2016","2017","2018"]:
                page.write('''<hr>''')
                page.write('''<a id='%s'> %s dataset </a>'''%(iYear,iYear))
                readFile = open("YearStats-%s/%s/DQPaper_%s/recapDefects.html"%(iSystem,iYear,iYear))
                for line in readFile:
                    page.write(line)                    
            addFooter(page)
            page.close()

################################################################
def createWeeklyOverview(weeklyYear,weeklyTag):
    weekly = open("%s/Weekly.html"%maindir,"w")
    addHeader(weekly)
    addHeaderWeekly(weekly)
    for iSystem in subsystemName.keys():
        if (iSystem not in subsystemNotInRun3):
            weekly.write('''<div style="text-align:left" class="rectangle"> <b>%s</b>'''%subsystemName[iSystem])
            weekly.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/Weekly/summary-0.png" alt=""  width="750" content="no-cache"/></figure>'''%(iSystem,weeklyYear,weeklyTag))
            weekly.write('''<a> Link to the <a href="%s-Weekly.html" > weekly plots.</a>'''%(iSystem))
            weekly.write('''</div>''')
    addFooter(weekly)
    weekly.close()

################################################################
def createWeeklySystem(weeklyYear,weeklyTag,system):
    page = open("%s/%s-Weekly.html"%(maindir,system),"w")
    addHeader(page)
    addHeaderWeekly(page)
    page.write('''<div style="text-align:left" class="rectangle"><b>%s</b> - <a href="YearStats-%s/%s/%s/daemon-weekly.out" target="_blank"> Daemon output </a>'''%(subsystemName[system],system,weeklyYear,weeklyTag))
    page.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/Weekly/defects--Run--%s.png" alt=""  width="750" content="no-cache" /></figure>'''%(system,weeklyYear,weeklyTag,weeklyTag))
    if system == "LAr" :
        page.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/Weekly/veto--Run--%s.png" alt=""  width="750" content="no-cache" /></figure>'''%(system,weeklyYear,weeklyTag,weeklyTag))
        page.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/Weekly/noiseBurst_veto_evol.png" alt=""  width="750" content="no-cache" /></figure>'''%(system,weeklyYear,weeklyTag))
    page.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/Weekly/summary-0.png" alt=""  width="750" content="no-cache" /></figure>'''%(system,weeklyYear,weeklyTag))
    addFooter(page)
    page.close()

################################################################
def createYearStatsSystem(year,tag,system):
    yearTag = "%s_%s"%(year,tag)
    page = open("%s/%s-YearStats-%s-%s.html"%(maindir,system,year,tag),"w")
    addHeader(page)
    addHeaderYearStatsRecap(page,year,tag,system)
    page.write('''<div style="text-align:left" class="rectangle"><b>%s - %s</b> - <a href="YearStats-%s/%s/%s/daemon-grl.out" target="_blank"> daemon output </a> - <a href="YearStats-%s/%s/%s/runs-notYetSignedOff.dat" target="_blank"> Runs not yet signed off </a> - Switch to the <a href="%s-recapDefects-%s-%s.html"> defect recap page</a> </div>'''%(year,tag_menuName[tag],system,year,tag,system,year,tag,system,year,tag))
    page.write('''<div style="text-align:left" class="rectangle"><figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/grl-defects.png" alt=""  width="750" content="no-cache" /></figure>'''%(system,year,tag))
    if system == "LAr" :
        page.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/grl-veto.png" alt=""  width="750" content="no-cache" /></figure>'''%(system,year,tag))
    page.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/grl-lumi.png" alt=""  width="750" content="no-cache" /></figure>'''%(system,year,tag))
    page.write('''</div>''')
    addFooter(page)
    page.close()

################################################################
def createYearStatsOverview(year,tag):
    yearTag = "%s_%s"%(year,tag)
    page = open("%s/YearStats-%s-%s.html"%(maindir,year,tag),"w")
    addHeader(page)
    addHeaderYearStatsRecap(page,year,tag)
    for iSystem in subsystemName.keys():
        if (iSystem not in subsystemNotInRun3):
            page.write('''<div style="text-align:left" class="rectangle"> <b>%s</b>'''%subsystemName[iSystem])
            page.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/grl-defects.png" alt=""  width="750" content="no-cache" /></figure>'''%(iSystem,year,tag))
            page.write('''</div>''')
    addFooter(page)
    page.close()

################################################################
def createDefectRecapSystem(year,tag,system):
    yearTag = "%s_%s"%(year,tag)
    page = open("%s/%s-recapDefects-%s-%s.html"%(maindir,system,year,tag),"w")
    addHeader(page)
    addHeaderYearStatsRecap(page,year,tag,system)
    page.write('''<div style="text-align:left" class="rectangle"><b>%s - %s</b> - <a href="YearStats-%s/%s/%s/daemon-recapDefects.out" target="_blank"> daemon output </a> - <a href="YearStats-%s/%s/%s/runs-notYetSignedOff.dat" target="_blank"> Runs not yet signed off </a> - Switch to the <a href="%s-YearStats-%s-%s.html"> year stats page</a> </div>'''%(year,tag_menuName[tag],system,year,tag,system,year,tag,system,year,tag))

    if (os.path.exists("YearStats-%s/%s/%s/recapDefects.html"%(system,year,tag))):        
        readFile = open("YearStats-%s/%s/%s/recapDefects.html"%(system,year,tag))
        for line in readFile: # NB: the recapDefects.html files contain a single line for the whole table. 
            page.write(addLink(line))

    addFooter(page)
    page.close()

################################################################
def createDefectRecapHighlights(year,tag):
    yearTag = "%s_%s"%(year,tag)
    page = open("%s/recapDefectsHighlights-%s-%s.html"%(maindir,year,tag),"w")
    addHeader(page)
    addHeaderYearStatsRecap(page,year,tag)

    defectsToKeep = {"Pixel":["DISABLED"],
                     "TRT":["DISABLED"],
                     "RPC":["DISABLED"],
                     "MDT":["DISABLED"],
                     "TGC":["DISABLED"],
                     "Global":["TOROID_OFF"],
                     }

    runHighlights = {}

    for iSystem in subsystemName.keys():
        if (iSystem in defectsToKeep.keys()) and (iSystem not in subsystemNotInRun3):
            if (os.path.exists("YearStats-%s/%s/%s/recapDefects.txt"%(iSystem,year,tag))):        
                readFile = open("YearStats-%s/%s/%s/recapDefects.txt"%(iSystem,year,tag))

                currentDefect = ""
                currentRun = ""

                for line in readFile:
                    if line.startswith("===== Recap for "):
                        currentDefect = ((line.split("===== Recap for ")[1]).split("===")[0]).replace(" ","")
                    elif line.startswith("Description:") or line.startswith("     Run") or (currentDefect not in defectsToKeep[iSystem]):
                        continue
                    else:
                        iovLine = line.split("|")
                        if (len(iovLine) == 8): # The defect to keep should be on a single line (i.e. not multiple iov in a given run)
                            runFound = iovLine[0]
                            if (runFound not in runHighlights.keys()):
                                runHighlights[runFound] = {}
                                runHighlights[runFound]["TotLumi"] = iovLine[2]
                            runHighlights[runFound][iSystem] = "<th>%s</th><th>%s : %s</th>"%(addLink(iovLine[3]),currentDefect,iovLine[7])

    page.write('''<table class="report"><tr class="out0intolerable"> <th width="150pix"></th><th width="150pix"></th></th><th width="1500pix"></th></tr>''')

    listOfRuns = list(runHighlights.keys())
    listOfRuns.sort(reverse=True)
    
    for iRun in listOfRuns:
        page.write('''<tr class="out0intolerable"> <th colspan="3"> Runs %s - Integrated luminosity : %s </th></tr>'''%(iRun,runHighlights[iRun]["TotLumi"]))
        page.write('''<tr class="out0"><th> System </th> <th> Lost lumi </th> <th> Comment </th> </tr> ''')
        for iSystem in runHighlights[iRun].keys():
            if iSystem == "TotLumi":
                continue
            page.write('''<tr class="out1"><th> %s </th> %s </tr>'''%(iSystem,runHighlights[iRun][iSystem]))

    page.write('''</table>''')


    addFooter(page)
    page.close()


###################################################################################################################
###################################################################################################################
# Main script

homepage = open("%s/index.html"%maindir,"w")
addHeader(homepage)
addFooter(homepage)
homepage.close()

## Run2 legacy pages
if (run2):
    createRun2LegacyPages()

# Run3 pages
if (run3):
    weeklyYear = "2022"
    weeklyTag = "Tier0_2022"

    ## Weekly page for all systems
    createWeeklyOverview(weeklyYear,weeklyTag)

    ## Weekly page per system
    for iSystem in subsystemName.keys():
        if iSystem not in subsystemNotInRun3:
            createWeeklySystem(weeklyYear,weeklyTag,iSystem)

    ## YearStats page for all systems
    for iYear in run3_yt.keys():
        for iTag in run3_yt[iYear]:
            createYearStatsOverview(iYear,iTag)

    ## YearStats page per system
    for iYear in run3_yt.keys():
        for iTag in run3_yt[iYear]:
            for iSystem in subsystemName.keys():
                if (iSystem not in subsystemNotInRun3):
                    createYearStatsSystem(iYear,iTag,iSystem)

    ## RecapDefect page per system
    for iYear in run3_yt.keys():
        for iTag in run3_yt[iYear]:
            for iSystem in subsystemName.keys():
                if (iSystem not in subsystemNotInRun3):
                    createDefectRecapSystem(iYear,iTag,iSystem)

    for iYear in run3_yt.keys():
        for iTag in run3_yt[iYear]:
            createDefectRecapHighlights(iYear,iTag)
