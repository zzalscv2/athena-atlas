# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# @file PyUtils.FilePeekerTool
# @purpose peek into APR files to read in-file metadata without Athena (based on PyAthena.FilePeekerLib code by Sebastian Binet) 
# @author Alexandre Vaniachine <vaniachine@anl.gov>
# @date May 2015

from __future__ import print_function

__author__ = "Alexandre Vaniachine <vaniachine@anl.gov>"
__doc__ = "peek into APR files to read in-file metadata"

### imports --------------------------------------------------------------------
import os
from CLIDComps.clidGenerator import clidGenerator
cgen = clidGenerator("")

class FilePeekerTool():
    """utility algorithm to inspect a file's content
    """

    def __init__(self, f_root):
        self.f = f_root
        self.peeked_data = {}

    def run(self):
        self.peeked_data = self._do_peeking()
        return self._save_db_cache()

    def _do_peeking(self):
        """ the real function doing all the work of peeking at the APR file
        @return a dict of peeked-at data
        """

        import uuid
        stdout_fname = (
            'athfile-%i-%s.log.txt' %
            (os.getpid(), uuid.uuid4())
            )
        stdout = open(stdout_fname, "w")
        print ("Extracting in-file metadata without athena sub-process from file", self.f.GetName(), file=stdout)
        print ("="*80, file=stdout)
        stdout.flush()

        pool = self.f.Get("##Params")
        import re
        pool_token = re.compile(r'[\[]NAME=(?P<name>.*?)[]]' r'[\[]VALUE=(?P<value>.*?)[]]').match
        params = []
        for i in range(pool.GetEntries()):
            if pool.GetEntry(i)>0:
                match = pool_token(pool.FindLeaf("db_string").GetValueString())
                if not match:
                    continue
            d = match.groupdict()
            params.append((d['name'], d['value']))
            if d['name'].lower() == 'fid':
                guid = d['value']

        meta = self.f.Get( 'MetaData' )
        if not meta:
            print ('No metadata', file=stdout)
            return {}

        from AthenaPython.FilePeekerLib import toiter

        meta.GetEntry( 0 )

        esiName= 'Stream'
        esiTypeName = 'EventStreamInfo'
        for l in meta.GetListOfLeaves():
            if l.GetTypeName().startswith(esiTypeName):
                esiTypeName = l.GetTypeName()
                esiName = l.GetName()
                break

        if esiTypeName != 'EventStreamInfo_p3':
            print ("old schema is not supported:", esiTypeName, file=stdout)
            return {}

        import cppyy

        esic = cppyy.gbl.EventStreamInfoPTCnv_p3()
        esi = getattr (meta, esiName)
        if esiName.startswith(esiTypeName):
            esiName = esiName[len(esiTypeName)+1:]

        peeked_data = {}

        if guid:
            peeked_data['file_guid'] = guid

        peeked_data['run_type'] = ['N/A']

        print (peeked_data, file=stdout)

        peeked_data['nentries'] = esic.getNumberOfEvents(esi)
        peeked_data['lumi_block'] = list(esic.lumiBlockNumbers(esi))
        peeked_data['run_number'] = list(esic.runNumbers(esi))
        peeked_data['stream_names'] = [s for s in esic.processingTags(esi)]

        item_list = list( (cgen.getNameFromClid(i.first), i.second) for i in esic.itemList(esi))
        #reorder items to match that of StoreGate
        if ('DataHeader', esiName) in item_list:
            item_list.remove(('DataHeader', esiName))
            item_list.append(('DataHeader', esiName))

        peeked_data['eventdata_items'] = item_list

        etc = cppyy.gbl.EventTypePTCnv_p3()
        def bit_mask(self):
            raw_bit_mask=etc.bit_mask(self)
            bit_mask_typecodes = [ ('IS_DATA','IS_SIMULATION'), ('IS_ATLAS', 'IS_TESTBEAM'), ('IS_PHYSICS','IS_CALIBRATION'),]
            def decode_bitmask(idx):
                if len(raw_bit_mask) <= idx:
                    return bit_mask_typecodes[idx][0]
                isa_idx = raw_bit_mask[idx]
                return bit_mask_typecodes[idx][isa_idx]
            bm = map(decode_bitmask,
                     range(len(bit_mask_typecodes)))
            return tuple(bm)

        def _get_detdescr_tags(evt_type):
            ddt = evt_type.get_detdescr_tags().split()
            # det_descr_tags is of the form:
            # [ 'key1', 'val1', 'key2', 'val2', ... ]
            ddt = dict(zip(ddt[0::2],  # select 'key?'
                           ddt[1::2])) # select 'val?'
            return ddt

        etp=esic.eventTypes(esi)
        if etp.size() > 0:
            et=etp[0]

            peeked_data['evt_type'] = bit_mask(et)
    
            #ddt = _get_detdescr_tags(et)
            #peeked_data['det_descr_tags'] = ddt

            peeked_data['mc_channel_number'] = [et.m_mc_channel_number]
            peeked_data['evt_number'] = [et.m_mc_event_number]
            #printf ('mc_event_number', et.m_mc_event_number, file=stdout)
            print ('mc_event_weights.size:', et.m_mc_event_weights.size(), file=stdout)
            print ('mc_event_weights value', et.m_mc_event_weights[0], file=stdout)
            print ('user_type', et.m_user_type, file=stdout)

        # handle event-less files
        if peeked_data['nentries'] == 0:
            peeked_data['stream_names'] = [esiName]

        # more event-less files handling - APR-from-BS-event-less files
        # see bug#98568
        if len (peeked_data['run_number']) == 0 and meta.FindBranch('ByteStreamMetadata'):
            bsmd = cppyy.gbl.ByteStreamMetadataPTCnv_p1()
