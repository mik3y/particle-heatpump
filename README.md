# heatpump-particle

Firmware for [Particle.io](https://particle.io) devices for controlling
a Mitsubishi Heatpump.

Based on https://github.com/SwiCago/HeatPump

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->


- [Quickstart](#quickstart)
- [Control interfaces](#control-interfaces)
  - [Particle Cloud interface](#particle-cloud-interface)
    - [Variables](#variables)
    - [Functions](#functions)
    - [Events](#events)
  - [MQTT interface](#mqtt-interface)
    - [Configuration](#configuration)
    - [Control topics (subscriptions)](#control-topics-subscriptions)
    - [Status topics (publishing)](#status-topics-publishing)
- [Changelog](#changelog)
- [Contributing](#contributing)
- [License](#license)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->


## Quickstart

```
$ particle flash <device name>
```

## Control interfaces

### Particle Cloud interface

You can monitor and control the device through Particle Cloud.

#### Variables

The following variables are exported directly from the controller to Particle cloud:

* `power`: The current device power setting, one of:
  * `"ON"`
  * `"OFF"`
* `mode`: The current device mode setting, one of:
  * `"HEAT"`
  * `"COOL"`
  * `"FAN"`
  * `"DRY"`
  * `"AUTO"`
* `temperature_c`: The current temperature setting, as a float, in degrees celsius
* `fan_speed`: The current fan speed setting, one of:
  * `"1"`
  * `"2"`
  * `"3"`
  * `"4"`
  * `"AUTO"`
  * `"QUIET"`
* `vane_setting`: The current vane (vertical baffle) setting, one of:
  * `"1"`
  * `"2"`
  * `"3"`
  * `"4"`
  * `"5"`
  * `"SWING"`
  * `"AUTO"`
* `wide_vane_setting`: The current wide vane (horizontal baffle) setting, one of:
  * `"<<"`
  * `"<"`
  * `"|"`
  * `">"`
  * `">>"`
  * `"<>"`
  * `"SWING"`
* `room_temperature_c`: The last observed room temperature, in degrees celsius
* `is_operating`: Operating status (compressor on/off), as a boolean
* `is_connecting`: Whether or not the serial device is connected, as a boolean

The following additional variables manage the device's configuration

* `mqtt_broker_host`: Host name or IP address of an MQTT broker; empty string to disable MQTT. (Default `""`)
* `mqtt_broker_port`: Port number at the MQTT broker, as an integer. (Default `1883`)

#### Functions

The following functions are available through Particle cloud to manage the device:

* `setPower()`: Change the device power setting. Allowed values:
  * `"ON"`
  * `"OFF"`
* `setMode()`: Change the device mode setting. Allowed values:
  * `"HEAT"`
  * `"COOL"`
  * `"FAN"`
  * `"DRY"`
  * `"AUTO"`
* `setTemperatureC()`: Change the desired temperature. Must be a value between `16` and `31`.
* `setFanSpeed()`: Change the fan speed setting. Allowed values:
  * `"1"`
  * `"2"`
  * `"3"`
  * `"4"`
  * `"AUTO"`
  * `"QUIET"`
* `setVaneDirection()`: Change the vane setting. Allowed values:
  * `"1"`
  * `"2"`
  * `"3"`
  * `"4"`
  * `"5"`
  * `"SWING"`
  * `"AUTO"`
* `setWideVaneDirection()`: Change the vane setting. Allowed values:
  * `"<<"`
  * `"<"`
  * `"|"`
  * `">"`
  * `">>"`
  * `"<>"`
  * `"SWING"`

The following additional functions manage the device's local configuration:

* `setMqttBrokerHost`: Set the host name or IP address of the MQTT broker; empty string to disable.
  * **Note:** Setting, changing, or clearing the host name will cause the device to connect, reconnect, or disconnect from the MQTT broker, respectively.
* `setMqttBrokerPort`: Set the port number at the MQTT broker.

#### Events

The following events are published from the device.

* `heatpump/status-changed`: Reported once a minute, and whenever status changes. A JSON dictionary of:
  * `room_temperature_c`: Current room temperature
  * `is_operating`: Compressor operation status
  * `compressor_frequency`: Numeric compressor frequency value
* `heatpump/settings-changed`: Reported whenever settings have changed. A JSON dictionary 
  * `power`: Power status. Possible values are same as the variable.
  * `mode`: Power status. Possible values are same as the variable.
  * `temperature_c`: Temperature, a number.
  * `fan`: Fan status. Possible values are same as the variable.
  * `vane`: Vane status. Possible values are same as the variable.
  * `wide_vane`: Wide vane status. Possible values are same as the variable.
* `heatpump/log`: Diagnostic log messages from the firmware. Unstructured.

### MQTT interface

You can control and monitor this device through MQTT.

#### Configuration

By default, the MQTT broker host is not set, and the MQTT feature is disabled. You can either set it dynamically through `setMqttBrokerHost()` via Particle Cloud, or set it statically by editing `config.h`.

#### Control topics (subscriptions)

The device subscribes to the following topics for control:

* `heatpump/+/control/power`: Change the power setting.
* `heatpump/+/control/mode`: Change the mode setting.
* `heatpump/+/control/temperature-c`: Change the temperature setting.
* `heatpump/+/control/fan-speed`: Change the fan speed setting.
* `heatpump/+/control/vane-setting`: Change the van setting.
* `heatpump/+/control/wide-vane-setting`: Change the wide setting.

For the position `+`, the device will look for its device name, or the special value `all`.

For example, a device named `mydevice-1` will respond to the following topics:
* `heatpump/mydevice-1/control/power`
* `heatpump/all/control/mode`

.. but will ignore messages on:
* `heatpump/otherdevice/control/power`
* `heatpump/bloop/control/power`

#### Status topics (publishing)

The device will post status updates on the following topics as JSON:

* `heatpump/<name>/status`: Device status, as a JSON object. Same format as the `heatpump/status-changed` Particle Cloud event.
* `heatpump/<name>/settings`: Device settings, as a JSON object. Same format as the `heatpump/settings-changed` Particle Cloud event.

Individual values are also posted to the following additional topics:

* `heatpump/<name>/settings`
  * `heatpump/<name>/settings/fan`
  * `heatpump/<name>/settings/is-connected`
  * `heatpump/<name>/settings/mode`
  * `heatpump/<name>/settings/power`
  * `heatpump/<name>/settings/temperature-c`
  * `heatpump/<name>/settings/vane`
  * `heatpump/<name>/settings/wide-vane`
* `heatpump/<name>/status`
  * `heatpump/<name>/status/room-temperature-c`
  * `heatpump/<name>/status/is-operating`

## Changelog

See [CHANGELOG.md](https://github.com/mik3y/heatpump-particle/blob/master/CHANGELOG.md) for the latest changes.

## Contributing

Contributions welcome! Open an issue or a pull request if you've got ideas.

## License

This is open source software offered under the MIT license.

This library includes a copy of `HeatPump.cpp` and `HeatPump.h` which are licensed under the LGPL.

See [LICENSE.txt](https://github.com/mik3y/heatpump-particle/blob/master/LICENSE.md) and individual source files for the full terms.
