# ifndef PINOUT_HPP
# define PINOUT_HPP

#define SPI_SCK   12 // Shared with Display
#define SPI_MISO  13 // Used by LIS3DH
#define SPI_MOSI  11 // Shared with Display

#define LIS3DH_CS 10 // Dedicated CS for LIS3DH

#define GPS_RX_PIN 37
#define GPS_TX_PIN 36
#define GPS_FORCE_ON_PIN 35

#define WIND_RX_PIN  16  // Connects to SP3485 RX-I
#define WIND_TX_PIN  15  // Connects to SP3485 TX-O
#define WIND_RTS_PIN 6   // Connects to SP3485 RTS (Transmit/Receive Control)

#define LCD_CS 5
#define LCD_DC 9
#define LCD_RES 4

# endif