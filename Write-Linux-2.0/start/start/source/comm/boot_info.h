#ifndef BOOT_INFO_H
#define BOOT_INFO_H


#include "types.h"

#define BOOT_RAM_REGION_MAX             10
//定义一个记录内存信息的结构体
typedef struct _boot_info_t{
    struct {
        uint32_t start;
        uint32_t size;
    }ram_region_cfg[BOOT_RAM_REGION_MAX];
    
    int ram_region_count; /*记录有效的数据的个数*/
}boot_info_t;



#endif //BOOT_INFO_H