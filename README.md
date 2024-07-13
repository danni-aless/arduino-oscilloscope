# SO-Oscilloscope
Arduino Oscilloscope project for SO course

### Main features
- Oscilloscope can sample up to 8 channels on the ADC
- Oscilloscope can operate in continuous or buffered mode
- PC client can configure which channels are being sampled, the mode and the sampling frequency
- A "trigger" can activate a channel in buffered mode
- Each channel can be plotted in real-time

### How to Compile and Run the project
1. Compile with ```make```

2. Upload the file to Arduino with ```make main_arduino.hex```

3. Run the program with ```./main_client.elf <serial_file> <baudrate>``` (baudrate must be 500000)