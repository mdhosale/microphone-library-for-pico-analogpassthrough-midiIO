# Microphone Library for Pico

Capture audio from an analog microphone on your [Raspberry Pi Pico](https://www.raspberrypi.org/products/raspberry-pi-pico/) or any [RP2040](https://www.raspberrypi.org/products/rp2040/) based board. 🎤

I am not planning to support this code but post it in case it is helpful to anyone.

Modified the the `usb_microphone` example  so that the example creates a USB Microphone device using the TinyUSB library and captures data from an analog microphone via the ADC (rather than PDM) using a sample rate of 48 kHz, to be sent the to PC as a USB audio device. i.e. this is an audio passthrough from ADC to USB (one direction only). Also included is MIDI IO. This would be useful for a synth that has USB audio out, is sending USB MIDI, and is controlled by USB MIDI.

Forked from: https://github.com/ArmDeveloperEcosystem/microphone-library-for-pico
Based on this article: https://www.hackster.io/sandeep-mistry/create-a-usb-microphone-with-the-raspberry-pi-pico-cc9bd5

In the original the analog microphone works and outputs to serial. The only problem is that the example that works with USB audio is designed for a PDM microphone. So this version was made to replace the PDM components with Analog (ADC) microphone calls. This was straight forward, but some changes needed to be made: 

- the CMAKE file to have it point to the analog_microphone library instead of the PDM library
- The bias was disabled
- in analog_microphone.c  on line 197 the code was changed to mask, shift, and scale the data coming from the DMA so it would match the USB data type:
```
    for (int i = 0; i < samples; i++) {
        *out++ = ((*in++ & 0xFFF) - 2048)*64;
    }
```

In order for the analog mucrophone to work at 48kHz it was necessary to modify tusb_config.h in line 110:
#define CFG_TUD_AUDIO_EP_SZ_IN                                        (48 + 1) * CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX      // 48 Samples (48 kHz) x 2 Bytes/Sample x 1 Channel


MIDI IO has been added as well.
See  "Getting Started with the Raspberry Pi Pico C/C++ SDK and TinyUSB MIDI" available at:
https://diyelectromusic.com/2022/10/04/getting-started-with-the-raspberry-pi-pico-c-c-sdk-and-tinyusb-midi/
for notes on the implementation.


Note: the PDM to serial example was left untouched.

## Hardware

 * RP2040 board
   * [Raspberry Pi Pico](https://www.raspberrypi.org/products/raspberry-pi-pico/)
 * Microphones
   * Analog
     * [Electret Microphone Amplifier - MAX9814 with Auto Gain Control](https://www.adafruit.com/product/1713) 
   * PDM
     * [Adafruit PDM MEMS Microphone Breakout](https://www.adafruit.com/product/3492)

### Default Pinout

#### Analog Microphone

| Raspberry Pi Pico / RP2040 | Analog Microphone |
| -------------------------- | ----------------- |
| 3.3V | VCC |
| GND | GND |
| GPIO 26 | OUT |

#### PDM Microphone

| Raspberry Pi Pico / RP2040 | PDM Microphone |
| -------------------------- | ----------------- |
| 3.3V | VCC |
| GND | GND |
| GND | SEL |
| GPIO 2 | DAT |
| GPIO 3 | CLK |

GPIO pins are configurable in examples or API.

## Examples

See [examples](examples/) folder.


## Cloning

```sh
git clone https://github.com/ArmDeveloperEcosystem/microphone-library-for-pico.git 
```

## Building

1. [Set up the Pico C/C++ SDK](https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf)
2. Set `PICO_SDK_PATH`
```sh
export PICO_SDK_PATH=/path/to/pico-sdk
```
3. Create `build` dir, run `cmake` and `make`:
```
mkdir build
cd build
cmake .. -DPICO_BOARD=pico
make
```
4. Copy example `.uf2` to Pico when in BOOT mode.

## License

[Apache-2.0 License](LICENSE)

## Acknowledgements

The original project was created on behalf of the [Arm Software Developers](https://developer.arm.com/) team, follow them on Twitter: [@ArmSoftwareDev](https://twitter.com/armsoftwaredev) and YouTube: [Arm Software Developers](https://www.youtube.com/channel/UCHUAckhCfRom2EHDGxwhfOg) for more resources!

forked from: https://github.com/ArmDeveloperEcosystem/microphone-library-for-pico
which is based on the TinyUSB audio_test example:
https://github.com/hathach/tinyusb/tree/master/examples/device/audio_test

MIDI set-up is based on "Getting Started with the Raspberry Pi Pico C/C++ SDK and TinyUSB MIDI" available at:
https://diyelectromusic.com/2022/10/04/getting-started-with-the-raspberry-pi-pico-c-c-sdk-and-tinyusb-midi/

---

Disclaimer: This is not an official Arm product.
