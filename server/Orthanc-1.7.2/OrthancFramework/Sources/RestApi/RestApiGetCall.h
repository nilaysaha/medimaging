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

#include "RestApiCall.h"

namespace Orthanc
{
  class RestApiGetCall : public RestApiCall
  {
  private:
    const IHttpHandler::Arguments& getArguments_;

  public:
    typedef void (*Handler) (RestApiGetCall& call);   

    RestApiGetCall(RestApiOutput& output,
                   RestApi& context,
                   RequestOrigin origin,
                   const char* remoteIp,
                   const char* username,
                   const IHttpHandler::Arguments& httpHeaders,
                   const IHttpHandler::Arguments& uriComponents,
                   const UriComponents& trailing,
                   const UriComponents& fullUri,
                   const IHttpHandler::Arguments& getArguments) :
      RestApiCall(output, context, origin, remoteIp, username, 
                  httpHeaders, uriComponents, trailing, fullUri),
      getArguments_(getArguments)
    {
    }

    std::string GetArgument(const std::string& name,
                            const std::string& defaultValue) const
    {
      return HttpToolbox::GetArgument(getArguments_, name, defaultValue);
    }

    bool HasArgument(const std::string& name) const
    {
      return getArguments_.find(name) != getArguments_.end();
    }

    virtual bool ParseJsonRequest(Json::Value& result) const;
  };
}
