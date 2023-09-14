/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef ASG_TOOLS__ANA_TOOL_HANDLE_H
#define ASG_TOOLS__ANA_TOOL_HANDLE_H

// Author: Will Buttinger


#include <AsgTools/AsgTool.h>
#include <AsgTools/AsgToolConfig.h>
#include <AsgTools/ToolHandle.h>
#include <type_traits>
#include <atomic>
#include <list>
#include <map>
#include <mutex>

namespace asg
{
#ifdef XAOD_STANDALONE
   typedef INamedInterface parentType_t;
   typedef IAsgTool interfaceType_t;
#else
   typedef INamedInterface parentType_t;
   typedef IAlgTool interfaceType_t;
#endif

  template<class T> class AnaToolHandle;

  namespace detail
  {
    class AnaToolShare;



    /// \brief the mode with which an \ref AnaToolHandle object is
    /// initialized

    enum class AnaToolHandleMode
    {
      /// do not create a tool
      ///
      /// this can be either explicitly an empty tool handle set by
      /// the user, or an AnaToolHandle that was never configured in
      /// the first place.
      EMPTY,

      /// create a private tool normally
      CREATE_PRIVATE,

      /// create a shared tool normally
      CREATE_SHARED,

      /// retrieve a shared tool
      RETRIEVE_SHARED,

      /// retrieve a tool from the user tool handle
      ///
      /// this can still refer to a tool handle that is empty, though
      /// usually it will point to an actual tool
      USER
    };
  }



  /// \brief standard output operator
  /// \par Guarantee
  ///   basic
  /// \par Failures
  ///   streaming errors
  template <typename T>
  std::ostream& operator << (std::ostream& str, const AnaToolHandle<T>& obj);


  /// \brief a modified tool handle that allows its owner to configure
  /// new tools from the C++ side
  ///
  /// One of the main differences between RootCore and Athena is that
  /// RootCore lacks a configuration manager.  This means that in
  /// RootCore we have to create and configure all tools on the spot,
  /// which is not (easily) possible in Athena.  This particular tool
  /// handle provides a dual-use way of doing so, while introducing a
  /// number of important safety pre-cautions that are lacking on the
  /// RootCore side.
  ///
  /// The primary (or at least original) purpose of this tool handle
  /// was to support dual-use analysis frameworks.  For these the
  /// actual configuration of the CP tools is a critical component,
  /// and having to configure them separately in RootCore and Athena
  /// is a potential source of rather difficult to track down bugs.
  /// An unfortunate side-effect of this is that we can't use the
  /// Athena configuration mechanism.  Once we have a dual-use
  /// configuration manager it will hopefully provide a more
  /// attractive option.
  ///
  /// A secondary benefit is that this allows to create and configure
  /// tools on the Athena side without having to write configuration
  /// files.  This is particularly beneficial when writing unit tests
  /// where you have to create a lot of rather short-lived tools and
  /// where separate configuration files would be impractical.  It may
  /// also be beneficial for new users who can (at first) be turned
  /// off by the complexity of Athena configuration.
  ///
  ///
  /// The basic interface is a mix between the ToolHandle interface
  /// for accessing the tool, and the AsgTool interface for
  /// configuring/initializing the tools.  In addition it allows to
  /// set this ToolHandle as a property on another configurable, to
  /// allow configuring it via the standard configuration mechanisms.
  ///
  /// If a parent tool is specified, the tool is private (and this
  /// tool handle is modeled after unique_ptr).  If no parent tool is
  /// specified, the tool is public/shared (and this tool handle is
  /// modeled after shared_ptr).  In public/shared mode, if you create
  /// and initialize one tool it is then available for use by other
  /// tool handles registered with the same name and type.  The
  /// subsequent tool handles with that name will then automatically
  /// pick up the tool in the configuration set by the first tool
  /// handle.  The user still has to call initialize() on the tool
  /// handle before using the tool.  While it is allowed to call the
  /// setProperty method on those tool handles these calls will be
  /// effectively ignored.  The net effect of that is that as a user
  /// of a handle for a shared tool you don't have to worry about
  /// whether you are the one creating the tool or just using it, you
  /// just initialize the tool handle in the same way.
  ///
  /// The tool handle can also be declared as a property on the parent
  /// tool by using the ToolHandle provided via its handle member
  /// function.  This allows the user to override the configuration
  /// done in the parent tool via the job options file in Athena or
  /// via setProperty in RootCore.  Like for shared tools, initialize
  /// still has to be called and the calls to setProperty get ignored.
  /// This is so that inside the parent tool you don't have to write
  /// any special detection code to see whether the user configured
  /// the AnaToolHandle or if you need to do so yourself.  Instead,
  /// you can just write your code as if the user didn't configure
  /// your tool handle and if the user does, the tool handle will
  /// automatically switch over.
  ///
  ///
  /// There are a number of ways of how a tool can be created/accessed
  /// through an AnaToolHandle (in *increasing* order of precedence):
  /// - normally an AnaToolHandle will create its tool based on the
  ///   configured type and properties under the configured name.
  /// - if an AnaToolHandle has been created without a parent (i.e. as
  ///   a public tool) and another AnaToolHandle already has created a
  ///   public tool with the same name it will use and share that
  ///   already configured tool.
  /// - each AnaToolHandle can be declared as a property on the parent
  ///   tool, allowing to configure the AnaToolHandle that way.  if
  ///   the user configures the AnaToolHandle that way, it will take
  ///   precedence over any properties set on the AnaToolHandle.
  /// - for the future it is foreseen to provide a second property on
  ///   the parent tool to allow the user picking a particular
  ///   creation or access pattern, overriding the auto-detected
  ///   default

