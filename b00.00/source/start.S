/**
 * 功能：16位与32位的启动混合代码
 *
 *创建时间：2022年8月31日
 *作者：李述铜
 *联系邮箱: 527676163@qq.com
 *相关信息：此工程为《从0写x86 Linux操作系统》的前置课程，用于帮助预先建立对32位x86体系结构的理解。整体代码量不到200行（不算注释）
 *课程请见：https://study.163.com/course/introduction.htm?courseId=1212765805&_trace_c_p_k2_=0bdf1e7edda543a8b9a0ad73b5100990
 */
	#include "os.h"

	// 声明本地以下符号是全局的，在其它源文件中可以访问
	.global _start, timer_int, syscall_handler

	// 指定以下的代码生成16位的机器指令，这样才能在启动时的实模式下运行
  	.code16

	// 以下是代码区
 	.text
_start:
//设置段寄存器
	mov $0, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %ss
	mov %ax, %gs
	mov %ax, %fs
	mov $_start, %esp

read_self_all:
	//首先设置读取到的位置
	mov $_start_32, %bx
	//在这里设置开始的扇区数
	mov $0x2, %cx
	//在这里设置的是读取的数量,以及是使用的读取模式
	mov $0x240, %ax
	//在这里设置读取的是C盘
	mov $0x80, %dx
	//调用中断
	int $0x13
	//读取失败再次读取
	jc read_self_all
	//进入保护模式
	//关中断
	cli
	//加载新的GDT表
	lgdt gdt_desc
	//加载中断向量表
	lidt idt_desc

	//设置CR0的0位, 操作的时候使用16位的操作寄存器
	mov $1, %eax
	lmsw %ax
	//跳转到内核代码段,进入32位模式,第一个参数是段的地址, 第二个数字是偏移量,也就是C语言程序被复制到的位置
	jmp $(KERNEL_CODE_SEG),$_start_32
	jmp .

	//以下是标志位
	.org 0x1fe				//十进制是510,在这里跳转到对应的位置
	.byte 0x55, 0xaa



	//标记下面是32位, 以及是代码段
	.code32
	.text
_start_32:
	//在这里设置段地址
	mov $KERNEL_DATA_SEG, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %ss
	mov %ax, %gs
	mov %ax, %fs
	mov $_start, %esp
	//这时候进行初始化时钟, 打开时钟中断,同时设置中断的向量表
	call os_init
	//打开页表
	mov $pg_dir, %eax
	mov %eax, %cr3
	//CR4里面有一个位控制是否允许这一个模式 
	mov %cr4, %eax
	orl $(1<<4), %eax
	mov %eax, %cr4
	//还需要控制PR0最高位w为1
	mov %cr0, %eax
	orl $(1<<31), %eax
	mov %eax, %cr0

	//打开中断, 由于在进入任务的时候会设置IF位, 所以在这里的时候不再进行设置
	//sti
	
	//告诉CPU正在运行的任务
	mov $TASK0_TSS_SEG, %ax
	ltr %ax


	//设置进入低特权级时候的所需要的栈
	push $APP_DATA_SEG
	//设置栈的指针
	push $task0_dpl3_stack + 1024*4
	//设置EFLAGS, 主要是设置中断的状态
	push $0x202
	//这是代码段
	push $APP_CODE_SEG
	push $task_0_entry

	//在这里进入低特权级
	iret
	jmp .

//进程0, 设置完栈以后会进入任务0里面
task_0_entry:
	//ss在出栈的时候已经设置了
	mov %ss, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %gs
	mov %ax, %fs
	jmp task_0



//中断处理函数, 会在这里进行任务的切换
timer_int:

	//对寄存器进行保护
	push %ds
	pusha

	//清除中断
	mov $0x20, %al
	outb %al, $0x20
	
	mov $KERNEL_DATA_SEG, %ax
	mov %ax, %ds

	call task_sched

	popa
	pop %ds
	
	iret

//这里是系统调用使用的代码段
syscall_handler:
	//对寄存器进行保护
	push %ds
	pusha
	//使用内核数据段, 会在之后的pop的时候进行复原
	mov $KERNEL_DATA_SEG, %ax
	mov %ax, %ds
	//获取传进来的参数, 之后再次入栈
	mov %esp, %ebp
	push 13*4(%ebp)
	push 12*4(%ebp)
	push 11*4(%ebp)

	call do_syscall
	add $(3*4), %esp
	popa
	pop %ds
	retf $(3*4)


//这里记录的是GDT表的数据,包括32位的基地址以及16位的大小界限,是用来设置时候的任务类型
gdt_desc:
	//界限+地址
	.word (256*8)-1
	.long gdt_table
//描述中断的数据
idt_desc:
	//界限+地址
	.word (256*8)-1
	.long idt_table