/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef SYSTEMATICS_HANDLES__SYS_READ_HANDLE_H
#define SYSTEMATICS_HANDLES__SYS_READ_HANDLE_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgDataHandles/VarHandleKey.h>
#include <AsgMessaging/AsgMessagingForward.h>
#include <PATInterfaces/SystematicSet.h>
#include <SystematicsHandles/ISysHandleBase.h>
#include <string>
#include <type_traits>
#include <unordered_map>

class StatusCode;

namespace CP
{
  class SysListHandle;
  class SystematicSet;


  /// \brief a data handle for reading systematics varied input data

  template<typename T> class SysReadHandle final
    : public ISysHandleBase, public asg::AsgMessagingForward
  {
    //
    // public interface
    //

    /**
     * @brief Standard constructor
     * @tparam T2 The type of the owner
     * @param owner Used to declare the property and for its messaging
     * @param propertyName The name of the property to declare
     * @param propertyValue The default value for the property
     * @param propertyDescription The description of the property
     *
     * This version of the constructor declares a property on the parent object
     * and should usually be preferred when the container to be read should be
     * configurable
     */
  public:
    template<typename T2>
    SysReadHandle (T2 *owner, const std::string& propertyName,
                   const std::string& propertyValue,
                   const std::string& propertyDescription);

    /// \brief Direct constructor which doesn't declare a property
    template<typename T2>
    SysReadHandle (const std::string &inputName, T2 *owner);

    /// \brief whether we have a name configured
  public:
    virtual bool empty () const noexcept override;

    /// \brief !empty()
  public:
    explicit operator bool () const noexcept;

    /// \brief get the name pattern before substitution
  public:
    virtual std::string getNamePattern () const override;


    /// \brief initialize this handle
    /// \{
  public:
    StatusCode initialize (SysListHandle& sysListHandle);
    StatusCode initialize (SysListHandle& sysListHandle, SG::AllowEmptyEnum);
    /// \}


    /// \brief get the name we retrieve from the event store
  public:
    const std::string& getName (const CP::SystematicSet& sys) const;


    /// \brief retrieve the object for the given name
  public:
    ::StatusCode retrieve (const T*& object,
                           const CP::SystematicSet& sys) const;

    /// \brief check if the object is available
    /// Note that this should only be used in special use cases
    /// and may be removed in the future.
  public:
    bool isValid (const CP::SystematicSet& sys) const;

    //
    // inherited interface
    //

  private:
    virtual CP::SystematicSet
    getInputAffecting (const ISystematicsSvc& svc) const override;
    virtual StatusCode
    fillSystematics (const ISystematicsSvc& svc,
                     const CP::SystematicSet& fullAffecting,
                     const std::vector<CP::SystematicSet>& sysList) override;



    //
    // private interface
    //

    /// \brief the input name we use
  private:
    std::string m_inputName;

    /// \brief the cache of names we use
  private:
    std::unordered_map<CP::SystematicSet,std::string> m_inputNameCache;


    /// \brief the type of the event store we use
  private:
    typedef std::decay<decltype(
      *(std::declval<EL::AnaAlgorithm>().evtStore()))>::type StoreType;

    /// \brief the event store we use
  private:
    StoreType *m_evtStore = nullptr;

    /// \brief the function to retrieve the event store
    ///
    /// This is an std::function to allow the parent to be either a
    /// tool or an algorithm.  Though we are not really supporting
    /// tools as parents when using \ref SysListHandle, so in
    /// principle this could be replaced with a pointer to the
    /// algorithm instead.
  private:
    std::function<StoreType*()> m_evtStoreGetter;
  };
}

#include "SysReadHandle.icc"

#endif
