//------------------------------------------------------------------------
// TFT_SPI.cpp
//  Management of an SPI connection to a TFT screen
// Copyright(c) 2024 Dad Design.
//------------------------------------------------------------------------

#include "TFT_SPI.h"

#if TFT_CONTROLEUR_TFT == 7735
    #include "ST7735_Defines.h"
#elif TFT_CONTROLEUR_TFT == 7789
    #include "ST7789_Defines.h"
#endif

namespace DadGFX {

//***********************************************************************************
// TFT_SPI
//  SPI management for communication with the TFT screen
//*********************************************************************************** 

// --------------------------------------------------------------------------
// Initialize the SPI connection
void TFT_SPI::Init_TFT_SPI() {
    TFTSPIConfig config;
    Init_TFT_SPI(config);
}

void TFT_SPI::Init_TFT_SPI(const TFTSPIConfig &config) {
    // SPI configuration
    m_spi_config.periph     = config.spi_port;                                   // Set the SPI peripheral
    m_spi_config.mode       = config.spi_mode;                                   // Master mode
    m_spi_config.direction  = SpiHandle::Config::Direction::TWO_LINES_TX_ONLY;   // TX-only mode
    m_spi_config.datasize   = 8;                                                 // 8-bit data size
    
    // Configure SPI mode based on the defined settings
    switch(config.clock_mode) {
        case SPIMode::Mode3:
            m_spi_config.clock_polarity = SpiHandle::Config::ClockPolarity::HIGH;
            m_spi_config.clock_phase    = SpiHandle::Config::ClockPhase::TWO_EDGE;
            break;
        case SPIMode::Mode2:
            m_spi_config.clock_polarity = SpiHandle::Config::ClockPolarity::LOW;
            m_spi_config.clock_phase    = SpiHandle::Config::ClockPhase::TWO_EDGE;
            break;
        case SPIMode::Mode1:
            m_spi_config.clock_polarity = SpiHandle::Config::ClockPolarity::HIGH;
            m_spi_config.clock_phase    = SpiHandle::Config::ClockPhase::ONE_EDGE;
            break;
        case SPIMode::Mode0:
        default:
            m_spi_config.clock_polarity = SpiHandle::Config::ClockPolarity::LOW;
            m_spi_config.clock_phase    = SpiHandle::Config::ClockPhase::ONE_EDGE;
            break;
    }
    
    // Other SPI settings
    m_spi_config.nss            = SpiHandle::Config::NSS::SOFT;                  // Software NSS
    m_spi_config.baud_prescaler = config.baud_prescaler;

    // Pin configuration for SPI
    m_spi_config.pin_config.sclk = config.sclk_pin;                              // Clock pin
    m_spi_config.pin_config.miso = dsy_gpio_pin();                               // MISO pin (not used)
    m_spi_config.pin_config.mosi = config.mosi_pin;                              // MOSI pin
    m_spi_config.pin_config.nss  = dsy_gpio_pin();                               // NSS pin (not used)
        
    // TFT control pin configuration
    m_dc.Init(config.dc_pin, GPIO::Mode::OUTPUT, GPIO::Pull::NOPULL, GPIO::Speed::VERY_HIGH);     // Data/Command pin
    m_reset.Init(config.reset_pin, GPIO::Mode::OUTPUT, GPIO::Pull::NOPULL, GPIO::Speed::VERY_HIGH); // Reset pin
    m_dc.Write(true);    // Set DC pin to high
    m_reset.Write(true); // Set Reset pin to high

    // Initialize SPI
    m_spi.Init(m_spi_config);
    
    // Reset the LCD screen
    m_reset.Write(false); // Hold Reset pin low
    System::Delay(50);    // Delay 50 ms
    m_reset.Write(true);  // Release Reset pin
    System::Delay(50);    // Delay 50 ms

    // Perform additional initialization
    Initialise();
}

// --------------------------------------------------------------------------
// Set the display rotation (0, 90, 180, 270 degrees)
void TFT_SPI::setTFTRotation(Rotation r) {
    // Send the memory access control command
    SendCommand(TFT_MADCTL);
    switch (r) {
        case Rotation::Degre_0: // Portrait
            SendData(TFT_MAD_COLOR_ORDER);
            break;
        case Rotation::Degre_90: // Landscape (Portrait + 90 degrees)
            SendData(TFT_MAD_MX | TFT_MAD_MV | TFT_MAD_COLOR_ORDER);
            break;
        case Rotation::Degre_180: // Inverted portrait
            SendData(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_COLOR_ORDER);
            break;
        case Rotation::Degre_270: // Inverted landscape
            SendData(TFT_MAD_MV | TFT_MAD_MY | TFT_MAD_COLOR_ORDER);
            break;
    }
}

} // namespace DadGFX
