# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: messages.proto
"""Generated protocol buffer code."""
from google.protobuf.internal import builder as _builder
from google.protobuf import descriptor as _descriptor
from google.protobuf import descriptor_pool as _descriptor_pool
from google.protobuf import symbol_database as _symbol_database
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n\x0emessages.proto\"\x8b\x01\n\x08SetLight\x12\n\n\x02id\x18\x01 \x02(\x05\x12\r\n\x05range\x18\x02 \x02(\x05\x12\x13\n\x0bstart_color\x18\x03 \x02(\x05\x12\x11\n\tend_color\x18\x04 \x02(\x05\x12\x0c\n\x04\x61hds\x18\x05 \x02(\x03\x12\x17\n\x0fstart_color_alt\x18\x06 \x02(\x05\x12\x15\n\rend_color_alt\x18\x07 \x02(\x05\" \n\x0b\x43urrentTime\x12\x11\n\ttimestamp\x18\x01 \x02(\x03\"T\n\x07Request\x12\x1e\n\tset_light\x18\x01 \x01(\x0b\x32\t.SetLightH\x00\x12\x1e\n\x14\x63urrent_time_request\x18\x02 \x01(\x08H\x00\x42\t\n\x07payload\"#\n\x03Log\x12\n\n\x02id\x18\x01 \x02(\x05\x12\x10\n\x08is_error\x18\x02 \x02(\x08\"/\n\x0c\x44\x65\x62ugMessage\x12\n\n\x02id\x18\x01 \x02(\x05\x12\x13\n\x0b\x64\x65scription\x18\x02 \x02(\t\"h\n\x10StackMeasurement\x12\n\n\x02id\x18\x01 \x02(\x05\x12\x0c\n\x04\x64\x61ta\x18\x02 \x02(\x05\x12\x0b\n\x03\x62ss\x18\x03 \x02(\x05\x12\x0c\n\x04heap\x18\x04 \x02(\x05\x12\x10\n\x08heap_gap\x18\x05 \x02(\x05\x12\r\n\x05stack\x18\x06 \x02(\x05\"\x80\x01\n\x08Response\x12$\n\x0c\x63urrent_time\x18\x01 \x01(\x0b\x32\x0c.CurrentTimeH\x00\x12\x13\n\x03log\x18\x02 \x01(\x0b\x32\x04.LogH\x00\x12.\n\x11stack_measurement\x18\x03 \x01(\x0b\x32\x11.StackMeasurementH\x00\x42\t\n\x07payload')

_builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, globals())
_builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, 'messages_pb2', globals())
if _descriptor._USE_C_DESCRIPTORS == False:

  DESCRIPTOR._options = None
  _SETLIGHT._serialized_start=19
  _SETLIGHT._serialized_end=158
  _CURRENTTIME._serialized_start=160
  _CURRENTTIME._serialized_end=192
  _REQUEST._serialized_start=194
  _REQUEST._serialized_end=278
  _LOG._serialized_start=280
  _LOG._serialized_end=315
  _DEBUGMESSAGE._serialized_start=317
  _DEBUGMESSAGE._serialized_end=364
  _STACKMEASUREMENT._serialized_start=366
  _STACKMEASUREMENT._serialized_end=470
  _RESPONSE._serialized_start=473
  _RESPONSE._serialized_end=601
# @@protoc_insertion_point(module_scope)
