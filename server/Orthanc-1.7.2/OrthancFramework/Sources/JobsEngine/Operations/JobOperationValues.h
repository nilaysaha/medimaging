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

#include "JobOperationValue.h"

#include <vector>

namespace Orthanc
{
  class IJobUnserializer;

  class ORTHANC_PUBLIC JobOperationValues : public boost::noncopyable
  {
  private:
    std::vector<JobOperationValue*>   values_;

    void Append(JobOperationValues& target,
                bool clear);

  public:
    ~JobOperationValues()
    {
      Clear();
    }

    void Move(JobOperationValues& target)
    {
      return Append(target, true);
    }

    void Copy(JobOperationValues& target)
    {
      return Append(target, false);
    }

    void Clear();

    void Reserve(size_t count)
    {
      values_.reserve(count);
    }

    void Append(JobOperationValue* value);  // Takes ownership

    size_t GetSize() const
    {
      return values_.size();
    }

    JobOperationValue& GetValue(size_t index) const;

    void Serialize(Json::Value& target) const;

    static JobOperationValues* Unserialize(IJobUnserializer& unserializer,
                                           const Json::Value& source);
  };
}
