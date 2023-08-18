#include <asm/hwcap.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <asm/io.h>

#define rd_arm_reg(id) ({                                    \
              unsigned long __val;                            \
              asm("mrs %0, "#id : "=r" (__val));              \
              printk("%-20s: 0x%016lx\n", #id, __val);        \
      })
void ge_arm_sysregs(void)
{
      rd_arm_reg(ID_AA64ISAR0_EL1);
      rd_arm_reg(ID_AA64ISAR1_EL1);
      rd_arm_reg(ID_AA64MMFR0_EL1);
      rd_arm_reg(ID_AA64MMFR1_EL1);
      rd_arm_reg(ID_AA64PFR0_EL1);
      rd_arm_reg(ID_AA64PFR1_EL1);
      rd_arm_reg(ID_AA64DFR0_EL1);
      rd_arm_reg(ID_AA64DFR1_EL1);

      rd_arm_reg(MIDR_EL1);
      rd_arm_reg(MPIDR_EL1);
      rd_arm_reg(REVIDR_EL1);
      rd_arm_reg(VBAR_EL1);
      rd_arm_reg(TPIDR_EL1);
      rd_arm_reg(SP_EL0);
      /* Unexposed register access causes SIGILL */
      rd_arm_reg(ID_MMFR0_EL1);
}

// switch JTAG signal for GDK8
#define GDK8_SYSCON_GRF_BASE    0xff100000
#define GDK8_SYSCON_GRF_SIZE    0x1000 // 1 page
#define RK3328_GRF_SOC_CON4		0x410
#define RK3328_GRF_GPIO1A_IOMUX 0x10

#define GRF_HIWORD_UPDATE(val, mask, shift) \
		((val) << (shift) | (mask) << ((shift) + 16))

int ge_arm_switch_jtag(int turn_on)
{
    int value;
    void* base = ioremap_nocache(GDK8_SYSCON_GRF_BASE, GDK8_SYSCON_GRF_SIZE);
    if (base == NULL) {
        printk(KERN_ERR "failed to map GRF memory at %x\n", GDK8_SYSCON_GRF_BASE);
        return -1;
    }
    value = readl(base + RK3328_GRF_SOC_CON4);
    printk("current value of SOC_CON4 = %x\n", value);
    value = GRF_HIWORD_UPDATE((turn_on == 0 ? 0 : 1), 1, 12);
    printk("value is set to = %x\n", value);
    writel(value, base + RK3328_GRF_SOC_CON4);
    value = readl(base + RK3328_GRF_SOC_CON4);
    printk("current value of SOC_CON4 = %x\n", value);
    //
    if(turn_on) 
    {
        value = readl(base + RK3328_GRF_GPIO1A_IOMUX);
        printk("current value of GPIO1A_IOMUX = %x\n", value);
        value = GRF_HIWORD_UPDATE(0, 1, 4); // bit 4 0
        value |= GRF_HIWORD_UPDATE(1, 1, 5); // bit 5 1
        value |= GRF_HIWORD_UPDATE(0, 1, 6); // bit 6 0
        value |= GRF_HIWORD_UPDATE(1, 1, 7); // bit 7 1
        printk("value is set to = %x\n", value);
        writel(value, base + RK3328_GRF_GPIO1A_IOMUX);
        value = readl(base + RK3328_GRF_GPIO1A_IOMUX);
        printk("current value of GPIO1A_IOMUX = %x\n", value);
    }
    iounmap(base);

    return 0;
}

