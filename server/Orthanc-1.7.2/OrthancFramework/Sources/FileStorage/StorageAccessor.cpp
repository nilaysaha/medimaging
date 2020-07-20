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
#include "StorageAccessor.h"

#include "../Compatibility.h"
#include "../Compression/ZlibCompressor.h"
#include "../MetricsRegistry.h"
#include "../OrthancException.h"
#include "../Toolbox.h"

#if ORTHANC_ENABLE_CIVETWEB == 1 || ORTHANC_ENABLE_MONGOOSE == 1
#  include "../HttpServer/HttpStreamTranscoder.h"
#endif


static const std::string METRICS_CREATE = "orthanc_storage_create_duration_ms";
static const std::string METRICS_READ = "orthanc_storage_read_duration_ms";
static const std::string METRICS_REMOVE = "orthanc_storage_remove_duration_ms";


namespace Orthanc
{
  class StorageAccessor::MetricsTimer : public boost::noncopyable
  {
  private:
    std::unique_ptr<MetricsRegistry::Timer>  timer_;

  public:
    MetricsTimer(StorageAccessor& that,
                 const std::string& name)
    {
      if (that.metrics_ != NULL)
      {
        timer_.reset(new MetricsRegistry::Timer(*that.metrics_, name));
      }
    }
  };


  FileInfo StorageAccessor::Write(const void* data,
                                  size_t size,
                                  FileContentType type,
                                  CompressionType compression,
                                  bool storeMd5)
  {
    std::string uuid = Toolbox::GenerateUuid();

    std::string md5;

    if (storeMd5)
    {
      Toolbox::ComputeMD5(md5, data, size);
    }

    switch (compression)
    {
      case CompressionType_None:
      {
        MetricsTimer timer(*this, METRICS_CREATE);

        area_.Create(uuid, data, size, type);
        return FileInfo(uuid, type, size, md5);
      }

      case CompressionType_ZlibWithSize:
      {
        ZlibCompressor zlib;

        std::string compressed;
        zlib.Compress(compressed, data, size);

        std::string compressedMD5;
      
        if (storeMd5)
        {
          Toolbox::ComputeMD5(compressedMD5, compressed);
        }

        {
          MetricsTimer timer(*this, METRICS_CREATE);

          if (compressed.size() > 0)
          {
            area_.Create(uuid, &compressed[0], compressed.size(), type);
          }
          else
          {
            area_.Create(uuid, NULL, 0, type);
          }
        }

        return FileInfo(uuid, type, size, md5,
                        CompressionType_ZlibWithSize, compressed.size(), compressedMD5);
      }

      default:
        throw OrthancException(ErrorCode_NotImplemented);
    }
  }


  void StorageAccessor::Read(std::string& content,
                             const FileInfo& info)
  {
    switch (info.GetCompressionType())
    {
      case CompressionType_None:
      {
        MetricsTimer timer(*this, METRICS_READ);
        area_.Read(content, info.GetUuid(), info.GetContentType());
        break;
      }

      case CompressionType_ZlibWithSize:
      {
        ZlibCompressor zlib;

        std::string compressed;

        {
          MetricsTimer timer(*this, METRICS_READ);
          area_.Read(compressed, info.GetUuid(), info.GetContentType());
        }

        IBufferCompressor::Uncompress(content, zlib, compressed);
        break;
      }

      default:
      {
        throw OrthancException(ErrorCode_NotImplemented);
      }
    }

    // TODO Check the validity of the uncompressed MD5?
  }


  void StorageAccessor::ReadRaw(std::string& content,
                                const FileInfo& info)
  {
    MetricsTimer timer(*this, METRICS_READ);
    area_.Read(content, info.GetUuid(), info.GetContentType());
  }


  void StorageAccessor::Remove(const std::string& fileUuid,
                               FileContentType type)
  {
    MetricsTimer timer(*this, METRICS_REMOVE);
    area_.Remove(fileUuid, type);
  }


#if ORTHANC_ENABLE_CIVETWEB == 1 || ORTHANC_ENABLE_MONGOOSE == 1
  void StorageAccessor::SetupSender(BufferHttpSender& sender,
                                    const FileInfo& info,
                                    const std::string& mime)
  {
    {
      MetricsTimer timer(*this, METRICS_READ);
      area_.Read(sender.GetBuffer(), info.GetUuid(), info.GetContentType());
    }

    sender.SetContentType(mime);

    const char* extension;
    switch (info.GetContentType())
    {
      case FileContentType_Dicom:
        extension = ".dcm";
        break;

      case FileContentType_DicomAsJson:
        extension = ".json";
        break;

      default:
        // Non-standard content type
        extension = "";
    }

    sender.SetContentFilename(info.GetUuid() + std::string(extension));
  }
#endif


#if ORTHANC_ENABLE_CIVETWEB == 1 || ORTHANC_ENABLE_MONGOOSE == 1
  void StorageAccessor::AnswerFile(HttpOutput& output,
                                   const FileInfo& info,
                                   const std::string& mime)
  {
    BufferHttpSender sender;
    SetupSender(sender, info, mime);
  
    HttpStreamTranscoder transcoder(sender, info.GetCompressionType());
    output.Answer(transcoder);
  }
#endif


#if ORTHANC_ENABLE_CIVETWEB == 1 || ORTHANC_ENABLE_MONGOOSE == 1
  void StorageAccessor::AnswerFile(RestApiOutput& output,
                                   const FileInfo& info,
                                   const std::string& mime)
  {
    BufferHttpSender sender;
    SetupSender(sender, info, mime);
  
    HttpStreamTranscoder transcoder(sender, info.GetCompressionType());
    output.AnswerStream(transcoder);
  }
#endif
}
