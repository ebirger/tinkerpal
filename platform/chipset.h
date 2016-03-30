#ifndef CHIPSET_START
#define CHIPSET_START(...)
#endif

/* UARTS */
#ifndef TI_UART_DEF
#define TI_UART_DEF(...)
#endif
#ifndef STM32_USART_DEF
#define STM32_USART_DEF(...)
#endif
#ifndef MSP430_USCI_DEF
#define MSP430_USCI_DEF(...)
#endif

/* SPI */
#ifndef TI_SSI_DEF
#define TI_SSI_DEF(...)
#endif
#ifndef STM32_SPI_DEF
#define STM32_SPI_DEF(...)
#endif

/* I2C */
#ifndef TI_I2C_DEF
#define TI_I2C_DEF(...)
#endif
#ifndef STM32_I2C_DEF
#define STM32_I2C_DEF(...)
#endif

/* PWM */
#ifndef TI_PWM_DEF
#define TI_PWM_DEF(...)
#endif

#ifndef CHIPSET_END
#define CHIPSET_END(...)
#endif

#include PLATFORM_CHIPSET_H

#undef CHIPSET_START
#undef TI_UART_DEF
#undef STM32_USART_DEF
#undef MSP430_USCI_DEF
#undef TI_SSI_DEF
#undef STM32_SPI_DEF
#undef TI_I2C_DEF
#undef STM32_I2C_DEF
#undef TI_PWM_DEF
#undef CHIPSET_END
