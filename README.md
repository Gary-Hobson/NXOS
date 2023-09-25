# NXOS
一个用于简化基于 NuttX 的应用开发的仓库，将常用仓库以 git submodules 的方式进行整合。

# 下载代码
```
git clone --recursive git@github.com:Gary-Hobson/NXOS.git
```

# 使用指南

```sh
# 安装依赖
sudo apt update
sudo apt install -y \
bison flex gettext texinfo libncurses5-dev libncursesw5-dev xxd \
gperf automake libtool pkg-config build-essential gperf genromfs \
libgmp-dev libmpc-dev libmpfr-dev libisl-dev binutils-dev libelf-dev \
libexpat-dev gcc-multilib g++-multilib picocom u-boot-tools util-linux \
kconfig-frontends gcc-arm-none-eabi binutils-arm-none-eabi zlib1g-dev
```

```
# 编译
./nx.sh sim:minibasic

# 运行 sim
./bin/nuttx
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
