//操作系统相关的配置文件

#define GDT_TABLE_SIZE              256
#define KERNEL_SELECTOR_CS          (1*8)
#define KERNEL_SELECTOR_DS          (2*8)
#define KERNEL_STACK_SIZE           (8*1024)
