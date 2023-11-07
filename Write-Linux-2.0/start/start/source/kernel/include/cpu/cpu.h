#ifndef CPU_H
#define CPU_H
//保存CPU相关的代码
#include "comm/types.h"

#pragma pack(1)
typedef struct _segment_desc_t {

    uint16_t limit15_0;
    uint16_t base15_0;
    uint8_t base23_16;
    uint16_t attr;
    uint8_t base31_24;

}segment_desc_t;


typedef struct _gate_desc_t
{
    uint16_t offset15_0;
    uint16_t selector;
    uint16_t attr;
    uint16_t offset31_16;
}gate_desc_t;



//GDT表的单位长度控制, 0为1bit, 1为4Kb
#define SEG_G               (1<<15)
//这个是在16为还是32位(1)的
#define SEG_D               (1<<14)
//当前存在的标志位
#define SEG_P_PRESENT       (1<<7)
//特权级
#define SEG_DPL_0           (0<<5)
#define SEG_DPL_3           (3<<5)
//是不是系统段, 还是代码数据段
#define SEG_S_SYSTEM        (0<<4)
#define SEG_S_NORMAL        (1<<4)
//类型
#define SEG_TYPE_CODE       (1<<3)
#define SEG_TYPE_DATA       (0<<3)
#define SEG_TYPE_RW         (1<<1)


//中断相关的设置
//设置为有效的
#define GATE_P_PERSENT      (1   << 15)
//设置特权级
#define GATE_DPL0           (0   << 13)
#define GATE_DPL3           (3   << 13)
//设置为中断门
#define GATE_TYPE_INT       (0xe << 8 )

void cpu_init(void);
void segment_desc_set (int selector, uint32_t base, uint32_t limit, uint16_t attr);
void gate_desc_set (gate_desc_t *desc, uint16_t selector, uint32_t offset, uint16_t attr);
#endif