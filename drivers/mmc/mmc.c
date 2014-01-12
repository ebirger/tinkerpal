/*-----------------------------------------------------------------------*/
/* MMC/SDC (in SPI mode) control module  (C)ChaN, 2007                   */
/*-----------------------------------------------------------------------*/
/* Only rcvr_spi(), xmit_spi(), disk_timerproc() and some macros         */
/* are platform dependent.                                               */
/*-----------------------------------------------------------------------*/

/*
 * This file was modified from a sample available from the FatFs
 * web site. It was modified to work with a Stellaris EK-LM3S6965
 * evaluation board.
 *
 * Note that the SSI port is shared with the osram display. The code
 * in this file does not attempt to share the SSI port with the osram,
 * it assumes the osram is not being used.
 *
 * File was modified to suit TinkerPal
 */

#include "util/debug.h"
#include "util/tp_types.h"
#include "main/console.h"
#include "platform/platform.h"
#include "drivers/gpio/gpio.h"
#include "drivers/spi/spi.h"
#include "drivers/block/block.h"
#include "drivers/mmc/mmc.h"

/* Definitions for MMC/SDC command */
#define CMD_GO_IDLE_STATE (0x40+0)
#define CMD_SEND_OP_COND (0x40+1)
#define CMD_SEND_IF_COND (0x40+8)
#define CMD_SEND_CSD (0x40+9)
#define CMD_SEND_CID (0x40+10)
#define CMD_STOP_TRANSMISSION (0x40+12)
#define CMD_SET_BLOCKLEN (0x40+16)
#define CMD_READ_SINGLE_BLOCK (0x40+17)
#define CMD_READ_MULTIPLE_BLOCK (0x40+18)
#define CMD_SET_BLOCK_COUNT (0x40+23)
#define CMD_WRITE_BLOCK (0x40+24)
#define CMD_WRITE_MULTIPLE_BLOCK (0x40+25)
#define CMD_SEND_OP_COND_ACMD (0x40+41)
#define CMD_APP_CMD (0x40+55)
#define CMD_READ_OCR (0x40+58)

struct mmc_t {
    resource_t spi_port;
    resource_t mosi;
    resource_t cs;
    volatile int disc_status;
};

#define CARD_MMC (1 << 0)
#define CARD_IS_MMC(c) ((c) & CARD_MMC)
#define CARD_SDC (1 << 1)
#define CARD_IS_SDC(c) ((c) & CARD_SDC)
#define CARD_BLOCK_ADDRESSING (1 << 2)
#define CARD_IS_BLOCK_ADDRESSING(c) ((c) & CARD_BLOCK_ADDRESSING)
static u8 card_type;
static mmc_t g_mmc = { 
    .disc_status = BLOCK_DISK_STATUS_NO_INIT|BLOCK_DISK_STATUS_NO_DISK 
};

/* Platform dependent functions */

static inline int TICKS(void)
{
    return platform_get_ticks_from_boot();
}

static inline void cs_low(void)
{
    /* asserts the CS pin to the card */
    gpio_digital_write(g_mmc.cs, 0);
}

static inline void cs_high(void)
{
    /* de-asserts the CS pin to the card */
    gpio_digital_write(g_mmc.cs, 1);
}

static void xmit_spi(u8 dat)
{
    spi_send(g_mmc.spi_port, dat);
}

static void xmit_spi_multi(const u8 *data, int cnt)
{
    while (cnt)
    {
	xmit_spi(*data++);
	xmit_spi(*data++);
	cnt -= 2;
    }
}

static u8 rcvr_spi(void)
{
    return (u8)spi_receive(g_mmc.spi_port);
}

static u8 wait_ready(void)
{
    u8 res;
    int expiry;

    /* Wait for ready in timeout of 500ms */
    expiry = TICKS() + 500;
    rcvr_spi();
    while (((res = rcvr_spi()) != 0xFF) && TICKS() < expiry);

    return res;
}

/* Send 80 or so clock transitions with CS and DI held high. This is
 * required after card power up to get it into SPI mode
 */
