#!/bin/sh

isp_version="0.2"
git_version=""

if [ -n $(which git) ] && [ -d .git ] ; then git_version=".dev-$(git rev-parse --short HEAD)" ; fi

echo "#define ISP_VERSION \"$isp_version\"" > src/version.h
echo "#define GIT_VERSION \"$git_version\"" >> src/version.h
