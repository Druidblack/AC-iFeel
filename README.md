# Kelon168 MQTT IR Transmitter for ESPHome

This small ESPHome external component subscribes to an MQTT topic containing a 21-byte Kelon168 frame in hex format and transmits it with `remote_transmitter`.

It is intended to work with the modified `ac_hi` Hisense UART project that publishes iFeel / Follow Me frames to MQTT.

## MQTT payload format

Supported hex examples:

```text
83 06 80 72 00 00 08 03 40 00 00 80 16 2F 00 00 00 00 08 00 08
```

or compact:

```text
830680720000080340000080162F00000000080008
```

The component validates the 21-byte length and Kelon168 checksums before transmitting.

## Example

See `examples/ir-transmitter-node.yaml`.

Important options:

```yaml
remote_transmitter:
  id: ir_tx
  pin: GPIO23
  carrier_duty_percent: 50%

kelon168_mqtt_ir:
  transmitter_id: ir_tx
  topic: hisense_ac_zal/ir/kelon168/tx
  qos: 0
  send_times: 1
```

## Hardware note

For reliable range, drive the IR LED through a transistor/MOSFET with a current-limiting resistor. A GPIO pin can blink a small LED directly for short tests, but it is usually weak for AC IR control.
