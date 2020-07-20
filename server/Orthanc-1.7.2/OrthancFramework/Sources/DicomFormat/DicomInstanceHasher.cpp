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
#include "DicomInstanceHasher.h"

#include "../OrthancException.h"
#include "../Toolbox.h"

namespace Orthanc
{
  void DicomInstanceHasher::Setup(const std::string& patientId,
                                  const std::string& studyUid,
                                  const std::string& seriesUid,
                                  const std::string& instanceUid)
  {
    patientId_ = patientId;
    studyUid_ = studyUid;
    seriesUid_ = seriesUid;
    instanceUid_ = instanceUid;

    if (studyUid_.size() == 0 ||
        seriesUid_.size() == 0 ||
        instanceUid_.size() == 0)
    {
      throw OrthancException(ErrorCode_BadFileFormat, "missing StudyInstanceUID, SeriesInstanceUID or SOPInstanceUID");
    }
  }

  DicomInstanceHasher::DicomInstanceHasher(const DicomMap& instance)
  {
    const DicomValue* patientId = instance.TestAndGetValue(DICOM_TAG_PATIENT_ID);

    Setup(patientId == NULL ? "" : patientId->GetContent(),
          instance.GetValue(DICOM_TAG_STUDY_INSTANCE_UID).GetContent(),
          instance.GetValue(DICOM_TAG_SERIES_INSTANCE_UID).GetContent(),
          instance.GetValue(DICOM_TAG_SOP_INSTANCE_UID).GetContent());
  }

  const std::string& DicomInstanceHasher::HashPatient()
  {
    if (patientHash_.size() == 0)
    {
      Toolbox::ComputeSHA1(patientHash_, patientId_);
    }

    return patientHash_;
  }

  const std::string& DicomInstanceHasher::HashStudy()
  {
    if (studyHash_.size() == 0)
    {
      Toolbox::ComputeSHA1(studyHash_, patientId_ + "|" + studyUid_);
    }

    return studyHash_;
  }

  const std::string& DicomInstanceHasher::HashSeries()
  {
    if (seriesHash_.size() == 0)
    {
      Toolbox::ComputeSHA1(seriesHash_, patientId_ + "|" + studyUid_ + "|" + seriesUid_);
    }

    return seriesHash_;
  }

  const std::string& DicomInstanceHasher::HashInstance()
  {
    if (instanceHash_.size() == 0)
    {
      Toolbox::ComputeSHA1(instanceHash_, patientId_ + "|" + studyUid_ + "|" + seriesUid_ + "|" + instanceUid_);
    }

    return instanceHash_;
  }
}
