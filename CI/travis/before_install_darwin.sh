#!/bin/sh

brew update
brew install qt
echo 'export PATH="$(brew --prefix qt)/bin:$PATH"' >> ~/.bash_profile
brew link --force qt
which qmake

brew unlink bison
brew install cmake fftw bison autoconf automake libtool libzip glibmm doxygen python3 wget boost gnu-sed
brew install gcc llvm
brew link --overwrite --force gcc
brew link --overwrite --force bison
#echo 'export PATH="/usr/local/opt/llvm/bin:$PATH"' >> ~/.bash_profile
#echo 'export PATH="/usr/local/opt/bison/bin:$PATH"' >> ~/.bash_profile
brew upgrade cmake
brew upgrade bison
brew link --overwrite --force gettext

# Required for Python
export PYTHONPATH=$(brew --prefix)/lib/python2.7/site-packages
export PATH=$PATH:$(brew --prefix)/share/python
#export PATH="$PATH:/usr/local/opt/ccache/libexec"
mkdir -p deps
cd deps
WORKDIR=$PWD

# Get pip
curl https://bootstrap.pypa.io/get-pip.py > get-pip.py
sudo python get-pip.py
 
brew install brew-pip
# qt5 brew link --force qt5
sudo pip install --ignore-installed six
brew pip mako
brew pip cheetah

cd ${WORKDIR}
rm volk-1.3.tar.gz*
if [ ! -d volk-1.3 ]; then
  wget http://libvolk.org/releases/volk-1.3.tar.gz
  tar -xzf volk-1.3.tar.gz
  cd volk-1.3
  mkdir build && cd build
  cmake ..
  make >/dev/null
else
  cd volk-1.3/build
fi
sudo make install

cd ${WORKDIR}
if [ ! -d gnuradio ]; then
  git clone https://github.com/gnuradio/gnuradio
  cd gnuradio
  mkdir build
  cd build
  cmake -DENABLE_INTERNAL_VOLK:BOOL=OFF -DENABLE_GR_FEC:BOOL=OFF -DENABLE_GR_DIGITAL:BOOL=OFF -DENABLE_GR_DTV:BOOL=OFF -DENABLE_GR_ATSC:BOOL=OFF -DENABLE_GR_AUDIO:BOOL=OFF -DENABLE_GR_CHANNELS:BOOL=OFF -DENABLE_GR_NOAA:BOOL=OFF -DENABLE_GR_PAGER:BOOL=OFF -DENABLE_GR_TRELLIS:BOOL=OFF -DENABLE_GR_VOCODER:BOOL=OFF ..
  make -j4 >/dev/null
else
  cd gnuradio
  git pull
  cd build
fi
sudo make install >/dev/null

cd ${WORKDIR}
if [ ! -d libsigrok ]; then
  git clone https://github.com/sschnelle/libsigrok/
  cd libsigrok
  ./autogen.sh
  #CXXFLAGS=-D_DARWIN_C_SOURCE ./configure
  CC=gcc CXX=g++ CXXFLAGS=-std=c++11 ./configure --disable-static --enable-shared --disable-all-drivers --enable-bindings --enable-cxx
  make
else
  cd libsigrok
  make >/dev/null
fi
sudo make install

cd ${WORKDIR}
rm libsigrokdecode-0.4.1.tar.gz*
ls
if [ ! -d libsigrokdecode-0.4.1 ]; then
  wget http://sigrok.org/download/source/libsigrokdecode/libsigrokdecode-0.4.1.tar.gz
  tar -xzvf libsigrokdecode-0.4.1.tar.gz
  cd libsigrokdecode-0.4.1
  ./configure
  make >/dev/null
else
  cd libsigrokdecode-0.4.1
fi
sudo make install

cd ${WORKDIR}
if [ ! -d qwt ]; then
  git clone https://github.com/osakared/qwt.git -b qwt-6.1-multiaxes
  cd qwt
  curl https://raw.githubusercontent.com/analogdevicesinc/scopy/osx/qwt-6.1-multiaxes.patch |patch -p1 --forward
  #full path to qmake
  qmake
  make -j4 >/dev/null
else
  cd qwt
  make -j4 >/dev/null
fi
sudo make install

cd ${WORKDIR}
rm qwtpolar-1.1.1.tar.bz2*
ls
if [ ! -d qwtpolar-1.1.1 ]; then
  wget https://downloads.sourceforge.net/project/qwtpolar/qwtpolar/1.1.1/qwtpolar-1.1.1.tar.bz2
  tar xvjf qwtpolar-1.1.1.tar.bz2
  cd qwtpolar-1.1.1
  curl -o qwtpolar-qwt-6.1-compat.patch https://raw.githubusercontent.com/analogdevicesinc/scopy-flatpak/master/qwtpolar-qwt-6.1-compat.patch
  patch -p1 < qwtpolar-qwt-6.1-compat.patch

  # Disable components that we won't build
  gsed -i "/^QWT_POLAR_CONFIG\\s*+=\\s*QwtPolarExamples$/s/^/#/g" qwtpolarconfig.pri
  gsed -i "/^QWT_POLAR_CONFIG\\s*+=\\s*QwtPolarDesigner$/s/^/#/g" qwtpolarconfig.pri
  gsed -i "/QWT_POLAR_CONFIG\\s*+=\\s*QwtPolarFramework$/s/^/#/g" qwtpolarconfig.pri
  # Fix prefix
  gsed -i "s/^\\s*QWT_POLAR_INSTALL_PREFIX.*$/QWT_POLAR_INSTALL_PREFIX=\/usr\/local/g" qwtpolarconfig.pri
  gsed -i "/^QWT_POLAR_INSTALL_HEADERS/s/$/\/qwt/g" qwtpolarconfig.pri
  cat qwtpolarconfig.pri
  qmake LIBS+="-L/usr/local/lib -lqwt" INCLUDEPATH+="/usr/local/include/qwt" qwtpolar.pro
  make >/dev/null
else
  cd qwtpolar-1.1.1
fi
sudo make install
locate libqwtpolar.dylib

cd ${WORKDIR}
wget http://swdownloads.analog.com/cse/travis_builds/latest_libiio${LDIST}.pkg
sudo installer -pkg latest_libiio${LDIST}.pkg -target /

cd ${WORKDIR}
if [ ! -d libad9361-iio ]; then
  git clone https://github.com/analogdevicesinc/libad9361-iio
  cd libad9361-iio
  mkdir build 
  cd build
  cmake ..
  make
else
  cd libad9361-iio
  git pull
  cd build
fi
sudo make install

cd ${WORKDIR}
rm -rf gr-iio
if [ ! -d gr-iio ]; then
  git clone https://github.com/analogdevicesinc/gr-iio 
  cd gr-iio
  mkdir build
  cd build
  cmake ..
  make -j4
else
  cd gr-iio
  git pull
  cd build
  make -j4
fi
sudo make install






if false
then
if ! brew ls --version cmake &>/dev/null; then
	brew install cmake
else
	echo "FOUND cmake"
	brew upgrade cmake
fi

fi