  template<class T>
  class AnaToolHandle final
  {
    //
    // public interface
    //

    /// \brief test the invariant of this object
    /// \par Guarantee
    ///   no-fail
  public:
    void testInvariant () const;


    /// \brief create a tool handle with the given name and parent
    ///
    /// The name can either be just a regular name or of the form
    /// type/name, in which case type indicates the type of the tool
    /// to be created.  If you don't explicitly call make (or pass no
    /// type into it), you have to rely on this form.
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   out of memory II
  public:
    explicit AnaToolHandle (const std::string& val_name = "",
			    parentType_t *val_parent = nullptr);


    /// \brief move constructor
    /// \par Guarantee
    ///   no-fail
  public:
    AnaToolHandle (AnaToolHandle<T>&& that);


    /// \brief copy constructor
    ///
    /// This will copy the configuration if the argument has not been
    /// initialized yet, and it will create a shared tool handle if it
    /// has.
    ///
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   out of memory II
  public:
    AnaToolHandle (const AnaToolHandle<T>& that);


    /// \brief standard destructor
    /// \par Guarantee
    ///   no-fail
  public:
    ~AnaToolHandle () noexcept;


    /// \brief assignment operator
    ///
    /// This will copy the configuration if the argument has not been
    /// initialized yet, and it will create a shared tool handle if it
    /// has.
    ///
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   out of memory II
  public:
    AnaToolHandle& operator = (const AnaToolHandle<T>& that);


    /// \brief standard move assignment operator
    /// \par Guarantee
    ///   no-fail
  public:
    AnaToolHandle<T>& operator = (AnaToolHandle<T>&& that);


    /// \brief standard swap
    /// \par Guarantee
    ///   no-fail
  public:
    void swap (AnaToolHandle<T>& that) noexcept;


    /// \brief whether this ToolHandle is completely empty, i.e. it
    /// has been default constructed and was never assigned any
    /// content by the user
    ///
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   out of memory II
  public:
    bool empty () const;

    /// \brief whether this is a public tool (or tool handle)
    /// \par Guarantee
    ///   no-fail
  public:
    bool isPublic () const noexcept;

    /// \brief whether initialize has been called successfully,
    ///   i.e. whether the tool is ready to be used
    /// \par Guarantee
    ///   no-fail
  public:
    bool isInitialized () const noexcept;


    /// \brief declare as property on the given tool
    /// \par Guarantee
    ///   basic
    /// \par Failures
    ///   out of memory II\n
    ///   property declaration failures\n
    ///   tool already made
  public:
    template<typename T2> void
    declarePropertyFor (T2 *tool, const std::string& name,
			const std::string& description = "");

    /// \brief the tool handle we wrap
    /// \par Guarantee
    ///   no-fail
  public:
    const ToolHandle<T>& getHandle () const noexcept;


    /// \brief set the given property of the tool.
    ///
    /// this will make the tool is not already created.
    ///
    /// if the tool is shared and has already been created, this call
    /// will be ignored.
    ///
    /// this can also be used to set tool handle properties by passing
    /// in either a regular \ref ToolHandle or an \ref AnaToolHandle
    /// object.
    ///
    /// \par Guarantee
    ///   basic
    /// \par Failures
    ///   property setting failures
    /// \pre !isInitialized()
    /// \{
  public:
    // ToolHandle and ToolHandleArray have ctors that can accept a single int,
    // so use enable_if to avoid ambiguities.
    template <class T2> StatusCode
    setProperty (const std::string& property, const T2& value);
    template <class T2>
    typename std::enable_if<std::is_base_of_v<parentType_t, T2>, StatusCode>::type
    setProperty (const std::string& property, const ToolHandle<T2>& value);
    template <class T2> StatusCode
    setProperty (const std::string& property, const AnaToolHandle<T2>& value);
    template <class T2>
    typename std::enable_if<std::is_base_of_v<parentType_t, T2>, StatusCode>::type
    setProperty (const std::string& property, const ToolHandleArray<T2>& value);
    /// \}


