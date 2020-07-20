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


#include "../PrecompiledHeaders.h"
#include "DicomStoreUserConnection.h"

#include "../DicomParsing/FromDcmtkBridge.h"
#include "../DicomParsing/ParsedDicomFile.h"
#include "../Logging.h"
#include "../OrthancException.h"
#include "DicomAssociation.h"

#include <dcmtk/dcmdata/dcdeftag.h>


namespace Orthanc
{
  bool DicomStoreUserConnection::ProposeStorageClass(const std::string& sopClassUid,
                                                     const std::set<DicomTransferSyntax>& syntaxes)
  {
    // Default transfer syntax for DICOM
    const bool addLittleEndianImplicit = (
      proposeUncompressedSyntaxes_ &&
      syntaxes.find(DicomTransferSyntax_LittleEndianImplicit) == syntaxes.end());
    
    const bool addLittleEndianExplicit = (
      proposeUncompressedSyntaxes_ &&
      syntaxes.find(DicomTransferSyntax_LittleEndianExplicit) == syntaxes.end());
    
    const bool addBigEndianExplicit = (
      proposeUncompressedSyntaxes_ &&
      proposeRetiredBigEndian_ &&
      syntaxes.find(DicomTransferSyntax_BigEndianExplicit) == syntaxes.end());
    
    size_t requiredCount = syntaxes.size();
    if (addLittleEndianImplicit)
    {
      requiredCount += 1;
    }
      
    if (addLittleEndianExplicit ||
        addBigEndianExplicit)
    {
      requiredCount += 1;
    }
      
    if (association_->GetRemainingPropositions() <= requiredCount)
    {
      return false;  // Not enough room
    }
    else
    {
      for (std::set<DicomTransferSyntax>::const_iterator
             it = syntaxes.begin(); it != syntaxes.end(); ++it)
      {
        association_->ProposePresentationContext(sopClassUid, *it);
        proposedOriginalClasses_.insert(std::make_pair(sopClassUid, *it));
      }

      if (addLittleEndianImplicit)
      {
        association_->ProposePresentationContext(sopClassUid, DicomTransferSyntax_LittleEndianImplicit);
        proposedOriginalClasses_.insert(std::make_pair(sopClassUid, DicomTransferSyntax_LittleEndianImplicit));
      }

      if (addLittleEndianExplicit ||
          addBigEndianExplicit)
      {
        std::set<DicomTransferSyntax> uncompressed;

        if (addLittleEndianExplicit)
        {
          uncompressed.insert(DicomTransferSyntax_LittleEndianExplicit);
        }

        if (addBigEndianExplicit)
        {
          uncompressed.insert(DicomTransferSyntax_BigEndianExplicit);
        }

        association_->ProposePresentationContext(sopClassUid, uncompressed);

        assert(!uncompressed.empty());
        if (addLittleEndianExplicit ^ addBigEndianExplicit)
        {
          // Only one transfer syntax was proposed for this presentation context
          assert(uncompressed.size() == 1);
          proposedOriginalClasses_.insert(std::make_pair(sopClassUid, *uncompressed.begin()));
        }
      }

      return true;
    }
  }


  bool DicomStoreUserConnection::LookupPresentationContext(
    uint8_t& presentationContextId,
    const std::string& sopClassUid,
    DicomTransferSyntax transferSyntax)
  {
    typedef std::map<DicomTransferSyntax, uint8_t>  PresentationContexts;

    PresentationContexts pc;
    if (association_->IsOpen() &&
        association_->LookupAcceptedPresentationContext(pc, sopClassUid))
    {
      PresentationContexts::const_iterator found = pc.find(transferSyntax);
      if (found != pc.end())
      {
        presentationContextId = found->second;
        return true;
      }
    }

    return false;
  }


  DicomStoreUserConnection::DicomStoreUserConnection(
    const DicomAssociationParameters& params) :
    parameters_(params),
    association_(new DicomAssociation),
    proposeCommonClasses_(true),
    proposeUncompressedSyntaxes_(true),
    proposeRetiredBigEndian_(false)
  {
  }
    

  void DicomStoreUserConnection::RegisterStorageClass(const std::string& sopClassUid,
                                                      DicomTransferSyntax syntax)
  {
    RegisteredClasses::iterator found = registeredClasses_.find(sopClassUid);

    if (found == registeredClasses_.end())
    {
      std::set<DicomTransferSyntax> ts;
      ts.insert(syntax);
      registeredClasses_[sopClassUid] = ts;
    }
    else
    {
      found->second.insert(syntax);
    }
  }


