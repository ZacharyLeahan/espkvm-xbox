<!-- SPDX-License-Identifier: Apache-2.0 -->
# How the four-player Xbox test setup works

**One main ESP-KVM board** coordinates testing and captures Xbox video/audio.
**Four small Waveshare ESP32-S3-Zero boards** each act as one Xbox controller.

```text
Test program / agent
        |
   Main ESP-KVM
        |
     Home Wi-Fi
        +-- S3 #1 -- USB cable + adapter -- Xbox controller port 1
        +-- S3 #2 -- USB cable + adapter -- Xbox controller port 2
        +-- S3 #3 -- USB cable + adapter -- Xbox controller port 3
        +-- S3 #4 -- USB cable + adapter -- Xbox controller port 4

Xbox HDMI --> capture board --> main ESP-KVM --> live picture/sound to tester
```

All five boards share the home Wi-Fi network. Each S3 plugs into an Xbox
controller port through a USB cable and adapter, drawing power from that port.
The Xbox sees four wired controllers; the ESP-to-ESP connection is wireless.

A test program or agent sends a player number plus buttons, trigger pressure
and stick positions. The main board routes that state to the paired S3, which
sends Xbox controller reports. Players have independent state, signed messages,
and automatic release after about 500 ms without fresh input.

## Why this matters

The goal is **repeatable four-player testing on real Xbox hardware**. One test
coordinator controls all four players, observes the picture and sound, and saves
evidence to reproduce multiplayer bugs that single-player tests miss.

**Already verified:** four human-player slots in Guilty Gear Isuka, independent
movement and attacks, and simultaneous jumps through the main ESP.
**Next:** longer runs, reusable test sequences, and latency measurements.
Frame-accurate synchronization remains future work.

[Gameplay evidence and limits](S3-FOUR-PLAYER-VALIDATION.md) |
[Build and pairing instructions](../s3-dongle/README.md)