    /// \brief the type configured for this AnaToolHandle
    /// \par Guarantee
    ///   no-fail
  public:
    const std::string& type () const noexcept;

    /// \brief set the value of \ref type
    /// \par Guarantee
    ///   no-fail
    /// \pre !isInitialized()
  public:
    void setType (std::string val_type) noexcept;


    /// \brief the name configured for this AnaToolHandle
    /// \par Guarantee
    ///   no-fail
  public:
    const std::string& name () const noexcept;

    /// \brief set the value of \ref name
    /// \par Guarantee
    ///   no-fail
    /// \pre !isInitialized()
  public:
    void setName (std::string val_name) noexcept;

    /// \brief the full name of the tool to be used during tool
    /// initialization
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   out of memory II
  public:
    std::string fullName () const;


    /// \brief the value of \ref type and \ref name
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   out of memory II
  public:
    std::string typeAndName () const;

    /// \brief set the value of \ref type and \ref name
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   out of memory II
    /// \pre !isInitialized()
  public:
    void setTypeAndName (const std::string& val_typeAndName);

    /// \brief set the value of \ref type and \ref name
    /// \par Guarantee
    ///   no-fail
    /// \pre !isInitialized()
  public:
    void setTypeAndName (std::string val_type, std::string val_name) noexcept;


    /// \brief initialize the tool
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   tool initialization failures
    /// \pre !isInitialized()
  public:
    StatusCode initialize ();

    /// \copydoc initialize
  public:
    StatusCode retrieve ();


    /// \brief access the tool
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   tool creation failures
    /// \post result != nullptr
  public:
    T *operator -> ();

    /// \brief access the tool
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   tool creation failures
    /// \post result != nullptr
  public:
    const T *operator -> () const;

    /// \brief access the tool
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   tool creation failures
  public:
    T& operator * ();

    /// \brief access the tool
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   tool creation failures
  public:
    const T& operator * () const;

    /// \brief access the tool
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   tool creation failures
    /// \post result != nullptr
  public:
    T *get ();

    /// \brief access the tool
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   tool creation failures
    /// \post result != nullptr
  public:
    const T *get () const;


    /// \brief whether this tool handle has been configured by the
    /// user
    ///
    /// i.e. somebody called declareProperty with our handle member,
    /// and then that got configured either from the job options file
    /// in Athena or via setProperty in RootCore
    /// \par Guarantee
    ///   no-fail
  public:
    bool isUserConfigured () const noexcept;


    /// \brief the \ref detail::AnaToolHandleMode for this handle
    ///
    /// This is mostly meant for internal use by the setProperty
    /// method, not for use by the end-user
    ///
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   out of memory II
  public:
    detail::AnaToolHandleMode mode () const;


    /// \brief the \ref detail::AnaToolConfig for this handle
    ///
    /// This is mostly meant for internal use by the setProperty
    /// method, not for use by the end-user
    ///
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   out of memory II
  public:
    const AsgToolConfig& config () const;


    /// \brief whether the user is allowed to configure this with an
    /// empty tool handle
    ///
    /// This normally defaults to whether the developer initialized
    /// this to a valid ToolHandle to begin with, but depending on the
    /// situation it can be overridden.
    ///
    /// \par Guarantee
    ///   no-fail
  public:
    bool allowEmpty () const noexcept;



    /// \brief set the value of \ref allowEmpty
    /// \par Guarantee
    ///   no-fail
    /// \pre !isInitialized()
  public:
    void setAllowEmpty (bool val_allowEmpty = true) noexcept;



    //
    // deprecated interface
    //

    /// \brief whether the tool can be configured by us, i.e. whether
    /// properties set via \ref setProperty will be used for something
    ///
    /// For the most part this is the reverse of \ref isUserConfigured
    /// except some shared tools may register as not user configured
    /// and not configurable either.  if the properties set are used
    /// to generate debugging output even a user configured tool may
    /// register as configurable.
    ///
    /// \par Guarantee
    ///   no-fail
    /// \warn this may go away at some point as we may be using the
    /// properties for more and more things (e.g. info printouts)
  public:
    bool isConfigurable () const;

  public:
    [[deprecated("please use isInitialized() instead")]]
    bool inPremakeState () const noexcept {
      return !isInitialized();}

