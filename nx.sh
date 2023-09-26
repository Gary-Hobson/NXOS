#!/usr/bin/env bash
# nx.sh
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

ROOTDIR=$(realpath $(dirname $(readlink -f ${0})))
NUTTXDIR=${ROOTDIR}/nuttx
TOOLSDIR=${NUTTXDIR}/tools

bear_version=`bear --version 2>&1 | awk -F '[ .]' '/bear/{print $2}'`
if [ "$bear_version" == "3" ]; then
	BEAR="bear --append --"
else
	BEAR="bear --append "
fi

function build_board()
{
  echo -e "Build command line:"
  echo -e "  ${TOOLSDIR}/configure.sh -e $1"
  echo -e "  make -C ${NUTTXDIR} ${@:2}"
  echo -e "  make -C ${NUTTXDIR} savedefconfig"

  if ! ${TOOLSDIR}/configure.sh $1; then
    make -C ${NUTTXDIR} distclean -j
    rm $ROOTDIR/compile_commands.json
    if ! ${TOOLSDIR}/configure.sh $1; then
      echo "Error: ############# config ${1} fail ##############"
      exit 1
    fi
  fi

  if ! ${BEAR} make -C ${NUTTXDIR} ${@:2} -j; then
    echo "Error: ############# build ${1} fail ##############"
    exit 2
  fi

  if [ "${2}" == "distclean" ]; then
    rm -rf $ROOTDIR/compile_commands.json
    return;
  fi

  make -C ${NUTTXDIR} savedefconfig
  if [ ! -d $1 ]; then
    cp ${NUTTXDIR}/defconfig ${ROOTDIR}/nuttx/boards/*/*/${1/[:|\/]//configs/}
  else
    cp ${NUTTXDIR}/defconfig $1
  fi
}

if [ $# == 0 ]; then
  echo "Usage: $0 <board-name>:<config-name> [make options]"
  echo "       $0 <config-path> [make options]"
  exit 1
fi

board_config=$1
shift

if [ -d ${ROOTDIR}/${board_config} ]; then
  build_board ${ROOTDIR}/${board_config} $*
else
  build_board ${board_config} $*
fi

