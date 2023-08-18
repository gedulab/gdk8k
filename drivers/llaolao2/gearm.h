#ifndef __GE_ARM_H__
#define __GE_ARM_H__

void ge_arm_sysregs(void);
int ge_arm_switch_jtag(int turn_on);
int ge_arm_enable_jtag_clk(int turn_on);
int ge_arm_read_tsadc(int channel);
int ge_iram(int para);

#endif 