  void DicomStoreUserConnection::LookupParameters(std::string& sopClassUid,
                                                  std::string& sopInstanceUid,
                                                  DicomTransferSyntax& transferSyntax,
                                                  DcmFileFormat& dicom)
  {
    if (dicom.getDataset() == NULL)
    {
      throw OrthancException(ErrorCode_InternalError);
    }
    
    OFString a, b;
    if (!dicom.getDataset()->findAndGetOFString(DCM_SOPClassUID, a).good() ||
        !dicom.getDataset()->findAndGetOFString(DCM_SOPInstanceUID, b).good())
    {
      throw OrthancException(ErrorCode_NoSopClassOrInstance,
                             "Unable to determine the SOP class/instance for C-STORE with AET " +
                             parameters_.GetRemoteModality().GetApplicationEntityTitle());
    }

    sopClassUid.assign(a.c_str());
    sopInstanceUid.assign(b.c_str());

    if (!FromDcmtkBridge::LookupOrthancTransferSyntax(transferSyntax, dicom))
    {
      throw OrthancException(ErrorCode_InternalError,
                             "Unknown transfer syntax from DCMTK");
    }
  }
  

  bool DicomStoreUserConnection::NegotiatePresentationContext(
    uint8_t& presentationContextId,
    const std::string& sopClassUid,
    DicomTransferSyntax transferSyntax)
  {
    /**
     * Step 1: Check whether this presentation context is already
     * available in the previously negotiated assocation.
     **/

    if (LookupPresentationContext(presentationContextId, sopClassUid, transferSyntax))
    {
      return true;
    }

    // The association must be re-negotiated
    if (association_->IsOpen())
    {
      LOG(INFO) << "Re-negotiating DICOM association with "
                << parameters_.GetRemoteModality().GetApplicationEntityTitle();

      if (proposedOriginalClasses_.find(std::make_pair(sopClassUid, transferSyntax)) !=
          proposedOriginalClasses_.end())
      {
        LOG(INFO) << "The remote modality has already rejected SOP class UID \""
                  << sopClassUid << "\" with transfer syntax \""
                  << GetTransferSyntaxUid(transferSyntax) << "\", don't renegotiate";
        return false;
      }
    }

    association_->ClearPresentationContexts();
    proposedOriginalClasses_.clear();
    RegisterStorageClass(sopClassUid, transferSyntax);  // (*)

    
    /**
     * Step 2: Propose at least the mandatory SOP class.
     **/

    {
      RegisteredClasses::const_iterator mandatory = registeredClasses_.find(sopClassUid);

      if (mandatory == registeredClasses_.end() ||
          mandatory->second.find(transferSyntax) == mandatory->second.end())
      {
        // Should never fail because of (*)
        throw OrthancException(ErrorCode_InternalError);
      }

      if (!ProposeStorageClass(sopClassUid, mandatory->second))
      {
        // Should never happen in real life: There are no more than
        // 128 transfer syntaxes in DICOM!
        throw OrthancException(ErrorCode_InternalError,
                               "Too many transfer syntaxes for SOP class UID: " + sopClassUid);
      }
    }

      
    /**
     * Step 3: Propose all the previously spotted SOP classes, as
     * registered through the "RegisterStorageClass()" method.
     **/
      
    for (RegisteredClasses::const_iterator it = registeredClasses_.begin();
         it != registeredClasses_.end(); ++it)
    {
      if (it->first != sopClassUid)
      {
        ProposeStorageClass(it->first, it->second);
      }
    }
      

    /**
     * Step 4: As long as there is room left in the proposed
     * presentation contexts, propose the uncompressed transfer syntaxes
     * for the most common SOP classes, as can be found in the
     * "dcmShortSCUStorageSOPClassUIDs" array from DCMTK. The
     * preferred transfer syntax is "LittleEndianImplicit".
     **/

    if (proposeCommonClasses_)
    {
      // The method "ProposeStorageClass()" will automatically add
      // "LittleEndianImplicit"
      std::set<DicomTransferSyntax> ts;
        
      for (int i = 0; i < numberOfDcmShortSCUStorageSOPClassUIDs; i++)
      {
        std::string c(dcmShortSCUStorageSOPClassUIDs[i]);
          
        if (c != sopClassUid &&
            registeredClasses_.find(c) == registeredClasses_.end())
        {
          ProposeStorageClass(c, ts);
        }
      }
    }


    /**
     * Step 5: Open the association, and check whether the pair (SOP
     * class UID, transfer syntax) was accepted by the remote host.
     **/

    association_->Open(parameters_);
    return LookupPresentationContext(presentationContextId, sopClassUid, transferSyntax);
  }


