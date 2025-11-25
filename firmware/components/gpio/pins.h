#ifndef PINS_H
#define PINS_H

constexpr bool DRIP_GPIO_PIN_ENABLE = true;
constexpr bool DRIP_GPIO_PIN_DISABLE = false;

constexpr gpio_num_t DRIP_GPIO_PIN_I2C_SDA = GPIO_NUM_0;
constexpr gpio_num_t DRIP_GPIO_PIN_I2C_SCL = GPIO_NUM_0;

constexpr gpio_num_t DRIP_GPIO_PIN_RELAY1 = GPIO_NUM_0;
constexpr gpio_num_t DRIP_GPIO_PIN_RELAY2 = GPIO_NUM_0;
constexpr gpio_num_t DRIP_GPIO_PIN_RELAY3 = GPIO_NUM_0;

/** Addr pin pulled to GND. */
#define ADS111RUGT_I2C_ADDR 0b1001000U
#define ADS111RUGT_I2C_CLK_FREQ_HZ 50000U

#endif
