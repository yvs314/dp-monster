rm -rf build
mkdir build
cd build
#gotta check whether we have to use cmake or cmake3
cmake -DCMAKE_BUILD_TYPE=Release ../src
make
