#include "cpu/irq.h"
#include "os_cfg.h"
#include "cpu/cpu.h"
#include "comm/cpu_instr.h"
#define IDE_TABLE_NR        128
static gate_desc_t idt_table[IDE_TABLE_NR];

//没有定义的中断对应的处理函数
void exception_handler_unknown(void);

//初始化外部的中断
static void init_pic(){
	outb(PIC0_ICW1, PIC_ICW1_ALWAYS_1 | PIC_ICW1_ICW4);
	outb(PIC0_ICW2, IRQ_PIC_START);
	outb(PIC0_ICW3, (1<<2));
	outb(PIC0_ICW4, PIC_ICW4_8086);
	//第二块芯片
	outb(PIC1_ICW1, PIC_ICW1_ALWAYS_1 | PIC_ICW1_ICW4);
	outb(PIC1_ICW2, IRQ_PIC_START + 8);
	outb(PIC1_ICW3, 2);	//连接的位置
	outb(PIC1_ICW4, PIC_ICW4_8086);	
	//关闭所有中断但是不禁止从片的中断
	outb(PIC0_IMR, 0xff & ~(1<<2));
	outb(PIC1_IMR, 0xff);
}

void irq_init(void){
    for(int i=0;i<IDE_TABLE_NR;i++)
    {
        gate_desc_set(idt_table+i, KERNEL_SELECTOR_CS, (uint32_t)exception_handler_unknown, (GATE_P_PERSENT | GATE_DPL0 | GATE_TYPE_INT));
    }
    //gate_desc_set()
    // 设置异常处理接口
    irq_install(IRQ0_DE, exception_handler_divider);
	irq_install(IRQ1_DB, exception_handler_Debug);
	irq_install(IRQ2_NMI, exception_handler_NMI);
	irq_install(IRQ3_BP, exception_handler_breakpoint);
	irq_install(IRQ4_OF, exception_handler_overflow);
	irq_install(IRQ5_BR, exception_handler_bound_range);
	irq_install(IRQ6_UD, exception_handler_invalid_opcode);
	irq_install(IRQ7_NM, exception_handler_device_unavailable);
	irq_install(IRQ8_DF, exception_handler_double_fault);
	irq_install(IRQ10_TS, exception_handler_invalid_tss);
	irq_install(IRQ11_NP, exception_handler_segment_not_present);
	irq_install(IRQ12_SS, exception_handler_stack_segment_fault);
	irq_install(IRQ13_GP, exception_handler_general_protection);
	irq_install(IRQ14_PF, exception_handler_page_fault);
	irq_install(IRQ16_MF, exception_handler_fpu_error);
	irq_install(IRQ17_AC, exception_handler_alignment_check);
	irq_install(IRQ18_MC, exception_handler_machine_check);
	irq_install(IRQ19_XM, exception_handler_smd_exception);
	irq_install(IRQ20_VE, exception_handler_virtual_exception);

    lidt((uint32_t)idt_table, sizeof(idt_table));

	init_pic();
}

//加载一个中断函数
int irq_install (int irq_num, irq_handler_t handler){
    if(irq_num >= IDE_TABLE_NR){
        return -1;
    }
    gate_desc_set(idt_table+irq_num, KERNEL_SELECTOR_CS, (uint32_t)handler, (GATE_P_PERSENT | GATE_DPL0 | GATE_TYPE_INT));
}


//最终的处理函数
void do_default_handler(exception_frame_t * frame, char *message){
    for(;;)
        hlt();
    ;
}

//实际中断跳转到的位置
void do_handler_unknown (exception_frame_t * frame) {
	do_default_handler(frame, "Unknown exception.");
}

//下面是不同的中断或者事件的处理函数 
void do_handler_divider(exception_frame_t * frame) {
	do_default_handler(frame, "Device Error.");
}

void do_handler_Debug(exception_frame_t * frame) {
	do_default_handler(frame, "Debug Exception");
}

void do_handler_NMI(exception_frame_t * frame) {
	do_default_handler(frame, "NMI Interrupt.");
}

void do_handler_breakpoint(exception_frame_t * frame) {
	do_default_handler(frame, "Breakpoint.");
}

void do_handler_overflow(exception_frame_t * frame) {
	do_default_handler(frame, "Overflow.");
}

void do_handler_bound_range(exception_frame_t * frame) {
	do_default_handler(frame, "BOUND Range Exceeded.");
}

void do_handler_invalid_opcode(exception_frame_t * frame) {
	do_default_handler(frame, "Invalid Opcode.");
}

void do_handler_device_unavailable(exception_frame_t * frame) {
	do_default_handler(frame, "Device Not Available.");
}

void do_handler_double_fault(exception_frame_t * frame) {
	do_default_handler(frame, "Double Fault.");
}

void do_handler_invalid_tss(exception_frame_t * frame) {
	do_default_handler(frame, "Invalid TSS");
}

void do_handler_segment_not_present(exception_frame_t * frame) {
	do_default_handler(frame, "Segment Not Present.");
}

void do_handler_stack_segment_fault(exception_frame_t * frame) {
	do_default_handler(frame, "Stack-Segment Fault.");
}

void do_handler_general_protection(exception_frame_t * frame) {
	do_default_handler(frame, "General Protection.");
}

void do_handler_page_fault(exception_frame_t * frame) {
	do_default_handler(frame, "Page Fault.");
}

void do_handler_fpu_error(exception_frame_t * frame) {
	do_default_handler(frame, "X87 FPU Floating Point Error.");
}

void do_handler_alignment_check(exception_frame_t * frame) {
	do_default_handler(frame, "Alignment Check.");
}

void do_handler_machine_check(exception_frame_t * frame) {
	do_default_handler(frame, "Machine Check.");
}

void do_handler_smd_exception(exception_frame_t * frame) {
	do_default_handler(frame, "SIMD Floating Point Exception.");
}

void do_handler_virtual_exception(exception_frame_t * frame) {
	do_default_handler(frame, "Virtualization Exception.");
}


void do_handler_control_exception(exception_frame_t * frame) {
	do_default_handler(frame, "Control Exception.");
}

void irq_disable_global (void){
	cli();
}
void irq_enable_global (void){
	sti();
}
void irq_enable(int irq_num)
{
	if(irq_num < IRQ_PIC_START){
		return;
	}
	irq_num -= IRQ_PIC_START;
	if(irq_num<8){
		uint8_t mask = inb(PIC0_IMR) & ~(1<<irq_num);
		outb(PIC0_IMR, mask);
	}else{
		uint8_t mask = inb(PIC1_IMR) & ~(1<<irq_num);
		outb(PIC0_IMR, mask);
	}
}

void irq_disable(int irq_num)
{
	if(irq_num < IRQ_PIC_START){
		return;
	}
	irq_num -= IRQ_PIC_START;
	if(irq_num<8){
		uint8_t mask = inb(PIC0_IMR) | (1<<irq_num);
		outb(PIC0_IMR, mask);
	}else{
		uint8_t mask = inb(PIC1_IMR) | (1<<irq_num);
		outb(PIC0_IMR, mask);
	}

