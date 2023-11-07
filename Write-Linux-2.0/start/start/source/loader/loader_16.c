__asm__(".code16gcc");

#include "loader.h"

void protect_mode_entry (void);

boot_info_t boot_info;			// 启动参数信息
//用于打印一个信息
static void show_msg(const char * msg)
{
    char c;
    while((c = *msg++) != '\0')
    {
        __asm__ __volatile__(
    	    "mov $0xe, %%ah\n\t"
    	    "mov %[ch], %%al\n\t"
    	    "int $0x10"::[ch]"r"(c)
        );
	    

    }
}

//用于获取可以使用的内存信息, 保存在结构体里面
static void detect_memory (void)
{
    //获取的信息存储的地址
    SMAP_entry_t smap_entry;
    SMAP_entry_t * entry = &smap_entry;
    uint32_t contID=0;
    uint32_t signature, bytes; 
    show_msg("try to detect memory");
    boot_info.ram_region_count = 0;
    for(int i = 0 ; i<BOOT_RAM_REGION_MAX;i++)
    {
        __asm__ __volatile__(
            "int $0x15"
            : "=a"(signature),"=c"(bytes),"=b"(contID)
            :"a"(0xE820), "b"(contID), "c"(24), "d"(0x534D4150), "D"(entry)
        );
        if(signature != 0x534D4150)
        {
            show_msg("Failed \r\n");
            return ;            
        }
        if(bytes>20 && (entry->ACPI &0x0001)==0)
        {
            //这一个数值被忽略
            continue;
        }
        if(entry->Type == 1)
        {
            //这个时候这一个内存可以被使用
            boot_info.ram_region_cfg[boot_info.ram_region_count].start = entry->BaseL;
            boot_info.ram_region_cfg[boot_info.ram_region_count].size  = entry->LengthL;
            boot_info.ram_region_count++;
        }


        if(contID == 0)
        {
            //没有可以使用的值
            break;
        }
    }
    show_msg("OK");

} 

uint16_t gdt_table[][4] = {
    {0, 0, 0, 0},
    {0xffff, 0x0000, 0x9a00, 0x00cf},
    {0xffff, 0x0000, 0x9200, 0x00cf},
};


//进入保护模式
static void enter_protect_mode(void){
    //关中断
    cli();
    uint8_t v = inb(0x92);
    outb(0x92, v | 0x2);
    lgdt((uint32_t)gdt_table, sizeof(gdt_table));
    //修改控制寄存器
    uint32_t cr0 = read_cr0();
    write_cr0(cr0 | (1<<0));

    far_jump(8, (uint32_t) protect_mode_entry);
    //sti();

}

void loader_entry(void){
    show_msg("Hello world\r\n");
    detect_memory();
    enter_protect_mode();
    for(;;){
    }
}

