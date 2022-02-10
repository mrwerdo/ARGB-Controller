# ARGB Controller

To install nanopb:

    pip install nanopb

To build protobuf messages:
```
$ protoc --python-out=. messages.proto
$ nanopb_generator messages.proto
```

Dependencies:
- [AceCRC](https://github.com/bxparks/AceCRC)
- [PacketSerial](https://github.com/bakercp/PacketSerial)
- [nanopb](https://github.com/nanopb/nanopb)