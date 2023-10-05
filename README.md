# NXOS

一个用于简化基于 NuttX 的应用开发的仓库，将常用仓库以 git submodules 的方式进行整合。

# 下载代码

```
# 默认分支 
git clone --recursiv https://github.com/Gary-Hobson/NXOS.git

# 主线开发分支,使用 apache 主线代码,向社区提交代码时使用
git clone --recursive -dev https://github.com/Gary-Hobson/NXOS.git
```

# 使用指南

**安装依赖**

```sh
# 将 apt 和 pip 换到清华源, 以提高下载速度
sed -i'.bak' 's,/[a-z]*.ubuntu.com,/mirrors.tuna.tsinghua.edu.cn,' /etc/apt/sources.list
python -m pip install -i https://pypi.tuna.tsinghua.edu.cn/simple --upgrade pip
python -m pip install --upgrade pip
pip config set global.index-url https://pypi.tuna.tsinghua.edu.cn/simple

sudo apt update
sudo apt install -y \
curl bison flex gettext texinfo libncurses5-dev libncursesw5-dev xxd \
gperf automake libtool pkg-config build-essential genromfs libx11-dev \
libgmp-dev libmpc-dev libmpfr-dev libisl-dev binutils-dev libelf-dev \
libexpat-dev gcc-multilib g++-multilib picocom u-boot-tools util-linux \
kconfig-frontends gcc-arm-none-eabi binutils-arm-none-eabi zlib1g-dev \
ccache bear minicom
pip install pyelftools cxxfilt
```

**编译运行**

```sh
# 编译命令语法
./nx.sh 配置文件路径 <Makefile 参数>
Makefile 参数:
    V=1: 查看详细编译命令
    distclean: 清理配置及编译缓存
    clean: 清理编译缓存
    menuconfig: 配置菜单

# 编译
./nx.sh boards/sim/configs/hello

# 运行
./nuttx/nuttx
```

**NSH 使用**
在 nuttx 启动后会出现一个 Shell，它支持基本的命令，nsh 中输入 `help` 查看支持的所有命令。

- help: 查看支持的所有命令行命令，其中 `Builtin Apps` 为可执行的 task，例如 `hello` 为一个独立的 task 可以运行，可以理解为一个进程。
- ps: nsh 内置命令，查看正在运行的线程信息
- ls: nsh 内置命令，查看文件系统中的文件
- hello: 进程，运行后会执行 apps/examples/hello/hello_main.c 的代码
- poweroff: 退出系统

```sh
NuttShell (NSH)
nsh> help
help usage:  help [-v] [<cmd>]

    .         cd        echo      hexdump   mkfifo    readlink  time      xd
    [         cp        env       insmod    mkrd      rm        true
    ?         cmp       exec      kill      mount     rmdir     truncate
    alias     dirname   exit      losetup   mv        rmmod     uname
    unalias   date      false     ln        poweroff  set       umount
    basename  dd        free      ls        printf    sleep     unset
    break     df        memdump   lsmod     ps        source    uptime
    cat       dmesg     help      mkdir     pwd       test      usleep

Builtin Apps:
    sh          setlogmask  dumpstack   hello       nsh
nsh>
nsh> hello
Hello, World!!
nsh>
nsh>
nsh> ps
  PID GROUP PRI POLICY   TYPE    NPX STATE    EVENT     SIGMASK           STACK   USED  FILLED    CPU COMMAND
    0     0   0 FIFO     Kthread N-- Ready              0000000000000000 2162592 007784   0.3%   99.3% Idle Task
    1     1 255 RR       Kthread --- Waiting  Signal    0000000000000000 2162560 003832   0.1%    0.6% loop_task
    2     2 224 RR       Kthread --- Waiting  Semaphore 0000000000000000 2162544 003384   0.1%    0.0% hpwork 0x5587b753b300
    3     3 100 RR       Kthread --- Waiting  Semaphore 0000000000000000 2162544 002888   0.1%    0.0% lpwork 0x5587b753b360
    5     5 100 RR       Task    --- Running            0000000000000000 2162560 011384   0.5%    0.0% nsh_main
nsh>
nsh> ls
/:
 bin/
 data/
 dev/
 etc/
 proc/
 tmp/
nsh>
nsh> poweroff
```

## commit specification

### commit format

```
<type> (<scope>): <subject>
<BLANK LINE>
<details>
```

### commit types

```
sync: 同步子仓库代码
feat： 新增 feature
fix: 修复 bug
docs: 仅仅修改了文档，比如 README, CHANGELOG, CONTRIBUTE等等
style: 仅仅修改了空格、格式缩进、逗号等等，不改变代码逻辑
refactor: 代码重构，没有加新功能或者修复 bug
perf: 优化相关，比如提升性能、体验
test: 测试用例，包括单元测试、集成测试等
chore: 改变构建流程、或者增加依赖库、工具等
revert: 回滚到上一个版本
```
