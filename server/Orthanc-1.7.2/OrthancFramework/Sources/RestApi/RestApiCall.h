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

#include "../HttpServer/IHttpHandler.h"
#include "../HttpServer/HttpToolbox.h"
#include "RestApiPath.h"
#include "RestApiOutput.h"

#include <boost/noncopyable.hpp>

namespace Orthanc
{
  class RestApi;

  class RestApiCall : public boost::noncopyable
  {
  private:
    RestApiOutput& output_;
    RestApi& context_;
    RequestOrigin origin_;
    const char* remoteIp_;
    const char* username_;
    const IHttpHandler::Arguments& httpHeaders_;
    const IHttpHandler::Arguments& uriComponents_;
    const UriComponents& trailing_;
    const UriComponents& fullUri_;

  protected:
    static bool ParseJsonRequestInternal(Json::Value& result,
                                         const void* body,
                                         size_t size);

  public:
    RestApiCall(RestApiOutput& output,
                RestApi& context,
                RequestOrigin origin,
                const char* remoteIp,
                const char* username,
                const IHttpHandler::Arguments& httpHeaders,
                const IHttpHandler::Arguments& uriComponents,
                const UriComponents& trailing,
                const UriComponents& fullUri) :
      output_(output),
      context_(context),
      origin_(origin),
      remoteIp_(remoteIp),
      username_(username),
      httpHeaders_(httpHeaders),
      uriComponents_(uriComponents),
      trailing_(trailing),
      fullUri_(fullUri)
    {
    }

    RestApiOutput& GetOutput()
    {
      return output_;
    }

    RestApi& GetContext()
    {
      return context_;
    }
    
    const UriComponents& GetFullUri() const
    {
      return fullUri_;
    }
    
    const UriComponents& GetTrailingUri() const
    {
      return trailing_;
    }

    std::string GetUriComponent(const std::string& name,
                                const std::string& defaultValue) const
    {
      return HttpToolbox::GetArgument(uriComponents_, name, defaultValue);
    }

    std::string GetHttpHeader(const std::string& name,
                              const std::string& defaultValue) const
    {
      return HttpToolbox::GetArgument(httpHeaders_, name, defaultValue);
    }

    const IHttpHandler::Arguments& GetHttpHeaders() const
    {
      return httpHeaders_;
    }

    void ParseCookies(IHttpHandler::Arguments& result) const
    {
      HttpToolbox::ParseCookies(result, httpHeaders_);
    }

    std::string FlattenUri() const;

    RequestOrigin GetRequestOrigin() const
    {
      return origin_;
    }

    const char* GetRemoteIp() const
    {
      return remoteIp_;
    }

    const char* GetUsername() const
    {
      return username_;
    }

    virtual bool ParseJsonRequest(Json::Value& result) const = 0;
  };
}
