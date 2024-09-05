#ifndef CC1101_H
#define CC1101_H

#include <stdint.h>
#include <stdbool.h>

// SPI 
#define CC1101_CS_PIN     5   //Orange Chip Select (CS) pin (GPIO 5)
#define CC1101_GDO0_PIN   3   //Yellow GDO0 pin (GPIO 3)
#define CC1101_SCLK_PIN   6   //Blue SPI Clock (SCK) pin (GPIO 6)
#define CC1101_MOSI_PIN   7   //Green SPI Master Out Slave In (MOSI) pin (GPIO 7)
#define CC1101_MISO_PIN   4   //Purple SPI Master In Slave Out (MISO) pin (GPIO 4)

#define CC1101_MAX_PAYLOAD_LENGTH 42   // Maximum length of payload

// CC1101 Command Strobes
#define CC1101_SRES          0x30  // Reset chip
#define CC1101_SFSTXON       0x31  // Enable and calibrate frequency synthesizer (if MCSM0.FS_AUTOCAL=1). If in RX/TX: Go to a wait state where only the synthesizer is running (for quick RX / TX turnaround).
#define CC1101_SXOFF         0x32  // Turn off crystal oscillator
#define CC1101_SCAL          0x33  // Calibrate frequency synthesizer and turn it off (enables quick start)
#define CC1101_SRX           0x34  // Enable RX. Perform calibration first if coming from IDLE and MCSM0.FS_AUTOCAL=1
#define CC1101_STX           0x35  // In IDLE state: Enable TX. Perform calibration first if MCSM0.FS_AUTOCAL=1. If in RX state and CCA is enabled: Only go to TX if channel is clear
#define CC1101_SIDLE         0x36  // Exit RX / TX, turn off frequency synthesizer and exit Wake-On-Radio mode if applicable
#define CC1101_SWOR          0x38  // Start automatic RX polling sequence (Wake-on-Radio)
#define CC1101_SPWD          0x39  // Enter power down mode when CSn goes high
#define CC1101_SFRX          0x3A  // Flush the RX FIFO buffer
#define CC1101_SFTX          0x3B  // Flush the TX FIFO buffer
#define CC1101_SWORRST       0x3C  // Reset real-time clock
#define CC1101_SNOP          0x3D  // No operation. May be used to get access to the chip status byte

// CC1101 Configuration Registers
#define CC1101_IOCFG2        0x00  // GDO2 output pin configuration
#define CC1101_IOCFG1        0x01  // GDO1 output pin configuration
#define CC1101_IOCFG0        0x02  // GDO0 output pin configuration
#define CC1101_FIFOTHR       0x03  // RX FIFO and TX FIFO thresholds
#define CC1101_SYNC1         0x04  // Sync word, high byte
#define CC1101_SYNC0         0x05  // Sync word, low byte
#define CC1101_PKTLEN        0x06  // Packet length
#define CC1101_PKTCTRL1      0x07  // Packet automation control
#define CC1101_PKTCTRL0      0x08  // Packet automation control
#define CC1101_ADDR          0x09  // Device address
#define CC1101_CHANNR        0x0A  // Channel number
#define CC1101_FSCTRL1       0x0B  // Frequency synthesizer control
#define CC1101_FSCTRL0       0x0C  // Frequency synthesizer control
#define CC1101_FREQ2         0x0D  // Frequency control word, high byte
#define CC1101_FREQ1         0x0E  // Frequency control word, middle byte
#define CC1101_FREQ0         0x0F  // Frequency control word, low byte
#define CC1101_MDMCFG4       0x10  // Modem configuration
#define CC1101_MDMCFG3       0x11  // Modem configuration
#define CC1101_MDMCFG2       0x12  // Modem configuration
#define CC1101_MDMCFG1       0x13  // Modem configuration
#define CC1101_MDMCFG0       0x14  // Modem configuration
#define CC1101_DEVIATN       0x15  // Modem deviation setting
#define CC1101_MCSM2         0x16  // Main Radio Control State Machine configuration
#define CC1101_MCSM1         0x17  // Main Radio Control State Machine configuration
#define CC1101_MCSM0         0x18  // Main Radio Control State Machine configuration
#define CC1101_FOCCFG        0x19  // Frequency Offset Compensation configuration
#define CC1101_BSCFG         0x1A  // Bit Synchronization configuration
#define CC1101_AGCCTRL2      0x1B  // AGC control
#define CC1101_AGCCTRL1      0x1C  // AGC control
#define CC1101_AGCCTRL0      0x1D  // AGC control
#define CC1101_WOREVT1       0x1E  // High byte Event0 timeout
#define CC1101_WOREVT0       0x1F  // Low byte Event0 timeout
#define CC1101_WORCTRL       0x20  // Wake On Radio control
#define CC1101_FREND1        0x21  // Front end RX configuration
#define CC1101_FREND0        0x22  // Front end TX configuration
#define CC1101_FSCAL3        0x23  // Frequency synthesizer calibration
#define CC1101_FSCAL2        0x24  // Frequency synthesizer calibration
#define CC1101_FSCAL1        0x25  // Frequency synthesizer calibration
#define CC1101_FSCAL0        0x26  // Frequency synthesizer calibration
#define CC1101_RCCTRL1       0x27  // RC oscillator configuration
#define CC1101_RCCTRL0       0x28  // RC oscillator configuration
#define CC1101_FSTEST        0x29  // Frequency synthesizer test
#define CC1101_PTEST         0x2A  // Preamble test
#define CC1101_AGCTEST       0x2B  // AGC test
#define CC1101_TEST2         0x2C  // Various test settings
#define CC1101_TEST1         0x2D  // Various test settings
#define CC1101_TEST0         0x2E  // Various test settings
#define CC1101_PARTNUM       0x30  // Part number
#define CC1101_VERSION       0x31  // Current version number
#define CC1101_FREQEST       0x32  // Frequency offset estimate
#define CC1101_LQI           0x33  // Demodulator LQI
#define CC1101_RSSI          0x34  // RSSI value
#define CC1101_MARCSTATE     0x35  // Control state machine state
#define CC1101_WORTIME1      0x36  // High byte of WOR timer
#define CC1101_WORTIME0      0x37  // Low byte of WOR timer
#define CC1101_PKTSTATUS     0x38  // Current GDOx status and auto RSSI threshold flag
#define CC1101_VCO_VC_DAC    0x39  // VCO power control
#define CC1101_TXBYTES       0x3A  // Underflow and number of bytes
#define CC1101_RXBYTES       0x3B  // Underflow and number of bytes

#define CC1101_TXFIFO_SINGLE_BYTE 0x3F  // Single byte access to TX FIFO
#define CC1101_TXFIFO_BURST 0x7F       // Burst access to TX FIFO
#define CC1101_RXFIFO_SINGLE_BYTE 0xBF // Single byte access to RX FIFO
#define CC1101_RXFIFO_BURST 0xFF       // Burst access to RX FIFO

// Prototypes
void cc1101_init(void);
void cc1101_write_reg(uint8_t addr, uint8_t value);
uint8_t cc1101_read_reg(uint8_t addr);
void cc1101_send_data(uint8_t* data, uint8_t length, uint8_t address);
void cc1101_receive_data(uint8_t* buffer, uint8_t length);
void cc1101_strobe(uint8_t strobe);
void cc1101_reset(void);
void cc1101_signal_strength(void);

#endif // CC1101_H
