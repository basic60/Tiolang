# Tiolang
Tiolang是一个基于LR1的自制编译器，采用类C语言文法，生成运行于tiovm的目标代码。

# Usage
## 编译
```
$ make all
```
## 编译代码
```
$ cd compiler
$ ./tio 源代码文件
```
## 虚拟机运行目标代码
```
$ cd vm
$ ./tiovm
```
# 施工进度
- 支持char,int,long
- 支持if语句和while循环
- 支持函数和参数传递
# License
Tiolang is licensed under MIT Licence.