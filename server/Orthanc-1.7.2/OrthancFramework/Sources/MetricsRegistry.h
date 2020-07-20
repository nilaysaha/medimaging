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

#include "OrthancFramework.h"

#if !defined(ORTHANC_SANDBOXED)
#  error The macro ORTHANC_SANDBOXED must be defined
#endif

#if ORTHANC_SANDBOXED == 1
#  error The class MetricsRegistry cannot be used in sandboxed environments
#endif

#include <boost/thread/mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace Orthanc
{
  enum MetricsType
  {
    MetricsType_Default,
    MetricsType_MaxOver10Seconds,
    MetricsType_MaxOver1Minute,
    MetricsType_MinOver10Seconds,
    MetricsType_MinOver1Minute
  };
  
  class ORTHANC_PUBLIC MetricsRegistry : public boost::noncopyable
  {
  private:
    class Item;

    typedef std::map<std::string, Item*>   Content;

    bool          enabled_;
    boost::mutex  mutex_;
    Content       content_;

    void SetValueInternal(const std::string& name,
                          float value,
                          MetricsType type);

  public:
    MetricsRegistry() :
      enabled_(true)
    {
    }

    ~MetricsRegistry();

    bool IsEnabled() const
    {
      return enabled_;
    }

    void SetEnabled(bool enabled);

    void Register(const std::string& name,
                  MetricsType type);

    void SetValue(const std::string& name,
                  float value,
                  MetricsType type)
    {
      // Inlining to avoid loosing time if metrics are disabled
      if (enabled_)
      {
        SetValueInternal(name, value, type);
      }
    }
    
    void SetValue(const std::string& name,
                  float value)
    {
      SetValue(name, value, MetricsType_Default);
    }

    MetricsType GetMetricsType(const std::string& name);

    // https://prometheus.io/docs/instrumenting/exposition_formats/#text-based-format
    void ExportPrometheusText(std::string& s);


    class ORTHANC_PUBLIC SharedMetrics : public boost::noncopyable
    {
    private:
      boost::mutex      mutex_;
      MetricsRegistry&  registry_;
      std::string       name_;
      float             value_;

    public:
      SharedMetrics(MetricsRegistry& registry,
                    const std::string& name,
                    MetricsType type) :
        registry_(registry),
        name_(name),
        value_(0)
      {
      }

      void Add(float delta);
    };


    class ORTHANC_PUBLIC ActiveCounter : public boost::noncopyable
    {
    private:
      SharedMetrics&   metrics_;

    public:
      ActiveCounter(SharedMetrics& metrics) :
        metrics_(metrics)
      {
        metrics_.Add(1);
      }

      ~ActiveCounter()
      {
        metrics_.Add(-1);
      }
    };


    class ORTHANC_PUBLIC Timer : public boost::noncopyable
    {
    private:
      MetricsRegistry&          registry_;
      std::string               name_;
      MetricsType               type_;
      bool                      active_;
      boost::posix_time::ptime  start_;

      void Start();

    public:
      Timer(MetricsRegistry& registry,
            const std::string& name) :
        registry_(registry),
        name_(name),
        type_(MetricsType_MaxOver10Seconds)
      {
        Start();
      }

      Timer(MetricsRegistry& registry,
            const std::string& name,
            MetricsType type) :
        registry_(registry),
        name_(name),
        type_(type)
      {
        Start();
      }

      ~Timer();
    };
  };
}
