<!-- SPDX-License-Identifier: Apache-2.0 -->
# Original Xbox Wi-Fi controller dongle

ESP-IDF 6.1 firmware for a Waveshare ESP32-S3-Zero (4 MB flash, 2 MB PSRAM).
One board emulates one Controller S in one physical original Xbox controller
port, powered through a controller-port USB adapter and USB-C data cable.
Player 1 has a verified 164-state real-console sweep. Four-player routing is
implemented and all four boards are provisioned. The isolation, simultaneous-state,
timeout and release checks passed with Xbox USB completions; see the
[four-player validation](../docs/S3-FOUR-PLAYER-VALIDATION.md). After repositioning
boards 2 and 4, the 2026-10-25 retest passed all routing checks and 90 continuity
samples per board without observed lease expiry or status failures. A subsequent
Guilty Gear Isuka VS match verified all four player slots, independent crouches,
analog movement, attacks and simultaneous jumps. See the report for screenshots
and the launch/capture restart limitation.
This is for the original Xbox, not Xbox 360/One/Series.

```text
Browser/agent -> main ESP32-P4 -> home Wi-Fi -> S3-Zero (one per player) -> Xbox controller ports 1..4
```

The main ESP retains video/audio capture and keyboard/touch/Bluetooth input
arbitration. Opt-in `KVM_XBOX_S3_PLAYER1` replaces its local USB-controller output
with authenticated Wi-Fi state. Both ends release stale input after 500 ms.
The Xbox readiness flag follows player 1's signed announcement of configured USB.
Players 2..4 have independent discovery, replay state, queues and timeouts.

## Configure, build, flash

Copy the root `.env.example` to ignored `.env` and enter the home Wi-Fi network.
Identify the S3 station MAC with esptool before flashing; USB port names can
change. First board: `ac:27:6e:b0:ce:a4`.

From the repository root, using ESP-IDF's Python:

```sh
python s3-dongle/tools/configure.py --mac ac:27:6e:b0:ce:a4
```

For additional boards, identify each station MAC and assign a unique slot:

```sh
python s3-dongle/tools/configure.py --player 2 --mac SECOND_BOARD_MAC --main-config build.s3-player1/sdkconfig
python s3-dongle/tools/configure.py --player 3 --mac THIRD_BOARD_MAC --main-config build.s3-player1/sdkconfig
python s3-dongle/tools/configure.py --player 4 --mac FOURTH_BOARD_MAC --main-config build.s3-player1/sdkconfig
```

The same private S3 firmware image works on all four boards: the main P4 maps
station MACs to player numbers. Label each board and plug it into the matching
Xbox port. Rebuild and flash the P4 after saving all identities. Reassigning a
slot preserves the other slots; assigning one board twice is rejected.

This preserves a saved pairing key in ignored `.env.s3-route` and writes two
ignored, mode-600 overlays. To preserve an existing P4 configuration, also pass
`--main-config build.s3-player1/sdkconfig`. Do not publish private overlays,
sdkconfigs, flash backups, or firmware images containing credentials.

Install the S3 compiler with ESP-IDF's `idf_tools.py install xtensa-esp-elf`,
source `tools/env.sh`, and ensure its compiler `bin` directory is on PATH if
that installation only exports P4 tools. Build the S3 from `s3-dongle/`:

```sh
idf.py -D 'SDKCONFIG_DEFAULTS=sdkconfig.defaults;../boards/s3_player1.private.defaults' build
idf.py -p /dev/cu.usbmodem101 flash
```

Build the P4 from the repository root, preserving any existing board-specific
settings in its sdkconfig. The example below selects Function EV + Xbox defaults:

```sh
idf.py -B build.s3-player1 -D SDKCONFIG=build.s3-player1/sdkconfig \
  -D 'SDKCONFIG_DEFAULTS=sdkconfig.defaults;boards/funcev_p4.defaults;boards/funcev_xbox.defaults;boards/s3_route.private.defaults' build
```

The S3 reuses the parent's TinyUSB dependency when present; otherwise it fetches
its pinned version itself. `-D S3_USE_PARENT_TINYUSB=OFF` exercises the standalone
registry build used in CI. No real credentials are required just to compile.
A thin S3 component shim reuses the existing vendored USB wrapper, with its
KVM-managed MSC class disabled; P4 defaults retain that class. Its XID source derives from
`../components/kvm_hid`, with the P4-only register include guarded.

Native USB is shared by XID and the ROM downloader, so the serial device disappears
when the controller starts. Hold BOOT while plugging into the Mac, then release
BOOT to program again. If an ordinary reset stays in download mode, esptool's
`--after watchdog-reset` starts the application reliably on the tested board.
The full original S3 flash and pre-routing P4 flash were backed up locally.

