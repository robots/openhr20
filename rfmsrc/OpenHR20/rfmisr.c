
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "task.h"

#include "config.h"
#include "rfm_config.h"
#include "../common/rfm.h"

#if RFM
void RFM_isr(void)
{
	if (RFM_PCMSK & _BV(RFM_SDO_PCINT)) {
		// RFM module interupt
		while (RFM_SDO_PIN & _BV(RFM_SDO_BITPOS)) {
			RFM_PCMSK &= ~_BV(RFM_SDO_PCINT); // disable RFM interrupt
			sei(); // enable global interrupts
			if (rfm_mode == rfmmode_tx) {
				RFM_WRITE(rfm_framebuf[rfm_framepos++]);
				if (rfm_framepos >= rfm_framesize) {
					rfm_mode = rfmmode_tx_done;
					task |= TASK_RFM; // inform the rfm task about end of transmition
					return; // \note !!WARNING!!
				}
			} else if (rfm_mode == rfmmode_rx) {
				rfm_framebuf[rfm_framepos++]=RFM_READ_FIFO();
				task |= TASK_RFM; // inform the rfm task about next RX byte
				if (rfm_framepos >= RFM_FRAME_MAX) {
					rfm_mode = rfmmode_rx_owf;
					// ignore any data in buffer
					return; // RFM interrupt disabled 
				}
			} 
			cli(); // disable global interrupts
			asm volatile("nop"); // we must have one instruction after cli() 
			RFM_PCMSK |= _BV(RFM_SDO_PCINT); // enable RFM interrupt
			asm volatile("nop"); // we must have one instruction after
		}
	}
}
#endif
