set path=c:\Program Files\CMake 2.6\bin;c:\MinGW\bin;%path%
mkdir build-real
cd build-real
cmake ..\.. -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE:string=Release
mingw32-make 2> log.txt
pause