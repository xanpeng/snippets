#ifdef __compiler_offsetof
    #define offsetof(TYPE, MEMBER) __compiler_offsetof(TYPE,MEMBER)
#else
    #define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

/*
typeof操作符是GCC编译器指定的，获取操作数的类型。
((type *)0)->member: 小技巧，搭配 typeof 可以定位到结构成员member的类型。
定义__mptr指针的意图：有人猜测是保存ptr，以防止外部被更改，我觉得另一个原因是利用类型检查。
将ptr减去member在结构体中的偏移，就得到结构体的指针。
*/
#define container_of(ptr, type, member) ({ \
   const typeof( ((type *)0)->member ) *__mptr = (ptr); \
    (type *)( (char *)__mptr - offsetof(type, member) );})}; 

// 示例结构
struct example_struct {
    int vala;
    struct example_member member_t;
    int valb; 
};

// 示例函数
struct example_struct *some_func(struct example_member *target_ptr) {
    return container_of(target_ptr, struct example_struct, member_t);
}
