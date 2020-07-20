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

#include "PrecompiledHeadersServer.h"
#include "OrthancGetRequestHandler.h"

#include "../../OrthancFramework/Sources/DicomFormat/DicomArray.h"
#include "../../OrthancFramework/Sources/DicomParsing/FromDcmtkBridge.h"
#include "../../OrthancFramework/Sources/Logging.h"
#include "../../OrthancFramework/Sources/MetricsRegistry.h"
#include "OrthancConfiguration.h"
#include "ServerContext.h"
#include "ServerJobs/DicomModalityStoreJob.h"

#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcistrmb.h>
#include <dcmtk/dcmnet/assoc.h>
#include <dcmtk/dcmnet/dimse.h>
#include <dcmtk/dcmnet/diutil.h>
#include <dcmtk/ofstd/ofstring.h>

#include <sstream>  // For std::stringstream

namespace Orthanc
{
  namespace
  {
    // Anonymous namespace to avoid clashes between compilation modules
    
    static void GetSubOpProgressCallback(
      void * /* callbackData == pointer to the "OrthancGetRequestHandler" object */,
      T_DIMSE_StoreProgress *progress,
      T_DIMSE_C_StoreRQ * /*req*/)
    {
      // SBL - no logging to be done here.
    }
  }

  OrthancGetRequestHandler::Status
  OrthancGetRequestHandler::DoNext(T_ASC_Association* assoc)
  {
    if (position_ >= instances_.size())
    {
      return Status_Failure;
    }
    
    const std::string& id = instances_[position_++];

    std::string dicom;
    context_.ReadDicom(dicom, id);
    
    if (dicom.size() <= 0)
    {
      return Status_Failure;
    }

    std::unique_ptr<DcmFileFormat> parsed(
      FromDcmtkBridge::LoadFromMemoryBuffer(dicom.c_str(), dicom.size()));

    if (parsed.get() == NULL ||
        parsed->getDataset() == NULL)
    {
      throw OrthancException(ErrorCode_InternalError);
    }
    
    DcmDataset& dataset = *parsed->getDataset();
    
    OFString a, b;
    if (!dataset.findAndGetOFString(DCM_SOPClassUID, a).good() ||
        !dataset.findAndGetOFString(DCM_SOPInstanceUID, b).good())
    {
      throw OrthancException(ErrorCode_NoSopClassOrInstance,
                             "Unable to determine the SOP class/instance for C-STORE with AET " +
                             originatorAet_);
    }

    std::string sopClassUid(a.c_str());
    std::string sopInstanceUid(b.c_str());
    
    OFCondition cond = PerformGetSubOp(assoc, sopClassUid, sopInstanceUid, parsed.release());
    
    if (getCancelled_)
    {
      LOG(INFO) << "C-GET SCP: Received C-Cancel RQ";
    }
    
    if (cond.bad() || getCancelled_)
    {
      return Status_Failure;
    }
    
    return Status_Success;
  }

  
  void OrthancGetRequestHandler::AddFailedUIDInstance(const std::string& sopInstance)
  {
    if (failedUIDs_.empty())
    {
      failedUIDs_ = sopInstance;
    }
    else
    {
      failedUIDs_ += "\\" + sopInstance;
    }
  }


