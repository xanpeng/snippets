// from http://weli.iteye.com/blog/1692038

#include <stdio.h>
#include <stdlib.h>

// 未赋值的全局变量放在dss段
int global_var;

// 已赋值的全局变量放在data段
int global_initialized_var = 5;

void function() {
	int stack_var; // 函数中的变量放在stack中
	// 放在stack中的变量
	// 显示其所在内存地值
	printf("the function's stack_var is at address 0x%08x\n", &stack_var);
}

int main() {
	int stack_var; // 函数中的变量放在stack中
	// 已赋值的静态变量放在data段
	static int static_initialized_var = 5;
	// 未赋值的静态变量放在dss段
	static int static_var;
	int *heap_var_ptr;
	// 由malloc在heap中分配所需内存，
	// heap_var_ptr这个指针指向这块
	// 分配的内存
	heap_var_ptr = (int *) malloc(4);
	// 放在stack中的变量
	// 显示其所在内存地值
	printf("====IN STACK====\n");
	printf("the main's stack_var is at address 0x%08x\n", &stack_var);
	function();
	// 放在heap中的变量
	// 显示其所在内存地值
	printf("\n====IN HEAP====\n");
	printf("heap_var is at address 0x%08x\n\n", heap_var_ptr);
	// 放在bss段的变量
	// 显示其所在内存地值
	printf("====IN BSS SEGMENT====\n");
	printf("static_var is at address 0x%08x\n", &static_var);
	printf("global_var is at address 0x%08x\n\n", &global_var);
	// 放在data段的变量
	// 显示其所在内存地值
	printf("====IN DATA SEGMENT====\n");
	printf("global_initialized_var is at address 0x%08x\n", &global_initialized_var);
	printf("static_initialized_var is at address 0x%08x\n\n", &static_initialized_var);
	return 0;
}
