# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# This script genrates a json file containing the Micromegas cabling correction map

import json

def zebraChannels(zebra):
    '''
    Returns a tuple with the first and the last channel in a MM zebra connector.

        Parameters:
            zebra (int) identifies the zebra connector number within a layer. Counting from 0 to 31
        
        Returns:
            tuple containing the first and the last strip of a given zebra connector
    '''

    # One zebra connector covers 256 channels
    # The athena channel numbering starts from 1
    # The athena channel numbering restarts from 1 for the second quadruplet (zebra connectors 20 to 31)

    if(zebra>19): zebra -= 20 # zebra counting covers a full layer of a sector while the channel numbering restarts in the second quadruplet
    firstChannel = zebra*256 + 1
    lastChannel = (zebra+1)*256
    return (firstChannel, lastChannel)

def MMFE8Channels(mmfe8):
    '''
    Returns a tuple with the first and the last channel in an MMFE (Micromegas front end board).

        Parameters:
            mmfe8 (int) identifies the zebra connector number within a layer. Counting from 0 to 15
        
        Returns:
            tuple containing the first and the last strip of a given MMFE8
    '''

    # One MMFE8 covers 512 channels
    # The athena channel numbering starts from 1
    # The athena channel numbering restarts from 1 for the second quadruplet (MMFE8s 10 to 15)

    if(mmfe8>9): mmfe8 -= 10 # MMFE8 counting covers a full layer of a sector while the channel numbering restarts in the second quadruplet
    firstChannel = mmfe8*512 + 1
    lastChannel = (mmfe8+1)*512
    return (firstChannel, lastChannel)



def addZebra(side, sector, layer, zebra, shift):
    '''
    Returns a dictionary which represents a shift of a zebra connector

        Parameters:
            side (string) identifies the wheel: possible values "A" and "C"
            sector (int) identifies the sector counting from 1 to 16
            layer (int) identifies the detector layer whithin a given sector. Counts from 1 to 8
            zebra (int) identifies the zebra connector number within a layer. Counting from 0 to 31
            shift (int) indicates by how much and in which direction the channels in a given connector should be moved
        
        Returns:
            dictionary with the fields that the NSW cabling alg will use to apply the shift
    '''
    ret = dict()
    ret["station"] = ("MML" if sector%2==1 else "MMS") # the even sectors are the small ones
    ret["eta"] = (-1 if side == "C" else 1) * (1 if zebra < 20  else 2) # side C is indicated by a negative station eta while side A by a positive one. The first station eta (or quadruplet) consist of 5 PCBs per layer which corresponds to zebra connectors 0 to 19. Connector 20 to 31 are reading out abs(stationEta) 2
    ret["phi"] = (sector-1)//2 + 1
    ret["multilayer"] = (1 if layer <=4 else 2) # each multilayer consists of 4 layers
    ret["gasgap"] = ((layer-1)%4) +1 # counting from 1 to 4
    ret["FirstZebra"], ret["LastZebra"] = zebraChannels(zebra)
    ret["ZebraShift"] = shift
    return ret

def addMMFE8(side, sector, layer, mmfe8, shift):
    '''
    Returns a dictionary which represents a shift of a zebra connector

        Parameters:
            side (string) identifies the wheel: possible values "A" and "C"
            sector (int) identifies the sector counting from 1 to 16
            layer (int) identifies the detector layer whithin a given sector. Counts from 1 to 8
            mmfe8 (int) identifies the MMFE8 number within a layer. Counting from 0 to 15
            shift (int) indicates by how much and in which direction the channels in a given connector should be moved
        
        Returns:
            dictionary with the fields that the NSW cabling alg will use to apply the shift
    '''
    ret = dict()
    ret["station"] = ("MML" if sector%2==1 else "MMS") # the even sectors are the small ones
    ret["eta"] = (-1 if side == "C" else 1) * (1 if mmfe8 < 10  else 2) # side C is indicated by a negative station eta while side A by a positive one. The first station eta (or quadruplet) consist of 5 PCBs per layer which corresponds to MMFE8s 0 to 9. MMFE8 10 to 15 are reading out abs(stationEta) 2
    ret["phi"] = (sector-1)//2 + 1
    ret["multilayer"] = (1 if layer <=4 else 2) # each multilayer consists of 4 layers
    ret["gasgap"] = ((layer-1)%4) +1 # counting from 1 to 4
    ret["FirstZebra"], ret["LastZebra"] = MMFE8Channels(mmfe8)
    ret["ZebraShift"] = shift
    return ret





if __name__ == "__main__":
    # for now we generate some dummy shifts for testing
    outList = []
    outList.append(addMMFE8("A",1,1,0,1))
    outList.append(addZebra("C",6, 3, 24, 1))

    with open("MMGZebraShift.json","w") as f:
        json.dump(outList,f)
