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

#include "DicomTag.h"
#include "DicomValue.h"
#include "../Enumerations.h"

#include <set>
#include <map>
#include <json/json.h>

namespace Orthanc
{
  class ORTHANC_PUBLIC DicomMap : public boost::noncopyable
  {
  public:
    typedef std::map<DicomTag, DicomValue*>  Content;
    
  private:
    friend class DicomArray;
    friend class FromDcmtkBridge;
    friend class ParsedDicomFile;

    Content content_;

    // Warning: This takes the ownership of "value"
    void SetValueInternal(uint16_t group, 
                          uint16_t element, 
                          DicomValue* value);

    static void GetMainDicomTagsInternal(std::set<DicomTag>& result,
                                         ResourceType level);

  public:
    DicomMap()
    {
    }

    ~DicomMap()
    {
      Clear();
    }

    size_t GetSize() const
    {
      return content_.size();
    }
    
    DicomMap* Clone() const;

    void Assign(const DicomMap& other);

    void Clear();

    void SetNullValue(uint16_t group, 
                      uint16_t element)
    {
      SetValueInternal(group, element, new DicomValue);
    }
    
    void SetNullValue(const DicomTag& tag)
    {
      SetValueInternal(tag.GetGroup(), tag.GetElement(), new DicomValue);
    }
    
    void SetValue(uint16_t group, 
                  uint16_t element, 
                  const DicomValue& value)
    {
      SetValueInternal(group, element, value.Clone());
    }

    void SetValue(const DicomTag& tag,
                  const DicomValue& value)
    {
      SetValueInternal(tag.GetGroup(), tag.GetElement(), value.Clone());
    }

    void SetValue(const DicomTag& tag,
                  const std::string& str,
                  bool isBinary)
    {
      SetValueInternal(tag.GetGroup(), tag.GetElement(), new DicomValue(str, isBinary));
    }

    void SetValue(uint16_t group, 
                  uint16_t element, 
                  const std::string& str,
                  bool isBinary)
    {
      SetValueInternal(group, element, new DicomValue(str, isBinary));
    }

    bool HasTag(uint16_t group, uint16_t element) const
    {
      return HasTag(DicomTag(group, element));
    }

    bool HasTag(const DicomTag& tag) const
    {
      return content_.find(tag) != content_.end();
    }

    const DicomValue& GetValue(uint16_t group, uint16_t element) const
    {
      return GetValue(DicomTag(group, element));
    }

    const DicomValue& GetValue(const DicomTag& tag) const;

    // DO NOT delete the returned value!
    const DicomValue* TestAndGetValue(uint16_t group, uint16_t element) const
    {
      return TestAndGetValue(DicomTag(group, element));
    }       

    // DO NOT delete the returned value!
    const DicomValue* TestAndGetValue(const DicomTag& tag) const;

    void Remove(const DicomTag& tag);

    void ExtractPatientInformation(DicomMap& result) const;

    void ExtractStudyInformation(DicomMap& result) const;

    void ExtractSeriesInformation(DicomMap& result) const;

    void ExtractInstanceInformation(DicomMap& result) const;

    static void SetupFindPatientTemplate(DicomMap& result);

    static void SetupFindStudyTemplate(DicomMap& result);

    static void SetupFindSeriesTemplate(DicomMap& result);

    static void SetupFindInstanceTemplate(DicomMap& result);

    void CopyTagIfExists(const DicomMap& source,
                         const DicomTag& tag);

    static bool IsMainDicomTag(const DicomTag& tag, ResourceType level);

    static bool IsMainDicomTag(const DicomTag& tag);

    static void GetMainDicomTags(std::set<DicomTag>& result, ResourceType level);

    static void GetMainDicomTags(std::set<DicomTag>& result);

    void GetTags(std::set<DicomTag>& tags) const;

    static bool IsDicomFile(const void* dicom,
                            size_t size);
    
    static bool ParseDicomMetaInformation(DicomMap& result,
                                          const void* dicom,
                                          size_t size);

    void LogMissingTagsForStore() const;

    bool LookupStringValue(std::string& result,
                           const DicomTag& tag,
                           bool allowBinary) const;
    
    bool ParseInteger32(int32_t& result,
                        const DicomTag& tag) const;

    bool ParseInteger64(int64_t& result,
                        const DicomTag& tag) const;                                

    bool ParseUnsignedInteger32(uint32_t& result,
                                const DicomTag& tag) const;

    bool ParseUnsignedInteger64(uint64_t& result,
                                const DicomTag& tag) const;

    bool ParseFloat(float& result,
                    const DicomTag& tag) const;

    bool ParseFirstFloat(float& result,
                         const DicomTag& tag) const;

    bool ParseDouble(double& result,
                     const DicomTag& tag) const;

    void FromDicomAsJson(const Json::Value& dicomAsJson);

    void Merge(const DicomMap& other);

    void MergeMainDicomTags(const DicomMap& other,
                            ResourceType level);

    void ExtractMainDicomTags(const DicomMap& other);

    bool HasOnlyMainDicomTags() const;
    
    void Serialize(Json::Value& target) const;

    void Unserialize(const Json::Value& source);

    void FromDicomWeb(const Json::Value& source);

    std::string GetStringValue(const DicomTag& tag,
                               const std::string& defaultValue,
                               bool allowBinary) const;

    void RemoveBinaryTags();

    void DumpMainDicomTags(Json::Value& target,
                           ResourceType level) const;

    void ParseMainDicomTags(const Json::Value& source,
                            ResourceType level);

    void Print(FILE* fp) const;  // For debugging only
  };
}