static void send_initial_clock_train(void)
{
    unsigned int i;

    cs_high();

    /* Switch the SSI TX line to a GPIO and drive it high too. */
    gpio_set_pin_mode(g_mmc.mosi, GPIO_PM_OUTPUT);
    gpio_digital_write(g_mmc.mosi, 1);

    /* Send 10 bytes over the SSI. This causes the clock to wiggle the */
    /* required number of times
     */
    for (i = 0; i < 10; i++)
	spi_send(g_mmc.spi_port, 0xFF); /* Write DUMMY data */

    /* Revert to hardware control of the SSI TX line. */
    spi_reconf(g_mmc.spi_port);
}

/* When the target system does not support socket power control, there
 * is nothing to do in these functions and chk_power always returns 1.
 */
static void power_on(void)
{
    /* This doesn't really turn the power on, but initializes the
     * SSI port and pins needed to talk to the card.
     */

    if (spi_init(g_mmc.spi_port))
	return;

    gpio_set_pin_mode(g_mmc.cs, GPIO_PM_OUTPUT);
    cs_high();

    /* Set DI and CS high and apply more than 74 pulses to SCLK for the card */
    /* to be able to accept a native command. */
    send_initial_clock_train();
}

/* Common MMC API */

/* Note: Byte count must be even number */
static int rcvr_datablock(u8 *buff, u32 byte_count)
{
    u8 token;
    int expiry;

    /* Wait for data packet in timeouts of 100ms */
    expiry = TICKS() + 100;
    while (((token = rcvr_spi()) == 0xFF) && TICKS() < expiry);

    /* Check for invalid token */
    if (token != 0xFE)
	return -1;

    do 
    {
	/* Receive the data block into buffer */
        *buff++ = rcvr_spi();
        *buff++ = rcvr_spi();
    } while (byte_count -= 2);
    rcvr_spi(); /* Discard CRC */
    rcvr_spi();
    return 0;
}

#ifndef MMC_READONLY
/* buff: 512 byte data block to be transmitted
 * token: Data/Stop token
 */
static int xmit_datablock(const u8 *buff, u8 token)
{
    if (wait_ready() != 0xFF) 
	return -1;

    xmit_spi(token);
    if (token == 0xFD) 
    {
	/* Stop token. Finished */
	return 0;
    }

    /* Data token */
    xmit_spi_multi(buff, 512);
    xmit_spi(0xFF); /* CRC (Dummy) */
    xmit_spi(0xFF);
    return ((rcvr_spi() & 0x1F) == 0x05) ? 0 : -1;
}
#endif

static u8 send_cmd(u8 cmd, u32 arg)
{
    u8 n, res;
    
    if (wait_ready() != 0xFF) 
	return 0xFF;

    /* Send command packet */
    xmit_spi(cmd); /* Command */
    xmit_spi((u8)(arg >> 24)); /* Argument[31..24] */
    xmit_spi((u8)(arg >> 16)); /* Argument[23..16] */
    xmit_spi((u8)(arg >> 8)); /* Argument[15..8] */
    xmit_spi((u8)arg); /* Argument[7..0] */
    n = 0xFF;
    if (cmd == CMD_GO_IDLE_STATE) 
	n = 0x95; /* CRC for CMD_GO_IDLE_STATE(0) */
    if (cmd == CMD_SEND_IF_COND) 
	n = 0x87; /* CRC for CMD_SEND_IF_COND(0x1AA) */
    xmit_spi(n);

    /* Receive command response */
    if (cmd == CMD_STOP_TRANSMISSION)
	rcvr_spi(); /* Skip a stuff byte when stop reading */

    /* Wait for a valid response in timeout of 10 attempts */
    n = 10;
    while (((res = rcvr_spi()) & 0x80) && --n);
    return res;
}

/*-----------------------------------------------------------------------*
 * Send the special command used to terminate a multi-sector read.
 *
 * This is the only command which can be sent while the SDCard is sending
 * data. The SDCard spec indicates that the data transfer will stop 2 bytes
 * after the 6 byte CMD_STOP_TRANSMISSION command is sent and that the card will
 * then send 0xFF for between 2 and 6 more bytes before the R1 response byte.
 * This response will be followed by another 0xFF byte.  In testing, however, 
 * it seems that some cards don't send the 2 to 6 0xFF bytes between the end of
 * data transmission and the response code.  This function, therefore, merely
 * reads 10 bytes and, if the last one read is 0xFF, returns the value of the
 * latest non-0xFF byte as the response code.
 *
 *-----------------------------------------------------------------------*/
