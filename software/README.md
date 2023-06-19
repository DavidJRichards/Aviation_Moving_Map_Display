# Moving map software

Raspberry Pi Pico 2040 software generating PWM signals to simulate resolvers to feen into moving map module. Software user interface has graphic LCD screen, Rotary encoder, Push butons, and serial USB communications.

[initial version](./pico_400hz_sine_PWM)

[current version](./pico_400hz_sine_PWM_EncoderMenu)

Two versions are available, the older initial version used a slower LLCD graphic library and serial command interface.

The current version uses a fast graphic LCD library and a rotary encoder menu system. The serial command interface has been dropped although serial status messages are still present.

