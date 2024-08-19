/*
 * Copyright (c) 2021 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * This examples creates a USB Microphone device using the TinyUSB
 * library and captures data from an analog microphone via the ADC using a sample
 * rate of 48 kHz, to be sent the to PC as a USB audio device.
 *
 * forked from: https://github.com/ArmDeveloperEcosystem/microphone-library-for-pico
 * which is based on the TinyUSB audio_test example:
 * https://github.com/hathach/tinyusb/tree/master/examples/device/audio_test
 *
 *  MIDI set-up is based on "Getting Started with the Raspberry Pi Pico C/C++ SDK and TinyUSB MIDI" available at:
 * https://diyelectromusic.com/2022/10/04/getting-started-with-the-raspberry-pi-pico-c-c-sdk-and-tinyusb-midi/
 */
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/analog_microphone.h"
#include "usb_microphone.h"
#include "bsp/board.h"

// configuration
const struct analog_microphone_config config = {
    // GPIO to use for input, must be ADC compatible (GPIO 26 - 28)
    .gpio = 26,
    
    // bias voltage of microphone in volts
    .bias_voltage = 1.25,
    
    // sample rate in Hz
    .sample_rate = SAMPLE_RATE,//defined in usb_microphone.h
    
    // number of samples to buffer
    .sample_buffer_size = SAMPLE_BUFFER_SIZE,//defined in usb_microphone.h
};

// variables
int16_t sample_buffer[SAMPLE_BUFFER_SIZE];// DMA captures as signed int
volatile int samples_read = 0;

//midi settings
uint8_t const cable_num = 0; // MIDI jack associated with USB endpoint
uint8_t const channel   = 0; // 0 for channel 1

// callback functions
void on_analog_samples_ready();
void on_pdm_samples_ready();
void on_usb_microphone_tx_ready();
void usb_midi_task();

int main(void)
{
    // initialize and start the Analog microphone
    analog_microphone_init(&config);
    analog_microphone_set_samples_ready_handler(on_analog_samples_ready);
    analog_microphone_start();
    
    // initialize the USB microphone interface
    usb_microphone_init();
    usb_microphone_set_tx_ready_handler(on_usb_microphone_tx_ready);
    
    while (1) {
        // run the USB microphone task continuously
        usb_microphone_task();
        usb_midi_task();
    }
    
    return 0;
}

void on_analog_samples_ready()
{
    int16_t currentSample = 0;
    
    // Callback from library when all the samples in the library
    // internal sample buffer are ready for reading.
    // internal sample buffer are ready for reading
    samples_read = analog_microphone_read(sample_buffer, SAMPLE_BUFFER_SIZE);
    
    
    // callback: process sample by sample
    for(int i=0;i< SAMPLE_BUFFER_SIZE;i++){
        
        currentSample = sample_buffer[i];
        //do nothing...
        //processed code may need to be constrained to prevent wrapping
//        if (sampleIn > 32767.0) {sampleIn = 32767.0;}
//        if (sampleIn < -32767.0) {sampleIn = -32767.0;}
        //write to USB buffer
        sample_buffer[i] = currentSample;
    }
}

void on_usb_microphone_tx_ready()
{
    // Callback from TinyUSB library when all data is ready
    // to be transmitted.
    //
    // Write local buffer to the USB microphone
    usb_microphone_write(sample_buffer, sizeof(sample_buffer));
}


//--------------------------------------------------------------------+
// MIDI Task
//--------------------------------------------------------------------+

// Variable that holds the current position in the sequence.
uint32_t note_pos = 0;

// Store example melody as an array of note values
uint8_t note_sequence[] =
{
  74,78,81,86,90,93,98,102,57,61,66,69,73,78,81,85,88,92,97,100,97,92,88,85,81,78,
  74,69,66,62,57,62,66,69,74,78,81,86,90,93,97,102,97,93,90,85,81,78,73,68,64,61,
  56,61,64,68,74,78,81,86,90,93,98,102
};

void usb_midi_task(void)
{
      static uint32_t start_ms = 0;

    
    // The MIDI interface always creates input and output port/jack descriptors
    // regardless of these being used or not. Therefore incoming traffic should be read
    // (possibly just discarded) to avoid the sender blocking in IO
    uint8_t inpacket[4];
    uint8_t outpacket[4];
    
    while ( tud_midi_available() ){
        tud_midi_packet_read(inpacket);
        outpacket[0] = inpacket[0];//cable_num?
        outpacket[1] = inpacket[1];//message type
        outpacket[2] = inpacket[2];//Data byte1
        outpacket[3] = inpacket[3];//Data byte2
        
    
        tud_midi_packet_write(outpacket);
    }
    
    
    
    
     // send note periodically
     if (board_millis() - start_ms < 286) return; // not enough time
     start_ms += 286;
     
     // Previous positions in the note sequence.
     int previous = (int) (note_pos - 1);
     
     // If we currently are at position 0, set the
     // previous position to the last note in the sequence.
     if (previous < 0) previous = sizeof(note_sequence) - 1;
     
     // Send Note On for current position at full velocity (127) on channel 1.
     uint8_t note_on[3] = { 0x90 | channel, note_sequence[note_pos], 127 };
     tud_midi_stream_write(cable_num, note_on, 3);
     
     // Send Note Off for previous note.
     uint8_t note_off[3] = { 0x80 | channel, note_sequence[previous], 0};
     tud_midi_stream_write(cable_num, note_off, 3);
     
     // Increment position
     note_pos++;
     
     // If we are at the end of the sequence, start over.
     if (note_pos >= sizeof(note_sequence)) note_pos = 0;
}