static u8 send_cmd_stop_transmission(void)
{
    u8 n, res, val;

    /* For CMD_STOP_TRANSMISSION, we don't wait for the card to be idle before we send
     * the new command.
     */

    /* Send command packet - the argument for CMD_STOP_TRANSMISSION is ignored. */
    xmit_spi(CMD_STOP_TRANSMISSION);
    xmit_spi(0);
    xmit_spi(0);
    xmit_spi(0);
    xmit_spi(0);
    xmit_spi(0);

    /* Read up to 10 bytes from the card, remembering the value read if it's
       not 0xFF */
    for(n = 0; n < 10; n++)
    {
        val = rcvr_spi();
        if (val != 0xFF)
            res = val;
    }

    return res;
}

/* MMC CBs */
int mmc_spi_disk_init(void)
{
    u8 ty;
    int expiry;

    if (g_mmc.disc_status & BLOCK_DISK_STATUS_NO_DISK)
    { 
	/* No card in the socket */
	return g_mmc.disc_status;
    }

    /* Force socket power on */
    power_on();
    send_initial_clock_train();

    cs_low();
    ty = 0;

    if (send_cmd(CMD_GO_IDLE_STATE, 0) != 1)
	goto Exit;

    /* Enter Idle state */
    expiry = TICKS() + 1000; /* Initialization timeout of 1000 msec */
    if (send_cmd(CMD_SEND_IF_COND, 0x1AA) == 1) 
    {
	u8 n, ocr[4];

	/* SDC Ver2+ */
	for (n = 0; n < 4; n++)
	    ocr[n] = rcvr_spi();
	if (ocr[2] == 0x01 && ocr[3] == 0xAA)
	{
	    /* The card can work at vdd range of 2.7-3.6V */
	    while (TICKS() < expiry)
	    {
		if (send_cmd(CMD_APP_CMD, 0) <= 1 &&
		    send_cmd(CMD_SEND_OP_COND_ACMD, 1UL << 30) == 0)
		{
		    /* ACMD41 with HCS bit */
		    break;
		}
	    }
	    if (TICKS() >= expiry || send_cmd(CMD_READ_OCR, 0))
		goto Exit;

	    /* Check CCS bit */
	    for (n = 0; n < 4; n++)
		ocr[n] = rcvr_spi();
	    ty = CARD_SDC | ((ocr[0] & 0x40) ? CARD_BLOCK_ADDRESSING : 0);
	}
    }
    else 
    {
	/* SDC Ver1 or MMC */
	ty = (send_cmd(CMD_APP_CMD, 0) <= 1 &&
	    send_cmd(CMD_SEND_OP_COND_ACMD, 0) <= 1) ? CARD_SDC : CARD_MMC;
	while (TICKS() < expiry)
	{
	    if (ty == CARD_SDC)
	    {
		if (send_cmd(CMD_APP_CMD, 0) <= 1 &&
		    send_cmd(CMD_SEND_OP_COND_ACMD, 0) == 0)
		{
		    break;
		}
	    }
	    else
	    {
		if (send_cmd(CMD_SEND_OP_COND, 0) == 0)
		    break;
	    }
	}
	/* Select R/W block length */
	if (TICKS() >= expiry || send_cmd(CMD_SET_BLOCKLEN, 512) != 0)
	    ty = 0;
    }

Exit:
    cs_high();
    rcvr_spi(); /* Idle (Release DO) */

    if (!(card_type = ty))
	return -1;

    /* Initialization succeded */
    g_mmc.disc_status &= ~BLOCK_DISK_STATUS_NO_INIT;
    spi_set_max_speed(g_mmc.spi_port, platform.get_system_clock() / 2);
    return 0;
}

int mmc_spi_disk_status(void)
{
    return g_mmc.disc_status ? -1 : 0;
}

