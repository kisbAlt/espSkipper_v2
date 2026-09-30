#include "Osptek_BWR_42.h"

OsptekBWR::OsptekBWR(int8_t cs, int8_t dc, int8_t rst, int8_t te, int8_t led) 
    : _cs(cs), _dc(dc), _rst(rst), _te(te), _led(led) {}

void OsptekBWR::begin(bool highSpeedMode) {
    _hpm_mode = highSpeedMode;

    pinMode(_cs, OUTPUT);
    pinMode(_dc, OUTPUT);
    pinMode(_rst, OUTPUT);
    if (_te >= 0) pinMode(_te, INPUT);
    if (_led >= 0) pinMode(_led, OUTPUT);

    digitalWrite(_cs, HIGH);

    freq = _hpm_mode ? 20000000 : 10000000;

    initRegs();
}

void OsptekBWR::writeCmd(uint8_t cmd) {
    SPI.beginTransaction(SPISettings(freq, MSBFIRST, SPI_MODE0));
    digitalWrite(_dc, LOW);
    digitalWrite(_cs, LOW);
    SPI.transfer(cmd);
    digitalWrite(_cs, HIGH);
    SPI.endTransaction();
}

void OsptekBWR::writeData(uint8_t data) {
    SPI.beginTransaction(SPISettings(freq, MSBFIRST, SPI_MODE0));
    digitalWrite(_dc, HIGH);
    digitalWrite(_cs, LOW);
    SPI.transfer(data);
    digitalWrite(_cs, HIGH);
    SPI.endTransaction();
}

void OsptekBWR::reset() {
    digitalWrite(_rst, HIGH); delay(20);
    digitalWrite(_rst, LOW);  delay(50);
    digitalWrite(_rst, HIGH); delay(150);
}

void OsptekBWR::initRegs() {
    reset();

    // --- ST7306 Initialization Sequence ---
    writeCmd(0xD6); writeData(0x17); writeData(0x02); // NVM Load
    writeCmd(0xD1); writeData(0x01); // Booster Enable
    writeCmd(0xC0); writeData(0x08); writeData(0x00); // Gate Voltages

    // Source Voltages (Critical for Color/BW separation)
    writeCmd(0xC1); writeData(0x2C); writeData(0x2C); writeData(0x2C); writeData(0x2C);
    writeCmd(0xC2); writeData(0x26); writeData(0x26); writeData(0x26); writeData(0x26);
    writeCmd(0xC4); writeData(0x42); writeData(0x42); writeData(0x42); writeData(0x42);
    writeCmd(0xC5); writeData(0x32); writeData(0x32); writeData(0x32); writeData(0x32);

    // Maximum refresh rate: 8Hz (LMP), 16Hz (HPM)
    if (_hpm_mode) {
        // 16Hz Config
        writeCmd(0xD8); writeData(0xA6); writeData(0xE9); // OSC Set (111)
        writeCmd(0xB2); writeData(0x02); // Frame Rate 16Hz //
    } else {
        // 1Hz Config
        writeCmd(0xB2); writeData(0x12); 
    }

    // Waveforms (HPM & LPM)
    writeCmd(0xB3); writeData(0xE5); writeData(0xF6); writeData(0x05); writeData(0x46); writeData(0x77); writeData(0x77); writeData(0x77); writeData(0x77); writeData(0x76); writeData(0x45);
    writeCmd(0xB4); writeData(0x05); writeData(0x46); writeData(0x77); writeData(0x77); writeData(0x77); writeData(0x77); writeData(0x77); writeData(0x76); writeData(0x45);

    writeCmd(0xB7); writeData(0x13);
    writeCmd(0xB0); writeData(0x64); // Gate Line: 400
    writeCmd(0xC9); writeData(0x00); // Source Select

    writeCmd(0x11); delay(255); // Sleep Out

    writeCmd(0x36); 
    // 0x00 -> Normal vertical direction
    // 0x80 -> Set MY bit (Bit 7) for Vertical Flip (Upside Down fix)
    writeData(0x00); // Start refreshing from the position far away from the fpc.
    writeCmd(0x3A); writeData(0x11); // 24-bit Interface (Mapped to 2bpp internally)
    
    // Gamma: 0x20 (Mono) works best with Nibble logic
    writeCmd(0xB9); writeData(0x20); 
    
    // Panel: 0x09 (One Line Interval / Interlaced)
    writeCmd(0xB8); writeData(0x09); 

    writeCmd(0xD0); writeData(0xFF);
    writeCmd(0x72); writeData(0x13);
    
    // Power Mode
    writeCmd(0x39); 
    
    writeCmd(0x29); // Display On
}

void OsptekBWR::clear(OspColor color) {
    // Pack two pixels (nibbles) into one byte
    uint8_t packed = (color << 4) | color;
    memset(_buffer, packed, OSP_BUFFER_SIZE);
}

