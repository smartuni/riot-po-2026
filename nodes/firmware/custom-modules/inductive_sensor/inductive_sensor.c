/**
 * @brief       Driver for inductive sensors connected via ADC
 *
 * @author      Michel Rottleuthner <michel.rottleuthner@haw-hamburg.de>
 *
 */
#include "periph/gpio.h"
#include "periph/adc.h"
#include "inductive_sensor.h"

typedef struct {
    adc_res_t res;
    uint32_t max_val;
} adc_res2max_t;

/* list of resolutions to try and the corresponding max sample value.
 * Must be ordered with the highest resolution first.
 * The driver will use the highest possible resolution automatically */
adc_res2max_t res_confs[] = {
    { .res = ADC_RES_16BIT, .max_val = ((1<<16) - 1) },
    { .res = ADC_RES_14BIT, .max_val = ((1<<14) - 1) },
    { .res = ADC_RES_12BIT, .max_val = ((1<<12) - 1) },
    { .res = ADC_RES_10BIT, .max_val = ((1<<10) - 1) },
    { .res = ADC_RES_8BIT,  .max_val = ((1<<8 ) - 1) },
    { .res = ADC_RES_6BIT,  .max_val = ((1<<6 ) - 1) },
};

int inductive_sensor_init(inductive_sensor_t *dev, gpio_t pwr_pin, bool pwr_pin_ah,
                          adc_t line, uint32_t adc_vref_mv, uint32_t sensor_vref_mv)
{
    int res = adc_init(line);
    if (res != 0) {
        return ANALOG_GATE_SENSOR_ADC_INIT_ERROR;
    }
    dev->adc_line = line;

    /* init power control pin and disable power by default */
    res = gpio_init(pwr_pin, GPIO_OUT);
    if (res != 0) {
        return ANALOG_GATE_SENSOR_GPIO_INIT_ERROR;
    }

    gpio_write(pwr_pin_ah, !pwr_pin_ah);
    dev->pwr_pin = pwr_pin;
    dev->pwr_pin_ah = pwr_pin_ah;
    dev->adc_vref_mv = adc_vref_mv;
    dev->sensor_vref_mv = sensor_vref_mv;

    for (unsigned i = 0; i < ARRAY_SIZE(res_confs); i++) {
        if (adc_sample(dev->adc_line, res_confs[i].res) >= 0) {
            dev->adc_res = res_confs[i].res;
            dev->max_val = res_confs[i].max_val;
            return ANALOG_GATE_SENSOR_SUCCESS;
        }
    }

    return ANALOG_GATE_SENSOR_ADC_RES_ERROR;
}

void inductive_sensor_power(const inductive_sensor_t *dev, bool on)
{
    if (dev->pwr_pin_ah) {
        gpio_write(dev->pwr_pin, on);
    } else {
        gpio_write(dev->pwr_pin, !on);
    }
}

uint32_t inductive_sensor_sample(const inductive_sensor_t *dev)
{
    int32_t sample = adc_sample(dev->adc_line, dev->adc_res);
    return sample;
}

uint32_t inductive_sensor_sample2adc_voltage(const inductive_sensor_t *dev, uint32_t sample)
{
    return sample * dev->adc_vref_mv / dev->max_val;
}

uint32_t inductive_sensor_sample2sensor_voltage(const inductive_sensor_t *dev, uint32_t sample)
{
    return sample * dev->sensor_vref_mv / dev->max_val;
}
