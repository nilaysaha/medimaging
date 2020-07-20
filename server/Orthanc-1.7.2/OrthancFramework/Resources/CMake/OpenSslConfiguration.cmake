# Orthanc - A Lightweight, RESTful DICOM Store
# Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
# Department, University Hospital of Liege, Belgium
# Copyright (C) 2017-2020 Osimis S.A., Belgium
#
# This program is free software: you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program. If not, see
# <http://www.gnu.org/licenses/>.


if (STATIC_BUILD OR NOT USE_SYSTEM_OPENSSL)
  if (OPENSSL_STATIC_VERSION STREQUAL "1.0.2")
    include(${CMAKE_CURRENT_LIST_DIR}/OpenSslConfigurationStatic-1.0.2.cmake)
  elseif (OPENSSL_STATIC_VERSION STREQUAL "1.1.1")
    include(${CMAKE_CURRENT_LIST_DIR}/OpenSslConfigurationStatic-1.1.1.cmake)
  else()
    message(FATAL_ERROR "Unsupported version of OpenSSL: ${OPENSSL_STATIC_VERSION}")
  endif()

  source_group(ThirdParty\\OpenSSL REGULAR_EXPRESSION ${OPENSSL_SOURCES_DIR}/.*)

elseif (CMAKE_CROSSCOMPILING AND
    "${CMAKE_SYSTEM_VERSION}" STREQUAL "CrossToolNg")

  CHECK_INCLUDE_FILE_CXX(openssl/opensslv.h HAVE_OPENSSL_H)
  if (NOT HAVE_OPENSSL_H)
    message(FATAL_ERROR "Please install the libopenssl-dev package")
  endif()

  CHECK_LIBRARY_EXISTS(crypto "OPENSSL_init" "" HAVE_OPENSSL_CRYPTO_LIB)
  if (NOT HAVE_OPENSSL_CRYPTO_LIB)
    message(FATAL_ERROR "Please install the libopenssl package")
  endif()  
  
  CHECK_LIBRARY_EXISTS(ssl "SSL_library_init" "" HAVE_OPENSSL_SSL_LIB)
  if (NOT HAVE_OPENSSL_SSL_LIB)
    message(FATAL_ERROR "Please install the libopenssl package")
  endif()  
  
  link_libraries(crypto ssl)

else()
  include(FindOpenSSL)

  if (NOT ${OPENSSL_FOUND})
    message(FATAL_ERROR "Unable to find OpenSSL")
  endif()

  include_directories(${OPENSSL_INCLUDE_DIR})
  link_libraries(${OPENSSL_LIBRARIES})
endif()
