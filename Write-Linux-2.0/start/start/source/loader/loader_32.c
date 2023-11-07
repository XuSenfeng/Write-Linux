
#include "loader.h"

extern boot_info_t boot_info;			// 启动参数信息

//进行磁盘的读取
static void read_disk(int sector, int sector_count, uint8_t *buf)
{
    //设置为主驱动器, 同时使用LBA模式进行读取
    outb(0x1f6, (uint8_t)0xe0);
    //读取的字节数高字节
    outb(0x1f2, (uint8_t)(sector_count >> 8));
    //读取的扇区的地址4, 5, 6字节, 这里只使用32位的扇区地址
    outb(0x1f3, (uint8_t)(sector >> 24));
    outb(0x1f4, (uint8_t)(0));
    outb(0x1f5, (uint8_t)(0));
    //读取的字节数低字节
    outb(0x1f2, (uint8_t)sector_count);
    //读取的扇区的地址1, 2, 3字节
    outb(0x1f3, (uint8_t)(sector));
    outb(0x1f4, (uint8_t)(sector >> 8));
    outb(0x1f5, (uint8_t)(sector >> 16));

    //读取
    outb(0x1f7, (uint8_t)0x24);

    //检查状态寄存器
    uint16_t * data_buf = (uint16_t *)buf;
    while(sector_count -- > 0)
    {
        //等待忙标志位结束
        while ((inb(0x1f7) & 0x88) != 0x8){}
        for(int i=0;i<SECTOR_SIZE/2; i++)
        {
            *data_buf++ = inw(0x1f0);
        }
    }
}
//加载elf文件, 同时返回需要跳转的文件的地址
static uint32_t reload_elf_file(uint8_t * file_buffer){
    Elf32_Ehdr *elf_hdr = (Elf32_Ehdr *)file_buffer;
    if((elf_hdr->e_ident[0] != 0x7f) || (elf_hdr->e_ident[1] != 'E') ||
         (elf_hdr->e_ident[2] != 'L') || (elf_hdr->e_ident[3] != 'F')){
        return 0;
    }
    for(int i=0;i<elf_hdr->e_phnum;i++)
    {
        Elf32_Phdr *phdr = (Elf32_Phdr *)(file_buffer + elf_hdr->e_phoff) + i;
        if(phdr->p_type != PT_LOAD){
            //内容不能加载
            continue;
        }
        //获取源文件地址以及需要加载的位置
        uint8_t *src = file_buffer + phdr->p_offset;
        uint8_t *dest = (uint8_t *)phdr->p_paddr;
        for(int j=0; j < phdr->p_filesz;j++)
        //进行文件的复制
        {
            *dest++ = *src++; 
        }
        //计算结束地址, 对bss区域进行清零
        dest = (uint8_t *)phdr->p_paddr + phdr->p_filesz;
        for(int j=0;j<phdr->p_memsz - phdr->p_filesz;j++)
        {
            *dest++ = 0;
        }
    }
    //返回进入的地址
    return elf_hdr->e_entry;
}
static void die(int code){
    for(;;){};
}

void load_kernel(void)
{
    //进行读取
    read_disk(100, 500, (uint8_t *)(SYS_KERNEL_LOAD_ADDR));
    //解析elf文件
    uint32_t kernel_entey = reload_elf_file((uint8_t *)SYS_KERNEL_LOAD_ADDR);
    if(kernel_entey == 0)
    {
        //加载程序失败
        die(-1);

    }
    //跳转到对应的位置
    ((void (*) (boot_info_t *))kernel_entey)(&boot_info);
    for(;;)
    {

    }
}