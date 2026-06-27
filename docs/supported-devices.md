# Supported Devices

This component supports every device that the [rtl_433_ESP](https://github.com/NorthernMan54/rtl_433_ESP) library can decode. The upstream project maintains the full device list in its README:

**[rtl_433_ESP supported devices](https://github.com/NorthernMan54/rtl_433_ESP#supported-devices)**

The list is derived from the [rtl_433 project](https://github.com/merbanan/rtl_433), which supports over 200 device protocols including weather stations, soil sensors, door/window contacts, tire-pressure monitors, and more.

## OOK vs FSK

Device support is split by modulation type:

- **OOK (On-Off Keying)** -- the default. Covers the majority of consumer 433 MHz devices: weather stations, thermometers, hygrometers, rain gauges, door/window sensors, motion detectors, soil moisture sensors, etc.
- **FSK (Frequency Shift Keying)** -- covers tire-pressure monitoring systems (TPMS), some industrial sensors, and a smaller set of consumer devices.

The CC1101 can only receive one modulation type at a time. By default, the component starts in OOK mode. To switch to FSK, set the `modulation` config key:

```yaml
rtl433:
  modulation: FSK
```

## Reducing Binary Size

If you only need a handful of specific devices, you can use the `MY_DEVICES` build flag to limit which decoders are compiled in. This reduces the binary size and can improve performance:

```yaml
esphome:
  platformio_options:
    build_flags:
      - "-DMY_DEVICES=true"
```

See the [rtl_433_ESP documentation](https://github.com/NorthernMan54/rtl_433_ESP) for instructions on creating a custom device list.

## Finding Field Names for Your Device

When you use the native sensor platforms (`sensor`, `text_sensor`, `binary_sensor`), the `field` config key must match a JSON key from the decoded message. Field names are determined by the rtl_433 decoder for each device protocol.

### Example decoded message

A typical decoded message from an Acurite-606TX weather sensor looks like this:

```json
{"model":"Acurite-606TX","id":1234,"battery_ok":1,"temperature_C":22.5,"mic":"CHECKSUM"}
```

Each key in the JSON object can be used as a `field:` value in your YAML:

| JSON key | Platform | Example use |
|----------|----------|-------------|
| `model` | `text_sensor` | Publish the device model name as a string |
| `id` | `text_sensor` | Publish the device ID as a string |
| `battery_ok` | `binary_sensor` | ON when battery is good (truthy: non-zero = ON) |
| `temperature_C` | `sensor` | Publish the temperature as a float |
| `mic` | `text_sensor` | Publish the integrity check type |

### How to discover your device's fields

The easiest way to find the exact field names for your device is to log the raw JSON messages:

```yaml
rtl433:
  cs_pin: 5
  gdo0_pin: 13
  on_message:
    - lambda: |-
        ESP_LOGI("rtl433", "raw: %s", x.c_str());
```

Flash this configuration, open the ESPHome logs, and trigger your device. The log output will show the full JSON object with all available keys. Use those keys as `field:` values in your sensor definitions.

Common fields you will see across many devices:

- `model` -- device protocol name (always present)
- `id` -- transmitter ID (present on most devices)
- `battery_ok` -- battery status flag (1 = good, 0 = low)
- `temperature_C` -- temperature in Celsius
- `humidity` -- relative humidity percentage
- `pressure_kPa` -- pressure in kilopascals
- `wind_avg_km_h` -- average wind speed
- `rain_mm` -- accumulated rainfall

The exact set of fields varies by device. Always check the raw JSON output to confirm what your specific device reports.