#            peeked_data['nentries'] = [bsmd.getNumEvents(meta.ByteStreamMetadata[0])]
            peeked_data['lumi_block'] = [bsmd.getLumiBlock(meta.ByteStreamMetadata[0])]
            peeked_data['run_number'] = [bsmd.getRunNumber(meta.ByteStreamMetadata[0])]
#            peeked_data['stream_names'] = [bsmd.getStream(meta.ByteStreamMetadata[0])]
            peeked_data['beam_type'] = [bsmd.getBeamType(meta.ByteStreamMetadata[0])]
            peeked_data['beam_energy'] = [bsmd.getBeamEnergy(meta.ByteStreamMetadata[0])]
            bs_metadata = {}
            for md in bsmd.getFreeMetaDataStrings(meta.ByteStreamMetadata[0]):
                if md.startswith('Event type:'):
                    k = 'evt_type'
                    v = []
                    if 'is sim' in md:   v.append('IS_SIMULATION')
                    else:                v.append('IS_DATA')
                    if 'is atlas' in md: v.append('IS_ATLAS')
                    else:                v.append('IS_TESTBEAM')
                    if 'is physics' in md: v.append('IS_PHYSICS')
                    else:                  v.append('IS_CALIBRATION')
                    bs_metadata[k] = tuple(v)
                elif md.startswith('GeoAtlas:'):
                    k = 'geometry'
                    v = md.split('GeoAtlas:')[1].strip()
                    bs_metadata[k] = v
                elif md.startswith('IOVDbGlobalTag:'):
                    k = 'conditions_tag'
                    v = md.split('IOVDbGlobalTag:')[1].strip()
                    bs_metadata[k] = v
                elif '=' in md:
                    k,v = md.split('=')
                    bs_metadata[k] = v
                    pass

            # ATMETADATA-6: without FreeMetaDataStrings this must be BS file from SFO
            bs_metadata['Stream'] = bsmd.getStream(meta.ByteStreamMetadata[0])
            bs_metadata['Project'] = bsmd.getProject(meta.ByteStreamMetadata[0])
            if len(bs_metadata.get('evt_type','')) == 0 :
                evt_type = ['IS_DATA', 'IS_ATLAS']
            if   bs_metadata.get('Stream', '').startswith('physics_'):
                evt_type.append('IS_PHYSICS')
            elif bs_metadata.get('Stream', '').startswith('calibration_'):
                evt_type.append('IS_CALIBRATION')
            elif bs_metadata.get('Project', '').endswith('_calib'):        
                evt_type.append('IS_CALIBRATION')
            else:
                evt_type.append('Unknown')
            bs_metadata['evt_type'] = evt_type

            peeked_data['evt_type']  = bs_metadata.get('evt_type', [])
            peeked_data['geometry']  = bs_metadata.get('geometry', None)
            peeked_data['conditions_tag'] = bs_metadata.get('conditions_tag', None)
            peeked_data['bs_metadata'] = bs_metadata
            pass

        cnv = cppyy.gbl.IOVMetaDataContainerPTCnv_p1()

        obj = cppyy.gbl.IOVMetaDataContainer()

        def process_metadata(obj, metadata_name):
            print ('processing container [%s]' % obj.folderName(), file=stdout)
            data = []
            payloads = obj.payloadContainer()
            payloads_sz = payloads.size()
            if hasattr(payloads, 'at'):
                # HACK for bug #77976
                _tmp = payloads
                payloads = []
                for ii in range(payloads_sz):
                    payloads.append(_tmp.at(ii))
                pass
            for ii,payload in zip(range(payloads_sz), payloads):
                #print ("-->",ii,payload,type(payload),'\n', file=stdout)
                if not payload:
                    print ("**error** null-pointer ?", file=stdout)
                    continue
                # names
                chan_names = []
                sz = payload.name_size()
                print ('==names== (sz: %s)' % sz, file=stdout)
                for idx in range(sz):
                    chan = payload.chanNum(idx)
                    chan_name = payload.chanName(chan)
                    #print ('--> (%s, %s)' % (idx, chan_name), file=stdout)
                    chan_names.append(chan_name)

                if 1: # we don't really care about those...
                # iovs
                    sz = payload.iov_size()
                    print ('==iovs== (sz: %s)' % sz, file=stdout)
                    for idx in range(sz):
                        chan = payload.chanNum(idx)
                        iov_range = payload.iovRange(chan)
                        iov_start = iov_range.start()
                        iov_stop  = iov_range.stop()
                        if 0:
                            print ('(%s, %s) => (%s, %s) valid=%s runEvt=%s' % (
                                   iov_start.run(),
                                   iov_start.event(),
                                   iov_stop.run(),
                                   iov_stop.event(),
                                   iov_start.isValid(),
                                   iov_start.isRunEvent()),
                                   file=stdout)

                # attrs
                attrs = [] # can't use a dict as spec.name() isn't unique
                sz = payload.size()
                print ('==attrs== (sz: %s)' % sz, file=stdout)
                for idx in range(sz):
                    chan = payload.chanNum(idx)
                    attr_list = payload.attributeList(chan)
                    attr_data = []
                    for a in list(toiter(attr_list.begin(), attr_list.end())):
                        spec   = a.specification()
                        a_type = spec.typeName()
                        if a_type.find('string') >= 0:
                            a_data = a.data['string']()
