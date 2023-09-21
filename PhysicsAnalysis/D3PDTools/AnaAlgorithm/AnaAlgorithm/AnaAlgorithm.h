// Dear emacs, this is -*- c++ -*-
/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef ANA_ALGORITHM__ANA_ALGORITHM_H
#define ANA_ALGORITHM__ANA_ALGORITHM_H

#include <AnaAlgorithm/Global.h>

#ifdef XAOD_STANDALONE
#include <AsgTools/AsgComponent.h>
#include <AsgTools/SgTEvent.h>
#include <AsgTools/SgTEventMeta.h>
#include <memory>
#include <vector>
#else
#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <AsgTools/MessageCheck.h>
#include <StoreGate/StoreGateSvc.h>
#include <GaudiKernel/IIncidentListener.h>
#include <GaudiKernel/ServiceHandle.h>
#endif

class TH1;
class TH2;
class TH3;
class TEfficiency;
class TTree;
class ISvcLocator;

namespace EL
{
#ifdef XAOD_STANDALONE
  class IWorker;
#endif

  /// \brief the (new) base class for EventLoop algorithms
  ///
  /// This is meant as a replacement for the old EventLoop algorithms
  /// (\ref EL::Algorithm) to address a number of problems:
  /// * The old algorithm class relied on streamers for configuration.
  ///   This is actually pretty bad in practice, as it leads to all
  ///   kinds of errors.  The new class instead uses the
  ///   `setProperty()` mechanism that is used for other components.
  /// * The old algorithm class was tied very closely to the \ref
  ///   EL::Worker class, making it difficult to run standalone,
  ///   e.g. for testing.
  /// * The old algorithm class had different signatures and calling
  ///   conventions from the corresponding methods in Athena
  ///   algorithms, making it near-impossible to write dual-use
  ///   algorithms.
  /// * The old algorithm class was tied in closely with the rest of
  ///   the EventLoop package, meaning every library implementing an
  ///   algorithm had to depend on the EventLoop package.
  ///
  /// \todo This class does not allow to do everything that can
  /// currently be done with a \ref `EL::Algorithm` since it neither
  /// knows about incidents, nor has all the virtual methods that were
  /// available before.
  ///
  /// \todo It might be nice to make this actually dual-use at some
  /// point, but since there seems little interest in a dual-use
  /// algorithm that is on the back-burner.  There are some
  /// conditional compilation directives here and there, but it was
  /// never tested for dual-use (or even in-athena compilation).

