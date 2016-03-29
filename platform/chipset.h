#ifndef CHIPSET_START
#define CHIPSET_START(...)
#endif

/* UARTS */
#ifndef UART_DEF
#define UART_DEF(...)
#endif
#ifndef USART_DEF
#define USART_DEF(...)
#endif

/* SPI */
#ifndef SSI_DEF
#define SSI_DEF(...)
#endif
#ifndef SPI_DEF
#define SPI_DEF(...)
#endif

/* I2C */
#ifndef I2C_DEF
#define I2C_DEF(...)
#endif

/* PWM */
#ifndef PWM_DEF
#define PWM_DEF(...)
#endif

#ifndef CHIPSET_END
#define CHIPSET_END(...)
#endif

#include PLATFORM_CHIPSET_H

#undef CHIPSET_START
#undef UART_DEF
#undef USART_DEF
#undef SSI_DEF
#undef SPI_DEF
#undef I2C_DEF
#undef PWM_DEF
#undef CHIPSET_END
