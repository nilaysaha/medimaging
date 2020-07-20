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

#include "../OrthancFramework.h"

#if !defined(ORTHANC_ENABLE_ZLIB)
#  error The macro ORTHANC_ENABLE_ZLIB must be defined
#endif

#if ORTHANC_ENABLE_ZLIB != 1
#  error ZLIB support must be enabled to include this file
#endif


#include <stdint.h>
#include <string>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace Orthanc
{
  class ORTHANC_PUBLIC ZipWriter : public boost::noncopyable
  {
  private:
    struct PImpl;
    boost::shared_ptr<PImpl> pimpl_;

    bool isZip64_;
    bool hasFileInZip_;
    bool append_;
    uint8_t compressionLevel_;
    std::string path_;

  public:
    ZipWriter();

    ~ZipWriter();

    void SetZip64(bool isZip64);

    bool IsZip64() const
    {
      return isZip64_;
    }

    void SetCompressionLevel(uint8_t level);

    uint8_t GetCompressionLevel() const
    {
      return compressionLevel_;
    }

    void SetAppendToExisting(bool append);
    
    bool IsAppendToExisting() const
    {
      return append_;
    }
    
    void Open();

    void Close();

    bool IsOpen() const;

    void SetOutputPath(const char* path);

    const std::string& GetOutputPath() const
    {
      return path_;
    }

    void OpenFile(const char* path);

    void Write(const void* data, size_t length);

    void Write(const std::string& data);
  };
}