int mmc_spi_disk_read(u8 *buff, int sector, int count)
{
    if (!count) 
	return -1;

    if (g_mmc.disc_status & BLOCK_DISK_STATUS_NO_INIT) 
	return -1;

    if (!(CARD_IS_BLOCK_ADDRESSING(card_type)))
    { 
	/* Convert to byte address if needed */
	sector *= 512;
    }

    cs_low();

    if (count == 1)
    {
	/* Single block read */
        if (send_cmd(CMD_READ_SINGLE_BLOCK, sector) ||
	   rcvr_datablock(buff, 512))
	{
	   goto Exit;
	}

	count = 0;
    }
    else
    {
	/* Multiple block read */
	if (send_cmd(CMD_READ_MULTIPLE_BLOCK, sector)) 
	    goto Exit;

	/* Do the actual reading */
	while (count-- && !rcvr_datablock(buff += 512, 512));

	send_cmd_stop_transmission();
    }

Exit:
    cs_high();
    rcvr_spi(); /* Idle (Release DO) */

    return count ? -1 : 0;
}

#ifndef MMC_READONLY
int mmc_spi_disk_write(const u8 *buff, int sector, int count)
{
    if (!count || g_mmc.disc_status & BLOCK_DISK_STATUS_NO_INIT ||
        g_mmc.disc_status & BLOCK_DISK_STATUS_PROTECTED)
    {
	return -1;
    }

    if (!(CARD_IS_BLOCK_ADDRESSING(card_type)))
    {
	/* Convert to byte address if needed */
	sector *= 512;
    }

    cs_low();

    if (count == 1) 
    {
        /* WRITE_BLOCK */
        if (send_cmd(CMD_WRITE_BLOCK, sector) || xmit_datablock(buff, 0xFE))
	    goto Exit;

	count = 0;
    }
    else 
    {
	/* Multiple block write */
	if (CARD_IS_SDC(card_type))
	{
	    send_cmd(CMD_APP_CMD, 0); 
	    send_cmd(CMD_SET_BLOCK_COUNT, count); /* ACMD23 */
	}

	if (send_cmd(CMD_WRITE_MULTIPLE_BLOCK, sector)) 
	    goto Exit;

	/* WRITE_MULTIPLE_BLOCK */
	while (count-- && !xmit_datablock(buff += 512, 0xFC));

	/* STOP_TRAN token */
	if (xmit_datablock(0, 0xFD))
	    count = 1;
    }

Exit:
    cs_high();
    rcvr_spi(); /* Idle (Release DO) */

    return count ? -1 : 0;
}
#endif

static int mmc_spi_ioctl_get_sector_count(void *buff)
{
    u8 csd[16];
    u16 csize;

    /* Get number of sectors on the disk (u32) */
    if (send_cmd(CMD_SEND_CSD, 0) || rcvr_datablock(csd, 16))
	return -1;

    if ((csd[0] >> 6) == 1) 
    {
	/* SDC ver 2.00 */
	csize = csd[9] + ((u16)csd[8] << 8) + 1;
	*(u32 *)buff = (u32)csize << 10;
    }
    else 
    {
	u8 n;

	/* MMC or SDC ver 1.XX */
	n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
	csize = (csd[8] >> 6) + ((u16)csd[7] << 2) + 
	    ((u16)(csd[6] & 3) << 10) + 1;
	*(u32 *)buff = (u32)csize << (n - 9);
    }
    return 0;
}

int mmc_spi_disk_ioctl(int cmd, void *buff)
{
    int res = -1;

    if (g_mmc.disc_status & BLOCK_DISK_STATUS_NO_INIT) 
	return -1;

    cs_low();

    switch (cmd) 
    {
    case BLOCK_IOCTL_GET_SECTOR_COUNT:
	res = mmc_spi_ioctl_get_sector_count(buff);
	break;
    case BLOCK_IOCTL_GET_SECTOR_SIZE:
    	/* Get sectors on the disk (u16) */
	*(u16 *)buff = 512;
	res = 0;
	break;
    case BLOCK_IOCTL_SYNC:
    	/* Make sure that data has been written */
	if (wait_ready() == 0xFF)
	    res = 0;
	break;
    default:
	break;
    }

    cs_high();
    rcvr_spi(); /* Idle (Release DO) */
    return res;
}

void mmc_init(const mmc_params_t *params)
{
    g_mmc.mosi = params->mosi;
    g_mmc.cs = params->cs;
    g_mmc.spi_port = params->spi_port;
    g_mmc.disc_status = BLOCK_DISK_STATUS_NO_INIT;
}
