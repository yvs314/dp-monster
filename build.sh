rm -rf build
mkdir build
cd build
#AppleClang has problems with omp; try clang 9.0 through llvm (MacPorts)
if [[ "$OSTYPE" == "darwin"* ]]; then
	export CC="/opt/local/libexec/llvm-9.0/bin/clang"
	export CXX="/opt/local/libexec/llvm-9.0/bin/clang++"
fi
cmake -DCMAKE_BUILD_TYPE=Release ../src
make
