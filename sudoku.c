/* This files provides address values that exist in the system */
#define BOARD "DE1-SoC"
/* Memory */
#define DDR_BASE 0x00000000
#define DDR_END 0x3FFFFFFF
#define A9_ONCHIP_BASE 0xFFFF0000
#define A9_ONCHIP_END 0xFFFFFFFF
#define SDRAM_BASE 0xC0000000
#define SDRAM_END 0xC3FFFFFF
#define FPGA_ONCHIP_BASE 0xC8000000
#define FPGA_ONCHIP_END 0xC803FFFF
#define FPGA_CHAR_BASE 0xC9000000
#define FPGA_CHAR_END 0xC9001FFF
/* Cyclone V FPGA devices */
#define LEDR_BASE 0xFF200000
#define HEX3_HEX0_BASE 0xFF200020
#define HEX5_HEX4_BASE 0xFF200030
#define SW_BASE 0xFF200040
#define KEY_BASE 0xFF200050
#define JP1_BASE 0xFF200060
#define JP2_BASE 0xFF200070
#define PS2_BASE 0xFF200100
#define PS2_DUAL_BASE 0xFF200108
#define JTAG_UART_BASE 0xFF201000
#define JTAG_UART_2_BASE 0xFF201008
#define IrDA_BASE 0xFF201020
#define TIMER_BASE 0xFF202000
#define A9_TIMER_BASE 0xFFFEC600
#define AV_CONFIG_BASE 0xFF203000
#define PIXEL_BUF_CTRL_BASE 0xFF203020
#define CHAR_BUF_CTRL_BASE 0xFF203030
#define AUDIO_BASE 0xFF203040
#define VIDEO_IN_BASE 0xFF203060
#define ADC_BASE 0xFF204000
/* Cyclone V HPS devices */
#define HPS_GPIO1_BASE 0xFF709000
#define HPS_TIMER0_BASE 0xFFC08000
#define HPS_TIMER1_BASE 0xFFC09000
#define HPS_TIMER2_BASE 0xFFD00000
#define HPS_TIMER3_BASE 0xFFD01000
#define FPGA_BRIDGE 0xFFD0501C
/* ARM A9 MPCORE devices */
#define PERIPH_BASE 0xFFFEC000 // base address of peripheral devices
#define MPCORE_PRIV_TIMER 0xFFFEC600 // PERIPH_BASE + 0x0600
/* Interrupt controller (GIC) CPU interface(s) */
#define MPCORE_GIC_CPUIF 0xFFFEC100 // PERIPH_BASE + 0x100
#define ICCICR 0x00 // offset to CPU interface control reg
#define ICCPMR 0x04 // offset to interrupt priority mask
#define ICCIAR 0x0C // offset to interrupt acknowledge reg
#define ICCEOIR 0x10 // offset to end of interrupt reg
/* Interrupt controller (GIC) distributor interface(s) */
#define MPCORE_GIC_DIST 0xFFFED000 // PERIPH_BASE + 0x1000
#define ICDDCR 0x00 // offset to distributor control reg
#define ICDISER 0x100 // offset to interrupt set-enable regs
#define ICDICER 0x180 // offset to interrupt clear-enable
#define ICDIPTR 0x800 // offset to interrupt processor
#define ICDICFR 0xC00 // offset to interrupt configuration
#define IRQ_MODE          0b10010
#define SVC_MODE          0b10011

#define INT_ENABLE        0b01000000
#define INT_DISABLE       0b11000000
#define ENABLE 0x1
#define KEYS_IRQ 73
#define PS2_IRQ 79
#define	A9PRIVATETIMER_IRQ 29
/* VGA colors */
#define WHITE 0xFFFF
#define YELLOW 0xFFE0
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define GREY 0xC618
#define PINK 0xFC18
#define ORANGE 0xFC00

#define ABS(x) (((x) > 0) ? (x) : -(x))

/* Screen size. */
#define RESOLUTION_X 320
#define RESOLUTION_Y 240

/* Constants for animation */
#define BOX_LEN 2
#define NUM_BOXES 8

#define FALSE 0
#define TRUE 1

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
// Begin part3.c code for Lab 7
void clear_screen();
void plot_pixel(int x, int y, short int line_color);
void draw_line(int x0, int y0, int x1, int y1, short int line_color);
void swap(int* a, int* b);
void wait_for_vsync();
void plot_box(int x, int y, short int box_color, int size);
void draw_string(int x, int y, char* s, int color, int scale);
void draw_letter(int x, int y, int color, char c, int scale);
void draw_number(int x, int y, short int color, int curNum);
void generateBoard(int board [9][9], bool boardChangeable [9][9], int  mode);
bool checkBoard(int board [9][9]);
void displayTime(int time, int x, int y);
bool isValid(int board[9][9], int x, int y, int value);
bool fillRemaining(int col, int row, int board [9][9], bool boardChangeable[9][9]);
int lives_remaining = 3;
void set_A9_IRQ_stack();
void config_GIC();
void config_ps2();
void config_HPS_timer();
void config_HPS_GPIO1();
void config_interval_timer(); 
void config_KEYs();
void enable_A9_interrupts();
void pushbutton_ISR();
void keyboard_ISR();
void config_a9timer();
void config_interrupt(int N, int CPU_target);
void privtimer_ISR();
void VGA_draw_point(int x, int y, short color) ;

