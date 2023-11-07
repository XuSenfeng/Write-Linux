    
#include "os_cfg.h"

    .text
    .extern kernel_init
    .extern init_main
    .global _start
    // void start(boot_info_t * boot_info);
_start:
    //保存使用的寄存器,这是为了gcc的标准
    push %ebp
    //获取栈
    mov %esp, %ebp
    //获取传入的参数
    mov 0x8(%ebp), %eax
    //传入获得的参数
    push %eax
    call kernel_init
    jmp $KERNEL_SELECTOR_CS, $gdt_reload
gdt_reload:
    
	mov $KERNEL_SELECTOR_DS, %ax		// 16为数据段选择子
	mov %ax, %ds
    mov %ax, %ss
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    //加载一个新的栈
    mov $(stack + KERNEL_STACK_SIZE), %esp
    jmp init_main
    jmp .

    .bss
    //设置一个栈
.comm stack, KERNEL_STACK_SIZE


//这是一个宏, 用来进行中断函数的重用,可以给出参数,名字,第几个,是否会传入错误码 
.macro exception_handler name num  with_err_code
    .text
    .extern do_handler_\name
    .global exception_handler_\name
//中断处理函数
exception_handler_\name:
    //传入一些参数
    .if \with_err_code == 0
        push $0
    .endif
    push $\num

    pusha
    push %ds
    push %es
    push %fs
    push %gs
    push %esp
    call do_handler_\name
    //还原传入的数字
    add $(1*4), %esp
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
    add $(2*4), %esp
    iret
.endm


exception_handler unknown, -1, 0
exception_handler divider, 0, 0
exception_handler Debug, 1, 0
exception_handler NMI, 2, 0
exception_handler breakpoint, 3, 0
exception_handler overflow, 4, 0
exception_handler bound_range, 5, 0
exception_handler invalid_opcode, 6, 0
exception_handler device_unavailable, 7, 0
exception_handler double_fault, 8, 1
exception_handler invalid_tss, 10, 1
exception_handler segment_not_present, 11, 1
exception_handler stack_segment_fault, 12, 1
exception_handler general_protection, 13, 1
exception_handler page_fault, 14, 1
exception_handler fpu_error, 16, 0
exception_handler alignment_check, 17, 1
exception_handler machine_check, 18, 0
exception_handler smd_exception, 19, 0
exception_handler virtual_exception, 20, 0
exception_handler control_exception, 21, 1