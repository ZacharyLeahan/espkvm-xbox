<!-- SPDX-License-Identifier: Apache-2.0 -->
# My approach: Xbox POWER/EJECT through an existing ESP-KVM

I'm adding remote POWER/EJECT control to an original Xbox development rig.
I already use an ESP32-P4 ESP-KVM for HDMI capture, audio and controller access,
so the goal is to reuse that controller with a simple button interface.

## How it connects

```text
Xbox front button board
  POWER switch -> two soldered leads --+
                                      +-> four-pin sidewall port
  EJECT switch -> two soldered leads --+          |
                                          removable cable
                                                 |
                                    two external isolated relays
                                                 |
                                         existing ESP-KVM
```

An installer is attaching a separate wire pair across each physical switch,
keeping the original buttons and LEDs working. Each relay briefly joins its
button's two contacts, then releases them. Only the isolated relay contacts
connect to the Xbox button leads; no ESP GPIO or accessory supply goes into them.

The selected parts are two [Adafruit #4409 relays](https://www.adafruit.com/product/4409),
with [#3894 control cables](https://www.adafruit.com/product/3894), and a
[#5319 sidewall socket](https://www.adafruit.com/product/5319) with its
[#5318 mating plug](https://www.adafruit.com/product/5318). One shared independent
5 V supply powers both relays; the ESP-KVM has its own supply. Neither relies
on Xbox LPC or standby power. The Xbox supplies its own standby button circuitry
when its AC is restored.

## Why I'm doing it

The main goal is recovery after a genuine Xbox hard freeze during development:

1. Use an independently controlled smart plug to cut Xbox AC.
2. Confirm the cut, wait for a qualified discharge interval, and restore AC.
3. Check fresh power evidence. If the console remains off, issue one short POWER press.
4. Use HDMI capture to observe a fresh boot progression and confirm useful response.

The KVM, relay supply and network stay powered throughout. A button press alone
is not a guaranteed hard power cycle. Commands must not be blindly repeated
after a timeout, since a second press could undo the intended action.
EJECT will also be independently commandable, but is outside default recovery.

## Relation to your PowerPlate

The shared idea is network-controlled electronics emulating the Xbox's physical
buttons. Your post describes an integrated replacement button board with its
own ESP32, smart-home control and RGB features. My setup keeps the stock button
board and uses the KVM already in the rig, with the relays outside the console.

The emphasis here is remote development: button control alongside video capture
and controller access, followed by verified recovery. I'm not adding another
internal controller or a lighting system. An integrated board is a neater option
for a standalone smart-home installation; this approach fits the hardware I
already have. I haven't reviewed your schematic or firmware, so I'm not making
a claim about comparative reliability or your board's hard-freeze behavior.

## Current status and details

This is the selected hardware plan, not a completed recovery demonstration.
Installer wire/connector parts have been ordered. Button firmware integration,
pulse safety, power-state checks and hardware qualification remain to be completed.
The overall aim is a simple, low-cost addition centered on the existing ESP-KVM.

- [Full wiring plan and photo diagram](https://github.com/ZacharyLeahan/espkvm-xbox/blob/main/XBOX-POWER-RELAY-SETUP.md)
- [Installer instructions and exact parts](https://github.com/ZacharyLeahan/espkvm-xbox/blob/main/XBOX-POWER-ADAPTER-BUILD-BRIEF.md)
- [ESP-KVM fork](https://github.com/ZacharyLeahan/espkvm-xbox)
