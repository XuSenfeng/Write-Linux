#include "init.h"
#include "comm/boot_info.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
void kernel_init(boot_info_t *boot_info)
{
    cpu_init();
    irq_init();
    //之后返回汇编文件
}

void init_main(boot_info_t *boot_info)
{
    int a = 3/0;
    for(;;);
}