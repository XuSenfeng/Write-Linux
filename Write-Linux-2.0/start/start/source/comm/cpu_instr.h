#ifndef CPU_INSTR_H
#define CPU_INSTR_H

#include "types.h"

//关中断
static inline void cli (void){
    __asm__ __volatile__("cli");
}
//开中断
static inline void sti (void){
    __asm__ __volatile__("sti");
}

//CPU休眠
static inline void hlt (void){
    __asm__ __volatile__("hlt");
}
//从外设读取
static inline uint8_t inb (uint16_t port){
    uint8_t rv;
    //inb al, dx
    __asm__ __volatile__("inb %[p],%[v]":[v]"=a"(rv) : [p]"d"(port));
    return rv;
}
//从外设读取
static inline uint16_t inw (uint16_t port){
    uint16_t rv;
    //in ax, dx
    __asm__ __volatile__("in %[p],%[v]":[v]"=a"(rv) : [p]"d"(port));
    return rv;
}
//向外设写入
static inline void outb (uint16_t port, uint8_t data){
    //outb al, dx
    __asm__ __volatile__("outb %[v],%[p]"::[p]"d"(port),[v]"a"(data));
}
//设置gdt表
static inline void lgdt (uint32_t start , uint32_t size){
    //加载gdt表
    struct{
        uint16_t limit;
        uint16_t start15_0;
        uint16_t start32_16;
    }gdt;
    gdt.start15_0 = start & 0xffff;
    gdt.start32_16 = start >> 16;
    gdt.limit = size -1;
    __asm__ __volatile__("lgdt %[g]"::[g]"m"(gdt));
}
//设置lidt
static inline void lidt(uint32_t start, uint32_t size) {
	struct {
		uint16_t limit;
		uint16_t start15_0;
		uint16_t start31_16;
	} idt;

	idt.start31_16 = start >> 16;
	idt.start15_0 = start & 0xFFFF;
	idt.limit = size - 1;

	__asm__ __volatile__("lidt %0"::"m"(idt));
}
//读取CR0
static inline uint8_t read_cr0 (void){
    uint32_t cr0;
    __asm__ __volatile__("mov %%cr0, %[v]": [v]"=r"(cr0));
    return cr0;
}
//写入
static inline void write_cr0 (uint32_t v){
    //inb al, dx
    __asm__ __volatile__("mov %[v], %%cr0"::[v]"r"(v));
    //return cr0;
}

//远跳转
static inline void far_jump(uint32_t selector, uint32_t offset) {
	uint32_t addr[] = {offset, selector };
	__asm__ __volatile__("ljmpl *(%[a])"::[a]"r"(addr));
}

#endif // !1 CPU_INSTR_H


