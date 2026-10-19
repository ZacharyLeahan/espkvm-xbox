# Xbox rig measurements, hardware, and multiplayer plan

## Evidence and limits

These are observations on our hardware, under different test configurations:

| Check | Result |
| --- | --- |
| Local 720p preview | 28–29 LCD updates/sec with three buffers and CPU scaling. |
| Wi-Fi MJPEG video | Ten minutes, two viewers: 4.74 delivered FPS average, zero reconnects, 6-FPS cap and JPEG quality 75. Capture was about 60 FPS. |
| Simultaneous 480p recording/preview | About 26 recording FPS and 26–27 LCD updates/sec, zero recorder-reported drops in that trial. |
| Live HDMI audio with web video | A 60-second HTTP capture stayed connected and saved 59.64 seconds of non-silent 48 kHz stereo PCM; brief DMA overruns recovered. Real browser audio also ran beyond 74 seconds. |
| MicroSD video + audio | One-minute 720p clip decoded with stereo 48 kHz AAC; zero recorder audio/video queue drops, about 14.4 video FPS with simultaneous network audio. [Test details](SD-RECORDING.md#hardware-verification-2026-10-16). |
| Bluetooth controller | All buttons operator-confirmed in Xbox Controller Tool on 2026-10-11. |

Audio is working, but brief gaps remain possible. Subjective sound quality, audio/video synchronization, and physical end-to-end latency are unverified; the browser's 43–44 ms queue measurement is only local buffering. There is **one network audio listener**: mute the viewer before using the WAV tool. The microSD recorder has its own audio subscription and can run alongside either. Stereo LPCM is required; Dolby/DTS passthrough is not decoded.

Connecting a remote video viewer reduces the LCD to at most 1 FPS; disconnecting restores local preview. Long sessions, repeated HDMI mode changes, remote Tailscale performance, and simultaneous 720p recording/preview need more validation. Bluetooth bonds do not survive an ESP reboot, reconnection needs reliability testing, and rumble is not forwarded.

Automated deployment/launch, reliable recovery from console hangs, named input macros, and an autonomous test runner remain future work. The current tools complement emulator/debugger workflows with real-console evidence.

[Audio measurements and configuration](HDMI-AUDIO.md) · [Video investigation](XBOX-CAPTURE-DEBUG.md) · [Historical tests and hardware costs](FORK-HISTORY.md)

## Hardware and connections

Our setup uses an **ESP32-P4 Function EV board** (P4 rev 3.2, C6 Wi-Fi), **Geekworm C790** HDMI-to-CSI bridge, and optional **Waveshare 3.5-inch DSI LCD (E)**. The three main purchases totaled $220.03. Including four Xbox controller-port adapters and four ordered ESP32-S3-Zero boards for the planned multiplayer setup, **documented all-in spend is $274.85**. [Receipt breakdown and exclusions](FORK-HISTORY.md#hardware-purchases) are documented separately.

- **Video:** Xbox HDMI → C790 → ESP **MIPI CAMERA**. Optional ESP **MIPI DISPLAY** → LCD. Power off before handling ribbons and check contact orientation.
- **Audio:** C790 I2S → four ESP header connections. Use the [verified Function EV wiring and audio settings](HDMI-AUDIO.md#header-cross-check-2026-10-16).
- **Controller:** Xbox controller port → female USB adapter → USB-A-to-USB-C data cable → ESP **DEVICE USB**. The firmware provides XID emulation; the adapter is just a cable.
- **Power/flashing:** use **USB-Serial-JTAG**. This is separate from the controller connection.

The Waveshare screen works in this fork but is not a vendor-supported configuration. LCD setup and investigation are in [DSI-PREVIEW.md](DSI-PREVIEW.md).

## Planned: four-player wireless homebrew testing

The next unlock is **agents in the loop for all four players on the real Xbox**:
exercise multiplayer interactions in homebrew games, reproduce simultaneous
inputs, and capture video/audio plus logs to investigate lag, frame-rate drops,
and failures under multiplayer load.

The solder-free plan uses **four Waveshare ESP32-S3-Zero boards**, one per Xbox
controller port, coordinated over the home Wi-Fi network by the main ESP-KVM:

```text
Main ESP-KVM -- Wi-Fi --+-- S3-Zero -- USB data cable + adapter -- Xbox port 1
                       +-- S3-Zero -- USB data cable + adapter -- Xbox port 2
                       +-- S3-Zero -- USB data cable + adapter -- Xbox port 3
                       +-- S3-Zero -- USB data cable + adapter -- Xbox port 4
```

Each small board would emulate one XID controller and draw power from its Xbox
port through the adapter and USB cable. Controller commands flow from the main
ESP to the dongles and into the Xbox; the main ESP continues capture and test
coordination without its own USB controller connection to the Xbox. The intended
setup needs no soldering, headers, jumper wires, or separate dongle power
supplies. Each board is initially programmed over USB from a computer.

**Status as of 2026-10-17:** four boards are ordered and have not arrived; four
Xbox-to-USB adapters have been purchased in total (the original plus three more).
This is a planned firmware/hardware extension. First validate one dongle's XID
emulation and Xbox-port power with Wi-Fi active, then validate all four together.
Physical wireless gamepad pairing to the dongles would be a separate feature.

The testing work should add per-player routing, bounded sequences with automatic
release on timeout/disconnect, and coordinated runs with sequences sent ahead
for local execution. Save build IDs, per-player input order/timestamps, scheduled
versus actual execution timing, transport delays/loss, and available game/capture
performance evidence. Compare one-player and four-player runs so wireless timing
variation and capture delays can be distinguished from game slowdowns; game-side
instrumentation will be needed for precise frame-time attribution. Four-player
synchronization, latency measurements, and this logging workflow remain unbuilt.

| Purchased hardware | Paid (USD) |
| --- | ---: |
| Main ESP32-P4 board, C790 capture bridge, and LCD | $220.03 |
| Four Xbox-to-USB adapters: original $6.25 + three more $17.97 | $24.22 |
| Four ESP32-S3-Zero boards: $5.15 each, plus $10 shipping | $30.60 |
| **Documented all-in spend** | **$274.85** |

Both adapter orders had free shipping; the board order had $10 shipping. These
are actual historical purchase totals, including applicable tax/discounts, not
current quotes. USB data cables and the other existing rig accessories listed in
the [purchase notes](FORK-HISTORY.md#hardware-purchases) are excluded.

