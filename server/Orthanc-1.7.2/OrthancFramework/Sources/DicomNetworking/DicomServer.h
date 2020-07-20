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

#if ORTHANC_ENABLE_DCMTK_NETWORKING != 1
#  error The macro ORTHANC_ENABLE_DCMTK_NETWORKING must be set to 1
#endif

#include "IFindRequestHandlerFactory.h"
#include "IMoveRequestHandlerFactory.h"
#include "IGetRequestHandlerFactory.h"
#include "IStoreRequestHandlerFactory.h"
#include "IWorklistRequestHandlerFactory.h"
#include "IStorageCommitmentRequestHandlerFactory.h"
#include "IApplicationEntityFilter.h"
#include "RemoteModalityParameters.h"

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>


namespace Orthanc
{
  class DicomServer : public boost::noncopyable
  {
  public:
    // WARNING: The methods of this class must be thread-safe
    class IRemoteModalities : public boost::noncopyable
    {
    public:
      virtual ~IRemoteModalities()
      {
      }
      
      virtual bool IsSameAETitle(const std::string& aet1,
                                 const std::string& aet2) = 0;

      virtual bool LookupAETitle(RemoteModalityParameters& modality,
                                 const std::string& aet) = 0;
    };
    
  private:
    struct PImpl;
    boost::shared_ptr<PImpl> pimpl_;

    bool checkCalledAet_;
    std::string aet_;
    uint16_t port_;
    bool continue_;
    uint32_t associationTimeout_;
    IRemoteModalities* modalities_;
    IFindRequestHandlerFactory* findRequestHandlerFactory_;
    IMoveRequestHandlerFactory* moveRequestHandlerFactory_;
    IGetRequestHandlerFactory* getRequestHandlerFactory_;
    IStoreRequestHandlerFactory* storeRequestHandlerFactory_;
    IWorklistRequestHandlerFactory* worklistRequestHandlerFactory_;
    IStorageCommitmentRequestHandlerFactory* storageCommitmentFactory_;
    IApplicationEntityFilter* applicationEntityFilter_;

    static void ServerThread(DicomServer* server);

  public:
    DicomServer();

    ~DicomServer();

    void SetPortNumber(uint16_t port);
    uint16_t GetPortNumber() const;

    void SetAssociationTimeout(uint32_t seconds);
    uint32_t GetAssociationTimeout() const;

    void SetCalledApplicationEntityTitleCheck(bool check);
    bool HasCalledApplicationEntityTitleCheck() const;

    void SetApplicationEntityTitle(const std::string& aet);
    const std::string& GetApplicationEntityTitle() const;

    void SetRemoteModalities(IRemoteModalities& modalities);
    IRemoteModalities& GetRemoteModalities() const;
    
    void SetFindRequestHandlerFactory(IFindRequestHandlerFactory& handler);
    bool HasFindRequestHandlerFactory() const;
    IFindRequestHandlerFactory& GetFindRequestHandlerFactory() const;

    void SetMoveRequestHandlerFactory(IMoveRequestHandlerFactory& handler);
    bool HasMoveRequestHandlerFactory() const;
    IMoveRequestHandlerFactory& GetMoveRequestHandlerFactory() const;

    void SetGetRequestHandlerFactory(IGetRequestHandlerFactory& handler);
    bool HasGetRequestHandlerFactory() const;
    IGetRequestHandlerFactory& GetGetRequestHandlerFactory() const;

    void SetStoreRequestHandlerFactory(IStoreRequestHandlerFactory& handler);
    bool HasStoreRequestHandlerFactory() const;
    IStoreRequestHandlerFactory& GetStoreRequestHandlerFactory() const;

    void SetWorklistRequestHandlerFactory(IWorklistRequestHandlerFactory& handler);
    bool HasWorklistRequestHandlerFactory() const;
    IWorklistRequestHandlerFactory& GetWorklistRequestHandlerFactory() const;

    void SetStorageCommitmentRequestHandlerFactory(IStorageCommitmentRequestHandlerFactory& handler);
    bool HasStorageCommitmentRequestHandlerFactory() const;
    IStorageCommitmentRequestHandlerFactory& GetStorageCommitmentRequestHandlerFactory() const;

    void SetApplicationEntityFilter(IApplicationEntityFilter& handler);
    bool HasApplicationEntityFilter() const;
    IApplicationEntityFilter& GetApplicationEntityFilter() const;

    void Start();
  
    void Stop();

    bool IsMyAETitle(const std::string& aet) const;
  };

}
