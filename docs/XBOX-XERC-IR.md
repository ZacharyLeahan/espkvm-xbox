<!-- SPDX-License-Identifier: Apache-2.0 -->
# XERC infrared control from the existing ESP-KVM

Status: unselected alternative retained for reference. The preferred design is
[existing ESP-KVM direct button control](XBOX-XWIFI-BUTTONS.md). This is not a validated replacement for
the [two-relay plan](../XBOX-POWER-RELAY-SETUP.md). No firmware, live settings,
Xbox power or unrelated working-tree changes were modified. Reviewed 2026-10-24.

## What connects to what

```text
Codex -> ESP-KVM network API -> ESP32-P4 RMT -> IR emitter
                                                 : infrared light
                                                 v
Xbox standby supply -> installed XERC + IR receiver -> POWER / EJECT circuitry

Independent smart plug -> Xbox AC only
Unswitched supply -> ESP-KVM, emitter and network
```

The ESP becomes a programmable remote. There is no new electrical connection
between its GPIO and the Xbox. The installer still solders the XERC inside the
console and mounts its receiver where the emitter can reach it. This path can
replace the external button relays if qualified; it does not eliminate Xbox work.

## Source-verified findings

- Espressif documents RMT transmission with carrier modulation and GPIO routing
  on ESP32-P4. One TX channel can send multiple button codes sequentially.
- The XERC 2 XE manual v1.2 describes configurable short/long power, eject and
  discrete-off actions, with supported RCA, Sony SIRC and Philips protocols.
  Short-power can start the console; holding its IR command invokes Safe Off.
  There is no documented universal discrete-ON command in that manual.
- The supplier describes standby-powered operation, including 3.3 V standby on
  Xbox 1.0-1.4. This supports operation independent of Xbox application execution;
  actual freeze behavior and Stellar interaction still need testing.
- Basic XERC use does not require the Microsoft DVD USB dongle. Do not install
  the optional shared-dongle/controller-port wiring: retain all four ports.
  XERC alone does not provide general dashboard/DVD remote input over USB.

## Candidate ESP wiring and parts

| Item | Proposal / verification gate |
| --- | --- |
| Emitter | Adafruit **5639** High Power IR LED Emitter; onboard FET drives LEDs |
| Cable | Adafruit **3894**, 3-pin JST-PH to female sockets; builder adapts power branch |
| Signal | GPIO46, J1 physical 36, candidate RMT TX; cannot also drive POWER relay |
| Ground | J1-39 to emitter ground and emitter supply negative |
| Supply | Regulated independent **3.3 V** supply with >=500 mA capacity proposed; validate pulse response |
| Optional capture tool | Appropriate IR receiver/logic analyzer for the selected carrier; not needed permanently |

Adafruit recommends matching emitter supply to MCU logic voltage. Use 3.3 V here;
its guide quotes about 200 mA total LED-on current at 3 V. This is not a tiny
GPIO-powered LED: the GPIO supplies signal only. Do not assume the ESP header
3.3 V rail has sufficient spare current. Do not feed the earlier 5 V relay supply
directly into this proposed 3.3 V arrangement. A separate regulator is an option.
Mount the emitter rigidly near the receiver; test alignment, ambient light and
close-range saturation. No battery is required.

The local Function EV header table maps GPIO46/47/48 to J1-36/37/33. GPIO47 is
an alternate TX candidate if unclaimed; GPIO48 remains reserved for optional
rail sensing. Audio remains GPIO22/53/27. `boards/funcev_p4.defaults` describes
capture I2C7/8, SD39-44, Ethernet31/52/50/51 plus RMII data pins, and C6 SDIO14-19
with reset54. `board_reserved.c` also reserves GPIO6 and45 for board functions.
Neither 46 nor47 is in that fixed reservation list, but runtime ATX/display
settings may claim them. The earlier relay plan reserves both. Select one backend
per pin; inspect all effective claims, DSI/touch auxiliaries and the exact PCB
before wiring. A cached `build.funcev-private/config/sdkconfig.h` assigns ATX POWER46 and
RESET47; it is not proof of current live ownership. No live GPIO dump was taken. Product naming does not independently establish PCB/silicon revision.

## Commands: what is known and what must be captured

The supplier-linked archive at commit `391a1725d4111018deee8f1ea7395d974bf63fbf`
contains `xerc2_source.rar`. `default_btn_map.asm` and `xerc2.inc` define:

| Stored function | Archived three-byte default |
| --- | --- |
| Short power / Safe Off | `15 2A 00` |
| Long power / Safe Off | `15 1D 00` |
| Eject / Safe Eject | `15 15 00` |
| SmartXX | `00 00 00` (unassigned) |
| Discrete off | `00 00 00` (unassigned) |

These are **internal decoded comparison records, not transmit packets**. Do not
paste them into a generic NEC encoder. The archive is a supplier-linked historical
reference, not proof of the installer's flashed image or saved assignments.

Preferred commissioning: record exact XERC version, then capture a known-working
remote's carrier, protocol/address/command, bit order, frame/repeat timing and hold
behavior. Alternatively teach the XERC a reproducible supported code set. Preserve
that profile across ESP and Xbox AC restarts. Keep short-power and discrete-off
on distinct codes. Do not guess carrier frequency from the phrase "Xbox remote".

Expose actions honestly: `press_power`, `press_eject`, and a separately validated
`discrete_off`. An eventual `ensure_on` operation must check fresh OFF evidence
before transmitting short-power. Neither TX completion nor HTTP success proves
IR reception or console state. No existing endpoint currently implements IR.

