
## Intor

Test on 5.3.0

## Build

Should down load libbpf and install it:

https://github.com/libbpf/libbpf

```sh
root@VM-74-51-ubuntu:/data/kernel/kernel_modules/bpf-exec-tracker# make
clang -c bpf_loader.c -o bpf_loader.o
clang -o bpf_loader bpf_loader.o -lelf -lbpf
clang -target bpf -g -O2 -c bpf_program.c -o bpf_program.o
root@VM-74-51-ubuntu:/data/kernel/kernel_modules/bpf-exec-tracker# ls
bpf_loader  bpf_loader.c  bpf_loader.o  bpf_program.c  bpf_program.o  Makefile  Readme.md
root@VM-74-51-ubuntu:/data/kernel/kernel_modules/bpf-exec-tracker# 
```

## Run
```sh
root@VM-74-51-ubuntu:/data/kernel/kernel_modules/bpf-exec-tracker# ./bpf_loader 
libbpf: elf: skipping unrecognized data section(5) .rodata.str1.1
New process 123458834952265 /bin/readlink [276]
  Arg 123458834952265 readlink [276]
  Arg 123458834952265 /proc/1/ns/mnt [276]
  Env 123458834952265 SHELL=/bin/sh [276]
  Env 123458834952265 PATH=/usr/local/bin:/usr/bin:/sbin:/usr/X11R6/bin:/usr/sbin:/bin:/usr/games [276]
  Env 123458834952265 PWD=/root [276]
  Env 123458834952265 TST_HACK_BASH_SESSION_ID=9201436678753 [276]
  Env 123458834952265 LANG=en_US.utf8 [276]
  Env 123458834952265 HOME=/root [276]
  Env 123458834952265 SHLVL=2 [276]
  Env 123458834952265 LOGNAME=root [276]
  Env 123458834952265 _=/bin/readlink [276]
New process 123463129919562 /usr/bin/touch [276]
  Arg 123463129919562 touch [276]
  Arg 123463129919562 /usr/local/sa/agent/watchdog.pid [276]
  Env 123463129919562 SHELL=/bin/sh [276]
  Env 123463129919562 PATH=/usr/local/bin:/usr/bin:/sbin:/usr/X11R6/bin:/usr/sbin:/bin:/usr/games [276]
  Env 123463129919562 PWD=/root [276]
  Env 123463129919562 TST_HACK_BASH_SESSION_ID=9201436678753 [276]
  Env 123463129919562 LANG=en_US.utf8 [276]
  Env 123463129919562 HOME=/root [276]
  Env 123463129919562 SHLVL=2 [276]
  Env 123463129919562 LOGNAME=root [276]
  Env 123463129919562 _=/usr/bin/touch [276]
```
