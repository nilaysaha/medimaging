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

#if !defined(ORTHANC_ENABLE_PUGIXML)
#  error Macro ORTHANC_ENABLE_PUGIXML must be defined to use this file
#endif

#include "ITagVisitor.h"

#include <json/value.h>


namespace Orthanc
{
  class ORTHANC_PUBLIC DicomWebJsonVisitor : public ITagVisitor
  {
  public:
    enum BinaryMode
    {
      BinaryMode_Ignore,
      BinaryMode_BulkDataUri,
      BinaryMode_InlineBinary
    };
    
    class IBinaryFormatter : public boost::noncopyable
    {
    public:
      virtual ~IBinaryFormatter()
      {
      }

      virtual BinaryMode Format(std::string& bulkDataUri,
                                const std::vector<DicomTag>& parentTags,
                                const std::vector<size_t>& parentIndexes,
                                const DicomTag& tag,
                                ValueRepresentation vr) = 0;
    };
    
  private:
    Json::Value        result_;
    IBinaryFormatter  *formatter_;

    static std::string FormatTag(const DicomTag& tag);
    
    Json::Value& CreateNode(const std::vector<DicomTag>& parentTags,
                            const std::vector<size_t>& parentIndexes,
                            const DicomTag& tag);

    static Json::Value FormatInteger(int64_t value);

    static Json::Value FormatDouble(double value);

  public:
    DicomWebJsonVisitor() :
      formatter_(NULL)
    {
      Clear();
    }

    void SetFormatter(IBinaryFormatter& formatter)
    {
      formatter_ = &formatter;
    }
    
    void Clear()
    {
      result_ = Json::objectValue;
    }

    const Json::Value& GetResult() const
    {
      return result_;
    }

#if ORTHANC_ENABLE_PUGIXML == 1
    void FormatXml(std::string& target) const;
#endif

    virtual void VisitNotSupported(const std::vector<DicomTag>& parentTags,
                                   const std::vector<size_t>& parentIndexes,
                                   const DicomTag& tag,
                                   ValueRepresentation vr)
      ORTHANC_OVERRIDE
    {
    }

    virtual void VisitEmptySequence(const std::vector<DicomTag>& parentTags,
                                    const std::vector<size_t>& parentIndexes,
                                    const DicomTag& tag)
      ORTHANC_OVERRIDE;

    virtual void VisitBinary(const std::vector<DicomTag>& parentTags,
                             const std::vector<size_t>& parentIndexes,
                             const DicomTag& tag,
                             ValueRepresentation vr,
                             const void* data,
                             size_t size)
      ORTHANC_OVERRIDE;

    virtual void VisitIntegers(const std::vector<DicomTag>& parentTags,
                               const std::vector<size_t>& parentIndexes,
                               const DicomTag& tag,
                               ValueRepresentation vr,
                               const std::vector<int64_t>& values)
      ORTHANC_OVERRIDE;

    virtual void VisitDoubles(const std::vector<DicomTag>& parentTags,
                              const std::vector<size_t>& parentIndexes,
                              const DicomTag& tag,
                              ValueRepresentation vr,
                              const std::vector<double>& values)
      ORTHANC_OVERRIDE;

    virtual void VisitAttributes(const std::vector<DicomTag>& parentTags,
                                 const std::vector<size_t>& parentIndexes,
                                 const DicomTag& tag,
                                 const std::vector<DicomTag>& values)
      ORTHANC_OVERRIDE;

    virtual Action VisitString(std::string& newValue,
                               const std::vector<DicomTag>& parentTags,
                               const std::vector<size_t>& parentIndexes,
                               const DicomTag& tag,
                               ValueRepresentation vr,
                               const std::string& value)
      ORTHANC_OVERRIDE;
  };
}
