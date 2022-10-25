#!/usr/bin/bash
ROOTDIR=$(dirname $(readlink -f ${0}))

NUTTXDIR=${ROOTDIR}/nuttx
APPSDIR=${ROOTDIR}/apps
TOOLSDIR=${NUTTXDIR}/tools
config=${ROOTDIR}/build.ini
source ${config}

# 保存编译参数和配置
function env_setup()
{
  # 编译次数
  if [ ! ${compile_count} ]; then
    compile_count=1
  else
    compile_count=`expr ${compile_count} + 1`
  fi
  sed 's/compile_count.*//g' ${config} -i
  echo "compile_count=${compile_count}" >> ${config}

  # 编译时间
  compile_time=$(date "+%Y-%m-%d %H:%M:%S")
  sed 's/compile_time.*//g' ${config} -i
  echo "compile_time=\"${compile_time}\"" >> ${config}
  sed '/^$/d' ${config} -i

  # 编译使用线程数
  if [ ! ${thread} ]; then
    thread=$(expr $(nproc --all) - 2)
    echo "thread=${thread}" >> ${config}
  fi

  # 开启编译详细信息
  if [ ! ${VERBOSE} ]; then
    VERBOSE="V=1"
    echo "VERBOSE=\"${VERBOSE}\"" >> ${config}
  fi

  # 生成 compile_commands.json
  if [ ! -z "$(type bear)" ] && [ ! ${bear_cmd} ];then
    bear_cmd="bear -a -o .vscode/compile_commands.json"
    echo "bear_cmd=\"${bear_cmd}\"" >> ${config}
  fi

  # 记录编译配置
  if [ ! ${board_config} ];then
    echo "board_config=\"${1}\"" >> ${config}
    board_config=$1
  else
    if [ "${board_config}" != "$1" ] && [ $1 ]; then
    echo "board_config=\"${1}\""
      echo "board_config=\"${1}\"" > ${config}
      board_config=$1
    fi
  fi

  if [ ! ${config_dir} ];then
    if [ ! -d $board_config ]; then
      config_dir=$(ls -d ${ROOTDIR}/nuttx/boards/*/*/${board_config/[:|\/]//configs/} 2> /dev/null)
      [ $? -ne 0 ] && usage
    else
      config_dir=${ROOTDIR}/${board_config}
    fi
    echo "config_dir=\"${config_dir}\"" >> ${config}
  fi
}

function build()
{
  # 编译 nuttx
  echo "Build ${config_dir}/defconfig command line:"
  echo "  make -C ${NUTTXDIR} ${VERBOSE} -j${thread} $@"
  echo "  make -C ${NUTTXDIR} savedefconfig"

  if ! ${TOOLSDIR}/configure.sh -e ${config_dir}; then
    usage
    exit 1
  fi

  ${bear_cmd} make -C ${NUTTXDIR} ${VERBOSE} -j${thread} $@
  if [ ! $? ]; then
    echo "Error: ############# build ${board_config} fail ##############"
    exit 2
  fi

  # 保存 defconfig
  make -C ${NUTTXDIR} savedefconfig
  if [ ! $? ]; then
    echo "Error: ############# save ${board_config} fail ##############"
    exit 3
  fi

  # 复制 defconfig
  cp ${NUTTXDIR}/defconfig ${config_dir}
}

function copy()
{
  mkdir -p ${ROOTDIR}/bin
  while read line
  do
    echo cp -p ${NUTTXDIR}/${line} ${ROOTDIR}/bin
    cp -p ${NUTTXDIR}/${line} ${ROOTDIR}/bin
  done < ${NUTTXDIR}/nuttx.manifest
}

function usage()
{
  echo "############## Usage ##############"
  echo "Usage: $0 <board-name>:<config-name> [make options]"
  exit 1
}

env_setup $@
build
copy