  static bool SelectPresentationContext(T_ASC_PresentationContextID& selectedPresentationId,
                                        DicomTransferSyntax& selectedSyntax,
                                        T_ASC_Association* assoc,
                                        const std::string& sopClassUid,
                                        DicomTransferSyntax sourceSyntax,
                                        bool allowTranscoding)
  {
    typedef std::map<DicomTransferSyntax, T_ASC_PresentationContextID> Accepted;

    Accepted accepted;

    /**
     * 1. Inspect and index all the accepted transfer syntaxes. This
     * is similar to the code from "DicomAssociation::Open()".
     **/

    LST_HEAD **l = &assoc->params->DULparams.acceptedPresentationContext;
    if (*l != NULL)
    {
      DUL_PRESENTATIONCONTEXT* pc = (DUL_PRESENTATIONCONTEXT*) LST_Head(l);
      LST_Position(l, (LST_NODE*)pc);
      while (pc)
      {
        DicomTransferSyntax transferSyntax;
        if (pc->result == ASC_P_ACCEPTANCE &&
            LookupTransferSyntax(transferSyntax, pc->acceptedTransferSyntax))
        {
          VLOG(0) << "C-GET SCP accepted: SOP class " << sopClassUid
                  << " with transfer syntax " << GetTransferSyntaxUid(transferSyntax);
          if (std::string(pc->abstractSyntax) == sopClassUid)
          {
            accepted[transferSyntax] = pc->presentationContextID;
          }
        }
        else
        {
          LOG(WARNING) << "C-GET: Unknown transfer syntax received: "
                       << pc->acceptedTransferSyntax;
        }
            
        pc = (DUL_PRESENTATIONCONTEXT*) LST_Next(l);
      }
    }

    
    /**
     * 2. Select the preferred transfer syntaxes, which corresponds to
     * the source transfer syntax, plus all the uncompressed transfer
     * syntaxes if transcoding is enabled.
     **/
    
    std::list<DicomTransferSyntax> preferred;
    preferred.push_back(sourceSyntax);

    if (allowTranscoding)
    {
      if (sourceSyntax != DicomTransferSyntax_LittleEndianImplicit)
      {
        // Default Transfer Syntax for DICOM
        preferred.push_back(DicomTransferSyntax_LittleEndianImplicit);
      }

      if (sourceSyntax != DicomTransferSyntax_LittleEndianExplicit)
      {
        preferred.push_back(DicomTransferSyntax_LittleEndianExplicit);
      }

      if (sourceSyntax != DicomTransferSyntax_BigEndianExplicit)
      {
        // Retired
        preferred.push_back(DicomTransferSyntax_BigEndianExplicit);
      }
    }


    /**
     * 3. Lookup whether one of the preferred transfer syntaxes was
     * accepted.
     **/
    
    for (std::list<DicomTransferSyntax>::const_iterator
           it = preferred.begin(); it != preferred.end(); ++it)
    {
      Accepted::const_iterator found = accepted.find(*it);
      if (found != accepted.end())
      {
        selectedPresentationId = found->second;
        selectedSyntax = *it;
        return true;
      }
    }

    // No preferred syntax was accepted
    return false;
  }                                                           