#                           a_data = getattr(a, 'data<std::basic_string<char> >') ()
                            try:
                                a_data = eval(a_data,{},{})
                            except Exception:
                                # swallow and keep as a string
                                pass
#                           print (spec.name(),a_data, file=stdout)
                        else:
                            a_data = a.data[a_type]()
                        #print ("%s: %s  %s" (spec.name(), a_data, type(a_data) ), file=stdout)
                        attr_data.append( (spec.name(), a_data) )
                    attrs.append(dict(attr_data))
                    #print (attrs[-1], file=stdout)
                if len(attrs) == len(chan_names):
                    data.append(dict(zip(chan_names,attrs)))
                else:
                    if len(attrs):
                        if len(attrs) == 1:
                            data.append(attrs[0])
                        else:
                            data.append(attrs)
                    else:
                        data.append(chan_names)
            pass # loop over payloads...
            return data

        metadata = {}
        def maybe_get(o, idx, default=None):
            try:
                return o[idx]
            except IndexError:
                return default
        def maybe_float(o):
            try:
                return float(o)
            except ValueError:
                return o

        # FIXME - read metadata_items from MetaDataHdr
        metadata_items = []

        for l in meta.GetListOfLeaves():
            clName = l.GetTypeName().split('_p')[0]
            flName = l.GetName()
            if l.GetTypeName() == 'IOVMetaDataContainer_p1':
                k = l.GetName()
                pers = getattr (meta, k)
                cnv.persToTrans(pers,obj)
                if 0:
                    try:
                        obj.payloadContainer().at(0).dump()
                    except Exception:
                        print (l.GetName(), file=stdout)
                        pass
                v = process_metadata(obj, k)
                #print (obj.folderName(),v, file=stdout)
                flName = obj.folderName()
                metadata[obj.folderName()] = maybe_get(v, -1)
