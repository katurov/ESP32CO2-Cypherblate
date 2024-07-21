# CO2 Cypherblate based on ESP32

Even a small increase of CO2 in the air by volume can be hazardous. While most people will never encounter CO2 levels above 1% indoors, the increased use of pressurized CO2 storage tanks in businesses has increased the likelihood of exposure to high levels of CO2. At 1% CO2, people can start to experience accelerated heart rate. At 4%, CO2 levels can be fatal. Normal CO2 levels in fresh air is approximately 400 ppm (part per million) or 0.04% CO2 in air by volume. The table below shows the effects of increased CO2 levels in an enclosed space as a percent of air by volume.

| ppm | Effect |
| ---- | ------ |
| 400 | Normal outdoor air |
| 400 - 1000 | Typical CO2 levels found indoors |
| 1000 - 2000 | Common complaints of drowsiness or poor air quality |
| 2000 - 5000 | Headaches, fatigue, stagnant, stuffiness, poor concentration, loss of focus, increased heart rate, nausea |

[Read more](https://www.co2meter.com/blogs/news/carbon-dioxide-indoor-levels-chart)


## Depencies

* Sensirion driver for [SDC41](https://github.com/Sensirion/arduino-i2c-scd4x/tree/master)
* Adafruit driver for [GC9A01A](https://github.com/adafruit/Adafruit_GC9A01A/tree/main)
* ESP-NOW (build in)

## Device list

* Waveshare [1.28inch Round LCD Display Module](https://www.waveshare.com/1.28inch-lcd-module.htm) (240×240 65K RGB)
* OEM ESP32 Development Board - DEVKIT V1 (ESP-WROOM-32 module) like [this](https://grobotronics.com/esp32-development-board-devkit-v1.html?sl=en) or [that](https://einstronic.com/product/esp32-devkit-v1/)
* [SCD41 CO2 sensor Breakout Board](https://tasmota.github.io/docs/SCD4x/#first-installation) - The [SCD41](https://sensirion.com/products/catalog/SCD41) is miniature CO2 sensor, which builds on the photoacoustic NDIR sensing principle and Sensirion’s patented PASens® and CMOSens® technology to offer high accuracy at an unmatched price and smallest form factor. 

## Wiring

Display, you don't need to use pin numbers if useing "normal" SPI:

| LCD | ESP | Code |
| ---- | ---- | ---- |
| VCC | 3V3 |  |
| GND | GND |  |
| DIN | MOSI |  |
| CLK | SCK |  |
| SC  | D27 | 27 |
| DC  | D26 | 26 |
| RST | D25 | 25 |
| BL  | | |

SCD41 uses I2C, so this is the plan:

| SCD41 | ESP | Code |
| ---- | ---- | ---- |
| GND | GND |  |
| VCC | 3V3 |  |
| SCL | SCL |  |
| SDA | SDA |  |

## Thanks 

1. [Bodmer](https://github.com/Bodmer) make a great comment [how to draw an arc](https://forum.arduino.cc/t/adafruit_gfx-fillarc/397741/7?u=katurov).
2. [truetype2gfx](https://rop.nl/truetype2gfx/) converts TTF to GFX
3. Please note that ```canvas``` [doesn't wotk with custom fints](https://learn.adafruit.com/adafruit-gfx-graphics-library/minimizing-redraw-flicker)
4. Some great online [RGB565 Color Picker](https://rgbcolorpicker.com/565)
