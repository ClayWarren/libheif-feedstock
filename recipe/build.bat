mkdir build
cd build

cmake -G "Ninja" ^
  -DCMAKE_INSTALL_PREFIX="%LIBRARY_PREFIX%" ^
  -DCMAKE_PREFIX_PATH="%LIBRARY_PREFIX%" ^
  -DCMAKE_SYSTEM_PREFIX_PATH="%LIBRARY_PREFIX%" ^
  -DCMAKE_BUILD_TYPE="Release" ^
  -DWITH_EXAMPLES=OFF ^
  -DWITH_DAV1D=ON ^
  -DWITH_DAV1D_PLUGIN=OFF ^
  -DWITH_SvtEnc=OFF ^
  -DWITH_RAV1E=OFF ^
  -DBUILD_TESTING=OFF ^
  ..
if errorlevel 1 exit 1

ninja
if errorlevel 1 exit 1

ninja install
if errorlevel 1 exit 1
