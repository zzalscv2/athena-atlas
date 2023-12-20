/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <EventLoop/OutputStreamData.h>

#include <AsgMessaging/StatusCode.h>
#include <RootCoreUtils/Assert.h>
#include <RootCoreUtils/RootUtils.h>
#include <TH1.h>
#include <TTree.h>

//
// method implementations
//

namespace EL
{
  namespace Detail
  {
    namespace
    {
      /// Helper class for managing the current ROOT directory during operations
      class DirectoryReset {
      public:
        /// Constructor, capturing which directory we need to return to
        DirectoryReset() : m_dir( *gDirectory ) {}
        /// Destructor, returning to the original directory
        ~DirectoryReset() { m_dir.cd(); }
      private:
        /// The directory we need to return to
        TDirectory& m_dir;
      };



      struct MyWriter : public SH::DiskWriter
      {
        /// description: this is a custom writer for the old-school
        ///   drivers that don't use an actual writer

      public:
        std::unique_ptr<TFile> m_file;

        explicit MyWriter (std::unique_ptr<TFile> val_file)
          : m_file (std::move (val_file))
        {
          if (m_file == nullptr)
            throw std::runtime_error ("encountered null pointer for output file");
          m_path = m_file->GetName();
        }

        ~MyWriter ()
        {
          if (m_file != 0)
            close();
        }

        std::string getPath () const
        {
          return m_path;
        }

        TFile *getFile ()
        {
          RCU_REQUIRE2_SOFT (m_file != 0, "file already closed");
          return m_file.get();
        }

        void doClose ()
        {
          RCU_REQUIRE2_SOFT (m_file != 0, "file already closed");
          m_file->Write ();
          m_file->Close ();
          m_file = 0;
        }
        /// \brief the path being used
        private:
          std::string m_path;
      };



      /// \brief open the given file, throwing if we fail
      std::unique_ptr<TFile> checkedOpenFile (const std::string& path, const std::string& mode)
      {
        std::unique_ptr<TFile> result (TFile::Open (path.c_str(), mode.c_str()));
        if (result == nullptr)
          throw std::runtime_error ("failed to open file " + path + " with mode " + mode);
        return result;
      }
    }



    void OutputStreamData ::
    testInvariant () const
    {
      RCU_INVARIANT (this != nullptr);
      RCU_INVARIANT (m_writer != nullptr);
    }



    OutputStreamData ::
    OutputStreamData (const std::string& val_fileName, const std::string& mode)
      : OutputStreamData (checkedOpenFile (val_fileName, mode))
    {
      // no invariant used
    }



    OutputStreamData ::
    OutputStreamData (std::unique_ptr<TFile> file)
      : OutputStreamData (std::make_unique<MyWriter> (std::move (file)))
    {
      // no invariant used
    }



    OutputStreamData ::
    OutputStreamData (std::unique_ptr<SH::DiskWriter> val_writer)
      : m_writer (std::move (val_writer))
    {
      RCU_NEW_INVARIANT (this);
    }



    TFile *OutputStreamData ::
    file () const noexcept
    {
      RCU_READ_INVARIANT (this);
      TFile *result = m_writer->file();
      RCU_PROVIDE (result != nullptr);
      return result;
    }



    void OutputStreamData ::
    close ()
    {
      RCU_CHANGE_INVARIANT (this);
      saveOutput ();
      m_writer->close ();
    }



    std::string OutputStreamData ::
    finalFileName () const
    {
      RCU_READ_INVARIANT (this);
      return m_writer->path ();
    }



    void OutputStreamData ::
    saveOutput ()
    {
      RCU_CHANGE_INVARIANT (this);
      TFile *const file {m_writer->file()};
      RCU_ASSERT (file != nullptr);
      for (std::unique_ptr<TObject>& object : m_output)
      {
        auto barePtr = object.release();
        std::string name = barePtr->GetName();
        TDirectory *dir = makeDirectoryFor (name);
        if (dir != file)
        {
          TNamed *named = dynamic_cast<TNamed*>(barePtr);
          if (named)
            named->SetName (name.c_str());
        }

        if (!RCU::SetDirectory (barePtr, dir))
          dir->WriteObject (barePtr, name.c_str());
      }
      m_outputHistMap.clear ();
      m_outputTreeMap.clear ();
      m_output.clear ();
    }



    void OutputStreamData ::
    addOutput (std::unique_ptr<TObject> outputObject)
    {
      RCU_CHANGE_INVARIANT (this);

      TTree *const tree = dynamic_cast<TTree*> (outputObject.get());
      if (tree)
      {
        std::string name = tree->GetName();
        std::string treeName = tree->GetName();

        TDirectory *dir = makeDirectoryFor (treeName);

        // if we are in a sub-directory we need to rename the tree
        if (name != treeName)
          tree->SetName (treeName.c_str());

        // pass ownership of the tree to the directory
        tree->SetDirectory (dir);
        outputObject.release();

        m_outputTreeMap[name] = tree;


      } else
      {
        TH1 *const hist = dynamic_cast<TH1*> (outputObject.get());

        m_outputHistMap[outputObject->GetName()] = outputObject.get();
        m_output.emplace_back (std::move (outputObject));
        if (hist)
          hist->SetDirectory (nullptr);
      }
    }



    void OutputStreamData ::
    addClone (const TObject& prototypeObject)
    {
      // no invariant used

      // Do not change the user's "current directory" during any of the
      // following...
      DirectoryReset dirReset;

      // Make a clone of the object, and make sure we are already in
      // the right directory if needed
      std::string name = prototypeObject.GetName();
      TDirectory *dir = makeDirectoryFor (name);
      dir->cd();

      std::unique_ptr<TObject> clone {prototypeObject.Clone()};

      // Hold on to the pointer of the tree in our internal cache.
      addOutput (std::move (clone));
    }



    TDirectory *OutputStreamData ::
    makeDirectoryFor (std::string& name)
    {
      RCU_CHANGE_INVARIANT (this);

      TDirectory *result = file();
      std::string::size_type split = name.rfind ("/");
      if (split == std::string::npos)
      {
        return result;
      } else
      {
        const std::string dirname = name.substr (0, split);
        name = name.substr (split + 1);
        TDirectory *subdir = dynamic_cast<TDirectory*>(result->Get (dirname.c_str()));
        if (!subdir)
        {
          result->mkdir (dirname.c_str());
          subdir = dynamic_cast<TDirectory*>(result->Get (dirname.c_str()));
          RCU_ASSERT (subdir != nullptr);
        }
        result = subdir;
        RCU_ASSERT (result != nullptr);
        return result;
      }
    }



    TObject *OutputStreamData ::
    getOutputHist (const std::string& name) const noexcept
    {
      RCU_READ_INVARIANT (this);

      auto iter = m_outputHistMap.find (name);
      return iter != m_outputHistMap.end() ? iter->second : nullptr;
    }



    TTree *OutputStreamData ::
    getOutputTree (const std::string& name) const noexcept
    {
      RCU_READ_INVARIANT (this);

      auto iter = m_outputTreeMap.find (name);
      return iter != m_outputTreeMap.end() ? iter->second : nullptr;
    }
  }
}