void VGA_draw_rectangle(int x, int y, int width, int height, short color, int fill) {
    int i, j;
    for(i = y; i < y+height; i++) {
        for(j = x; j < x+width; j++) {
            if(fill) {
                VGA_draw_point(j, i, color);
            } else {
                if(i == y || i == y+height-1 || j == x || j == x+width-1) {
                    VGA_draw_point(j, i, color);
                }
            }
        }
    }
}

void VGA_draw_point(int x, int y, short color) {
    short *pixel_address = (short *) (FPGA_ONCHIP_BASE + (y << 10) + (x << 1));
    *pixel_address = color;
}

volatile int pixel_buffer_start; // global variable
int letterBitMap[26][5][5] = {
	{{0,1,1,0,0},
	 {1,0,0,1,0},
	 {1,1,1,1,0},
	 {1,0,0,1,0},
	 {1,0,0,1,0}},
	{{1,1,1,0,0},
	 {1,0,0,1,0},
	 {1,1,1,0,0},
	 {1,0,0,1,0},
	 {1,1,1,0,0}},
	{{0,1,1,0,0},
	 {1,0,0,1,0},
	 {1,0,0,0,0},
	 {1,0,0,1,0},
	 {0,1,1,0,0}},
	{{1,1,1,0,0},
	 {1,0,0,1,0},
	 {1,0,0,1,0},
	 {1,0,0,1,0},
	 {1,1,1,0,0}},
	{{1,1,1,1,0},
	 {1,0,0,0,0},
	 {1,1,1,0,0},
	 {1,0,0,0,0},
	 {1,1,1,1,0}},
	{{1,1,1,1,0},
	 {1,0,0,0,0},
	 {1,1,1,0,0},
	 {1,0,0,0,0},
	 {1,0,0,0,0}},
	{{0,1,1,0,0},
	 {1,0,0,0,0},
	 {1,0,1,1,0},
	 {1,0,0,1,0},
	 {0,1,1,0,0}},
	{{1,0,0,1,0},
	 {1,0,0,1,0},
	 {1,1,1,1,0},
	 {1,0,0,1,0},
	 {1,0,0,1,0}},
	{{1,1,1,0,0},
	 {0,1,0,0,0},
	 {0,1,0,0,0},
	 {0,1,0,0,0},
	 {1,1,1,0,0}},
	{{0,0,0,1,0},
	 {0,0,0,1,0},
	 {0,0,0,1,0},
	 {1,0,0,1,0},
	 {0,1,1,0,0}},
	{{1,0,0,1,0},
	 {1,0,1,0,0},
	 {1,1,0,0,0},
	 {1,0,1,0,0},
	 {1,0,0,1,0}},
	{{1,0,0,0,0},
	 {1,0,0,0,0},
	 {1,0,0,0,0},
	 {1,0,0,0,0},
	 {1,1,1,1,0}},
	{{1,0,0,0,1},
	 {1,1,0,1,1},
	 {1,0,1,0,1},
	 {1,0,0,0,1},
	 {1,0,0,0,1}},
	{{1,0,0,1,0},
	 {1,1,0,1,0},
	 {1,0,1,1,0},
	 {1,0,0,1,0},
	 {1,0,0,1,0}},
	{{0,1,1,0,0},
	 {1,0,0,1,0},
	 {1,0,0,1,0},
	 {1,0,0,1,0},
	 {0,1,1,0,0}},
	{{1,1,1,0,0},
	 {1,0,0,1,0},
	 {1,1,1,0,0},
	 {1,0,0,0,0},
	 {1,0,0,0,0}},
	{{0,1,1,0,0},
	 {1,0,0,1,0},
	 {1,0,0,1,0},
	 {1,0,1,0,0},
	 {0,1,0,1,0}},
	{{1,1,1,0,0},
	 {1,0,0,1,0},
	 {1,1,1,0,0},
	 {1,0,1,0,0},
	 {1,0,0,1,0}},
	{{0,1,1,1,0},
	 {1,0,0,0,0},
	 {0,1,1,0,0},
	 {0,0,0,1,0},
	 {1,1,1,0,0}},
	{{1,1,1,1,1},
	 {0,0,1,0,0},
	 {0,0,1,0,0},
	 {0,0,1,0,0},
	 {0,0,1,0,0}},
	{{1,0,0,1,0},
	 {1,0,0,1,0},
	 {1,0,0,1,0},
	 {1,0,0,1,0},
	 {0,1,1,0,0}},
	{{1,0,0,0,1},
	 {1,0,0,0,1},
	 {0,1,0,1,0},
	 {0,1,0,1,0},
	 {0,0,1,0,0}},
	{{1,0,0,0,1},
	 {1,0,0,0,1},
	 {1,0,1,0,1},
	 {1,1,0,1,1},
	 {1,0,0,0,1}},
	{{1,0,0,1,0},
	 {1,0,0,1,0},
	 {0,1,1,0,0},
	 {1,0,0,1,0},
	 {1,0,0,1,0}},
	{{1,0,0,0,1},
	 {1,0,0,0,1},
	 {0,1,1,1,0},
	 {0,0,1,0,0},
	 {0,0,1,0,0}},
	{{1,1,1,1,0},
	 {0,0,1,0,0},
	 {0,1,0,0,0},
	 {1,0,0,0,0},
	 {1,1,1,1,0}}};