  void DicomStoreUserConnection::Store(std::string& sopClassUid,
                                       std::string& sopInstanceUid,
                                       DcmFileFormat& dicom,
                                       bool hasMoveOriginator,
                                       const std::string& moveOriginatorAET,
                                       uint16_t moveOriginatorID)
  {
    DicomTransferSyntax transferSyntax;
    LookupParameters(sopClassUid, sopInstanceUid, transferSyntax, dicom);

    uint8_t presID;
    if (!NegotiatePresentationContext(presID, sopClassUid, transferSyntax))
    {
      throw OrthancException(ErrorCode_NetworkProtocol,
                             "No valid presentation context was negotiated for "
                             "SOP class UID [" + sopClassUid + "] and transfer "
                             "syntax [" + GetTransferSyntaxUid(transferSyntax) + "] "
                             "while sending to modality [" +
                             parameters_.GetRemoteModality().GetApplicationEntityTitle() + "]");
    }
    
    // Prepare the transmission of data
    T_DIMSE_C_StoreRQ request;
    memset(&request, 0, sizeof(request));
    request.MessageID = association_->GetDcmtkAssociation().nextMsgID++;
    strncpy(request.AffectedSOPClassUID, sopClassUid.c_str(), DIC_UI_LEN);
    request.Priority = DIMSE_PRIORITY_MEDIUM;
    request.DataSetType = DIMSE_DATASET_PRESENT;
    strncpy(request.AffectedSOPInstanceUID, sopInstanceUid.c_str(), DIC_UI_LEN);

    if (hasMoveOriginator)
    {    
      strncpy(request.MoveOriginatorApplicationEntityTitle, 
              moveOriginatorAET.c_str(), DIC_AE_LEN);
      request.opts = O_STORE_MOVEORIGINATORAETITLE;

      request.MoveOriginatorID = moveOriginatorID;  // The type DIC_US is an alias for uint16_t
      request.opts |= O_STORE_MOVEORIGINATORID;
    }

    if (dicom.getDataset() == NULL)
    {
      throw OrthancException(ErrorCode_InternalError);
    }

    // Finally conduct transmission of data
    T_DIMSE_C_StoreRSP response;
    DcmDataset* statusDetail = NULL;
    DicomAssociation::CheckCondition(
      DIMSE_storeUser(&association_->GetDcmtkAssociation(), presID, &request,
                      NULL, dicom.getDataset(), /*progressCallback*/ NULL, NULL,
                      /*opt_blockMode*/ (GetParameters().HasTimeout() ? DIMSE_NONBLOCKING : DIMSE_BLOCKING),
                      /*opt_dimse_timeout*/ GetParameters().GetTimeout(),
                      &response, &statusDetail, NULL),
      GetParameters(), "C-STORE");

    if (statusDetail != NULL) 
    {
      delete statusDetail;
    }
    
    /**
     * New in Orthanc 1.6.0: Deal with failures during C-STORE.
     * http://dicom.nema.org/medical/dicom/current/output/chtml/part04/sect_B.2.3.html#table_B.2-1
     **/
    
    if (response.DimseStatus != 0x0000 &&  // Success
        response.DimseStatus != 0xB000 &&  // Warning - Coercion of Data Elements
        response.DimseStatus != 0xB007 &&  // Warning - Data Set does not match SOP Class
        response.DimseStatus != 0xB006)    // Warning - Elements Discarded
    {
      char buf[16];
      sprintf(buf, "%04X", response.DimseStatus);
      throw OrthancException(ErrorCode_NetworkProtocol,
                             "C-STORE SCU to AET \"" +
                             GetParameters().GetRemoteModality().GetApplicationEntityTitle() +
                             "\" has failed with DIMSE status 0x" + buf);
    }
  }


  void DicomStoreUserConnection::Store(std::string& sopClassUid,
                                       std::string& sopInstanceUid,
                                       const void* buffer,
                                       size_t size,
                                       bool hasMoveOriginator,
                                       const std::string& moveOriginatorAET,
                                       uint16_t moveOriginatorID)
  {
    std::unique_ptr<DcmFileFormat> dicom(
      FromDcmtkBridge::LoadFromMemoryBuffer(buffer, size));

    if (dicom.get() == NULL)
    {
      throw OrthancException(ErrorCode_InternalError);
    }
    
    Store(sopClassUid, sopInstanceUid, *dicom, hasMoveOriginator, moveOriginatorAET, moveOriginatorID);
  }


