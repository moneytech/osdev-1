#include "vga.hpp"
#include "modes.hpp"
#include "device.hpp"

syscall_table *SYSCALL_TABLE;
char dbgbuf[256];

#pragma GCC diagnostic ignored "-Wunused-parameter"

volatile uint8_t * const vga_memory=(uint8_t*)0xA0000;
volatile uint8_t * const text_memory=(uint8_t*)0xB8000;

extern "C" int module_main(syscall_table *systbl, char *params){
	SYSCALL_TABLE=systbl;
	init_modes();
	init_device();
	return 0;
}

uint8_t read_graphics(uint8_t index){
	outb(VGA_Ports::GraphicsAddress, index);
	return inb(VGA_Ports::GraphicsData);
}

void write_graphics(uint8_t index, uint8_t byte){
	outb(VGA_Ports::GraphicsAddress, index);
	outb(VGA_Ports::GraphicsData, byte);
}

uint8_t read_sequencer(uint8_t index){
	outb(VGA_Ports::SequencerAddress, index);
	return inb(VGA_Ports::SequencerData);
}

void write_sequencer(uint8_t index, uint8_t byte){
	outb(VGA_Ports::SequencerAddress, index);
	outb(VGA_Ports::SequencerData, byte);
}

uint8_t read_crtc(uint8_t index){
	outb(VGA_Ports::CRTCAddress, index);
	return inb(VGA_Ports::CRTCData);
}

void write_crtc(uint8_t index, uint8_t byte){
	outb(VGA_Ports::CRTCAddress, index);
	outb(VGA_Ports::CRTCData, byte);
}

uint8_t read_attribute(uint8_t index){
	inb(VGA_Ports::InputStatus1);
	outb(VGA_Ports::AttributeWrite, index);
	return inb(VGA_Ports::AttributeRead);
}

void write_attribute(uint8_t index, uint8_t byte){
	inb(VGA_Ports::InputStatus1);
	outb(VGA_Ports::AttributeWrite, index);
	outb(VGA_Ports::AttributeWrite, byte);
}

void write_dac(uint8_t index, uint8_t r, uint8_t g, uint8_t b){
	if((r & 0x3F) != r) dbgpf("VGA: Bad red value: %x\n", r);
	if((g & 0x3F) != g) dbgpf("VGA: Bad green value: %x\n", g);
	if((b & 0x3F) != b) dbgpf("VGA: Bad blue value: %x\n", b);
	outb(VGA_Ports::DACWriteAddress, index);
	outb(VGA_Ports::DACData, r);
	outb(VGA_Ports::DACData, g);
	outb(VGA_Ports::DACData, b);
}

void read_dac(uint8_t index, uint8_t &r, uint8_t &g, uint8_t &b){
	outb(VGA_Ports::DACReadAddress, index);
	r=inb(VGA_Ports::DACData);
	g=inb(VGA_Ports::DACData);
	b=inb(VGA_Ports::DACData);
}

void unlock_crtc(){
	uint8_t reg= read_crtc(CRTC_Registers::EndVrtRetrace);
	reg |= (1 << 7);
	write_crtc(CRTC_Registers::EndVrtRetrace, reg);
	reg= read_crtc(CRTC_Registers::EndHrzBlanking);
	reg &= ~(1 << 7);
	write_crtc(CRTC_Registers::EndHrzBlanking, reg);
}

void lock_crtc(){
	uint8_t reg= read_crtc(CRTC_Registers::EndVrtRetrace);
	reg &= ~(1 << 7);
	write_crtc(CRTC_Registers::EndVrtRetrace, reg);
	reg= read_crtc(CRTC_Registers::EndHrzBlanking);
	reg &= ~(1 << 7);
	write_crtc(CRTC_Registers::EndHrzBlanking, reg);
}

void disable_display(){
	uint8_t reg = read_sequencer(Sequencer_Registers::ClockingMode);
	reg |= (1 << 5);
	write_sequencer(Sequencer_Registers::ClockingMode, reg);
}

void enable_display(){
	uint8_t reg = read_sequencer(Sequencer_Registers::ClockingMode);
	reg &= ~(1 << 5);
	write_sequencer(Sequencer_Registers::ClockingMode, reg);
}