#            if flName[:15] == 'TriggerMenuAux.' and clName[:6] == 'vector': continue
            if flName[:11] in ['TriggerMenu','CutBookkeep','IncompleteC'] and clName[:6] != 'xAOD::': continue
            metadata_items.append((clName,flName))
            if clName == 'EventStreamInfo':
                metadata_items.append(('DataHeader', ';00;MetaDataSvc'))

        peeked_data['metadata'] = metadata
        peeked_data['metadata_items'] = metadata_items

        # try to make unique Stream Tags set (but not for eventless files)
        if '/TRIGGER/HLT/Menu' in metadata and peeked_data['nentries'] > 0:
            ti = set()
            for d in metadata.get('/TRIGGER/HLT/Menu'):
                ti.add(d['StreamInfo'])

            # build stream_tags from /TRIGGER/HLT/Menu metadata
            stream_tags = []
            for item in ti:
                i = item.split(',')
                if len(i)!=3:
                    print('**error** Invalid StreamInfo entry:',item, file=stdout)
                else:
                    stream_tags.append( { 'obeys_lbk':bool(i[2]), 'stream_name':i[0], 'stream_type':i[1] } )

            peeked_data['stream_tags'] = stream_tags

        # AtlasRelease - reproduce AthFile behavior
        from PyUtils.Helpers import release_metadata
        rel_metadata = release_metadata()
        project = rel_metadata['project name']
        version = rel_metadata['release']
        release = project + '-' + version

        ## -- taginfo
        taginfo = {}
        if '/TagInfo' in metadata:
            taginfo = metadata['/TagInfo'].copy()

        # but not for eventless files
        if peeked_data['nentries'] > 0:
            taginfo['AtlasRelease'] = release

        peeked_data['tag_info'] = taginfo
        if taginfo:
            peeked_data['det_descr_tags'] = taginfo
            peeked_data['geometry'] = taginfo.get('GeoAtlas', None)
            peeked_data['conditions_tag'] = taginfo.get('IOVDbGlobalTag', None)
            peeked_data['beam_type'] = [taginfo.get('beam_type','N/A')]
            peeked_data['beam_energy']= [maybe_float(taginfo.get('beam_energy',
                                                                 'N/A'))]

        if 'evt_type' not in peeked_data: # must be eventless MC file
            if '/Simulation/Parameters' in metadata:
                peeked_data['evt_type'] = ['IS_SIMULATION', 'IS_ATLAS', 'IS_PHYSICS']
                peeked_data['run_number'] = [metadata['/Simulation/Parameters'].get('RunNumber','')]
            else:
                peeked_data['evt_type'] = []

        if 'geometry' not in peeked_data:
            peeked_data['geometry'] = None
        if 'conditions_tag' not in peeked_data:
            peeked_data['conditions_tag'] = None
        if 'det_descr_tags' not in peeked_data:
            peeked_data['det_descr_tags'] = {}

        ## -- summary
        print (':::::: summary ::::::', file=stdout)
        print (' - nbr events:  %s' % peeked_data['nentries'], file=stdout)
        print (' - run numbers: %s' % peeked_data['run_number'], file=stdout)
        #print (' - evt numbers: %s' % peeked_data['evt_number'], file=stdout)
        print (' - lumiblocks: %s' % peeked_data['lumi_block'], file=stdout)
        print (' - evt types: ', peeked_data['evt_type'], file=stdout)
        print (' - item list: %s' % len(peeked_data['eventdata_items']), file=stdout)
        #print (' - item list: ', peeked_data['eventdata_items'], file=stdout)
        print (' - processing tags: %s' % peeked_data['stream_names'], file=stdout)
        #print (' - stream tags: %s' % peeked_data['stream_tags'], file=stdout)
        print (' - geometry: %s' % peeked_data['geometry'], file=stdout)
        print (' - conditions tag: %s' % peeked_data['conditions_tag'], file=stdout)
        #print (' - metadata items: %s' % len(peeked_data['metadata_items']), file=stdout)
        print (' - tag-info: %s' % peeked_data['tag_info'].keys(), file=stdout)
        #print (' - item list: ' % peeked_data['eventdata_items'], file=stdout)
        stdout.flush()
        stdout.close()
        #os.remove(stdout.name)                                                                                             

        return peeked_data

    def _save_db_cache(self):
        """save file informations using sqlite"""
        import tempfile
        fd_pkl,out_pkl_fname = tempfile.mkstemp(suffix='.pkl')
#        import os
        os.close(fd_pkl)
        if os.path.exists(out_pkl_fname):
            os.remove(out_pkl_fname)
        oname = os.path.expanduser(os.path.expandvars(out_pkl_fname))
        if os.path.exists(oname):
            os.remove(oname)

        import PyUtils.dbsqlite as dbsqlite
        db = dbsqlite.open(oname,flags='w')
        db['fileinfos'] = self.peeked_data
        db.close()

        if os.path.exists(oname) and len(self.peeked_data) > 0:
            return 0, out_pkl_fname
        else:
            os.remove(oname)
            return 1, out_pkl_fname
