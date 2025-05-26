# 软件开发综合实验-数据备份软件(backuppro)

## 开发环境

- 操作系统：Ubuntu 22.04.2 LTS (GNU/Linux 5.15.90.1-microsoft-standard-WSL2 x86_64)
- 开发工具：Visual Studio Code
- g++ 11.4.0
- cmake 3.22.1
- OpenSSL 1.1.1n



## 编译安装

### 方法一：cmake

安装编译工具和依赖库
```shell
apt install -y build-essential cmake libssl-dev
```
切换到项目根目录下，依次执行如下命令
```shell
mkdir build && cd build
cmake ..
make
sudo make install
```

### 方法二：使用Dockerfile

切换到项目根目录下，执行如下命令构建镜像

```
docker build -f ./DockerFile -t myubuntu .
```

启动容器

```
docker run -it myubuntu /bin/bash
```
### 方法三：直接使用镜像
```
docker import backuppro-image.tar myubuntu
docker images
docker run -it myubuntu /bin/bash
```


## 实现的功能

- 数据备份
- 数据还原
- 文件类型支持
  - 普通文件

  - 目录文件

  - 管道文件

  - 软链接文件

  - 硬链接文件
- 元数据支持
- 自定义备份
  - 路径
  - 名字
  - 时间
- 打包解包
- 加密备份





## 使用方法

```
a backup master for Linux
Usage:
  backuppro [OPTION...]

  -b, --backup        备份文件
  -r, --restore       恢复文件
  -c, --compare arg   比较指定备份文件与现有文件的差异
  -i, --input arg     输入文件路径
  -o, --output arg    输出文件路径
  -p, --password arg  指定密码
  -h, --help          查看帮助文档

 Backup options:
  -e, --encrypt    备份时加密文件
      --path arg   过滤路径：正则表达式
      --name arg   过滤文件名：正则表达式
      --atime arg  文件的访问时间区间，例"2023-10-11 08:00:00 2023-10-12 20:00:00"
      --mtime arg  文件的修改时间区间，格式同atime
      --ctime arg  文件的改变时间区间，格式同atime

 Restore options:
  -a, --metadata  恢复文件的元数据
```

> `test`文件夹下存放了测试用的文件，使用前请先解压

**测试文件目录树**

.
├── a.cpp
├── a.txt
├── b.txt
├── dir
│   └── test.txt
├── exist_hard_a.txt
├── exist_symbolic_a.txt -> a.txt
├── fifo
├── inexist_hard_a.txt
└── inexist_symbolic_a.txt -> ../a.txt

1 directory, 9 files

**查看帮助文档**

```
backuppro -h
```

**比较备份文件差异**

```
backuppro -c bakdir/test.pak
```

**普通备份(打包和加密)**

`-b`表示备份，
`-e`表示对文件进行加密，
`-p`用于指定加密密码

```shell
backuppro -be -i test -o bakdir/test -p 2020060902021
```

**自定义备份**

`--path`指定**路径**的匹配规则(正则表达式)，
下面的指令只备份`test/dir`下的内容

```
backuppro -b -i test -o bakdir/test --path "^test/dir"
```

`--name`指定**文件名**的匹配规则(正则表达式)，
下面的指令只备份以`.cpp`为后缀的文件

```
backuppro -b -i test -o bakdir/test --name "\.cpp$"
```

`--atime`,`--mtime`, `--ctime`分别用于指定**文件的时间戳范围**，
下面的指令只备份在`2023-10-22 08:00:00`到`2023-10-23 20:00:00`之间修改过的文件

```
backuppro -b -i test -o bakdir/test --mtime "2023-10-22 08:00:00 2023-10-23 20:00:00"
```

**恢复文件**

`-r`表示恢复文件，
`-a`表示需要恢复文件元数据，
`-i`指定备份文件，
`-o`指定文件恢复到哪个位置
`-p`用于指定解密用的密码

```
backuppro -ra -i bakdir/test.pak.ept -o resdir/ -p 2020060902021
```

