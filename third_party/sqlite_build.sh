#!/bin/bash
snapshot_ver=sqlite-snapshot-202408161851

name=sqlite

abs_dir=`pwd`
build_dir=${abs_dir}/${name}

echo ${abs_dir}

mkdir -p ${build_dir}

cd ${snapshot_ver}

./configure --prefix=${build_dir}

make -j4 install