#define GDK8_CRU_BASE 0xff440000
#define RK3328_CRU_CLKGATE_CON7 0x21c
int ge_arm_enable_jtag_clk(int turn_on)
{
    int value;
    void* base = ioremap_nocache(GDK8_CRU_BASE, GDK8_SYSCON_GRF_SIZE);
    if (base == NULL) {
        printk(KERN_ERR "failed to map CRU memory at %x\n", GDK8_CRU_BASE);
        return -1;
    }
    value = readl(base + RK3328_CRU_CLKGATE_CON7);
    printk("current value of CLKGATE_CON7 = %x\n", value);
    value = GRF_HIWORD_UPDATE((turn_on == 0 ? 1 : 0), 1, 2);
    printk("value is set to = %x\n", value);
    writel(value, base + RK3328_CRU_CLKGATE_CON7);
    value = readl(base + RK3328_CRU_CLKGATE_CON7);
    printk("current value of CLKGATE_CON7 = %x\n", value);

    iounmap(base);

    return 0;
}
/**
 * The system has two Temperature Sensors.
 * sensor0 is for CPU, and sensor1 is for GPU.
 */
enum sensor_id {
    SENSOR_CPU = 0,
    SENSOR_GPU,
};

/**
 * The conversion table has the adc value and temperature.
 * ADC_DECREMENT: the adc value is of diminishing.(e.g. rk3288_code_table)
 * ADC_INCREMENT: the adc value is incremental.(e.g. rk3368_code_table)
 */
enum adc_sort_mode {
    ADC_DECREMENT = 0,
    ADC_INCREMENT,
};

/**
 * The max sensors is two in rockchip SoCs.
 * Two sensors: CPU and GPU sensor.
 */
#define SOC_MAX_SENSORS	2

 /**
  * struct chip_tsadc_table - hold information about chip-specific differences
  * @id: conversion table
  * @length: size of conversion table
  * @data_mask: mask to apply on data inputs
  * @mode: sort mode of this adc variant (incrementing or decrementing)
  */
struct chip_tsadc_table {
    const struct tsadc_table* id;
    unsigned int length;
    u32 data_mask;
    enum adc_sort_mode mode;
};
/**
 * struct tsadc_table - code to temperature conversion table
 * @code: the value of adc channel
 * @temp: the temperature
 * Note:
 * code to temperature mapping of the temperature sensor is a piece wise linear
 * curve.Any temperature, code faling between to 2 give temperatures can be
 * linearly interpolated.
 * Code to Temperature mapping should be updated based on manufacturer results.
 */
struct tsadc_table {
    u32 code;
    int temp; // /* millicelsius */
};
#define TSADCV2_DATA_MASK			0xfff
static const struct tsadc_table rk3328_code_table[] = {
    {0, -40000},
    {296, -40000},
    {304, -35000},
    {313, -30000},
    {331, -20000},
    {340, -15000},
    {349, -10000},
    {359, -5000},
    {368, 0},
    {378, 5000},
    {388, 10000},
    {398, 15000},
    {408, 20000},
    {418, 25000},
    {429, 30000},
    {440, 35000},
    {451, 40000},
    {462, 45000},
    {473, 50000},
    {485, 55000},
    {496, 60000},
    {508, 65000},
    {521, 70000},
    {533, 75000},
    {546, 80000},
    {559, 85000},
    {572, 90000},
    {586, 95000},
    {600, 100000},
    {614, 105000},
    {629, 110000},
    {644, 115000},
    {659, 120000},
    {675, 125000},
    {TSADCV2_DATA_MASK, 125000},
};
static int rk_tsadcv2_code_to_temp(struct chip_tsadc_table* table, u32 code,
    int* temp)
{
    unsigned int low = 1;
    unsigned int high = table->length - 1;
    unsigned int mid = (low + high) / 2;
    unsigned int num;
    unsigned long denom;

    WARN_ON(table->length < 2);

    switch (table->mode) {
    case ADC_DECREMENT:
        code &= table->data_mask;
        if (code < table->id[high].code)
            return -EAGAIN;		/* Incorrect reading */

        while (low <= high) {
            if (code >= table->id[mid].code &&
                code < table->id[mid - 1].code)
                break;
            else if (code < table->id[mid].code)
                low = mid + 1;
            else
                high = mid - 1;

            mid = (low + high) / 2;
        }
        break;
    case ADC_INCREMENT:
        code &= table->data_mask;
        if (code < table->id[low].code)
            return -EAGAIN;		/* Incorrect reading */

        while (low <= high) {
            if (code <= table->id[mid].code &&
                code > table->id[mid - 1].code)
                break;
            else if (code > table->id[mid].code)
                low = mid + 1;
            else
                high = mid - 1;

            mid = (low + high) / 2;
        }
        break;
    default:
        pr_err("%s: Invalid the conversion table mode=%d\n",
            __func__, table->mode);
    }

    /*
     * The 5C granularity provided by the table is too much. Let's
     * assume that the relationship between sensor readings and
     * temperature between 2 table entries is linear and interpolate
     * to produce less granular result.
     */
    num = table->id[mid].temp - table->id[mid - 1].temp;
    num *= abs(table->id[mid - 1].code - code);
    denom = abs(table->id[mid - 1].code - table->id[mid].code);
    *temp = table->id[mid - 1].temp + (num / denom);

    return 0;
}
//channel0: CPU temperature
//channel1: GPU temperature
// read GRF for the temperature sensor
#define GDK8_TSADC_BASE             0xff250000
#define GDK8_TSADC_GRF_SIZE         0x1000 // 4KB

