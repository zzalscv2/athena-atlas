/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//    Root Database implementation
//--------------------------------------------------------------------
//
//    Package    : APR RootStorageSvc (former POOL project)
//
//    Author     : M.Frank M.Nowak S.Snyder
//====================================================================
#ifndef POOL_ROOTTREECONTAINER_H
#define POOL_ROOTTREECONTAINER_H 1

// Framework include files
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbContainerImp.h"
#include "RootAuxDynIO/RootAuxDynIO.h"  //needed in all files that include this header because of the unique_ptr 

#include <map>
#include <vector>
#include <set>
#include <functional>

// Forward declarations
class TObject;
class TBranch;
class TBuffer;
class TTree;
class TLeaf;
class TClass;

namespace SG { class IAuxStoreIO; }
namespace RootAuxDynIO {
   class IRootAuxDynWriter;
   class IRootAuxDynReader;
}
     

/*
 * POOL namespace declaration
 */
namespace pool  { 

  // Forward declaration
   class DbColumn;
   class RootDatabase;

  /** @class RootTreeContainer RootTreeContainer.h src/RootTreeContainer.h
    *
    * Description:
    * ROOT specific implementation of Database container.
    * Since objects in root are stored in "trees with branches",
    * this object corresponds to a tuple (Tree/Branch), where
    * each object type (determined by the location of the transient
    * object within the data store) is accessed by the "Event" number
    * inside its tree.
    *
    * @author  M.Frank M.Nowak
    */
  class RootTreeContainer : public DbContainerImp
  {
    /// Definiton of a branch descriptor
    struct BranchDesc {
    public:
      TClass*           clazz;
      TBranch*          branch;
      TLeaf*            leaf;
      void*             object;
      void*             buffer;
      const DbColumn*   column;

      // ----  extra variables used for AuxDyn attributes
      // number of rows written to this branch so far
      size_t            rows_written = 0;

      /// IOStore interface offset for object type in this branch (for casting)
      int               aux_iostore_IFoffset = -1;
      // AuxDyn writer and the reader for this branch (object type)
      std::unique_ptr<RootAuxDynIO::IRootAuxDynWriter>  auxdyn_writer;
      std::unique_ptr<RootAuxDynIO::IRootAuxDynReader>  auxdyn_reader;


      BranchDesc()
            : clazz(nullptr),
              branch(nullptr),
              leaf(nullptr),
              object(nullptr),
              buffer(nullptr),
              column(nullptr)
      {}

      BranchDesc( TClass* cl,
                  TBranch* b,
                  TLeaf* l,
                  void* o,
                  const DbColumn* c)
            : clazz(cl),
              branch(b),
              leaf(l),
              object(nullptr),
              buffer(o),
              column(c)
      {}

      ~BranchDesc();
       BranchDesc(BranchDesc const& other) = delete;
       BranchDesc(BranchDesc && other) = default;
       BranchDesc& operator=(BranchDesc const& other) = delete;
       BranchDesc& operator=(BranchDesc && other) = default;

      SG::IAuxStoreIO* getIOStorePtr() {
         return ( aux_iostore_IFoffset >= 0 ?
                  reinterpret_cast<SG::IAuxStoreIO*>( (char*)object + aux_iostore_IFoffset) : nullptr );
      }
    };

    /// Definition of the branch container
    typedef std::vector<BranchDesc> Branches;

  protected:
    /// Reference to the root tree object
    TTree*             m_tree;
    /// reference to exact type description
    const DbTypeInfo*  m_type;
    /// Branch map
    Branches           m_branches;
    /// Parent Database handle
    DbDatabase         m_dbH;
    /// Root database file reference
    RootDatabase*      m_rootDb;
    /// Branch name, if branch is specified by user.
    std::string        m_branchName;
    /// Number of bytes written/read during last operation. Set to -1 if it failed.
    int                m_ioBytes;
    /// if True, branch container will not use Branch->Fill() but will be filled with its Tree by the database instance
    bool               m_treeFillMode;
    /// flag set when a branch container was updated (but the branch was not Filled)
    bool               m_isDirty;

