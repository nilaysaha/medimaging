/**
 * Orthanc - A Lightweight, RESTful DICOM Store
 * Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
 * Department, University Hospital of Liege, Belgium
 * Copyright (C) 2017-2020 Osimis S.A., Belgium
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * In addition, as a special exception, the copyright holders of this
 * program give permission to link the code of its release with the
 * OpenSSL project's "OpenSSL" library (or with modified versions of it
 * that use the same license as the "OpenSSL" library), and distribute
 * the linked executables. You must obey the GNU General Public License
 * in all respects for all of the code used other than "OpenSSL". If you
 * modify file(s) with this exception, you may extend this exception to
 * your version of the file(s), but you are not obligated to do so. If
 * you do not wish to do so, delete this exception statement from your
 * version. If you delete this exception statement from all source files
 * in the program, then also delete it here.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/


#pragma once

#include "../../OrthancFramework/Sources/DicomFormat/DicomInstanceHasher.h"
#include "../../OrthancFramework/Sources/DicomFormat/DicomMap.h"
#include "DicomInstanceOrigin.h"
#include "ServerEnumerations.h"

#include <boost/shared_ptr.hpp>

namespace Orthanc
{
  class ParsedDicomFile;

  class DicomInstanceToStore : public boost::noncopyable
  {
  public:
    typedef std::map<std::pair<ResourceType, MetadataType>, std::string>  MetadataMap;

  private:
    class PImpl;
    boost::shared_ptr<PImpl>  pimpl_;

  public:
    DicomInstanceToStore();

    void SetOrigin(const DicomInstanceOrigin& origin);
    
    const DicomInstanceOrigin& GetOrigin() const;

    // WARNING: The buffer is not copied, it must not be removed as
    // long as the "DicomInstanceToStore" object is alive
    void SetBuffer(const void* dicom,
                   size_t size);

    void SetParsedDicomFile(ParsedDicomFile& parsed);

    void SetSummary(const DicomMap& summary);

    void SetJson(const Json::Value& json);

    const MetadataMap& GetMetadata() const;

    MetadataMap& GetMetadata();

    void AddMetadata(ResourceType level,
                     MetadataType metadata,
                     const std::string& value);

    const void* GetBufferData() const;

    size_t GetBufferSize() const;

    const DicomMap& GetSummary();
    
    const Json::Value& GetJson() const;

    bool LookupTransferSyntax(std::string& result) const;

    DicomInstanceHasher& GetHasher();

    bool HasPixelData() const;

    ParsedDicomFile& GetParsedDicomFile() const;
  };
}
