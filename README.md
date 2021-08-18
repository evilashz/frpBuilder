# frpBuilder
To Make frp with no arguments ,which Conveniently in red teaming



**I will give a simple modified source code of frp and Builder(MFC C++) in the project**


**All it does is write the configuration into the root.go and compile it, so first make sure that you can run go build in that directory**

### Usage:

open the Builder `FRPBuider\cmd\frpc\builder.exe`

![image-20210810092734550](https://images-1258433570.cos.ap-beijing.myqcloud.com/images/20210810092735.png)

### Did:

1. It will generate a frp without arguments

   in other words, when building a tunnel, you only need to execute this PE file directly

2. The tls encryption is enabled by default
3. Simply remove some not often used packages
4. But UPX compression is the default. If you want to reduce the size, please notice that could be Noticed by Anti-virus
5. Finally, the source code is here, you can further optimize, such as remove the tls features

