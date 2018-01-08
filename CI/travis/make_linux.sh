#!/bin/sh

cd ${TRAVIS_BUILD_DIR}/build
mkdir -p appdir/usr/bin/decoders
mkdir -p appdir/usr/lib/python3.6
mkdir -p appdir/usr/lib/python2.7

#sudo mv /usr/bin/python3 /usr/bin/python3-old
sudo apt-get remove --auto-remove python3.4
#sudo ln -s /usr/bin/python3.5 /usr/bin/python3
cp /usr/lib/python3.6/* appdir/usr/lib/python3.6/
cp /usr/lib/python2.7/* appdir/usr/lib/python2.7/
cp ${TRAVIS_BUILD_DIR}/resources/decoders/* appdir/usr/bin/decoders/

echo "import sys,os
prefix = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(sys.path[0]))))
sys.path = [ prefix+s for s in sys.path if not s.startswith(prefix) ]" > appdir/usr/lib/python3.6/sitecustomize.py

echo "import sys,os
prefix = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(sys.path[0]))))
sys.path = [ prefix+s for s in sys.path if not s.startswith(prefix) ]" > appdir/usr/lib/python2.7/sitecustomize.py

find . | grep python
export PYTHONPATH=${TRAVIS_BUILD_DIR}/build/appdir/usr/lib/python3.6

ls -la /usr/bin | grep python
python3 --version
cmake -DCMAKE_PREFIX_PATH=/opt/qt59/lib/cmake -DCMAKE_INSTALL_PREFIX=appdir/usr ..
make -j4
ldd scopy
make -j4 install ; find appdir/

