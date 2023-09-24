/**
 * 功能：32位代码，完成多任务的运行
 *
 *创建时间：2022年8月31日
 *作者：李述铜
 *联系邮箱: 527676163@qq.com
 *相关信息：此工程为《从0写x86 Linux操作系统》的前置课程，用于帮助预先建立对32位x86体系结构的理解。整体代码量不到200行（不算注释）
 *课程请见：https://study.163.com/course/introduction.htm?courseId=1212765805&_trace_c_p_k2_=0bdf1e7edda543a8b9a0ad73b5100990
 */
#include "os.h"

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

//系统调用
void do_syscall(int func, char * str, char color)
{
    static int row=1;
    if(func==2)
    {
        unsigned short *dest = (unsigned short *)0xb8000 + 80*row;
        while(*str){
            *dest ++ = *str ++ | (color<<8);
        }
        row = (row>=25)?0:row+1;
        for(int i=0;i<0xffffff;i++);
    }
    
}
//这个是打印字符串的函数, 参数是要打印的字符串和显示的颜色,之后会使用系统调用
void sys_show(char *str, char color)
{
    uint32_t addr[] = {0, SYSCALL_SEG};

    __asm__ __volatile__("push %[color];push %[str];push %[id];lcalll *(%[a])"::
                    [a]"r"(addr), [color]"m"(color), [str]"m"(str), [id]"r"(2));
}


//任务1
void task_0(void)
{
    char * str = "task1 a:1234";
    uint8_t color = 0;

    for(;;){
        sys_show(str, color++);
    }
}
//任务2
void task_1(void)
{
    char * str = "task2 b:5678";

    uint8_t color = 0xff;
    for(;;){
        sys_show(str, color--);
    }
}

//这个是两个LDT表, 分别被两个任务使用
struct {uint16_t limit_l, base_l, basehl_attr, base_limit;}task0_ldt_table[256] __attribute__((aligned(8))) = {
    //这里是进程使用的段, 这里使用的是重合的模式, 设置优先级为3, 设置为可执行可读
    [TASK_CODE_SEG /8] = {0xffff, 0x0000, 0xfa00, 0x00cf},
    //设置为可读可写可访问
    [TASK_DATA_SEG /8] = {0xffff, 0x0000, 0xf300, 0x00cf},
};
struct {uint16_t limit_l, base_l, basehl_attr, base_limit;}task1_ldt_table[256] __attribute__((aligned(8))) = {
    //这里是进程使用的段, 这里使用的是重合的模式, 设置优先级为3, 设置为可执行可读
    [TASK_CODE_SEG /8] = {0xffff, 0x0000, 0xfa00, 0x00cf},
    //设置为可读可写可访问
    [TASK_DATA_SEG /8] = {0xffff, 0x0000, 0xf300, 0x00cf},
};


//这一个表需要进行八字节对齐
struct {uint16_t limit_l, base_l, basehl_attr, base_limit;}gdt_table[256] __attribute__((aligned(8))) = {
    // 0x00cf9a000000ffff - 从0地址开始，P存在，DPL=0，Type=非系统段，32位代码段（非一致代码段），界限4G，
    [KERNEL_CODE_SEG / 8] = {0xffff, 0x0000, 0x9a00, 0x00cf},
    // 0x00cf93000000ffff - 从0地址开始，P存在，DPL=0，Type=非系统段，数据段，界限4G，可读写
    [KERNEL_DATA_SEG/ 8] = {0xffff, 0x0000, 0x9200, 0x00cf},
    //这里是进程使用的段, 这里使用的是重合的模式, 设置优先级为3, 设置为可执行可读
    [APP_CODE_SEG /8] = {0xffff, 0x0000, 0xfa00, 0x00cf},
    //设置为可读可写可访问
    [APP_DATA_SEG /8] = {0xffff, 0x0000, 0xf300, 0x00cf},
    //TSS表, 由于直接使用一个数组作为TSS会导致报错,这里基地址初始化为0
    [TASK0_TSS_SEG /8] = {0x68, 0, 0xe900, 0},
    [TASK1_TSS_SEG /8] = {0x68, 0, 0xe900, 0},
    //这里是系统调用,首先不初始化任务的函数地址, 之后是代码段, 权限设置为3
    [SYSCALL_SEG / 8] = {0x0000, KERNEL_CODE_SEG, 0xec03, 0},
    //任务的LDT
    [TASK0_LDT_SEG / 8] = {sizeof(task0_ldt_table-1), 0x0, 0xe200, 0x00c0},
    [TASK1_LDT_SEG / 8] = {sizeof(task1_ldt_table-1), 0x0, 0xe200, 0x00c0},


};
//这个表是否有效
#define PDE_P                   (1<<0)
//是否可写
#define PDE_W                   (1<<1)
//是否可以被低权限访问
#define PDE_U                   (1<<2)
//设置使用的模式
#define PDE_PS                  (1<<7)

#define MAG_ADDR                0x80000000
//使用二级表进行控制内存测试
uint8_t map_phy_buffer[4096] __attribute__((aligned(4096))) = {0x36};
//创建一个二级表项,随便给一个值
static uint32_t page_table[1024] __attribute__((aligned(4096))) = {PDE_U};

//定义一个页表的结构体,需要设置低0的表项
uint32_t pg_dir[1024] __attribute__((aligned(4096))) = {
    [0] = (0) | PDE_P |  PDE_W | PDE_U | PDE_PS,
};

//中断向量表
struct {uint16_t offset_l, selector, attr, offset_h;}idt_table[256] __attribute__((aligned(8)));

