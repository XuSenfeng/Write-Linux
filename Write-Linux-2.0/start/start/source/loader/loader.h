#ifndef LOADER_H
#define LOADER_H

#include "comm/boot_info.h"
#include "comm/types.h"
#include "comm/cpu_instr.h"
#include "comm/elf.h"

// 内存检测信息结构
typedef struct SMAP_entry {
    uint32_t BaseL; // base address uint64_t
    uint32_t BaseH;
    uint32_t LengthL; // length uint64_t
    uint32_t LengthH;
    uint32_t Type; // entry Type
    uint32_t ACPI; // extended
}__attribute__((packed)) SMAP_entry_t;


//磁盘的一个扇区的大小
#define SECTOR_SIZE 256
//内核放在1M字节的位置
#define SYS_KERNEL_LOAD_ADDR  (1024 * 1024)
#endif //LOADER_H