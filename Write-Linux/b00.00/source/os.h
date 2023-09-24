/**
 * 功能：公共头文件
 *
 *创建时间：2022年8月31日
 *作者：李述铜
 *联系邮箱: 527676163@qq.com
 *相关信息：此工程为《从0写x86 Linux操作系统》的前置课程，用于帮助预先建立对32位x86体系结构的理解。整体代码量不到200行（不算注释）
 *课程请见：https://study.163.com/course/introduction.htm?courseId=1212765805&_trace_c_p_k2_=0bdf1e7edda543a8b9a0ad73b5100990
 */
#ifndef OS_H
#define OS_H
// 各选择子
#define KERNEL_CODE_SEG         (1 * 8)
#define KERNEL_DATA_SEG         (2 * 8)
//任务段, 设置任务等级为3  之后由于使用LDT便不再使用
#define APP_CODE_SEG            ((3 * 8) | 3)
#define APP_DATA_SEG            ((4 * 8) | 3)

//任务的TSS保存位置
#define TASK0_TSS_SEG           ((5 * 8))
#define TASK1_TSS_SEG           ((6 * 8))
//系统调用
#define SYSCALL_SEG             ((7 * 8))
//保存两个LDT的描述
#define TASK0_LDT_SEG           ((8 * 8))
#define TASK1_LDT_SEG           ((9 * 8))


//这里是任务的LDT的配置
#define TASK_CODE_SEG           (0 * 8 | 0x4 | 3)
#define TASK_DATA_SEG           (1 * 8 | 0x4 | 3)



#endif // OS_H