  public:
    [[deprecated("please use isInitialized() instead")]]
    bool inBrokenState () const noexcept {
      return false;};

  public:
    [[deprecated("no longer need to call make()")]]
    StatusCode make () {
      return StatusCode::SUCCESS;};

  public:
    [[deprecated("please use setType() or setTypeAndName() instead")]]
    StatusCode make (const std::string& val_type) noexcept {
      if (!val_type.empty()) {
	if (val_type.find ('/') != std::string::npos)
	  setTypeAndName (val_type);
	else
	  setType (val_type); }
      return StatusCode::SUCCESS; };



    //
    // private interface
    //

    /// \brief any extra initialization we need to do before creating
    /// the tool
    ///
    /// This is mostly/only for creating public tools the tool needs
    /// to use.  This is not ideal for various reasons, but it is what
    /// it is, and this is legacy code.
    ///
    /// \warn This member has to come before \ref m_cleanup
  private:
    std::vector<std::function<StatusCode ()>> m_extraInit;

    /// \brief any stuff we need to release as part of cleanup
    ///
    /// \warn This member has to come before \ref m_handleUser
  private:
    std::shared_ptr<void> m_cleanup;

    /// \brief the configuration for this tool
  private:
    AsgToolConfig m_config;

    /// \brief the value of \ref name
  private:
    std::string m_name;

    /// \brief the pointer to the parent
  private:
    parentType_t *m_parentPtr = nullptr;


    /// \brief the \ref ToolHandle exposed to the user
    ///
    /// This primarily serves to allow the user to set this like a
    /// regular ToolHandle property.  During initialize it is then
    /// updated to refer to the actually used tool.
    ///
    /// This is done as a shared pointer, so that the include
    /// dependency is only pulled in for the source file that actually
    /// uses the AnaToolHandle.  That in turn reduces the number of
    /// public dependencies an Athena package has to expose.
  private:
    std::shared_ptr<ToolHandle<T>> m_handleUser;

    /// \brief the typeAndName at time of creation
    ///
    /// This is used to determine whether the user has been overriding
    /// the ToolHandle during configuration time.
  private:
    std::string m_originalTypeAndName;

    /// \brief the value of \ref isInitialized
  private:
    std::atomic<bool> m_isInitialized {false};

    /// \brief the pointer to the tool we use
    ///
    /// This is used for actually accessing the tool, independent of
    /// who created it or how.  Mostly this is a performance
    /// optimization to avoid going back to the ToolHandle every
    /// single time.
    ///
    /// This is protected by \ref m_isInitialized and should not be
    /// accessed until \ref m_isInitialized is true.
  private:
    T *m_toolPtr = nullptr;

    /// \brief the value of \ref getMode cached when we initialize the
    /// tool
    ///
    /// This is protected by \ref m_isInitialized and should not be
    /// accessed until \ref m_isInitialized is true.
  private:
    detail::AnaToolHandleMode m_mode = detail::AnaToolHandleMode::EMPTY;

    /// \brief get the mode with which this ToolHandle will be initialized
    ///
    /// This exists in two modes, one passing back a shared tool
    /// pointer and one without it.  as a rule of thumb, the version
    /// passing back the shared tool should be used by initialize,
    /// whereas all the places just wanting to check that will happen
    /// can/should call the version without.
    ///
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   out of memory II
  private:
    detail::AnaToolHandleMode
    getMode (std::shared_ptr<detail::AnaToolShare>& sharedTool) const;

    /// \copydoc getMode
  private:
    detail::AnaToolHandleMode
    getMode () const;


    /// \brief the value of \ref allowEmpty
  private:
    bool m_allowEmpty = false;


    /// \brief make a tool by retrieving the ToolHandle
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   tool creation failures
  private:
    StatusCode makeToolRetrieve
      (T*& toolPtr, ToolHandle<T>& toolHandle) const;


    /// \brief a mutex to ensure that we don't call initialize twice
    /// concurrently
    ///
    /// This is a recursive mutex so that we can lock both in \ref
    /// initialize and \ref get without conflict.
  private:
    std::recursive_mutex m_initializeMutex;
  };
}

/// \brief create the tool in the given tool handle
#define ASG_MAKE_ANA_TOOL(handle,type)	\
  (ASG_SET_ANA_TOOL_TYPE(handle,type), StatusCode (StatusCode::SUCCESS))

/// \brief set the tool type on the tool handle, using new in rootcore
#define ASG_SET_ANA_TOOL_TYPE(handle,type)	\
  (handle).setType (#type)

#include <AsgTools/AnaToolHandle.icc>


#endif