//初始化一个32位的栈, 使用模式为特权级3
uint32_t task0_dpl3_stack[1024];
uint32_t task1_dpl3_stack[1024];
uint32_t task0_dpl0_stack[1024];
uint32_t task1_dpl0_stack[1024];


//定义一个TSS结构, 这个是任务一的表
uint32_t task0_tss[] = {
    // prelink, esp0, ss0, esp1, ss1, esp2, ss2
    0,  (uint32_t)task0_dpl0_stack + 4*1024, KERNEL_DATA_SEG , /* 后边不用使用 */ 0x0, 0x0, 0x0, 0x0,
    // cr3, eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi,
    (uint32_t)pg_dir,  (uint32_t)task_0/*入口地址*/, 0x202, 0xa, 0xc, 0xd, 0xb, (uint32_t)task0_dpl3_stack + 4*1024/* 栈 */, 0x1, 0x2, 0x3,
    // es, cs, ss, ds, fs, gs, ldt, iomap
    TASK_DATA_SEG, TASK_CODE_SEG, TASK_DATA_SEG, TASK_DATA_SEG, TASK_DATA_SEG, TASK_DATA_SEG, TASK0_LDT_SEG, 0x0,
};
//这个是任务二的表
uint32_t task1_tss[] = {
    // prelink, esp0, ss0, esp1, ss1, esp2, ss2
    0,  (uint32_t)task1_dpl0_stack + 4*1024, KERNEL_DATA_SEG , /* 后边不用使用 */ 0x0, 0x0, 0x0, 0x0,
    // cr3, eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi,
    (uint32_t)pg_dir,  (uint32_t)task_1/*入口地址*/, 0x202, 0xa, 0xc, 0xd, 0xb, (uint32_t)task1_dpl3_stack + 4*1024/* 栈 */, 0x1, 0x2, 0x3,
    // es, cs, ss, ds, fs, gs, ldt, iomap
    TASK_DATA_SEG, TASK_CODE_SEG, TASK_DATA_SEG, TASK_DATA_SEG, TASK_DATA_SEG, TASK_DATA_SEG, TASK1_LDT_SEG, 0x0,
};

//对汇编指令进行一个封装
void outb(uint8_t data,uint16_t port){
    //这里传入两个数据,第一个数据是data,第二个数据是port,之后进行数据的关联,"d"会识别为%dx,之后会把port放进去
    __asm__ __volatile__("outb %[v], %[p]"::[p]"d"(port), [v]"a"(data));
}

//在汇编文件中实现的中断处理函数, 在这里进行声明
void timer_int(void);
void syscall_handler(void);


//初始化一个页表,并在之后设置为0x80000000映射到这一个数组的位置
void os_init(void){
    //初始化定时器
    //初始化中断控制器
    outb(0x11, 0x20);
    outb(0x11, 0xA0);
    //设置主片的中断起始位置
    outb(0x20, 0x21);
    //设置从片的中断起始位置
    outb(0x28, 0xa1);
    //设置从片连接主片的位置
    outb((1<<2), 0x21);
    //设置从片连接主片使用的引脚
    outb(2, 0xa1);
    //设置连接的模式
    outb(0x1,0x21);
    outb(0x1,0xa1);
    //设置中断的屏蔽
    outb(0xfe,0x21);    //只有最低的中断也就是时钟中断没有被屏蔽
    outb(0xff,0xa1);
    //配置外部时钟源,是一个16位的计数器,减到0的时候会产生中断,这里计算每秒产生100次中断大概需要的数值
    int tmo = 1193180 / 100;
    //写入数值
    //设置使用的时钟以及自动加载
    outb(0x36, 0x43);
    //中断频率的设置
    outb((uint8_t)tmo, 0x40);
    outb(tmo>>8, 0x40);
    //记录中断处理函数的地址
    idt_table[0x20].offset_l = (uint32_t)timer_int & 0xffff;
    idt_table[0x20].offset_h = (uint32_t)timer_int >> 16;
    idt_table[0x20].selector = KERNEL_CODE_SEG;
    //设置为中断门,32位模式
    idt_table[0x20].attr = 0x8e00;

    //设置两个任务控制段的地址
    gdt_table[TASK0_TSS_SEG / 8].base_l = (uint16_t)(uint32_t)task0_tss;
    gdt_table[TASK1_TSS_SEG / 8].base_l = (uint16_t)(uint32_t)task1_tss;
    //设置系统调用函数的地址
    gdt_table[SYSCALL_SEG / 8].limit_l = (uint16_t)(uint32_t)syscall_handler;

    //设置LDT的起始位置
    gdt_table[TASK0_LDT_SEG / 8].base_l = (uint16_t)(uint32_t)task0_ldt_table;
    gdt_table[TASK1_LDT_SEG / 8].base_l = (uint16_t)(uint32_t)task1_ldt_table;

    //设置一级表,使用的是表的高10位
    pg_dir[MAG_ADDR>>22] = (uint32_t)page_table | PDE_P | PDE_W | PDE_U;
    //初始化表的二级,这里是实际的地址,之后需要设置对应的位置
    page_table[(MAG_ADDR>>12)&0x3ff] = (uint32_t)map_phy_buffer | PDE_P | PDE_W | PDE_U;
}


//进行任务的切换
void task_sched(void){
    static int task_tss = TASK0_TSS_SEG;
    
    task_tss = (task_tss == TASK0_TSS_SEG) ? TASK1_TSS_SEG : TASK0_TSS_SEG;
    //设置为没有偏移跳转到新的任务段
    uint32_t addr[] = {0, task_tss};
    __asm__ __volatile__("ljmpl *(%[a])"::[a]"r"(addr));
}

