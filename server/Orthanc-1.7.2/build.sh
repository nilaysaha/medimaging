
sudo apt-get install build-essential unzip cmake mercurial \
    libdpkg-perl \
    libsqlite3-dev \
    uuid-dev libcurl4-openssl-dev \
    libgtest-dev libpng-dev libsqlite3-dev libssl-dev libjpeg-dev \
    zlib1g-dev libdcmtk2-dev libboost-all-dev libwrap0-dev \
    libcharls-dev libjsoncpp-dev libpugixml-dev 


BUILDDIR="$PWD/Build"

if [ -d "${BUILDDIR}" ]; then
  # Control will enter here if $DIRECTORY exists.
    cd ${BUILDDIR}    
else
    mkdir -p ${BUILDDIR}
    cd ${BUILDDIR}
fi

export LC_ALL="en_US.UTF-8"

cmake -DALLOW_DOWNLOADS=ON \
    -DUSE_GOOGLE_TEST_DEBIAN_PACKAGE=ON \
    -DUSE_SYSTEM_CIVETWEB=OFF \
    -DUSE_SYSTEM_JSONCPP=OFF \
    -DUSE_SYSTEM_LUA=OFF \
    -DDCMTK_LIBRARIES=dcmjpls \
    -DCMAKE_BUILD_TYPE=Release \
    ../OrthancServer/
make