// --- Graphics Core ---
// Maps (x,y) to the buffer. Handles 2:1 vertical interlacing logical mapping.
void OsptekBWR::drawPixel(int16_t x, int16_t y, OspColor color) {
    // Fix the horizontal mirror from hardware register 0x00
    x = (OSP_LCD_WIDTH - 1) - x;

    // Original boundary check
    if (x < 0 || x >= OSP_LCD_WIDTH || y < 0 || y >= OSP_LCD_HEIGHT) return;

    // Convert physical Y to logical Y (Interlaced: 2 physical lines = 1 data unit)
    int16_t logical_y = y / 2; 

    // Buffer Address Calculation
    uint32_t index = (logical_y * 150) + (x / 2);

    if (index >= OSP_BUFFER_SIZE) return;

    // Modify High or Low Nibble
    if (x % 2 == 0) {
        _buffer[index] = (_buffer[index] & 0x0F) | (color << 4);
    } else {
        _buffer[index] = (_buffer[index] & 0xF0) | (color & 0x0F);
    }
}

void OsptekBWR::update() {
    // Set Window 0x05 -> 0x36 (50 units width)
    writeCmd(0x2A); writeData(0x05); writeData(0x36);
    // Set Window 0x00 -> 0xC7 (200 units height)
    writeCmd(0x2B); writeData(0x00); writeData(0xC7);
    
    writeCmd(0x2C); // Write Start

    // START TRANSACTION FOR BUFFER DUMP
    uint32_t freq = _hpm_mode ? 20000000 : 10000000;
    SPI.beginTransaction(SPISettings(freq, MSBFIRST, SPI_MODE0));

    digitalWrite(_dc, HIGH);
    digitalWrite(_cs, LOW);
    
    // Transfer entire buffer
    uint16_t chunkSize = 512;
    for (uint32_t i = 0; i < OSP_BUFFER_SIZE; i += chunkSize) {
        uint32_t remain = OSP_BUFFER_SIZE - i;
        SPI.transferBytes(&_buffer[i], NULL, (remain < chunkSize) ? remain : chunkSize);
    }
    
    digitalWrite(_cs, HIGH);
    
    // END TRANSACTION
    SPI.endTransaction();
}

// --- Helper Primitives ---
void OsptekBWR::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, OspColor color) {
    for (int16_t i = x; i < x + w; i++) {
        drawPixel(i, y, color);
        drawPixel(i, y + h - 1, color);
    }
    for (int16_t i = y; i < y + h; i++) {
        drawPixel(x, i, color);
        drawPixel(x + w - 1, i, color);
    }
}

void OsptekBWR::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, OspColor color) {
    for (int16_t i = x; i < x + w; i++) {
        for (int16_t j = y; j < y + h; j++) {
            drawPixel(i, j, color);
        }
    }
}

void OsptekBWR::drawCircle(int16_t x0, int16_t y0, int16_t r, OspColor color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    drawPixel(x0, y0 + r, color);
    drawPixel(x0, y0 - r, color);
    drawPixel(x0 + r, y0, color);
    drawPixel(x0 - r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        drawPixel(x0 + x, y0 + y, color);
        drawPixel(x0 - x, y0 + y, color);
        drawPixel(x0 + x, y0 - y, color);
        drawPixel(x0 - x, y0 - y, color);
        drawPixel(x0 + y, y0 + x, color);
        drawPixel(x0 - y, y0 + x, color);
        drawPixel(x0 + y, y0 - x, color);
        drawPixel(x0 - y, y0 - x, color);
    }
}

void OsptekBWR::drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, OspColor color)
{
    int16_t byteWidth = (w + 7) / 8; // Bytes per row
    
    for (int16_t j = 0; j < h; j++) {
        for (int16_t i = 0; i < w; i++) {
            // Read the exact byte, then mask the exact bit
            if (bitmap[j * byteWidth + i / 8] & (128 >> (i & 7))) {
                drawPixel(x + i, y + j, color);
            }
        }
    }
}

void OsptekBWR::drawBitmapRotated(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, OspColor color, float angle) {
    int16_t byteWidth = (w + 7) / 8; 
    
    // Convert angle from degrees to radians
    float rad = angle * 0.01745329251; // (angle * PI / 180)
    
    // Pre-calculate sine and cosine to save processing power inside the loop
    float s = sin(rad);
    float c = cos(rad);
    
    // Find the center of the bitmap to act as the pivot point
    float cx = w / 2.0;
    float cy = h / 2.0;
    
    for (int16_t j = 0; j < h; j++) {
        for (int16_t i = 0; i < w; i++) {
            // Check if the pixel should be drawn
            if (bitmap[j * byteWidth + i / 8] & (128 >> (i & 7))) {
                
                // Apply 2D rotation matrix relative to the center of the bitmap
                int16_t drawX = x + (int16_t)round((i - cx) * c - (j - cy) * s + cx);
                int16_t drawY = y + (int16_t)round((i - cx) * s + (j - cy) * c + cy);
                
                drawPixel(drawX, drawY, color);
            }
        }
    }
}