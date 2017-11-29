#!/bin/sh

cd ${TRAVIS_BUILD_DIR}/build
cmake -DCMAKE_PREFIX_PATH=/usr/local/opt/qt/lib/cmake ..
make -j4
otool -L Scopy.app/Contents/MacOS/Scopy

cp /Library/Frameworks/iio.framework Scopy.app/Contents/Frameworks/
cp /usr/local/lib/libqwtpolar.dylib Scopy.app/Contents/Frameworks/
install_name_tool -change libqwtpolar.dylib @executable_path/../Frameworks/libqwtpolar.dylib Scopy.app/Contents/MacOS/Scopy
install_name_tool -change /Library/Frameworks/iio.framework @executable_path/../Frameworks/iio.framework Scopy.app/Contents/MacOS/Scopy

/usr/local/opt/qt/bin/macdeployqt Scopy.app -verbose=3 -dmg
otool -L Scopy.app/Contents/MacOS/Scopy

curl --upload-file ./Scopy.dmg https://transfer.sh/scopy-v0.9.dmg