  void DicomStoreUserConnection::LookupTranscoding(std::set<DicomTransferSyntax>& acceptedSyntaxes,
                                                   const std::string& sopClassUid,
                                                   DicomTransferSyntax sourceSyntax)
  {
    acceptedSyntaxes.clear();

    // Make sure a negotiation has already occurred for this transfer
    // syntax. We don't use the return code: Transcoding is possible
    // even if the "sourceSyntax" is not supported.
    uint8_t presID;
    NegotiatePresentationContext(presID, sopClassUid, sourceSyntax);

    std::map<DicomTransferSyntax, uint8_t> contexts;
    if (association_->LookupAcceptedPresentationContext(contexts, sopClassUid))
    {
      for (std::map<DicomTransferSyntax, uint8_t>::const_iterator
             it = contexts.begin(); it != contexts.end(); ++it)
      {
        acceptedSyntaxes.insert(it->first);
      }
    }
  }


  void DicomStoreUserConnection::Transcode(std::string& sopClassUid /* out */,
                                           std::string& sopInstanceUid /* out */,
                                           IDicomTranscoder& transcoder,
                                           const void* buffer,
                                           size_t size,
                                           bool hasMoveOriginator,
                                           const std::string& moveOriginatorAET,
                                           uint16_t moveOriginatorID)
  {
    std::unique_ptr<DcmFileFormat> dicom(FromDcmtkBridge::LoadFromMemoryBuffer(buffer, size));
    if (dicom.get() == NULL ||
        dicom->getDataset() == NULL)
    {
      throw OrthancException(ErrorCode_NullPointer);
    }

    DicomTransferSyntax inputSyntax;
    LookupParameters(sopClassUid, sopInstanceUid, inputSyntax, *dicom);

    std::set<DicomTransferSyntax> accepted;
    LookupTranscoding(accepted, sopClassUid, inputSyntax);

    if (accepted.find(inputSyntax) != accepted.end())
    {
      // No need for transcoding
      Store(sopClassUid, sopInstanceUid, *dicom,
            hasMoveOriginator, moveOriginatorAET, moveOriginatorID);
    }
    else
    {
      // Transcoding is needed
      std::set<DicomTransferSyntax> uncompressedSyntaxes;

      if (accepted.find(DicomTransferSyntax_LittleEndianImplicit) != accepted.end())
      {
        uncompressedSyntaxes.insert(DicomTransferSyntax_LittleEndianImplicit);
      }

      if (accepted.find(DicomTransferSyntax_LittleEndianExplicit) != accepted.end())
      {
        uncompressedSyntaxes.insert(DicomTransferSyntax_LittleEndianExplicit);
      }

      if (accepted.find(DicomTransferSyntax_BigEndianExplicit) != accepted.end())
      {
        uncompressedSyntaxes.insert(DicomTransferSyntax_BigEndianExplicit);
      }

      IDicomTranscoder::DicomImage source;
      source.AcquireParsed(dicom.release());
      source.SetExternalBuffer(buffer, size);

      const std::string sourceUid = IDicomTranscoder::GetSopInstanceUid(source.GetParsed());
      
      IDicomTranscoder::DicomImage transcoded;
      if (transcoder.Transcode(transcoded, source, uncompressedSyntaxes, false))
      {
        if (sourceUid != IDicomTranscoder::GetSopInstanceUid(transcoded.GetParsed()))
        {
          throw OrthancException(ErrorCode_Plugin, "The transcoder has changed the SOP "
                                 "instance UID while transcoding to an uncompressed transfer syntax");
        }
        else
        {
          DicomTransferSyntax transcodedSyntax;
          
          // Sanity check
          if (!FromDcmtkBridge::LookupOrthancTransferSyntax(transcodedSyntax, transcoded.GetParsed()) ||
              accepted.find(transcodedSyntax) == accepted.end())
          {
            throw OrthancException(ErrorCode_InternalError);
          }
          else
          {
            Store(sopClassUid, sopInstanceUid, transcoded.GetParsed(),
                  hasMoveOriginator, moveOriginatorAET, moveOriginatorID);
          }
        }
      }
    }
  }
}