## What we can validate before installation

- **Established:** P4 RMT/carrier capability, supported XERC protocol families,
  candidate emitter electrical requirements and board-header mapping.
- **New source finding:** archived `xerc2.asm` checks `PO_PIN` for on/off state.
  While ON, normal POWER and EJECT go through `SAFE_CMD_CHECK`; `SAFE_COUNT_=50`
  counts matching repeats before actuation. Discrete-off is separately gated
  on the ON input. Off-state short-power actuates directly. Do not equate this
  to generic toggle behavior or assume one frame ejects an already-on console.
  This is evidence about the archived implementation, not the installer's unit.
- **Still needed now:** exact XERC board/firmware identity, receiver part number,
  remote/code assignment and installer confirmation of basic (no USB dongle)
  wiring. Capture or select protocol/carrier only after those are identified.
- **Software bench stage:** compile RMT integration against the target profile,
  verify generated waveform/repeat/stop behavior with an analyzer or matching
  receiver, and test request deduplication. Compile success alone cannot prove
  optical delivery or XERC response. No firmware was built/flashed in this review.

Ask the installer to demonstrate these before accepting the finished installation:

| Demonstration | Required result |
| --- | --- |
| AC restored, console off | XERC becomes ready; remote starts it; record readiness delay |
| Already running | Brief POWER does not unexpectedly shut down; qualify exact safe-hold behavior |
| Deliberate OFF | Dedicated OFF works when on and does not start it when off |
| EJECT | Works in standby and while running with the required repeat timing; record Stellar boot behavior |
| Persistent setup | Same commands still work after several AC disconnect/restores |
| Ports | All four controller ports remain usable; no DVD-dongle port takeover |
| ESP emitter | Same results at installed distance/angle and ambient lighting |
| Freeze recovery | Supervised hard-freeze AC cycle produces a new boot while KVM stays reachable |

A working handheld remote proves the Xbox installation first. The ESP must then
reproduce that exact optical command profile. Keep these as two separate tests
so a transmitter/software problem is not confused with XERC installation failure.

## Focused firmware plan

1. Add disabled-by-default `kvm_ir` using ESP-IDF6.1 `esp_driver_rmt`, one TX
   channel, configured carrier and a protocol encoder or captured raw timings.
   Use finite transmissions, bounded repeats, idle-low output and a physical
   pull-down if needed for boot sequencing. Never default to infinite TX loops.
2. Add settings/capability reporting and complete pin-conflict validation,
   including compiled audio and runtime ATX/display assignments. No first-party
   RMT use was found in `components/` or `main/`; dependencies must also be checked
   for channel/memory availability at build/runtime.
3. Add authenticated semantic commands and operation status with request IDs,
   busy rejection, finite deadlines and transmitted/observed distinction. Update
   companion skill/MCP API documentation when implementation lands. Existing
   `/api/v1/power/click` is ATX, not an IR sender; do not silently change meaning.
4. Test encoder frames/repeats against a trusted capture, startup idle state,
   duplicate/lost replies, resource exhaustion and disable/reset during TX.
   Exercise under capture/audio/display/SD/network load. No flash in this phase.

## Recovery and installer acceptance

Use explicit smart-plug OFF, confirmed rail decay/dwell, explicit ON and confirmed
restoration. Wait for XERC readiness; measure that delay rather than guessing.
If fresh calibrated power evidence says OFF, send one bounded short-power action;
if ON, do not press. UNKNOWN inhibits action. After timeout, reconcile state
before retrying. Bound retries/cooldowns and reconcile after automation restarts.

Confirm a fresh HDMI boot progression plus useful response; HDMI presence,
watts or ping alone is insufficient. A frozen console may ignore button events,
so retain AC cycling as the hard-freeze mechanism. XERC discrete off is not an
AC disconnection. Qualify real frozen-state recovery separately from normal boots.

Installer must demonstrate: four simultaneous controller ports; physical buttons;
IR POWER/EJECT and selected Stellar boot paths; learned-code retention after AC
loss; no unintended startup during ESP reset; emitter alignment margin; repeated
AC restore/startup with no duplicate toggles; and KVM/network remaining powered.
Record exact code profile, receiver location, timing, state thresholds and results.

## Sources

- [XERC supplier installation/features](https://tkscustomsupermodz.com/products/xerc-2-xe)
- [Original XERC 2 XE manual, supplier-linked archive](https://github.com/htbrdd/Xerc-2-Remake/blob/391a1725d4111018deee8f1ea7395d974bf63fbf/XERC2XE-UsersManual.pdf), pp.2-5
- [Archived source package](https://github.com/htbrdd/Xerc-2-Remake/blob/391a1725d4111018deee8f1ea7395d974bf63fbf/xerc2_source.rar)
- [ESP-IDF6.1 P4 RMT](https://docs.espressif.com/projects/esp-idf/en/stable/esp32p4/api-reference/peripherals/rmt.html)
- [Emitter5639](https://www.adafruit.com/product/5639) and [electrical pinout/current](https://learn.adafruit.com/adafruit-high-power-infrared-led-emitter/pinouts)
- Local: `components/kvm_board/board_header.c`, `board_reserved.c`,
  `components/kvm_config/pin_conflict.c`, `boards/funcev*.defaults`,
  `components/kvm_atx/kvm_atx.c`, `components/kvm_web/http_server.c`.
