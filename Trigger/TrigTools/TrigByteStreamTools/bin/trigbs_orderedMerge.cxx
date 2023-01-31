/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file
 * Standalone executable to order and decompress events from multiple files and write them into a single file.
 *
 * This is intended mainly for merging files from same run and LB but different streams. The reordering ensures
 * uniform distribution of different kinds of events from different streams and removes duplicates. Using this script
 * with files from different LBs is pointless since all events in LB N+1 are always after all events from LB N.
 * The reordering is not strict and the level of out-of-order events is tied to the buffer size. The smaller the buffer,
 * the larger the chance of finding lower-ID events still to be read from input files.
 */

#include "eformat/FullEventFragmentNoTemplates.h"
#include "eformat/ROBFragmentNoTemplates.h"
#include "eformat/write/FullEventFragment.h"
#include "eformat/write/ROBFragment.h"
#include "eformat/compression.h"
#include "EventStorage/DataReader.h"
#include "EventStorage/pickDataReader.h"
#include "EventStorage/DataWriter.h"
#include "EventStorage/DRError.h"
#include "EventStorage/DWError.h"
#include "EventStorage/EventStorageRecords.h"
#include "EventStorage/ESCompression.h"

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <functional>
#include <unordered_set>

namespace {
  constexpr bool s_debugLogging{false};
  constexpr size_t s_printInterval{500};
  constexpr size_t s_defaultBufferSize{100};
}

struct Event {
  std::unique_ptr<const uint32_t[]> blob;
  eformat::read::FullEventFragment frag;
  explicit Event(std::unique_ptr<const uint32_t[]>&& b) : blob(std::move(b)) {
    frag = eformat::read::FullEventFragment(blob.get());
  }
};

class WriteEvent {
public:
  enum class Status {OK=0, NOT_OK=1};

private:
  Status m_status{Status::NOT_OK};
  std::unique_ptr<uint32_t[]> m_blob;
  eformat::write::FullEventFragment m_frag;
  std::vector<eformat::write::ROBFragment> m_robs;
  std::vector<std::unique_ptr<uint32_t[]>> m_robBlobs;
  uint32_t m_size{0};

public:
  explicit WriteEvent(const Event& readEvent) {
    m_frag.status(readEvent.frag.nstatus(), readEvent.frag.status());
    m_frag.run_type(readEvent.frag.run_type());
    m_frag.run_no(readEvent.frag.run_no());
    m_frag.global_id(readEvent.frag.global_id());
    m_frag.lumi_block(readEvent.frag.lumi_block());
    m_frag.bc_id(readEvent.frag.bc_id());
    m_frag.bc_time_seconds(readEvent.frag.bc_time_seconds());
    m_frag.bc_time_nanoseconds(readEvent.frag.bc_time_nanoseconds());
    m_frag.lvl1_id(readEvent.frag.lvl1_id());
    m_frag.lvl1_trigger_type(readEvent.frag.lvl1_trigger_type());
    m_frag.lvl1_trigger_info(readEvent.frag.nlvl1_trigger_info(), readEvent.frag.lvl1_trigger_info());
    m_frag.hlt_info(readEvent.frag.nhlt_info(), readEvent.frag.hlt_info());
    m_frag.stream_tag(readEvent.frag.nstream_tag(), readEvent.frag.stream_tag());
    m_frag.compression_type(eformat::Compression::UNCOMPRESSED);
    m_frag.compression_level(0);
    std::vector<eformat::read::ROBFragment> readRobs;
    readEvent.frag.robs(readRobs);
    m_robs.reserve(readRobs.size());
    for (const eformat::read::ROBFragment& rob : readRobs) {
      m_robs.emplace_back(rob.start());
      const eformat::write::node_t* top = m_robs.back().bind();
      const uint32_t writeSize = m_robs.back().size_word();
      m_robBlobs.push_back(std::make_unique<uint32_t[]>(writeSize));
      const uint32_t writtenSize = eformat::write::copy(*top, m_robBlobs.back().get(), writeSize);
      if (writtenSize != writeSize) {
        std::cerr << "Error in ROB serialisation, copied " << writtenSize << " instead of " << writeSize
                  << " words, skipping ROB" << std::endl;
        m_status = Status::NOT_OK;
        continue;
      }
      m_frag.append_unchecked(m_robBlobs.back().get());
    }

    const eformat::write::node_t* top = m_frag.bind();
    m_size = m_frag.size_word();
    m_blob = std::make_unique<uint32_t[]>(m_size);
    const uint32_t writtenSize = eformat::write::copy(*top, m_blob.get(), m_size);
    if (writtenSize != m_size) {
      std::cerr << "Error in event serialisation, copied " << writtenSize << " instead of " << m_size << " words" << std::endl;
      m_status = Status::NOT_OK;
    }

    m_status = Status::OK;
  }
  Status status() const {return m_status;}
  const std::unique_ptr<uint32_t[]>& blob() const {return m_blob;}
  const eformat::write::FullEventFragment& frag() const {return m_frag;}
  uint32_t size() const {return m_size;}
};

