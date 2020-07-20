/**
 * Orthanc - A Lightweight, RESTful DICOM Store
 * Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
 * Department, University Hospital of Liege, Belgium
 * Copyright (C) 2017-2020 Osimis S.A., Belgium
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 **/


#pragma once

#include "ZipWriter.h"

#include <map>
#include <list>
#include <boost/lexical_cast.hpp>

#if ORTHANC_BUILD_UNIT_TESTS == 1
#  include <gtest/gtest_prod.h>
#endif

namespace Orthanc
{
  class ORTHANC_PUBLIC HierarchicalZipWriter : public boost::noncopyable
  {
#if ORTHANC_BUILD_UNIT_TESTS == 1
    FRIEND_TEST(HierarchicalZipWriter, Index);
    FRIEND_TEST(HierarchicalZipWriter, Filenames);
#endif

  private:
    class ORTHANC_PUBLIC Index
    {
    private:
      struct Directory
      {
        typedef std::map<std::string, unsigned int>  Content;

        std::string name_;
        Content  content_;
      };

      typedef std::list<Directory*> Stack;
  
      Stack stack_;

      std::string EnsureUniqueFilename(const char* filename);

    public:
      Index();

      ~Index();

      bool IsRoot() const
      {
        return stack_.size() == 1;
      }

      std::string OpenFile(const char* name);

      void OpenDirectory(const char* name);

      void CloseDirectory();

      std::string GetCurrentDirectoryPath() const;

      static std::string KeepAlphanumeric(const std::string& source);
    };

    Index indexer_;
    ZipWriter writer_;

  public:
    HierarchicalZipWriter(const char* path);

    ~HierarchicalZipWriter();

    void SetZip64(bool isZip64)
    {
      writer_.SetZip64(isZip64);
    }

    bool IsZip64() const
    {
      return writer_.IsZip64();
    }

    void SetCompressionLevel(uint8_t level)
    {
      writer_.SetCompressionLevel(level);
    }

    uint8_t GetCompressionLevel() const
    {
      return writer_.GetCompressionLevel();
    }

    void SetAppendToExisting(bool append)
    {
      writer_.SetAppendToExisting(append);
    }
    
    bool IsAppendToExisting() const
    {
      return writer_.IsAppendToExisting();
    }
    
    void OpenFile(const char* name);

    void OpenDirectory(const char* name);

    void CloseDirectory();

    std::string GetCurrentDirectoryPath() const
    {
      return indexer_.GetCurrentDirectoryPath();
    }

    void Write(const void* data, size_t length)
    {
      writer_.Write(data, length);
    }

    void Write(const std::string& data)
    {
      writer_.Write(data);
    }
  };
}
