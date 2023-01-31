/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ROOTAUXDYN_IO_H
#define ROOTAUXDYN_IO_H

#include <string>
#include <mutex>

class TBranch;
class TTree;
class TFile;

namespace SG { class IAuxStoreIO;  class auxid_set_t; }


namespace RootAuxDynIO
{
   class IRootAuxDynReader;
   class IRootAuxDynWriter;

   /// Common post-fix for the names of auxiliary containers in StoreGate
   constexpr char AUX_POSTFIX[] = "Aux.";
   constexpr char AUXDYN_POSTFIX[] = "Dyn.";

   inline bool endsWithAuxPostfix(std::string_view str) {
      constexpr size_t postfix_size = sizeof(AUX_POSTFIX);
      return str.size() >= postfix_size and str.compare(str.size()-postfix_size, postfix_size, AUX_POSTFIX);
   }

  /**
   * @brief Check is a branch holds AuxStore objects
   * @param branch TBranch to check
   */
   bool isAuxDynBranch(TBranch *branch);

   /**
   * @brief Construct branch name for a given dynamic attribute
   * @param attr_name the name of the attribute
   * @param baseBranchName branch name for the main AuxStore object
   */
   std::string auxBranchName(const std::string& attr_name, const std::string& baseBranchName);

   /**
   * @brief Construct field name for a given dynamic attribute
   * @param attr_name the name of the attribute
   * @param baseBranchName branch name for the main AuxStore object
   */
   std::string auxFieldName(const std::string& attr_name, const std::string& baseBranchName);

  /**
   * @brief Exctract the Aux object SG Key from the branch name
   * @param branch TBranch with Key in its name
   */
   std::string getKeyFromBranch(TBranch* branch);

   std::unique_ptr<IRootAuxDynReader> getBranchAuxDynReader(TTree*, TBranch*);
   std::unique_ptr<IRootAuxDynWriter> getBranchAuxDynWriter(TTree*, int offsettab_len, bool do_branch_fill);
   

   class IRootAuxDynReader
   {
   public :
      /**
       * @brief Attach specialized AuxStore for reading dynamic attributes
       * @param object object instance to which the store will be attached to - has to be an instance of the type the reader was created for
       * @param ttree_row

       Use this method to instrument an AuxStore object AFTER it was read (every time it is read)
       This will attach its dynamic attributes with read-on-demand capability
      */
      virtual void addReaderToObject(void* object, size_t row, std::recursive_mutex* iomtx = nullptr) = 0;

      virtual const SG::auxid_set_t& auxIDs() const = 0;

      virtual size_t getBytesRead() const = 0;

      virtual void resetBytesRead() = 0; 


      virtual ~IRootAuxDynReader() {}
   };


   /// Interface for an AuxDyn Writer - TTree based 
   class IRootAuxDynWriter {
   public:
      virtual ~IRootAuxDynWriter() {}

      /// handle writing of dynamic xAOD attributes of an AuxContainer - called from RootTreeContainer::writeObject()
      /// should report bytes written (see concrete implementation)
      //  may throw exceptions
      virtual int writeAuxAttributes(const std::string& base_branch, SG::IAuxStoreIO* store, size_t rows_written ) = 0;

      /// is there a need to call commit()?
      virtual bool needsCommit() = 0;

      /// Call Fill() on the ROOT object used by this writer
      virtual int commit() = 0;

      /// set per-branch independent commit/fill mode
      virtual void setBranchFillMode(bool) = 0;
   };


} // namespace

#endif