  OFCondition OrthancGetRequestHandler::PerformGetSubOp(T_ASC_Association* assoc,
                                                        const std::string& sopClassUid,
                                                        const std::string& sopInstanceUid,
                                                        DcmFileFormat* dicomRaw)
  {
    assert(dicomRaw != NULL);
    std::unique_ptr<DcmFileFormat> dicom(dicomRaw);
    
    DicomTransferSyntax sourceSyntax;
    if (!FromDcmtkBridge::LookupOrthancTransferSyntax(sourceSyntax, *dicom))
    {
      nFailed_++;
      AddFailedUIDInstance(sopInstanceUid);
      LOG(ERROR) << "C-GET SCP: Unknown transfer syntax: ("
                 << dcmSOPClassUIDToModality(sopClassUid.c_str(), "OT") << ") " << sopClassUid;
      return DIMSE_NOVALIDPRESENTATIONCONTEXTID;
    }

    bool allowTranscoding = (context_.IsTranscodeDicomProtocol() &&
                             remote_.IsTranscodingAllowed());
    
    T_ASC_PresentationContextID presId = 0;  // Unnecessary initialization, makes code clearer
    DicomTransferSyntax selectedSyntax;
    if (!SelectPresentationContext(presId, selectedSyntax, assoc, sopClassUid,
                                   sourceSyntax, allowTranscoding) ||
        presId == 0)
    {
      nFailed_++;
      AddFailedUIDInstance(sopInstanceUid);
      LOG(ERROR) << "C-GET SCP: storeSCU: No presentation context for: ("
                 << dcmSOPClassUIDToModality(sopClassUid.c_str(), "OT") << ") " << sopClassUid;
      return DIMSE_NOVALIDPRESENTATIONCONTEXTID;
    }
    else
    {
      LOG(INFO) << "C-GET SCP selected transfer syntax " << GetTransferSyntaxUid(selectedSyntax)
                << ", for source instance with SOP class " << sopClassUid
                << " and transfer syntax " << GetTransferSyntaxUid(sourceSyntax);

      // make sure that we can send images in this presentation context
      T_ASC_PresentationContext pc;
      ASC_findAcceptedPresentationContext(assoc->params, presId, &pc);
      // the acceptedRole is the association requestor role

      if (pc.acceptedRole != ASC_SC_ROLE_DEFAULT &&  // "DEFAULT" is necessary for GinkgoCADx
          pc.acceptedRole != ASC_SC_ROLE_SCP &&
          pc.acceptedRole != ASC_SC_ROLE_SCUSCP)
      {
        // the role is not appropriate
        nFailed_++;
        AddFailedUIDInstance(sopInstanceUid);
        LOG(ERROR) << "C-GET SCP: storeSCU: [No presentation context with requestor SCP role for: ("
                   << dcmSOPClassUIDToModality(sopClassUid.c_str(), "OT") << ") " << sopClassUid;
        return DIMSE_NOVALIDPRESENTATIONCONTEXTID;
      }
    }

    const DIC_US msgId = assoc->nextMsgID++;
    
    T_DIMSE_C_StoreRQ req;
    memset(&req, 0, sizeof(req));
    req.MessageID = msgId;
    strncpy(req.AffectedSOPClassUID, sopClassUid.c_str(), DIC_UI_LEN);
    strncpy(req.AffectedSOPInstanceUID, sopInstanceUid.c_str(), DIC_UI_LEN);
    req.DataSetType = DIMSE_DATASET_PRESENT;
    req.Priority = DIMSE_PRIORITY_MEDIUM;
    req.opts = 0;
    
    T_DIMSE_C_StoreRSP rsp;
    memset(&rsp, 0, sizeof(rsp));

    LOG(INFO) << "Store SCU RQ: MsgID " << msgId << ", ("
              << dcmSOPClassUIDToModality(sopClassUid.c_str(), "OT") << ")";
    
    T_DIMSE_DetectedCancelParameters cancelParameters;
    memset(&cancelParameters, 0, sizeof(cancelParameters));

    std::unique_ptr<DcmDataset> stDetail;

    OFCondition cond;

    if (sourceSyntax == selectedSyntax)
    {
      // No transcoding is required
      DcmDataset *stDetailTmp = NULL;
      cond = DIMSE_storeUser(
        assoc, presId, &req, NULL /* imageFileName */, dicom->getDataset(),
        GetSubOpProgressCallback, this /* callbackData */,
        (timeout_ > 0 ? DIMSE_NONBLOCKING : DIMSE_BLOCKING), timeout_,
        &rsp, &stDetailTmp, &cancelParameters);
      stDetail.reset(stDetailTmp);
    }
    else
    {
      // Transcoding to the selected uncompressed transfer syntax
      IDicomTranscoder::DicomImage source, transcoded;
      source.AcquireParsed(dicom.release());

      std::set<DicomTransferSyntax> ts;
      ts.insert(selectedSyntax);
      
      if (context_.Transcode(transcoded, source, ts, true))
      {
        // Transcoding has succeeded
        DcmDataset *stDetailTmp = NULL;
        cond = DIMSE_storeUser(
          assoc, presId, &req, NULL /* imageFileName */,
          transcoded.GetParsed().getDataset(),
          GetSubOpProgressCallback, this /* callbackData */,
          (timeout_ > 0 ? DIMSE_NONBLOCKING : DIMSE_BLOCKING), timeout_,
          &rsp, &stDetailTmp, &cancelParameters);
        stDetail.reset(stDetailTmp);
      }
      else
      {
        // Cannot transcode
        nFailed_++;
        AddFailedUIDInstance(sopInstanceUid);
        LOG(ERROR) << "C-GET SCP: Cannot transcode " << sopClassUid
                   << " from transfer syntax " << GetTransferSyntaxUid(sourceSyntax)
                   << " to " << GetTransferSyntaxUid(selectedSyntax);
        return DIMSE_NOVALIDPRESENTATIONCONTEXTID;
      }      
    }
    
    if (cond.good())
    {
      if (cancelParameters.cancelEncountered)
      {
        if (origPresId_ == cancelParameters.presId &&
            origMsgId_ == cancelParameters.req.MessageIDBeingRespondedTo)
        {
          getCancelled_ = OFTrue;
        }
        else
        {
          LOG(ERROR) << "C-GET SCP: Unexpected C-Cancel-RQ encountered: pid=" << (int)cancelParameters.presId
                     << ", mid=" << (int)cancelParameters.req.MessageIDBeingRespondedTo;
        }
      }
      
      if (rsp.DimseStatus == STATUS_Success)
      {
        // everything ok
        nCompleted_++;
      }
      else if ((rsp.DimseStatus & 0xf000) == 0xb000)
      {
        // a warning status message
        warningCount_++;
        LOG(ERROR) << "C-GET SCP: Store Warning: Response Status: "
                   << DU_cstoreStatusString(rsp.DimseStatus);
      }
      else
      {
        nFailed_++;
        AddFailedUIDInstance(sopInstanceUid);
        // print a status message
        LOG(ERROR) << "C-GET SCP: Store Failed: Response Status: "
                   << DU_cstoreStatusString(rsp.DimseStatus);
      }
    }
    else
    {
      nFailed_++;
      AddFailedUIDInstance(sopInstanceUid);
      OFString temp_str;
      LOG(ERROR) << "C-GET SCP: storeSCU: Store Request Failed: " << DimseCondition::dump(temp_str, cond);
    }
    
    if (stDetail.get() != NULL)
    {
      // It is impossible to directly use the "<<" stream construct
      // with "DcmObject::PrintHelper" using MSVC2008
      std::stringstream s;
      DcmObject::PrintHelper obj(*stDetail);
      obj.dcmobj_.print(s);

      LOG(INFO) << "  Status Detail: " << s.str();
    }
    
    return cond;
  }

