#!/bin/bash

set +e

ROOT=$(dirname $(readlink -f "$0"))
BUILDDIR=$1
INSTALLDIR=$2
SOURCEDIR="$ROOT"

function USAGE {
  echo ""
  echo "Usage: \"install_arbalest.sh [BUILD_DIR] [INSTALL_DIR]\""
  echo "[BUILD_DIR]: the directory where cmake & ninja compile the source code"
  echo "[INSTALL_DIR]: the directory where the customized LLVM will be installed"
  echo "Please make sure that both directories have been granted full permissions (write,read,execute)"
}

if [ -z ${BUILDDIR:+x} ]; then
  echo "[BUILD_DIR] is not set"
  USAGE
  exit 1
fi

if [ -z ${INSTALLDIR:+z} ]; then
  echo "[INSTALL_DIR] is not set"
  USAGE
  exit 1
fi

PROJECTS="clang;clang-tools-extra"
RUNTIMES="compiler-rt;libcxx;libcxxabi;libunwind;openmp"

LLVM_SOURCE=$SOURCEDIR/llvm

echo "LLVM Source: ${LLVM_SOURCE}"

exit 0

mkdir -p $BUILDDIR

cd $SOURCEDIR


export CXXFLAGS="-Wno-unused-command-line-argument $CXXFLAGS" 
export CFLAGS="-Wno-unused-command-line-argument $CFLAGS" 

cd $BUILDDIR
#rem(){
CPATH="/usr/include/x86_64-linux-gnu" LIBRARY_PATH="/usr/lib/x86_64-linux-gnu" \
cmake -GNinja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_INSTALL_PREFIX=$INSTALLDIR \
  -DLLVM_ENABLE_LIBCXX=ON \
  -DLLVM_LIT_ARGS="-sv -j12" \
  -DCLANG_DEFAULT_CXX_STDLIB=libc++ \
  -DLIBOMPTARGET_ENABLE_DEBUG=ON \
  -DLLVM_ENABLE_PROJECTS=$PROJECTS \
  -DLLVM_ENABLE_RUNTIMES=$RUNTIMES \
  -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
  -DLLVM_INSTALL_UTILS=ON \
  -DLIBOMPTARGET_BUILD_CUDA_PLUGIN=False \
  -DLIBOMPTARGET_BUILD_AMDGPU_PLUGIN=False \
  $LLVM_SOURCE
#}
time nice -n 10 ninja -j$(nproc --all) -l$(nproc --all) || exit -1
time nice -n 10 ninja -j$(nproc --all) -l$(nproc --all) install
