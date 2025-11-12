/**
 * @brief       Driver interface for inductive sensors connected via ADC
 *
 * @author      Michel Rottleuthner <michel.rottleuthner@haw-hamburg.de>
 *
 */
#pragma once

#include "periph/adc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ANALOG_GATE_SENSOR_SUCCESS = 0,
    ANALOG_GATE_SENSOR_ADC_INIT_ERROR = -1,
    ANALOG_GATE_SENSOR_ADC_RES_ERROR = -2,
    ANALOG_GATE_SENSOR_GPIO_INIT_ERROR = -3,
} inductive_sensor_init_result_t;

typedef struct {
    uint32_t max_val;
    uint32_t adc_vref_mv;
    uint32_t sensor_vref_mv;
    gpio_t pwr_pin;
    adc_t adc_line;
    adc_res_t adc_res;
    bool pwr_pin_ah;
} inductive_sensor_t;

/*
 * @brief  Initialize inductive sensor driver.
 *
 * @param[in] dev             Inductive sensor device handle (caller allocated)
 * @param[in] pwr_pin         GPIO pin used for power gating of the sensor
 * @param[in] pwr_pin_ah      True if the power pin is active high, false if active low
 * @param[in] line            ADC line the sensor is connected to (will be initialized by this driver)
 * @param[in] adc_vfref_mv    Reference Voltage of the ADC in mV
 * @param[in] sensor_vref_mv  Reference Voltage of the connected sensor in mV
 *
 * @return                    0 on success
 * @return                    negative error otherwise (see @ref inductive_sensor_init_result_t)
 */
int inductive_sensor_init(inductive_sensor_t *dev, gpio_t pwr_pin, bool pwr_pin_ah,
                          adc_t line, uint32_t adc_vref_mv, uint32_t sensor_vref_mv);
/*
 * @brief  Control power to the inductive sensor.
 *
 * @param[in] dev             Initialized sensor device handle
 * @param[in] on              True for power on, False for power off
 *
 */
void inductive_sensor_power(const inductive_sensor_t *dev, bool on);

/*
 * @brief  Sample the inductive sensor.
 *
 * @param[in] dev             Initialized sensor device handle
 *
 * @return                    The raw ADC value
 *
 */
uint32_t inductive_sensor_sample(const inductive_sensor_t *dev);

/*
 * @brief  Convert a raw sample to the voltage at the ADC line.
 *
 * @param[in] dev             Initialized sensor device handle
 * @param[in] sample          A raw sample obtained from @ref inductive_sensor_sample()
 *
 * @return                    The corresponding voltage at the ADC line
 *
 */
uint32_t inductive_sensor_sample2adc_voltage(const inductive_sensor_t *dev, uint32_t sample);

/*
 * @brief  Convert a raw sample to the voltage at the sensor output.
 *
 * @param[in] dev             Initialized sensor device handle
 * @param[in] sample          A raw sample obtained from @ref inductive_sensor_sample()
 *
 * @return                    The corresponding voltage at the sensor output
 *
 */
uint32_t inductive_sensor_sample2sensor_voltage(const inductive_sensor_t *dev, uint32_t sample);

#ifdef __cplusplus
}
#endif
