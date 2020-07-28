# TinyToolPkg
a edk2 tool set package


## build:
$ build your edk2 env,   reference https://github.com/tianocore/edk2
$ cd edk2
$ . edksetup.sh
$ git clone https://github.com/Tinycl/TinyToolPkg.git
$ build -a X64 -p TinyToolPkg/TinyToolPkg.dsc


## program note:
(1) when you need use a hex, must use like 0xffff not 0xFFFF
(2) gcc x64 function register param, when you need to use gas to do something
    like:
    .S
    fun:
        ...
        ret

    .c
    void fun(int a, char b, int c,) -- rdi, rsi, rdx, rcx, r8, r9
    void fun(float a, )   -- xmm0, xmm1, xmm2, xmm3, xmm4, xmm5