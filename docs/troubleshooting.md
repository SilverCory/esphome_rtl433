# Troubleshooting

## No messages received

**Check your wiring.** Double-check that the CC1101 module is wired correctly to the ESP32, paying special attention to `cs_pin` and `gdo0_pin`. A loose connection on any SPI line will prevent communication entirely.

**Verify the frequency.** Make sure the `frequency` setting matches your devices. Most consumer 433 MHz devices operate at 433.92 MHz (the default). North American devices may use 315 MHz. European 868 MHz devices require a different CC1101 module.

**Check modulation type.** The CC1101 can only receive OOK or FSK at a time, not both. The default is OOK. If your device uses FSK modulation, set the `modulation` config key:

```yaml
rtl433:
  modulation: FSK
```

**RSSI threshold too high.** If you have set `min_rssi`, try lowering it or removing it temporarily. The default value of `-82` is a reasonable starting point but may filter out weak or distant transmitters.

**Antenna.** Make sure the CC1101 module has an antenna connected. Many modules ship with a coil antenna that must be soldered on. Without an antenna, reception range drops to a few centimetres at best.

**Enable debug logging.** Add verbose logging to see what the library is doing:

```yaml
logger:
  level: DEBUG

rtl433:
  rtl_debug: 4
  publish_unparsed: true
```

Setting `rtl_debug: 4` enables maximum verbosity from the rtl_433_ESP library. `publish_unparsed: true` causes the `on_message` trigger to fire even for signals that could not be decoded, which helps confirm that the radio is receiving something.

## Build failures

**Arduino framework required.** This component only works with the Arduino framework on ESP32. Make sure your YAML includes:

```yaml
esp32:
  board: esp32dev
  framework:
    type: arduino
```

**ESP32 only.** The component declares `DEPENDENCIES = ["esp32"]`. It will not compile for ESP8266 or RP2040.

**Library conflicts.** The component auto-injects `rtl_433_ESP`, `RadioLib ^7.2.1`, and `lib_ldf_mode: chain+`. If you have manually added conflicting versions of these libraries in your YAML, remove them and let the component manage the dependencies.

**Compilation errors about `yield` or `millis`.** The component header (`rtl433.h`) undefines several Arduino macros that conflict with rtl_433_ESP internals. If you see errors related to these symbols, make sure you are not including the component header from your own code before the Arduino headers.

## Signal noise and false positives

**Set an RSSI threshold.** If you are getting many spurious or garbled messages, add a minimum RSSI filter:

```yaml
rtl433:
  min_rssi: -82
```

The default is `-82`. A higher value (e.g., `-70`) filters more aggressively; a lower value (e.g., `-90`) lets more signals through.

**Reduce registered decoders.** With all decoders enabled, there is a higher chance of a noise burst matching a decoder's bit pattern. Use `MY_DEVICES=true` to limit decoders to only the devices you actually own:

```yaml
esphome:
  platformio_options:
    build_flags:
      - "-DMY_DEVICES=true"
```

**Relocate the antenna.** Physical distance from noise sources (switching power supplies, motors, LED drivers) makes a significant difference at 433 MHz.

## How to enable debug logging

Set the ESPHome logger to `DEBUG` or `VERBOSE` level and enable rtl_433 debug output:

```yaml
logger:
  level: DEBUG

rtl433:
  rtl_debug: 4
```

The component itself logs at two levels:
- `ESP_LOGI` -- startup message confirming the receiver frequency and GPIO (always shown at `INFO` level and above)
- `ESP_LOGD` -- each received message JSON (shown at `DEBUG` level and above)

## Native sensors not updating

If you have declared `sensor`, `text_sensor`, or `binary_sensor` entries with `platform: rtl433` but they remain in an unknown state or never update, check the following:

**Wrong `device_model`.** The `device_model` value must exactly match the `model` field in the decoded JSON. This is case-sensitive and includes hyphens. For example, `Acurite-606TX` is correct, but `acurite-606tx` or `Acurite 606TX` will not match.

**Wrong `field` name.** The `field` value must exactly match a key in the decoded JSON object. If the device reports `temperature_C` but you wrote `field: temperature`, the sensor will never update. See [docs/supported-devices.md](supported-devices.md) for how to discover the correct field names.

**`device_id` mismatch.** If you have set `device_id`, it must match the `id` field in the decoded JSON as a string comparison. Some devices report numeric IDs; the component compares them as strings, so `"1234"` will not match `"01234"`.

**Wrong platform type.** Make sure you are using the correct platform for the field's data type:
- `sensor` -- for numeric values (float): `temperature_C`, `humidity`, `pressure_kPa`
- `text_sensor` -- for string values: `model`, `id`, `mic`
- `binary_sensor` -- for boolean/truthy values: `battery_ok`, `alarm`, `tamper`

Using the wrong platform (e.g., `sensor` for a string field) will silently fail because the JSON value cannot be cast to the expected type.

### Tip: log raw JSON to find correct names

Add an `on_message` lambda to your `rtl433` config to see the exact JSON output from your devices:

```yaml
rtl433:
  cs_pin: 5
  gdo0_pin: 13
  on_message:
    - lambda: |-
        ESP_LOGI("rtl433", "raw: %s", x.c_str());
```

Open the ESPHome logs, trigger your device, and look at the JSON keys. Copy the exact `model` value into `device_model` and the exact key name into `field`.

### Tip: omit device_id while testing

If you are unsure of your device's ID, omit `device_id` entirely. The sensor will then match any device with the specified model, making it easy to confirm that the model name and field are correct:

```yaml
sensor:
  - platform: rtl433
    name: "Test Temperature"
    device_model: "Acurite-606TX"
    # device_id deliberately omitted -- matches any Acurite-606TX transmitter
    field: temperature_C
```

Once you see values updating, check the raw JSON log for the `id` value and add `device_id` to lock the sensor to a specific transmitter.