class Buffer {
private:
  std::unique_ptr<EventStorage::DataReader> m_reader;
  size_t m_size{0};
  std::vector<Event> m_events;
  void sort() {
    std::sort(m_events.begin(), m_events.end(), [](const Event& a, const Event& b){
      return a.frag.global_id() < b.frag.global_id();
    });
  }
  EventStorage::DRError fillBuffer() {
    while (m_reader->good() && m_events.size() < m_size) {
      char* blobChars{nullptr};
      unsigned int blobCharsSize{0};
      if (const EventStorage::DRError err_code = m_reader->getData(blobCharsSize, &blobChars); err_code != EventStorage::DRError::DROK) {
        std::cerr << "Error code " << err_code << " from EventStorage::DataReader::getData" << std::endl;
        return err_code;
      }
      const uint32_t* blobWords = reinterpret_cast<const uint32_t*>(blobChars);
      m_events.emplace_back(std::unique_ptr<const uint32_t[]>(blobWords));
    }
    sort();
    return EventStorage::DRError::DROK;
  }

public:
  Buffer(std::unique_ptr<EventStorage::DataReader>&& reader, size_t size)
  : m_reader(std::move(reader)), m_size(size) {
    fillBuffer();
    if (s_debugLogging) {
      std::cout << "Constructed a Buffer with " << m_events.size() << " events loaded, global_id range: ["
                << m_events.front().frag.global_id() << ", " << m_events.back().frag.global_id() << "]" << std::endl;
    }
  }
  std::optional<std::reference_wrapper<const Event>> peek() const {
    if (m_events.empty()) {return std::nullopt;}
    return m_events.at(0);
  }
  Event next() {
    Event event{std::move(m_events[0].blob)};
    m_events[0].blob.reset();
    m_events.erase(m_events.begin());
    fillBuffer();
    return event;
  }
  unsigned int eventsInFile() {
    return m_reader->eventsInFile();
  }
  const EventStorage::DataReader& reader() const {
    return *m_reader;
  }
};

std::unique_ptr<EventStorage::DataWriter> createWriter(const EventStorage::DataReader& reader) {
  EventStorage::run_parameters_record rp{};
  rp.run_number = reader.runNumber();
  rp.max_events = 0;
  rp.rec_enable = 0;
  rp.trigger_type = reader.triggerType();
  constexpr static std::bitset<128> bitMask64{0xffffffffffffffff};
  rp.detector_mask_LS = (reader.detectorMask() & bitMask64).to_ulong();
  rp.detector_mask_MS = ((reader.detectorMask() >> 64) & bitMask64).to_ulong();
  rp.beam_type = reader.beamType();
  rp.beam_energy = reader.beamEnergy();

  return std::make_unique<EventStorage::DataWriter>(
    /* writingPath  = */ ".",
    /* fileNameCore = */ "orderedMerge",
    /* rPar         = */ rp,
    /* fmdStrings   = */ reader.freeMetaDataStrings(),
    /* startIndex   = */ 1,
    /* compression  = */ EventStorage::CompressionType::NONE,
    /* compLevel    = */ 0
  );
}

