cexception
==========

copy from http://throwtheswitch.org/white-papers/cexception-intro.html

文档：[CExceptionSummary.pdf](https://github.com/ThrowTheSwitch/CException/blob/master/docs/CExceptionSummary.pdf)  
更多：[ThrowTheSwitch/CException](https://github.com/ThrowTheSwitch/CException)  

##适用范围  

cexception为c语言提供类似于C++、Java中的exception handling机制。  
我还不是很清楚exception handle的适用范围、实现方法和最佳实践，不过cexception提供的exception机制是指程序逻辑的exception，这个看CExceptionSummary.pdf便一目了然。  

##实现原理  

cexception是利用setjmp+longjmp实现的。举例说明：  
```c
void function_b(void) {
    Throw(1);
}

void function_a(void) {
    function_b();
}

int main() {
    CEXCEPTION_T e;
    Try {
        function_a();
    }
    Catch(e) {
        printf("there was an error: %d\n", e);
        return e;
    }

    return 0;
}
```

解释：  
1、在try block中，程序通过setjmp()保存当前函数(这里是main函数)的stack frame(栈帧)。该栈帧是全局可见的。  
2、在function_b中，throw通过调用longjmp直接跳到setjmp保存的栈帧中。使得错误不需要逐层向上传递。  
_3、function_b的栈帧估计会在longjmp之前销毁。_  
_4、setjmp会返回两次，调用setjmp本身返回时，返回值为零。这里估计即使setjmp返回了，它记录的目标函数栈帧仍然是不会释放的，因为需要等待longjmp的进入。在longjmp进入之后，做些事情，setjmp再次返回，并且返回值不为0。_  

`man setjmp`:  
> int setjmp(jmp_buf env);  
> ...setjmp() saves the stack context/environment in env for later use by longjmp. The stack  context  will be invalidated if the function which called setjmp() returns.  
> ...return 0 if returning directly, and non-zero when returning from longjmp using the saved context.

`man longjmp`:  
> void longjmp(jmp_buf env, int val);  
> ...These functions never return.  
> ...longjmp() restores the environment saved by the last call of setjmp(3) with the corresponding env argument.  
> ...After longjmp() is completed, program execution continues as if the corresponding call of setjmp(3) had just returned the value val.  
> ...longjmp() cannot cause 0 to be returned.  If longjmp() is invoked with a second argument of 0, 1 will be returned instead.

##我的改动

1、命名格式，由驼峰法改成“全小写+下划线”(Linux格式)。  
2、增加更多示例。  