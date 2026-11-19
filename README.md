# ESP-KVM for the Original Xbox

**Give a coding agent eyes, ears, and controller input on a real Xbox.**
This [ESP-KVM](https://github.com/espkvm/espkvm) fork turns an ESP32-P4 into a
hardware testing interface: an agent can control the console, see its HDMI
output, hear captured HDMI audio, take screenshots, and save video/audio
recordings to guide its next code change.

## Why an agentic loop matters

**Build → deploy → play → see and hear the result → fix → repeat.**

Instead of asking a person to test every change, an agent can send repeatable
controller inputs, inspect the actual console output, and use screenshots and
recordings to investigate failures and check the next build. That shortens the
feedback loop for gameplay, menus, graphics, and sound while keeping evidence
from real hardware.

[Slime Volley for Xbox](https://github.com/ZacharyLeahan/slimevolley-nxdk) is a
practical example: we rapidly developed and tested the playable nxdk port using
this agentic loop. The agent used ESP-KVM to inspect the screen, send controller
inputs, and capture the results while iterating on the game. The project's
[gameplay screenshots and testing notes](https://github.com/ZacharyLeahan/slimevolley-nxdk#agent-testing-on-real-hardware)
show that workflow on an original Xbox. Builds were deployed and read back
through the Xbox dashboard's FTP service; ESP-KVM supplied observation and input.

The pieces work today through agent tools and scripts. The
[scenario runner](docs/XBOX-SCENARIOS.md) repeats reviewed controller sequences,
collects timestamped screenshots/status, and stops on failed checks or lost
control. Screen interpretation, automatic deployment/launch, and reliable
recovery from console hangs remain separate work.

**Credit:** the runner's repeat-and-classify workflow is inspired by
[kooscode/OXOS](https://github.com/kooscode/oxos), especially
[`tools/modxo/bootrun.py`](https://github.com/kooscode/oxos/blob/bfec2af8a33528c835cc9dada484142ecaf58f21/tools/modxo/bootrun.py).
We borrowed the test-loop technique and wrote a new ESP-KVM implementation;
no OXOS code was copied. Our new [Xbox diagnostic title](diagnostics/xbox/README.md)
also borrows the technique of an advancing status panel and explicit controller
acknowledgments from OXOS's
[image test](https://github.com/kooscode/oxos/tree/bfec2af8a33528c835cc9dada484142ecaf58f21/tests/image-test-title) and
[controller test](https://github.com/kooscode/oxos/tree/bfec2af8a33528c835cc9dada484142ecaf58f21/tests/controller-test-title).
It adds a machine-readable visual panel so the runner can check progress and
input delivery through existing ESP-KVM screenshots, without OCR or a debug BIOS.
The title builds locally; physical-console validation remains pending.
OXOS's Modxo power control, serial boot checks and kernel netboot are not included.

## What it can do

| Capability | For people and agents |
| --- | --- |
| **Control** | Emulate an original Xbox controller over USB; send bounded network input sequences with automatic release. Browser keys, touchscreen sticks, and a Bluetooth Xbox Series controller also work. |
| **See** | Capture 480p/720p HDMI, view over LAN/Tailscale, and grab screenshots as test evidence. An optional touchscreen provides local preview. |
| **Hear** | Capture stereo HDMI audio for browser listening or bounded WAV recordings on a computer, giving agents sound evidence alongside images. |
| **Record** | Save H.264 video with optional AAC stereo audio to microSD, then export MP4 with sound. GIF exports are silent. |

Audio uses the C790's **separate I2S connector**; the camera ribbon carries video
only. Stereo LPCM is required. There is one network audio listener, so mute the
browser before using the WAV tool; microSD recording has its own subscription.
Brief audio gaps remain possible, and audio/video synchronization and end-to-end
latency are unverified. A remote video viewer reduces local LCD preview to at
most 1 FPS. See [measurements and remaining limits](docs/XBOX-RIG-DETAILS.md).

**Known issue: HDMI audio is unstable over Wi-Fi.** Fully wired Ethernet audio
was stable in a three-minute stereo capture with no measured transport gaps,
and browser playback was verified. Wi-Fi captures still lose audio: a recent
30-second test delivered only 25 seconds of samples. Use Ethernet on both the
KVM and viewing computer for reliable audio on the tested rig. The wireless
cause remains unresolved; see [hardware test results](docs/C790-TRANSPORT-2026-10-25.md).

## Hardware

The tested rig uses an **ESP32-P4 Function EV** (rev 3.2, C6 Wi-Fi),
**Geekworm C790** HDMI-to-CSI bridge, and optional **Waveshare 3.5-inch DSI LCD (E)**.

- **Video:** Xbox HDMI → C790 → ESP **MIPI CAMERA**; optional **MIPI DISPLAY** → LCD.
- **Audio:** C790 I2S → ESP headers using the [verified wiring](docs/HDMI-AUDIO.md#header-cross-check-2026-10-16).
- **Input:** Xbox **player 1** port → USB adapter → USB data cable → ESP **DEVICE USB**.
- **Power/flashing:** separate **USB-Serial-JTAG** connection. Power off before handling ribbons.

The main board, bridge, and LCD cost $220.03. Documented spend including four
controller adapters and four S3-Zero boards is **$274.85**;
[receipt details and exclusions](docs/FORK-HISTORY.md#hardware-purchases).

**Standing wiring rule: DEVICE USB is always player 1**, never P2-P4. Optional
Keep the original four labelled Wi-Fi S3 assignments. Enable
`CONFIG_KVM_XBOX_S3_PLAYER1` and `CONFIG_KVM_XBOX_DIRECT_PLAYER1` together
for automatic P1 switching: configured DEVICE USB takes priority, and P1
returns to its S3 when that cable is removed. P2-P4 keep their own S3 routes.
The LCD keeps a compact four-controller status row and readable ESP LAN IPs;
tap a player for its IP, counters and input details. Unseen/stale slots stay
visible without covering the game with diagnostic cards. See the [agent hardware/test-loop entrypoint](docs/AGENT-XBOX-TESTING.md#fixed-controller-wiring-and-current-test-loop).

**Previously validated all-S3 configuration:** each Waveshare ESP32-S3-Zero emulates one Xbox
controller, receiving full-resolution state from the main KVM over home Wi-Fi.
For player 1, all buttons, trigger pressure, both stick axes and circular sweeps were checked
in Xbox Controller Tool: [164-state hardware validation](docs/S3-PLAYER1-VALIDATION.md).
All four boards passed route isolation, independent expiry and global release,
then controlled separate characters and simultaneous jumps in a real
Guilty Gear Isuka four-player match:
[four-player validation and limits](docs/S3-FOUR-PLAYER-VALIDATION.md).
[Build and pairing instructions](s3-dongle/README.md) cover all four boards.

## For coding agents in other game repositories

Start with the [agent testing entrypoint](docs/AGENT-XBOX-TESTING.md). The
[reusable Xbox hardware-testing skill](agent-skills/xbox-hardware-testing/SKILL.md)
routes agents to the shared tools and current hardware limits while keeping
game build/deploy instructions in the game's own repo. Local user-level pointers
are installed for Codex and Claude Code on this Mac; another machine needs its
own skill/instruction installation.

## Build and use

Clone with submodules and follow the [Xbox build instructions](docs/XBOX-FORK.md)
(ESP-IDF 6.1). For the LCD, use the
[recommended profile](docs/DSI-PREVIEW.md#recommended-build); initial startup can
take roughly 30 seconds. Copy [`.env.example`](.env.example) to an ignored `.env`
for Wi-Fi settings and keep credentials and firmware containing them private.
After editing `web/`, run its typecheck, tests, and build before rebuilding firmware.

Enable **HDMI audio** in Settings for capture and microSD sound; click
**Enable audio** to listen in the browser. See the [audio setup](docs/HDMI-AUDIO.md).

**Security:** this experimental profile uses HTTP without an application login.
Use a trusted, restricted LAN and do not port-forward it. Tailscale protects
traffic carried over the tailnet.

| Guide | Contents |
| --- | --- |
| [Controller status](docs/CONTROLLER-STATUS.md) | Web and touchscreen P1-P4 indicators, activity and USB report diagnostics. |
| [Four-player overview](docs/FOUR-PLAYER-OVERVIEW.md) | How the main ESP and four wireless controller dongles support multiplayer testing. |
| [Agent testing](docs/XBOX-TESTING.md) | Observation helper, screenshots, fresh-frame evidence, and regression checks. |
| [Controller setup](docs/XBOX-REMOTE-CONTROLLER.md) | USB wiring, Bluetooth pairing, mappings, and limits. |
| [Recording](docs/SD-RECORDING.md) | microSD recording, MP4 export, and GIF workflow. |
| [Rig details](docs/XBOX-RIG-DETAILS.md) | Measurements, connections, limitations, and four-player plan. |
| [Fork history](docs/FORK-HISTORY.md) | Receipts, earlier tests, and development direction. |
| [Upstream review](docs/UPSTREAM-REVIEW.md) | Selective adaptations and pending device acceptance. |

The maintained branch is [`main`](https://github.com/ZacharyLeahan/espkvm-xbox/tree/main).
Feature branches hold work under review; combined-device acceptance for
`upstream-reliability-xbox` remains in progress in the review ledger.

## Credits

Included software and protocol references are distinguished below. Original attribution remains in [NOTICE](NOTICE); the project is licensed under [Apache-2.0](LICENSE).

| Repository | Contribution or reference |
| --- | --- |
| [espkvm/espkvm](https://github.com/espkvm/espkvm) | Upstream capture, networking, web KVM, recording, and USB infrastructure. |
| [jrowny/p4kvm](https://github.com/jrowny/p4kvm) | Inherited TC358743 bring-up and CSI bridge programming. |
| [espkvm/console](https://github.com/espkvm/console) and [espkvm/microlink](https://github.com/espkvm/microlink) | Web console and native Tailscale foundations; this tree uses console and [our microlink fork](https://github.com/ZacharyLeahan/microlink) as submodules. |
| [Ryzee119/ogx360](https://github.com/Ryzee119/ogx360) and [ogx360_t4](https://github.com/Ryzee119/ogx360_t4) | XID protocol/design references for our independent USB implementation; their implementation code was not incorporated. |
| [rear16/ESP32-XboxBLE](https://github.com/rear16/ESP32-XboxBLE) and [asukiaaa/esp32-client-for-xbox-controller-with-nim-ble](https://github.com/asukiaaa/esp32-client-for-xbox-controller-with-nim-ble) | Series controller report mapping and NimBLE client/identity references. |
| [waveshareteam/Waveshare-ESP32-components](https://github.com/waveshareteam/Waveshare-ESP32-components) | Display-driver comparison and compatibility investigation; our 3.5-inch E findings are in [issue #184](https://github.com/waveshareteam/Waveshare-ESP32-components/issues/184#issuecomment-5973339430). |
| [torvalds/linux](https://github.com/torvalds/linux) and [raspberrypi/linux](https://github.com/raspberrypi/linux) | ICN6211/DSI panel and host references; Linux's TC358743 register map and HDMI-mode audio PLL handling informed the audio fix. |
| [espressif/esp-adf-libs](https://github.com/espressif/esp-adf-libs/tree/master/esp_audio_codec) | Optional `esp_audio_codec` AAC-LC encoder for microSD audio; its Espressif-only license and VisualOn notices remain in the managed dependency. |
| [espressif/esp-idf](https://github.com/espressif/esp-idf), [esp-hosted-mcu](https://github.com/espressif/esp-hosted-mcu), and [esp-bsp](https://github.com/espressif/esp-bsp) | Platform, C6 connectivity, and GT911 touch references; [esp-hosted-mcu issue #241](https://github.com/espressif/esp-hosted-mcu/issues/241) informed the BLE pairing crypto fix. |

Fork work combines these foundations for Xbox: capture/LCD tuning, panel bring-up, touchscreen recording and overlays, XID output, Bluetooth translation, input history, and remote HDMI audio. Detailed provenance and tests are in the [controller notes](docs/XBOX-REMOTE-CONTROLLER.md), [LCD investigation](docs/DSI-PREVIEW.md), [audio notes](docs/HDMI-AUDIO.md), and [upstream review ledger](docs/UPSTREAM-REVIEW.md).

## Automatic Xbox player-one handoff

With `KVM_XBOX_S3_PLAYER1` and `KVM_XBOX_DIRECT_PLAYER1`, main DEVICE USB
(always wired to Xbox P1) takes priority when configured; otherwise P1 uses its
paired S3. Wi-Fi P2-P4 retain their assignments. The overlay changes P1
DIRECT/WIFI automatically. See [configuration and hardware verification](docs/CONTROLLER-HANDOFF.md).