int main(int argc, char** argv) {

  if (argc < 3) {
    std::cout << "Usage: " << argv[0] << " FILE FILE [FILE]" << std::endl
              << "At least 2 input files are required" << std::endl;
    return 1;
  }

  std::vector<Buffer> buffers;
  size_t total_events{0}, read_events{0};
  std::unordered_set<uint64_t> written_event_ids;
  uint64_t max_event_id{0};

  std::cout << "Reading " << argc-1 << " files:" << std::endl;
  for (int i=1; i<argc; ++i) {
    std::cout << " - " << argv[i] << std::endl;
    buffers.emplace_back(std::unique_ptr<EventStorage::DataReader>(pickDataReader(argv[i])), s_defaultBufferSize);
    total_events += buffers.back().eventsInFile();
  }

  std::cout << "Input files contain " << total_events << " events in total" << std::endl;

  std::unique_ptr<EventStorage::DataWriter> writer = createWriter(buffers[0].reader());

  while (read_events < total_events) {
    std::optional<std::reference_wrapper<Buffer>> next_buffer;
    uint64_t lowest_event_id{std::numeric_limits<uint64_t>::max()};
    int buffer_id{-1}, next_buffer_id{-1};
    for (Buffer& buffer : buffers) {
      ++buffer_id;
      std::optional<std::reference_wrapper<const Event>> event = buffer.peek();
      if (not event.has_value()) {continue;}
      const uint64_t event_id{event.value().get().frag.global_id()};
      if (event_id < lowest_event_id) {
        lowest_event_id = event_id;
        next_buffer = buffer;
        if (s_debugLogging) {
          next_buffer_id = buffer_id;
        }
      }
    }
    if (not next_buffer.has_value()) {
      std::cout << "End of inputs reached, read " << read_events << " out of " << total_events << " events" << std::endl;
      break;
    }

    const Event event = next_buffer.value().get().next();
    ++read_events;
    const uint64_t event_id{event.frag.global_id()};
    if (not written_event_ids.insert(event_id).second) {
      std::cout << "Duplicate event with global_id " << event_id << ", skipping" << std::endl;
      continue;
    }
    if (event_id > max_event_id) {
      max_event_id = event_id;
    } else if (s_debugLogging) {
      std::cout << "The current event global_id " << event_id << " is lower than previously written event "
                << max_event_id << " - event ordering not strictly preserved" << std::endl;
    }

    if (s_debugLogging) {
      std::cout << "Writing event " << written_event_ids.size() << " with global_id " << event_id
                << " read from file " << next_buffer_id << std::endl;
    } else if (read_events % s_printInterval == 0) {
      std::cout << "Read " << read_events << " out of " << total_events << " events and wrote "
                << written_event_ids.size() << " events" << std::endl;
    }

    const WriteEvent writeEvent{event};
    if (writeEvent.status() != WriteEvent::Status::OK) {
      std::cerr << "Error in event serialisation" << std::endl;
      return 1;
    }

    const uint32_t sizeInBytes = writeEvent.size() * sizeof(uint32_t);
    uint32_t writeSizeOnDisk{0};
    const EventStorage::DWError err_code = writer->putData(sizeInBytes, writeEvent.blob().get(), writeSizeOnDisk);
    if (err_code != EventStorage::DWError::DWOK or not writer->good()) {
      std::cerr << "Error writing to file, exiting..." << std::endl;
      return 1;
    }
    if (writeSizeOnDisk != sizeInBytes) {
      std::cerr << "Error writing to file, wrote " << writeSizeOnDisk << " instead of " << sizeInBytes << " words" << std::endl;
      return 1;
    }
  }

  std::cout << "Merging successful, read " << read_events << " out of " << total_events << " events and wrote "
            << written_event_ids.size() << " events" << std::endl;

  return 0;
}
