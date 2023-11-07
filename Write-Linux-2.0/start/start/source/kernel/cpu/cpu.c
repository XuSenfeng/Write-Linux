#include "cpu/cpu.h"
#include "os_cfg.h"
#include "comm/cpu_instr.h"
static segment_desc_t gdt_table_s[GDT_TABLE_SIZE];

//参数1选择表项, 基地址, 界限, 用来设置GDT表项
void segment_desc_set (int selector, uint32_t base, uint32_t limit, uint16_t attr)
{
    segment_desc_t * desc = gdt_table_s + selector  / sizeof(segment_desc_t);
    if(limit>0xfffff){
        //这时候的界限是4KB的
        limit/=0x1000;
        attr |= SEG_G;
    }
    desc->limit15_0 = limit & 0xffff;
	desc->base15_0 = base & 0xffff;
	desc->base23_16 = (base >> 16) & 0xff;
	desc->attr = attr | (((limit >> 16) & 0xf) << 8);
	desc->base31_24 = (base >> 24) & 0xff;

}

void gate_desc_set (gate_desc_t *desc, uint16_t selector, uint32_t offset, uint16_t attr){
    desc->offset15_0 = offset & 0xffff;
    desc->selector = selector;
    desc->attr = attr;
    desc->offset31_16 = (offset>>16)&0xffff;

}


//初始化GDT表的代码段数据段, 以及注册
void init_gdt(void){
    int i;
    for(i=0;i<GDT_TABLE_SIZE;i++){
        segment_desc_set(i<<3, 0, 0, 0);
    }
    //设置代码段
    segment_desc_set(KERNEL_SELECTOR_CS, 0, 0xffffffff, 
    (SEG_P_PRESENT | SEG_DPL_0 | SEG_D | SEG_S_NORMAL| SEG_TYPE_CODE | SEG_TYPE_RW));
    //数据段
    segment_desc_set(KERNEL_SELECTOR_DS, 0, 0xffffffff, 
    (SEG_P_PRESENT | SEG_DPL_0 | SEG_D | SEG_S_NORMAL| SEG_TYPE_DATA | SEG_TYPE_RW));

    //重新加载
    lgdt((uint32_t)gdt_table_s, sizeof(gdt_table_s));
}

void cpu_init(void){
    init_gdt();

}