int numberStringBitMap[10][5][5] = {
	{{0,1,1,0,0},
	 {1,0,0,1,0},
	 {1,0,0,1,0},
	 {1,0,0,1,0},
	 {0,1,1,0,0}},
	{{0,1,0,0,0},
	 {1,1,0,0,0},
	 {0,1,0,0,0},
	 {0,1,0,0,0},
	 {1,1,1,0,0}},
	{{1,1,1,1,0},
	 {0,0,0,1,0},
	 {1,1,1,1,0},
	 {1,0,0,0,0},
	 {1,1,1,1,0}},
	{{1,1,1,1,0},
	 {0,0,0,1,0},
	 {0,1,1,1,0},
	 {0,0,0,1,0},
	 {1,1,1,1,0}},
	{{1,0,0,1,0},
	 {1,0,0,1,0},
	 {1,1,1,1,0},
	 {0,0,0,1,0},
	 {0,0,0,1,0}},
	{{1,1,1,1,0},
	 {1,0,0,0,0},
	 {1,1,1,1,0},
	 {0,0,0,1,0},
	 {1,1,1,1,0}},
	{{1,1,1,1,0},
	 {1,0,0,0,0},
	 {1,1,1,1,0},
	 {1,0,0,1,0},
	 {1,1,1,1,0}},
	{{1,1,1,1,0},
	 {0,0,0,1,0},
	 {0,0,0,1,0},
	 {0,0,0,1,0},
	 {0,0,0,1,0}},
	{{1,1,1,1,0},
	 {1,0,0,1,0},
	 {1,1,1,1,0},
	 {1,0,0,1,0},
	 {1,1,1,1,0}},
	{{1,1,1,1,0},
	 {1,0,0,1,0},
	 {1,1,1,1,0},
	 {0,0,0,1,0},
	 {1,1,1,1,0}}
};
int numberBitMap[10][19][19] = {
		{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},	
	
		{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0},
		 {0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0},
		 {0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
	
		{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
	
	
		{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
	
	
		{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
	
		{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
	
		{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
	
		{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
	
		{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
	
		{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
	
	};

int board[9][9] = {{0,0,0,0,0,0,0,0,0},
				   {0,0,0,0,0,0,0,0,0},
				   {0,0,0,0,0,0,0,0,0},
				   {0,0,0,0,0,0,0,0,0},
				   {0,0,0,0,0,0,0,0,0},
				   {0,0,0,0,0,0,0,0,0},
				   {0,0,0,0,0,0,0,0,0},
				   {0,0,0,0,0,0,0,0,0},
				   {0,0,0,0,0,0,0,0,0}};
bool boardChangeable[9][9] = {
	{true ,true ,true ,true ,true ,true ,true ,true ,true },
	{true ,true,true ,true ,true ,true ,true ,true ,true },
	{true ,true ,true ,true ,true ,true,true ,true,true },
	{true ,true ,true ,true, true,true ,true ,true ,true },
	{true ,true ,true ,true ,true ,true ,true,true ,true },
	{true ,true ,true,true ,true ,true,true ,true ,true },
	{true ,true ,true,true ,true ,true ,true ,true,true },
	{true ,true ,true ,true,true ,true ,true ,true ,true },
	{true ,true ,true ,true ,true ,true ,true ,true ,true }};

volatile int* key_ptr = KEY_BASE;
volatile int* sw_ptr = SW_BASE;
volatile int* timer_ptr = A9_TIMER_BASE;
volatile int* ps2_ptr = PS2_BASE;


int selectedRow = 0;
int selectedCol = 0;
bool flicker = false;
char * title = "sudoku";
char * authors = "by ahmet and tanushanth";
char * startInst1 = "key 0 - beginner";
char * startInst2 = "key 1 - medium";
char * startInst3 = "key 2 - hard";
char * endInst = "press any key to restart";
char * key3Inst = "sel dir";
char * key1Inst = "l-r";
char * key2Inst = "d-u";
char * key0Inst = "check";
char * key0 = "key 0";
char * key1 = "key 1";
char * key2 = "key 2";
char * key3 = "key 3";
char * gameover = "you lose";
char * gamewin = "you win";
char * highScoreText = "high scores";
bool upDown = false; // Down = False, Up = True
bool leftRight = false; // Left = False, Right = True
int currentScreen = 0; // 0 - Start screen, 1 - Game screen, 2 - End screen
// Game Loop
int currentTime = 0;
int mode = 0;
int beginnerHighScore = -1;
int mediumHighScore = -1;
int hardHighScore = -1;
char ps2byte1 = 0x0;
char ps2byte2 = 0x0;
char ps2byte3 = 0x0;

int main(void)
{
	set_A9_IRQ_stack(); 
	config_GIC();
	config_KEYs(); 
	config_ps2();
	config_a9timer();
	enable_A9_interrupts(); // enable interrupts
	//Setting up buffers
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;

    /* set front pixel buffer to start of FPGA On-chip memory */
    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the 
                                        // back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer
    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer

	
    clear_screen(); // pixel_buffer_start points to the pixel buffer



    while (1)
    {
		if(currentScreen == 0){
			
			clear_screen();
			draw_string(20, 20, title, 0x0, 5);
			draw_string(20, 60, authors, 0x0, 2);
			draw_string(20, 150, startInst1, 0x0, 2);
			draw_string(20, 180, startInst2, 0x0, 2);
			draw_string(20, 210, startInst3, 0x0, 2);
			draw_string(200,120,highScoreText, 0x0, 2);
			displayTime(beginnerHighScore, 210, 145);
			displayTime(mediumHighScore, 210, 175);
			displayTime(hardHighScore, 210, 205);
			//Update Selected Key
						//Update Selected Key
			
			
		}
		else if(currentScreen == 1){
			clear_screen();
			draw_string(70, 20, title, 0x0, 3);
			draw_string(12, 55, key0, 0x0, 2);
			draw_string(8, 70, key0Inst, 0x0, 2);
			draw_string(12, 95, key1, 0x0, 2);
			draw_string(18, 110, key1Inst, 0x0, 2);
			draw_string(12, 135, key2, 0x0, 2);
			draw_string(18, 150, key2Inst, 0x0, 2);
			draw_string(12, 170, key3, 0x0, 2);
			draw_string(4, 185, key3Inst, 0x0, 2);
			displayTime(currentTime, 175, 20);
			draw_string(260, 55, "lives", 0x0, 2);
			draw_string(265, 70, "left", 0x0, 2);
			 // replace with actual number of lives remaining
			int i;
			for (i = 0; i < lives_remaining; i++) {
    			plot_box(275 , 100+i*30, GREEN, 20);
			}
			for (; i < 3; i++) {
    			plot_box(275, 100+i*30, RED, 20);
			}
			// Drawing Grid
			for(int gridCoord = 45; gridCoord <= 225; gridCoord = gridCoord + 20){
				draw_line(70, gridCoord, 250, gridCoord, 0x0);

			}
			for(int gridCoord = 70; gridCoord <= 250; gridCoord = gridCoord + 20){
				draw_line(gridCoord, 45, gridCoord, 225, 0x0);
			}
			//Bold squares
			for(int gridCoord = 44; gridCoord <= 224; gridCoord = gridCoord + 60){
				draw_line(70, gridCoord, 250, gridCoord, 0x0);

			}
			for(int gridCoord = 46; gridCoord <= 226; gridCoord = gridCoord + 60){
				draw_line(70, gridCoord, 250, gridCoord, 0x0);

			}
			for(int gridCoord = 69; gridCoord <= 249; gridCoord = gridCoord + 60){
				draw_line(gridCoord, 45, gridCoord, 225, 0x0);
			}
			for(int gridCoord = 71; gridCoord <= 251; gridCoord = gridCoord + 60){
				draw_line(gridCoord, 45, gridCoord, 225, 0x0);
			}


			for(int i = 0; i < 9; i++){
				for(int j = 0; j < 9; j++){
					int topLeftX = 70 + (20 * (i)) + 1;
					int topLeftY = 45 + (20 * (j)) + 1;
					if(board[i][j] != 0){
						short int color = BLUE;
						if(boardChangeable[i][j] == false){
							color = 0x0;
						}
						draw_number(topLeftX, topLeftY, color, board[i][j]);	
					}
				}
			}
					// Drawing Flicker Select
			if(flicker){
				draw_line(70+((selectedCol)*20), 45 + (selectedRow * 20), 70 + ((selectedCol+1)*20), 45 + (selectedRow * 20), RED);
				draw_line(70+((selectedCol)*20), 45 + ((selectedRow+1) * 20), 70 + ((selectedCol+1)*20), 45 + ((selectedRow+1) * 20), RED);
				draw_line(70+((selectedCol)*20), 45 + ((selectedRow) * 20), 70 + ((selectedCol)*20), 45 + ((selectedRow+1) * 20), RED);
				draw_line(70+((selectedCol+1)*20), 45 + ((selectedRow) * 20), 70 + ((selectedCol+1)*20), 45 + ((selectedRow+1) * 20), RED);
			}

			
			
			
			
			if(boardChangeable[selectedCol][selectedRow] == true){
				int swValue = *sw_ptr;
				if(swValue == 0b0000000001){
					if(isValid(board, selectedCol, selectedRow,0)){
						board[selectedCol][selectedRow] = 0;
					}

				}
				else if(swValue == 0b0000000010){
					if(isValid(board, selectedCol, selectedRow,1)){
						board[selectedCol][selectedRow] = 1;
					}
				}
				else if(swValue == 0b0000000100){
					if(isValid(board, selectedCol, selectedRow,2)){
						board[selectedCol][selectedRow] = 2;
					}
				}
				else if(swValue == 0b0000001000){
					if(isValid(board, selectedCol, selectedRow,3)){
						board[selectedCol][selectedRow] = 3;
					}
				}
				else if(swValue == 0b0000010000){
					if(isValid(board, selectedCol, selectedRow,4)){
						board[selectedCol][selectedRow] = 4;
					}
				}
				else if(swValue == 0b0000100000){
					if(isValid(board, selectedCol, selectedRow,5)){
						board[selectedCol][selectedRow] = 5;
					}
				}
				else if(swValue == 0b0001000000){
					if(isValid(board, selectedCol, selectedRow,6)){
						board[selectedCol][selectedRow] = 6;
					}
				}
				else if(swValue == 0b0010000000){
					if(isValid(board, selectedCol, selectedRow,7)){
						board[selectedCol][selectedRow] = 7;
					}
				}
				else if(swValue == 0b0100000000){
					if(isValid(board, selectedCol, selectedRow,8)){
						board[selectedCol][selectedRow] = 8;
					}
				}
				else if(swValue == 0b1000000000){
					if(isValid(board, selectedCol, selectedRow,9)){
						board[selectedCol][selectedRow] = 9;
					}
				}			
			}
			

			
			
			
			
		}
		
		else if (currentScreen == 2){ // Game over screen
			clear_screen();
			draw_string(20, 20, title, 0x0, 5);
			draw_string(20, 60, gameover, 0x0, 2);
			draw_string(60, 200, endInst, 0x0, 2);
			
		}
		
		else if (currentScreen == 3){ // Win screen
			clear_screen();
			draw_string(20, 20, title, 0x0, 5);
			draw_string(20, 60, gamewin, 0x0, 2);
			draw_string(60, 200, endInst, 0x0, 2);
			if(mode == 0 && (currentTime < beginnerHighScore || beginnerHighScore == -1)){beginnerHighScore = currentTime;}
			else if(mode == 1 && (currentTime < mediumHighScore || mediumHighScore == -1)){mediumHighScore = currentTime;}
			else if(mode == 2 && (currentTime < hardHighScore || hardHighScore == -1)){hardHighScore = currentTime;}


		}
		
		wait_for_vsync(); // swap front and back buffers on VGA vertical sync

		pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer

    }
}

// code for subroutines (not shown)
void wait_for_vsync(){
	volatile int* pixel_ctrl_ptr = 0xFF203020;	
	*pixel_ctrl_ptr = 1;
	int status = *(pixel_ctrl_ptr + 3);
	while((status & 0x01) != 0){
		status = *(pixel_ctrl_ptr + 3);
	}
}


// code not shown for clear_screen() and draw_line() subroutines
void clear_screen()
{
	for(int x = 0; x < RESOLUTION_X; x++){
		for(int y = 0; y < RESOLUTION_Y; y++){
			plot_pixel(x,y, 0xFFFF);
		}
	}
}

void plot_pixel(int x, int y, short int line_color)
{
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void swap(int* a, int* b){
	int temp = *a;
	*a = *b;
	*b = temp;
}

void draw_line(int x0, int y0, int x1, int y1, short int line_color)
{
	bool is_steep = ABS(y1 - y0) > ABS(x1 - x0);
	if (is_steep)
	{
		swap(&x0, &y0);
		swap(&x1, &y1);
	}
	if (x0 > x1)
	{
		swap(&x0, &x1);
		swap(&y0, &y1);
	}
	
	int deltax = x1 - x0;
	int deltay = ABS(y1 - y0);
	int error = -1 * (deltax / 2);
	int y = y0;
	int y_step;
	if (y0 < y1)
	{ 
		y_step = 1;
	}
	else
	{
		y_step = -1;
	}

	for(int x = x0; x < x1; x++){
		
		if (is_steep){
			plot_pixel(y, x, line_color);
		}
		
		else{
			plot_pixel(x, y, line_color);
		}
		
		error = error + deltay;
		if (error> 0){
			y = y + y_step;
			error = error - deltax;
		}
	}
}
void plot_box(int x, int y, short int box_color, int size){
	for(int i = 0; i <= size; i++){
		for(int j = 0; j <= size; j++){
			plot_pixel(x + i, y + j, box_color);
		}
	}
}
void draw_number(int x, int y, short int color, int curNum){
	for(int col = 0; col < 19; col++){
		for(int row = 0; row < 19; row++){
			if(numberBitMap[curNum][col][row] == 1){
				plot_pixel(x + row, y + col, color);
			}
		}
	}
}

//Draws string given font, supports lowercase letters & numbers
void draw_string(int x, int y, char* s, int color, int scale){
	for(int i = 0; i < strlen(s); i++){
		if(*(s+i) == ' ') {
			x += 1 * scale;
		}
		else if (*(s+i) == '-'){
			plot_box(x + scale, y + (2*scale), color, scale);
			plot_box(x + (2*scale), y + (2*scale), color, scale);
			x += 5 * scale;
		}
		else{
			
			draw_letter(x,y,color,*(s+i), scale);

			//Space before next letter, some letters are larger than others
			if(*(s+i) == 'm' || *(s+i) == 'w' || *(s+i) == 't' 
			   || *(s+i) == 'v' || *(s+i) == 'y'){
				x += 6 * scale;
			}	
			else if (*(s+i) == 'i' || *(s+i) == '1'){
				x+= 4 * scale;
			}
			else{
				x+= 5 * scale;
			}

		}
	}
}
// Supports lowercase letter & numbers
void draw_letter(int x, int y, int color, char c, int scale){
	for(int col = 0; col < (5*scale); col += scale){
		for(int row = 0; row < (5*scale); row += scale){
			if(c - 'a' >= 0){
				if(letterBitMap[c - 'a'][col/scale][row/scale] == 1){
					plot_box(x + row, y + col, color, scale);
				}
			}
			else{
				if(numberStringBitMap[c - '0'][col/scale][row/scale] == 1){
					plot_box(x + row, y + col, color, scale);
				}
			}
		}
	}
}

void generateBoard(int board [9][9], bool boardChangeable [9][9], int mode){
	//srand(time(0)); // seed the random number generator
	int N = 9;
	//srand(time(0)); 
	for(int i = 0; i < 9; i++){
		for(int j = 0; j < 9; j++){
			board[i][j] = 0;
			boardChangeable[i][j] = true;
		}

	}
	
	//Fill Diagonals
	//Top left box
	for(int i = 0; i <= 2; i++){
		for(int j = 0; j <= 2; j++){
			int num = rand() % N + 1;
			while (!isValid(board, i, j, num)) {
				num = rand() % N + 1;
			}
            board[i][j] = num;
			boardChangeable[i][j] = false;
        }
	}
	// middle box
	for(int i = 3; i <= 5; i++){
		for(int j = 3; j <= 5; j++){
			int num = rand() % N + 1;
			while (!isValid(board, i, j, num)) {
				num = rand() % N + 1;
			}
            board[i][j] = num;
			boardChangeable[i][j] = false;
        }
	}
	//Bottom right box
	for(int i = 6; i <= 8; i++){
		for(int j = 6; j <= 8; j++){
			int num = rand() % N + 1;
			while (!isValid(board, i, j, num)) {
				num = rand() % N + 1;
			}
            board[i][j] = num;
			boardChangeable[i][j] = false;
        }
	}
	
	
	fillRemaining(0,3, board, boardChangeable);

	
    int hint = 0;
    if(mode == 0){
		hint = 20; // subtract 41 numbers
    }
  	 else if(mode == 1){
		hint = 51; //subtract 51 numbers
    }
   	else{
		hint = 61; //subtract 61 numbers
    }
    int count = 0;
    while (count < hint) {
        int row = rand() % N;
        int col = rand() % N;
        if (board[row][col] != 0) {
            board[row][col] = 0;
			boardChangeable[row][col] = true;
            count++;
        }
    }
	
	

}

bool fillRemaining(int col, int row, int board [9][9], bool boardChangeable[9][9]){
        // System.out.println(i+" "+j);
        if (row >= 9 && col < 9 - 1) {
            col = col + 1;
            row = 0;
        }
        if (col >= 9 && row >= 9) {
            return true;
        }
        if (col < 3) {
            if (row < 3) {
                row = 3;
            }
        }
        else if (col < 6) {
            if (row == (col/3) * 3) {
                row = row + 3;
            }
        }
        else {
            if (row == 6) {
                col = col + 1;
                row = 0;
                if (col >= 9) {
                    return true;
                }
            }
        }
        for (int num = 1; num <= 9; num++) {
            if (isValid(board, col, row, num)) {
                board[col][row] = num;
				boardChangeable[col][row] = false;
                if (fillRemaining(col, row + 1, board, boardChangeable)) {
                    return true;
                }
                board[col][row] = 0;
				boardChangeable[col][row] = true;
            }

        }

        return false;
}

void displayTime(int time, int x, int y){
	if(time != -1){
		int seconds = time % 60;
		int minutes = time / 60;

		int minuteTens = minutes / 10;
		draw_number(x, y, 0x0, minuteTens);
		x+= 17;
		int minuteOnes = minutes % 10;
		draw_number(x,y,0x0, minuteOnes);
		x+= 19;

		//Draw Colon
		plot_box(x, y+5, 0x0, 3);
		plot_box(x, y+10, 0x0, 3);
		x += 5;

		int secondTens = seconds / 10;
		draw_number(x, y, 0x0, secondTens);
		x+= 17;
		int secondOnes = seconds % 10;
		draw_number(x,y,0x0, secondOnes);
		x+= 17;
	}else{ // No time passed in, make it --:--
		x += 20;
		int scale = 3;
		plot_box(x + scale, y + (2*scale), 0x0, scale);
		plot_box(x + (2*scale), y + (2*scale), 0x0, scale);
		plot_box(x + (3*scale), y + (2*scale), 0x0, scale);
		plot_box(x + (4*scale), y + (2*scale), 0x0, scale);
		plot_box(x + (5*scale), y + (2*scale), 0x0, scale);
		plot_box(x + (6*scale), y + (2*scale), 0x0, scale);
		plot_box(x + (7*scale), y + (2*scale), 0x0, scale);
		plot_box(x + (8*scale), y + (2*scale), 0x0, scale);
		plot_box(x + (9*scale), y + (2*scale), 0x0, scale);
		plot_box(x + (10*scale), y + (2*scale), 0x0, scale);
	}
	
	
}

bool checkBoard(int board [9][9]){
        for (int i = 0; i < 9; ++i) {
        bool seen[10] = { false };
        for (int j = 0; j < 9; ++j) {
            if (board[i][j] == 0 || seen[board[i][j]]) {
                return false; // Invalid or duplicate value
            }
            seen[board[i][j]] = true;
        }
    }
    
    // Check each column
    for (int j = 0; j < 9; ++j) {
        bool seen[10] = { false };
        for (int i = 0; i < 9; ++i) {
            if (board[i][j] == 0 || seen[board[i][j]]) {
                return false; // Invalid or duplicate value
            }
            seen[board[i][j]] = true;
        }
    }
    
    // Check each 3x3 box
    for (int bx = 0; bx < 3; ++bx) {
        for (int by = 0; by < 3; ++by) {
            bool seen[10] = { false };
            for (int i = bx*3; i < bx*3+3; ++i) {
                for (int j = by*3; j < by*3+3; ++j) {
                    if (board[i][j] == 0 || seen[board[i][j]]) {
                        return false; // Invalid or duplicate value
                    }
                    seen[board[i][j]] = true;
                }
            }
        }
    }
    
    // All checks passed
    return true;
}

bool isValid(int board[9][9], int x, int y, int value) {
    // Check if value is already in row or column
    for (int i = 0; i < 9; i++) {
        if ((board[x][i] == value)|| (board[i][y] == value)) {
            return false;
        }
    }
    
    // Check if value is already in 3x3 sub-grid
    int startX = (x / 3) * 3;
    int startY = (y / 3) * 3;
    for (int i = startX; i < startX + 3; i++) {
        for (int j = startY; j < startY + 3; j++) {
            if (board[i][j] == value) {
                return false;
            }
        }
    }
    
    // If value passes all checks, it's valid
    return true;
}


void set_A9_IRQ_stack(void) {
	int stack, mode;
	stack = A9_ONCHIP_END - 7; // top of A9 onchip memory, aligned to 8 bytes /* change processor to IRQ mode with interrupts disabled */
	mode = INT_DISABLE | IRQ_MODE;
	__asm("msr cpsr, %[ps]" : : [ps] "r"(mode));
	/* set banked stack pointer */
	__asm__("mov sp, %[ps]" : : [ps] "r"(stack));
		/* go back to SVC mode before executing subroutine return! */
	mode = INT_DISABLE | SVC_MODE;
	__asm__("msr cpsr, %[ps]" : : [ps] "r"(mode));
}

/*
 * Turn on interrupts in the ARM processor
*/
void enable_A9_interrupts(void) {
int status = SVC_MODE | INT_ENABLE;
    __asm__("msr cpsr, %[ps]" : : [ps] "r"(status));
}

/* setup the KEY interrupts in the FPGA */
void config_KEYs() {
	volatile int * KEY_ptr = (int *)KEY_BASE; // pushbutton KEY address
	*(KEY_ptr + 2) = 0xF; 
}
	
/* setup the ps2 interrupts in the FPGA */
void config_ps2() {
	*((int *) PS2_BASE + 1) = 0x1; // enable interrupts for KEY[1] 
}
	
void config_a9timer(){
	*timer_ptr = 200000000;
	*(timer_ptr + 2) = 0b111;
}


void config_GIC() {
	int address; // used to calculate register addresses
	
	config_interrupt(79,1);
	config_interrupt(29,1);
	config_interrupt(73,1);
	//volatile int* ps2Controller = PS2_BASE;
	
	
	// Set Interrupt Priority Mask Register (ICCPMR). Enable interrupts of all // priorities
	address = MPCORE_GIC_CPUIF + ICCPMR;
	*((int *)address) = 0xFFFF;

	// Set CPU Interface Control Register (ICCICR). Enable signaling of // interrupts
	address = MPCORE_GIC_CPUIF + ICCICR;
	*((int *)address) = ENABLE;
	// Configure the Distributor Control Register (ICDDCR) to send pending // interrupts to CPUs
	address = MPCORE_GIC_DIST + ICDDCR;
	*((int *)address) = ENABLE;

}



// Define the IRQ exception handler
void __attribute__((interrupt)) __cs3_isr_irq(void) {
    // Read the ICCIAR from the processor interface
	int address = MPCORE_GIC_CPUIF + ICCIAR;
	int int_ID = *((int *)address);
	if (int_ID == KEYS_IRQ){ // check if interrupt is from the KEYs
		pushbutton_ISR();
	}
	else if (int_ID == 79){
		keyboard_ISR();
	}
	else if (int_ID == 29){
		privtimer_ISR();
	}
	else{
		while (1);
	}// if unexpected, then stay here
	
	// Write to the End of Interrupt Register (ICCEOIR)
	address = MPCORE_GIC_CPUIF + ICCEOIR; *((int *)address) = int_ID;
	return; 
}

void pushbutton_ISR() {
	volatile int * KEY_ptr = (int *)KEY_BASE; 
	int press;
	press = *(KEY_ptr + 3); // read the pushbutton interrupt register *(KEY_ptr + 3) = press; // Clear the interrupt
	*(KEY_ptr + 3) = press;
	if(currentScreen == 0){
		if(press == 0b0001){
			selectedRow = 0;
			selectedCol = 0;
			currentTime = 0;
			currentScreen = 1;
			mode = 0;
			lives_remaining = 3;
			generateBoard(board, boardChangeable, mode);

		}
		else if (press == 0b0010){
			selectedRow = 0;
			selectedCol = 0;
			currentTime = 0;
			currentScreen = 1;
			mode = 1;
			lives_remaining = 3;
			generateBoard(board, boardChangeable, mode);

		}
		else if (press == 0b0100){
			selectedRow = 0;
			selectedCol = 0;
			currentTime = 0;
			currentScreen = 1;
			mode = 2;
			lives_remaining = 3;
			generateBoard(board, boardChangeable, mode);

		}
	}
	else if (currentScreen == 1){
		//Update Selected Key
		if(press == 0b0001){
			bool result = checkBoard(board);
			if(result){
				currentScreen = 3;
			}
			else{
				lives_remaining -= 1;
				if(lives_remaining <= 0){
					currentScreen = 2;
				}

			}
		}
		else if (press == 0b0010){
			if(leftRight && selectedCol < 8){
				selectedCol++;
			}
			else if(!leftRight && selectedCol > 0){
				selectedCol--;
			}
		}
		else if (press == 0b0100){
			if(!upDown && selectedRow < 8){
				selectedRow++;
			}
			else if(upDown && selectedRow > 0){
				selectedRow--;
			}
		}
		else if (press == 0b1000){
			upDown = !upDown;
			leftRight = !leftRight;

		}
	}
	else if (currentScreen == 2){
		if(press != 0){
			currentScreen = 0;
		}
	}
	else if (currentScreen == 3){
		if(press != 0){
			currentScreen = 0;
		}
	}
	
}


void keyboard_ISR(){
	int PS2_DATA = *ps2_ptr;
	int validPS2 = PS2_DATA & 0x8000;
	while(validPS2){
		ps2byte1 = ps2byte2;
		ps2byte2 = ps2byte3;
		ps2byte3 = PS2_DATA & 0xFF;
		PS2_DATA = *ps2_ptr;
		validPS2 = PS2_DATA & 0x8000;
		if(currentScreen == 0){
			if(ps2byte2 == 0xF0){
				if(ps2byte1 == 0x45 && ps2byte3 == 0x45){
					selectedRow = 0;
					selectedCol = 0;
					currentTime = 0;
					currentScreen = 1;
					mode = 0;
					lives_remaining = 3;
					generateBoard(board, boardChangeable, mode);
				}
				else if(ps2byte1 == 0x16 && ps2byte3 == 0x16){
					selectedRow = 0;
					selectedCol = 0;
					currentTime = 0;
					currentScreen = 1;
					mode = 1;
					lives_remaining = 3;
					generateBoard(board, boardChangeable, mode);
				}
				else if(ps2byte1 == 0x1E && ps2byte3 == 0x1E){
					selectedRow = 0;
					selectedCol = 0;
					currentTime = 0;
					currentScreen = 1;
					mode = 2;
					lives_remaining = 3;
					generateBoard(board, boardChangeable, mode);
				}
			}
		}
		else if(currentScreen == 1){
			// Up arrow pressed
			if(ps2byte1 == 0xE0 && ps2byte2 == 0xF0 && ps2byte3 == 0x75){
				if(selectedRow > 0){
					selectedRow--;
				}
				ps2byte1 = 0x0;
				ps2byte2 = 0x0;
				ps2byte3 = 0x0;
			}
			// Down Arrow
			if(ps2byte1 == 0xE0 && ps2byte2 == 0xF0 && ps2byte3 == 0x72){
				if(selectedRow < 8){
					selectedRow++;
				}
				ps2byte1 = 0x0;
				ps2byte2 = 0x0;
				ps2byte3 = 0x0;
			}
			// Left Arrow
			if(ps2byte1 == 0xE0 && ps2byte2 == 0xF0 && ps2byte3 == 0x6B){
				if(selectedCol > 0){
					selectedCol--;
				}
				ps2byte1 = 0x0;
				ps2byte2 = 0x0;
				ps2byte3 = 0x0;
			}
			// Right Arrow
			if(ps2byte1 == 0xE0 && ps2byte2 == 0xF0 && ps2byte3 == 0x74){
				if(selectedCol < 8){
					selectedCol++;
				}
				ps2byte1 = 0x0;
				ps2byte2 = 0x0;
				ps2byte3 = 0x0;
			}

			//Possible Number or backspace
			if(ps2byte2 == 0xF0){
				if(boardChangeable[selectedCol][selectedRow] == true){
					if(ps2byte1 == 0x66 && ps2byte3 == 0x66){
						board[selectedCol][selectedRow] = 0;
					}
					else if(ps2byte1 == 0x16 && ps2byte3 == 0x16){
						board[selectedCol][selectedRow] = 1;
					}
					else if(ps2byte1 == 0x1E && ps2byte3 == 0x1E){
						board[selectedCol][selectedRow] = 2;
					}
					else if(ps2byte1 == 0x26 && ps2byte3 == 0x26){
						board[selectedCol][selectedRow] = 3;
					}
					else if(ps2byte1 == 0x25 && ps2byte3 == 0x25){
						board[selectedCol][selectedRow] = 4;
					}
					else if(ps2byte1 == 0x2E && ps2byte3 == 0x2E){
						board[selectedCol][selectedRow] = 5;
					}
					else if(ps2byte1 == 0x36 && ps2byte3 == 0x36){
						board[selectedCol][selectedRow] = 6;
					}
					else if(ps2byte1 == 0x3D && ps2byte3 == 0x3D){
						board[selectedCol][selectedRow] = 7;
					}
					else if(ps2byte1 == 0x3E && ps2byte3 == 0x3E){
						board[selectedCol][selectedRow] = 8;
					}
					else if(ps2byte1 == 0x46 && ps2byte3 == 0x46){
						board[selectedCol][selectedRow] = 9;
					}
				}
			}
		}
		
		
		else if (currentScreen == 2){
			if(ps2byte3 != 0x0){
				currentScreen = 0;
			}
		}
		else if (currentScreen == 3){
			if(ps2byte3 != 0x0){
				currentScreen = 0;
			}
		}
	}

}

void privtimer_ISR(){
	//1 second

	currentTime++;
	//Flip flicker effect
	flicker = !flicker;

	*(timer_ptr + 3) = 0x1;
}
void config_interrupt (int N, int CPU_target)
{
	int reg_offset, index, value, address;
	/* Configure the Interrupt Set-Enable Registers (ICDISERn).
	* reg_offset = (integer_div(N / 32) * 4; value = 1 << (N mod 32) */
	reg_offset = (N >> 3) & 0xFFFFFFFC;
	index = N & 0x1F;
	value = 0x1 << index;
	address = 0xFFFED100 + reg_offset;
	/* Using the address and value, set the appropriate bit */
	*(int *)address |= value;
	/* Configure the Interrupt Processor Targets Register (ICDIPTRn)
	* reg_offset = integer_div(N / 4) * 4; index = N mod 4 */
	reg_offset = (N & 0xFFFFFFFC);
	index = N & 0x3;
	address = 0xFFFED800 + reg_offset + index;
	/* Using the address and value, write to (only) the appropriate byte */
	*(char *)address = (char) CPU_target;
}