  private:

    /// Add item branch
    DbStatus addBranch( const DbColumn* col,
                        BranchDesc& dsc,
                        const std::string& desc);

    /// Add BLOB
    DbStatus addObject( DbDatabase& dbH,
                        const DbColumn* col,
                        BranchDesc& dsc,
                        const std::string& desc,
                        int splitLevel,
                        int bufferSize,
                        int branchOffsetTabLen);

    /// Find entry identified by his number (=primary key) in the Database
    DbStatus selectRow( const DataCallBack* call,
                        const Token::OID_t& linkH,
                        DbAccessMode        mode);

    // Routine needed for TRANSACT_FLUSH, if branch is specified by user.
    DbStatus finishTransAct();

    /// Access branch by name
    TBranch* branch(const std::string& nam)  const;

    /// Recursively set Offset Table length for a branch and it's subbranches
    void setBranchOffsetTabLen(TBranch* b, int offsettab_len);	
    
  public:
    /// Standard constructor
    RootTreeContainer();

    /// Standard destructor
    virtual ~RootTreeContainer();

    /// Close the container and deallocate resources
    virtual DbStatus close() override;

    /// Open the container for object access
    virtual DbStatus open(DbDatabase& dbH, 
                          const std::string& nam, 
                          const DbTypeInfo* info,
                          DbAccessMode mod) override;

    /// Access options
    /** @param opt      [IN]  Reference to option object.
      *
      * @return DbStatus code indicating success or failure.  
      */
    virtual DbStatus getOption(DbOption& opt) override;

    /// Set options
    /** @param opt      [IN]  Reference to option object.
      *
      * @return DbStatus code indicating success or failure.  
      */
    virtual DbStatus setOption(const DbOption& opt) override;

    /// Ask if a given shape is supported
    virtual DbStatus isShapeSupported(const DbTypeInfo* typ) const override;

    /// Define selection criteria
    virtual DbStatus select(DbSelect& criteria) override;

    /// Number of entries within the container
    virtual uint64_t size() override;

    /// return the TTree in which this container is stored
    TTree*      getTTree() { return m_tree; }

    /// return the name of the container
    const std::string& getName() const { return m_name; }

    /// is this a container in a TBranch? (regular ones take the whole TTree)
    bool        isBranchContainer() const { return !m_branchName.empty(); }

    /// set Tree Filling mode (true/false) for branch containers
    void        setTreeFillMode(bool mode);

    /// return true if this branch container is using TTree Filling mode
    bool        usingTreeFillMode() const { return  m_treeFillMode; }

    /// return true if this branch container was updated and it's TTree needs to be Filled
    bool        isDirty() const { return m_isDirty; }

    /// clear the dirty status (used after containing TTree was Filled)
    void        clearDirty() { m_isDirty = false; }

    /// Fetch next object address of the selection to set token
    virtual DbStatus fetch(DbSelect& sel) override;

    /// Fetch refined object address. Default implementation returns identity
    virtual DbStatus fetch(const Token::OID_t& linkH, Token::OID_t& stmt) override;

    /// Find object by object identifier and load it into memory
    /** @param  ptr    [IN/OUT]  ROOT-style address of the pointer to object
      * @param  shape     [IN]   Object type
      * @param  oid      [OUT]   Object OID
      *
      * @return Status code indicating success or failure.
      */
    virtual DbStatus loadObject( void** ptr, ShapeH shape, 
                                 Token::OID_t& oid) override;

    /// Commit single entry to container
    virtual DbStatus writeObject(ActionList::value_type&) override;

    /// Execute Transaction action
    virtual DbStatus transAct(Transaction::Action action) override;
  };
}
#endif //POOL_ROOTTREECONTAINER_H
