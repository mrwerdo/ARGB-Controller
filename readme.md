# ARGB Controller

Install arduino-cli or use the IDE.

To install the board firmware:

    arduino-cli core install arduino:avr

To install python dependencies:

    pip install -r requirements.txt

To activate env:

    source .venv/bin/activate

To install FastLED:

    arduino-cli lib install "FastLED"
    arduino-cli lib install SnappyProto
    arduino-cli lib install PacketSerial
    arduino-cli lib install AceCRC

To build protobuf messages:
```
$ protoc --python-out=. messages.proto
$ nanopb_generator messages.proto
```

To compile the sketch:

    arduino-cli compile --fqbn arduino:avr:uno argb_controller

To upload the sketch:

    arduino-cli upload --port /dev/ttyACM0 --fqbn arduino:avr:uno argb_controller.ino

To ensure the device is not reset by the operating system:

    stty -F /dev/ttyACM0 -hupcl

To run the python program:

    python -m argb

Dependencies:
- [AceCRC](https://github.com/bxparks/AceCRC)
- [PacketSerial](https://github.com/bakercp/PacketSerial)
- [nanopb](https://github.com/nanopb/nanopb)
- [FastLED](https://github.com/fastled/fastled)