## Connectivity and diagnostics

The S3 joins the configured home Wi-Fi and retains recovery AP
`Xbox-S3-<MAC suffix>` with test password `XboxTest2026`. Its manual test page is
at `http://192.168.4.1` on that AP, or its DHCP station address on home Wi-Fi.
Anyone connected to the AP can use manual controls while P4 routing is inactive.

- `GET /`: manual controls for buttons, triggers and stick endpoints.
- `GET /status`: station address, accepted network packets, routing lease,
  XID-open state, USB reports submitted/host-completed, and held-input labels.
- `POST /report`: `application/octet-stream`, exactly 20 XID input bytes.
  Bytes 0/1/3 must be 0/20/0. Returns 400 for invalid input and 409 while fresh
  P4 routing owns input, preventing an open test page from overwriting it.

Signed discovery goes to subnet broadcast UDP 3335 once per second. The P4
sends 80-byte HMAC-SHA256 packets containing the full 20-byte XID state to UDP
3334 every 20 ms, with changed state sent immediately. The pairing key
authenticates but does not encrypt packets. The S3 boot nonce rejects prior-boot
state; sequence numbers reject same-session duplicates/reordering. A new P4
session waits for the previous 500 ms lease to expire. Lost discovery expires
P4 readiness after four seconds, but an already authenticated learned route
continues unicast output through lost beacons. Receiver boot checks and input
timeouts remain independent of discovery. Client isolation/separate subnets can prevent
discovery. This prototype is intended for the trusted home network.

Version 0.2.1 fixes a 5 ms receiver wait rounding to zero at 100 Hz. The receiver
always blocks at least one tick and runs below TinyUSB priority. Without that,
network packet acceptance could continue while USB service was starved.
Configured USB and accepted packet counts alone do not prove Xbox response;
check host completions and the real console's displayed input.

## Full-resolution control through the main ESP

The existing authenticated/control-owned `/ws` gains opcode `0x08` only in
Xbox firmware: `[0x08, ...20 XID bytes]` for player 1, or
`[0x08, player, ...20 XID bytes]` with player 1..4. Other
existing WebSocket formats and REST endpoints are unchanged. Invalid size,
output-report header, or reserved digital bits are ignored. The raw command
uses the same touch/Bluetooth priority and 500 ms expiry as keyboard input.
Release-all and control disconnect clear it. See the
[wire contract](../docs/XBOX-REMOTE-CONTROLLER.md#full-resolution-controller-state).

## Repeating the tests

Open Xbox Controller Tool's **Gamepad / Control Test Page** before sending test
states. These tools deliberately take control of the main ESP's session.

```sh
node s3-dongle/tools/test-player1.mjs http://esp.local http://S3_IP
node s3-dongle/tools/test-raw-player1.mjs http://esp.local http://S3_IP
node s3-dongle/tools/sweep-player1.mjs http://esp.local http://S3_IP NEW_EVIDENCE_DIR [PLAYER]
node s3-dongle/tools/test-four-players.mjs http://esp.local NEW_EVIDENCE_DIR http://P1_IP http://P2_IP http://P3_IP http://P4_IP
python s3-dongle/tools/probe.py --player 2 --test-neutral
```

The sweep captures all 164 expected states, screenshot evidence and actual USB
completion counts. It starts a live viewer so JPEG snapshots are current.
**Capture completion is not a display-verification PASS:** compare the Xbox's
numbers/highlights against `plan.json`. Small stick values may be gray while
still appearing numerically; do not infer a zero value from highlighting alone.
OCR can miss signs or text overlapping the rotating controller model and needs
visual review. A standalone JPEG can reuse an older published frame if no live
viewer is keeping encoding active.

Before the P4 starts routing, `python s3-dongle/tools/probe.py --test-neutral`
checks discovery, HMAC interoperability, malformed/replayed packet rejection,
and receiver lease expiry using only neutral state. Do not run it during the
live controller sweep.

See the [hardware validation report](../docs/S3-PLAYER1-VALIDATION.md) for the
observed ranges, method, results and limits. The four-player test checks per-board input isolation, simultaneous states,
independent expiry, global release and disconnect, with Xbox USB completions.
It does not establish physical Xbox port assignment or numeric display values;
check those in the console test app separately. Coordinated execution, rumble,
OTA, and a proportional browser gamepad panel remain future work. Return to local P4 USB output by disabling `KVM_XBOX_S3_PLAYER1` and rebuilding.