#define TSADCV2_USER_CON			0x00
#define TSADCV2_AUTO_CON			0x04
#define TSADCV2_INT_EN				0x08
#define TSADCV2_INT_PD				0x0c
#define TSADCV2_DATA(chn)			(0x20 + (chn) * 0x04)
#define TSADCV2_COMP_INT(chn)		        (0x30 + (chn) * 0x04)
#define TSADCV2_COMP_SHUT(chn)		        (0x40 + (chn) * 0x04)
#define TSADCV2_HIGHT_INT_DEBOUNCE		0x60
#define TSADCV2_HIGHT_TSHUT_DEBOUNCE		0x64
#define TSADCV2_AUTO_PERIOD			0x68
#define TSADCV2_AUTO_PERIOD_HT			0x6c
struct chip_tsadc_table rk3328_therm_table = {
    .id = rk3328_code_table,
    .length = ARRAY_SIZE(rk3328_code_table),
    .data_mask = TSADCV2_DATA_MASK,
    .mode = ADC_INCREMENT,
};

int ge_arm_read_tsadc(int channel)
{
    int val = 0, reads, ret;
    void* base;

    if ((channel != 0) && (channel != 1))
    {
        printk("bad channel no.\n");
        return -1;
    }
    base = ioremap_nocache(GDK8_TSADC_BASE, GDK8_TSADC_GRF_SIZE);
    if (base == NULL) {
        printk(KERN_ERR "failed to map TSADC GRF at %x\n", GDK8_TSADC_BASE);
        return -1;
    }
    reads = readl(base + TSADCV2_DATA(channel));
    ret = rk_tsadcv2_code_to_temp(&rk3328_therm_table, reads, &val);
    if (ret != 0) {
        printk("bad reading %d\n", ret);
        return ret;
    }

    //d = (int)(0.5823 * (float)reads-273.62); //y = 0.5823x - 273.62

    printk("tsadc_temp[%d] = %d 0x%x\n", channel, val, reads);

    return val;
}

#define GDK8_IRAM_BASE 0xff090000
#define GDK8_IRAM_SIZE 0x9000 
#define GDK8_NDB_NOTE_OFFSET 0x10
int ge_iram(int para)
{
    void* base;
    int reads;

    base = ioremap_nocache(GDK8_IRAM_BASE + 0x8000, 0x1000/*GDK8_IRAM_SIZE*/);
    if (base == NULL) {
        printk(KERN_ERR "failed to map IRAM at %x\n", GDK8_IRAM_BASE);
        return -1;
    }
    reads = readl(base + GDK8_NDB_NOTE_OFFSET);
    printk(KERN_ERR "GDK8_NDB_NOTE %x\n", reads);
    if (para >= 0x888) {
        writel(para + 0x88880000, base + GDK8_NDB_NOTE_OFFSET);
    }

    iounmap(base);

    return 0;
}