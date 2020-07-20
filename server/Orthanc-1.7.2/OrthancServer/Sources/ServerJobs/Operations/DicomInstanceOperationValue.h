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

#include "../../../../OrthancFramework/Sources/JobsEngine/Operations/JobOperationValue.h"

namespace Orthanc
{
  class ServerContext;
  
  class DicomInstanceOperationValue : public JobOperationValue
  {
  private:
    ServerContext&   context_;
    std::string      id_;

  public:
    DicomInstanceOperationValue(ServerContext& context,
                                const std::string& id) :
      JobOperationValue(Type_DicomInstance),
      context_(context),
      id_(id)
    {
    }

    ServerContext& GetServerContext() const
    {
      return context_;
    }

    const std::string& GetId() const
    {
      return id_;
    }

    void ReadDicom(std::string& dicom) const;

    virtual JobOperationValue* Clone() const
    {
      return new DicomInstanceOperationValue(context_, id_);
    }

    virtual void Serialize(Json::Value& target) const
    {
      target = Json::objectValue;
      target["Type"] = "DicomInstance";
      target["ID"] = id_;
    }
  };
}