  class AnaAlgorithm
#ifdef XAOD_STANDALONE
    : public asg::AsgComponent
#else
    : public AthHistogramAlgorithm, virtual public IIncidentListener
#endif
  {
    //
    // public interface
    //

    /// \brief constructor with parameters
    ///
    /// This matches the Athena algorithm constructor (for dual-use
    /// purposes).  Within EventLoop the `pSvcLocator` will always be
    /// `nullptr` (unless we ever have dual-use services).
    ///
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   out of memory II
  public:
    AnaAlgorithm (const std::string& name, 
                  ISvcLocator* pSvcLocator);

    /// \brief standard (virtual) destructor
    /// \par Guarantee
    ///   no-fail
  public:
    virtual ~AnaAlgorithm() noexcept;



    //
    // services interface
    //

#ifdef XAOD_STANDALONE
    /// Type of the metadata store pointer in standalone mode
    typedef asg::SgTEventMeta* MetaStorePtr_t;
    typedef const asg::SgTEventMeta* ConstMetaStorePtr_t;
#else
    /// Type of the metadata store pointer in standalone mode
    typedef ServiceHandle< StoreGateSvc >& MetaStorePtr_t;
    typedef const ServiceHandle< StoreGateSvc >& ConstMetaStorePtr_t;
#endif // XAOD_STANDALONE

    ///@{
    /// Accessor for the input metadata store
    ConstMetaStorePtr_t inputMetaStore() const;
    MetaStorePtr_t inputMetaStore();
    ///@}

    ///@{
    /// Accessor for the output metadata store
    ConstMetaStorePtr_t outputMetaStore() const;
    MetaStorePtr_t outputMetaStore();
    ///@}

#ifdef XAOD_STANDALONE
    /// \brief get the (main) event store for this algorithm
    ///
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   job not configured for xAODs
    /// \post result != nullptr
  public:
    asg::SgTEvent *evtStore() const;


    /// \brief book the given histogram
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   histogram booking error
  public:
    ::StatusCode book (const TH1& hist);


    /// \brief book the given histogram
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   histogram booking error
  public:
    ::StatusCode book (const TEfficiency& hist);


    /// \brief get the histogram with the given name
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   histogram not found
  public:
    TH1 *hist (const std::string& name) const;


    /// \brief get the 2-d histogram with the given name
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   histogram not found
  public:
    TH2 *hist2d (const std::string& name) const;


    /// \brief get the 3-d histogram with the given name
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   histogram not found
  public:
    TH3 *hist3d (const std::string& name) const;


    /// \brief get the 3-d histogram with the given name
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   histogram not found
  public:
    TEfficiency *histeff (const std::string& name) const;


    /// \brief the histogram worker interface
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   no histogram worker configured
    /// \post result != nullptr
  public:
    IHistogramWorker *histogramWorker () const;


    /// \brief book the given tree
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   tree booking error
  public:
    ::StatusCode book (const TTree& tree);


    /// \brief get the tree with the given name
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   histogram not found
  public:
    TTree *tree (const std::string& name) const;


    /// \brief the histogram worker interface
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   no histogram worker configured
    /// \post result != nullptr
  public:
    ITreeWorker *treeWorker () const;


    /// \brief the filter worker interface
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   no filter worker configured
    /// \post result != nullptr
  public:
    IFilterWorker *filterWorker () const;


    /// \brief whether the current algorithm passed its filter
    /// criterion for the current event
    /// \par Guarantee
    ///   no-fail
  public:
    bool filterPassed() const;

    /// \brief set the value of \ref filterPassed
    /// \par Guarantee
    ///   no-fail
  public:
    void setFilterPassed (bool val_filterPassed);


    /// \brief the worker that is controlling us (if working in
    /// EventLoop)
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   job not running in EventLoop
    /// \post result != nullptr
  public:
    IWorker *wk () const;
#endif


    /// \brief register this algorithm to have an implementation of
    /// \ref fileexecute
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   fileExecute not supported
  public:
    ::StatusCode requestFileExecute ();


    /// \brief register this algorithm to have an implementation of
    /// \ref beginInputFile
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   beginInputFile not supported
  public:
    ::StatusCode requestBeginInputFile ();


    /// \brief register this algorithm to have an implementation of
    /// \ref endInputFile
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   endInputFile not supported
  public:
    ::StatusCode requestEndInputFile ();



    //
    // virtual interface
    //

    /// \brief initialize this algorithm
    ///
    /// Note that unlike the original EventLoop algorithms, this gets
    /// called before any events are in memory (or at least it can
    /// be).  As such you should *not* try to access the current event
    /// in here.
  protected:
    virtual ::StatusCode initialize ();

    /// \brief execute this algorithm
    ///
    /// This gets called once on every event and is where the bulk of
    /// the processing ought to be happening.
  protected:
    virtual ::StatusCode execute ();

    /// \brief finalize this algorithm
    ///
    /// This gets called after event processing has finished.  The
    /// last event may no longer be in memory, and the code should not
    /// try to access it.
  protected:
    virtual ::StatusCode finalize ();

    /// \brief print the state of the algorithm
    ///
    /// This is mostly to allow algorithms to add a little debugging
    /// information if they feel like it.
  protected:
    virtual void print () const;

    /// \brief perform the action exactly once for each file in the
    /// dataset
    ///
    /// Ideally you don't use this, but instead rely on meta-data
    /// tools instead.  However, there are enough people asking for it
    /// that I decided to implement it anyways.
    ///
    /// \warn To use this you have to call \ref requestFileExecute
    /// to use this.
    ///
    /// \warn The user should not expect this to be called at any
    /// particular point in execution.  If a file is split between
    /// multiple jobs this will be called in only one of these jobs,
    /// and not the others.  It usually gets called before the first
    /// event in a file, but that is **not** guaranteed and relying on
    /// this is a bug.
    ///
    /// \warn The execution order of \ref beginInputFile and \ref
    /// fileExecute is currently unspecified.
    ///
    /// \warn fileExecute does not work with sub-file splitting in
    /// Athena, i.e. processing half the events of a file in one job
    /// the other half in another job.  this should not *normally*
    /// happen, unless you do crazy things like run AthenaMP or
    /// explicitly select sub-file splitting in panda.  in that case
    /// you are on your own.
  protected:
    virtual ::StatusCode fileExecute ();

    /// \brief perform the action for the beginning of an input file
    ///
    /// Ideally you don't use this, but instead rely on meta-data
    /// tools instead.  However, there are enough people asking for it
    /// that I decided to implement it anyways.
    ///
    /// \warn To use this you have to call \ref requestBeginInputFile
    /// to use this.
    ///
    /// \warn If a file is split across multiple jobs this will be
    /// called more than once.  This only happens for specific batch
    /// drivers and/or if it is explicitly configured by the user.
    /// With PROOF it could even happen multiple times within the same
    /// job, and while PROOF is no longer supported that behavior may
    /// come back if support for a similar framework is added in the
    /// future.  As such, this method should not be used for
    /// accounting that relies to be called exactly once per file,
    /// take a look at \ref fileExecute if you want something that is
    /// guaranteed to be executed exactly once per input file.
    ///
    /// \warn The execution order of \ref beginInputFile and \ref
    /// fileExecute is currently unspecified.
  protected:
    virtual ::StatusCode beginInputFile ();

    /// \brief perform the action for the end of an input file
    ///
    /// Ideally you don't use this, but instead rely on meta-data
    /// tools instead.  However, there are enough people asking for it
    /// that I decided to implement it anyways.
    ///
    /// \warn To use this you have to call \ref requestEndInputFile
    /// to use this.
    ///
    /// \warn If a file is split across multiple jobs this will be
    /// called more than once.  This only happens for specific batch
    /// drivers and/or if it is explicitly configured by the user.
    /// With PROOF it could even happen multiple times within the same
    /// job, and while PROOF is no longer supported that behavior may
    /// come back if support for a similar framework is added in the
    /// future.  As such, this method should not be used for
    /// accounting that relies to be called exactly once per file,
    /// take a look at \ref fileExecute if you want something that is
    /// guaranteed to be executed exactly once per input file.
    ///
    /// \warn The execution order of \ref endInputFile and \ref
    /// fileExecute is currently unspecified.
  protected:
    virtual ::StatusCode endInputFile ();



    //
    // framework interface
    //

#ifdef XAOD_STANDALONE
    /// \brief call \ref initialize
  public:
    ::StatusCode sysInitialize ();

    /// \brief call \ref execute
  public:
    ::StatusCode sysExecute ();

    /// \brief call \ref finalize
  public:
    ::StatusCode sysFinalize ();

    /// \brief call \ref print
  public:
    void sysPrint ();

    /// \brief call \ref fileExecute
  public:
    ::StatusCode sysFileExecute ();

    /// \brief call \ref beginInputFile
  public:
    ::StatusCode sysBeginInputFile ();

    /// \brief call \ref endInputFile
  public:
    ::StatusCode sysEndInputFile ();


    /// \brief set the value of \ref evtStore
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   service already configured
  public:
    void setEvtStore (asg::SgTEvent *val_evtStore);

    /// \brief set the value of \ref histogramWorker
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   service already configured
  public:
    void setHistogramWorker (IHistogramWorker *val_histogramWorker);

    /// \brief set the value of \ref treeWorker
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   service already configured
  public:
    void setTreeWorker (ITreeWorker *val_treeWorker);

    /// \brief set the value of \ref filterWorker
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   service already configured
  public:
    void setFilterWorker (IFilterWorker *val_filterWorker);

    /// \brief set the value of \ref wk
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   service already configured
  public:
    void setWk (IWorker *val_wk);


    /// \brief whether we have an implementation for \ref
    /// fileExecute
    /// \par Guarantee
    ///   no-fail
  public:
    bool hasFileExecute () const noexcept;


    /// \brief whether we have an implementation for \ref
    /// beginInputFile
    /// \par Guarantee
    ///   no-fail
  public:
    bool hasBeginInputFile () const noexcept;


    /// \brief whether we have an implementation for \ref
    /// endInputFile
    /// \par Guarantee
    ///   no-fail
  public:
    bool hasEndInputFile () const noexcept;
#endif


#ifndef XAOD_STANDALONE
    /// \brief receive the given incident
    /// \par Guarantee
    ///   basic
    /// \par Failures
    ///   incident handling errors
  public:
    void handle (const Incident& inc);
#endif



    //
    // private interface
    //

#ifdef XAOD_STANDALONE
    /// \brief the value of \ref evtStore
  private:
    asg::SgTEvent *m_evtStore = nullptr;
#endif

#ifdef XAOD_STANDALONE
    /// Type of the metadata store variable in standalone mode
    typedef asg::SgTEventMeta MetaStore_t;
#else
    /// Type of the metadata store variable in Athena
    typedef ServiceHandle< StoreGateSvc > MetaStore_t;
#endif // XAOD_STANDALONE

    /// \brief Object accessing the input metadata store
  private:
    MetaStore_t m_inputMetaStore;

    /// \brief Object accessing the output metadata store
  private:
    MetaStore_t m_outputMetaStore;

#ifdef XAOD_STANDALONE
    /// \brief the value of \ref histogramWorker
  private:
    IHistogramWorker *m_histogramWorker = nullptr;
#endif

#ifdef XAOD_STANDALONE
    /// \brief the value of \ref treeWorker
  private:
    ITreeWorker *m_treeWorker = nullptr;
    /// \brief the output stream name for tree writing
  private:
    std::string m_treeStreamName;
#endif

#ifdef XAOD_STANDALONE
    /// \brief the value of \ref filterWorker
  private:
    IFilterWorker *m_filterWorker = nullptr;
#endif

#ifdef XAOD_STANDALONE
    /// \brief the value of \ref wk
  private:
    IWorker *m_wk = nullptr;
#endif

    /// \brief the value of \ref hasFileExecute
  private:
    bool m_hasFileExecute {false};

    /// \brief the value of \ref hasBeginInputFile
  private:
    bool m_hasBeginInputFile {false};

    /// \brief the value of \ref hasEndInputFile
  private:
    bool m_hasEndInputFile {false};
  };
}

#include "AnaAlgorithm.icc"

#endif