  bool OrthancGetRequestHandler::LookupIdentifiers(std::list<std::string>& publicIds,
                                                   ResourceType level,
                                                   const DicomMap& input) const
  {
    DicomTag tag(0, 0);   // Dummy initialization

    switch (level)
    {
      case ResourceType_Patient:
        tag = DICOM_TAG_PATIENT_ID;
        break;

      case ResourceType_Study:
        tag = (input.HasTag(DICOM_TAG_ACCESSION_NUMBER) ?
               DICOM_TAG_ACCESSION_NUMBER : DICOM_TAG_STUDY_INSTANCE_UID);
        break;
        
      case ResourceType_Series:
        tag = DICOM_TAG_SERIES_INSTANCE_UID;
        break;
        
      case ResourceType_Instance:
        tag = DICOM_TAG_SOP_INSTANCE_UID;
        break;

      default:
        throw OrthancException(ErrorCode_ParameterOutOfRange);
    }

    if (!input.HasTag(tag))
    {
      return false;
    }

    const DicomValue& value = input.GetValue(tag);
    if (value.IsNull() ||
        value.IsBinary())
    {
      return false;
    }
    else
    {
      std::vector<std::string> tokens;
      Toolbox::TokenizeString(tokens, value.GetContent(), '\\');

      for (size_t i = 0; i < tokens.size(); i++)
      {
        std::vector<std::string> tmp;
        context_.GetIndex().LookupIdentifierExact(tmp, level, tag, tokens[i]);

        if (tmp.empty())
        {
          LOG(ERROR) << "C-GET: Cannot locate resource \"" << tokens[i]
                     << "\" at the " << EnumerationToString(level) << " level";
          return false;
        }
        else
        {
          for (size_t i = 0; i < tmp.size(); i++)
          {
            publicIds.push_back(tmp[i]);
          }
        }
      }

      return true;      
    }
  }


    OrthancGetRequestHandler::OrthancGetRequestHandler(ServerContext& context) :
      context_(context)
    {
      position_ = 0;
      nRemaining_ = 0;
      nCompleted_  = 0;
      warningCount_ = 0;
      nFailed_ = 0;
      timeout_ = 0;
    }


  bool OrthancGetRequestHandler::Handle(const DicomMap& input,
                                        const std::string& originatorIp,
                                        const std::string& originatorAet,
                                        const std::string& calledAet,
                                        uint32_t timeout)
  {
    MetricsRegistry::Timer timer(context_.GetMetricsRegistry(), "orthanc_get_scp_duration_ms");

    LOG(WARNING) << "C-GET-SCU request received from AET \"" << originatorAet << "\"";

    {
      DicomArray query(input);
      for (size_t i = 0; i < query.GetSize(); i++)
      {
        if (!query.GetElement(i).GetValue().IsNull())
        {
          LOG(INFO) << "  " << query.GetElement(i).GetTag()
                    << "  " << FromDcmtkBridge::GetTagName(query.GetElement(i))
                    << " = " << query.GetElement(i).GetValue().GetContent();
        }
      }
    }

    /**
     * Retrieve the query level.
     **/

    const DicomValue* levelTmp = input.TestAndGetValue(DICOM_TAG_QUERY_RETRIEVE_LEVEL);

    assert(levelTmp != NULL);
    ResourceType level = StringToResourceType(levelTmp->GetContent().c_str());      


    /**
     * Lookup for the resource to be sent.
     **/

    std::list<std::string> publicIds;

    if (!LookupIdentifiers(publicIds, level, input))
    {
      LOG(ERROR) << "Cannot determine what resources are requested by C-GET";
      return false; 
    }

    localAet_ = context_.GetDefaultLocalApplicationEntityTitle();
    position_ = 0;
    originatorAet_ = originatorAet;
    
    {
      OrthancConfiguration::ReaderLock lock;
      remote_ = lock.GetConfiguration().GetModalityUsingAet(originatorAet);
    }

    for (std::list<std::string>::const_iterator
           resource = publicIds.begin(); resource != publicIds.end(); ++resource)
    {
      LOG(INFO) << "C-GET: Sending resource " << *resource
                << " to modality \"" << originatorAet << "\"";
      
      std::list<std::string> tmp;
      context_.GetIndex().GetChildInstances(tmp, *resource);
      
      instances_.reserve(tmp.size());
      for (std::list<std::string>::iterator it = tmp.begin(); it != tmp.end(); ++it)
      {
        instances_.push_back(*it);
      }
    }

    failedUIDs_.clear();
    getCancelled_ = OFFalse;

    nRemaining_ = GetSubOperationCount();
    nCompleted_ = 0;
    nFailed_ = 0;
    warningCount_ = 0;
    timeout_ = timeout;

    return true;
  }
};
