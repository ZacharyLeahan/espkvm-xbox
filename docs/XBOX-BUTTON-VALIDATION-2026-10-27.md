<!-- SPDX-License-Identifier: Apache-2.0 -->
# Xbox button control: actual KVM validation, 2026-10-27

**Result:** GPIO46/47 are supported by the actual saved configuration and reported
pin reservations. The electrical installation and production button firmware are
not yet validated. This is permission to plan those pins, not a claim that the
finished remote-power system has passed acceptance.

Only read-only network queries and host-side tests were performed. No settings
writes, button endpoints, GPIO output commands, serial resets, flashing or Xbox
power operations were performed. No relay hardware was connected by this audit.

## What was checked on the running KVM

The KVM was reached at `esp.local` using GET `/api/v1/system/info`, `/api/v1/pins`,
`/api/v1/settings`, `/api/v1/settings/schema` and `/api/v1/video/status`.
The [filtered evidence snapshot](XBOX-BUTTON-VALIDATION-2026-10-27.json) excludes
credentials, SSIDs, addresses and device network identifiers.

| Live finding | Meaning |
| --- | --- |
| Board ID `funcev`, ESP-IDF v6.1 | Actual firmware identifies Function EV family |
| Firmware `v.0.56.1-167-g0c57c55-dirty`, built Oct 25 13:15:15, valid `ota_1` | Running firmware is older than current repository main; dirty image limits exact source reconstruction |
| `atx_enable=false`; POWER46, RESET47; sense=-1 | The intended two GPIOs are saved for button control, but output feature is disabled |
| `atx_short_ms=200`, long=5000, active-high=true | Existing short setting is a candidate; PC long-hold must not be used for Xbox recovery |
| `disp_enable=false` | Saved optional SPI-display pins are inactive; this does not disable the separate compiled DSI preview |
| Neither46 nor47 in all live fixed reservations | No reported conflict with capture, SD, Ethernet or C6 Wi-Fi |
| `headerVerified=false` | Software header table is not a physical board/pin measurement |
| ATX `known=false` | There is no configured GPIO power-state sensor; `on=false` does NOT establish Xbox OFF |
| Wi-Fi active at audit time | Do not assume Ethernet was the active recovery path; test both independently later |
| Video status showed signal720x480/60 and fresh frame age | Capture is reachable; this is not proof of Xbox liveness or future recovery |

The live schema permits button GPIO settings and short pulses50-2000ms. We did
not send a settings write to exercise validation or enable any output.

## Build/source cross-check

`build.s3-player1/espkvm.bin` contains the running version string. Its adjacent
configuration enables Function EV, audio22/53/27 and DSI preview, with ATX46/47.
This supports the assignment; a matching string does not prove identical flashed
bytes. Neither exact silicon revision3.2 nor physical PCBv1.6 was read from the
live API. Existing local profile notes support them but remain distinct evidence.

| Subsystem | Audited assignment / result |
| --- | --- |
| Capture | Live fixed I2C7/8 and reset23; MIPI path is separate |
| Audio | Version-matching local config: BCLK22, WS53, DIN27 |
| DSI/touch | Compiled DSI path uses MIPI and shared I2C control; inspected driver has no46/47 claim; external physical wiring still needs inspection |
| SD | Live43/44/39/40/41/42;45 reserved for SD_PWRn |
| Ethernet | Live28/29/30/31/34/35/49/50/51/52 reserved |
| Wi-Fi | Live14-19,54 and board-linked6 reserved |
| Debug | Local matching config uses USB Serial/JTAG; no46/47 claim found |
| POWER/EJECT |46/47 saved for ATX, not present in any of the above fixed claims |

**Header position is not GPIO number:** J1 physical36 is GPIO46; physical37 is
GPIO47; physical39 is GND. GPIO39 itself is microSD D0 and must not be used as
the proposed ground connection. Official P4X J1 table agrees with this mapping.

The existing host pin-conflict suite was compiled with C11, Wall/Wextra/Werror
and all10 checks passed. Additional read-only assertions against live JSON passed:
46/47 outside fixed reservations, saved ATX46/47, ATX disabled, display disabled;
matching local audio assignments are distinct. These tests do not measure pads.

## Firmware limitations confirmed by source audit

Current `components/kvm_atx/kvm_atx.c` has one worker and four queue slots. It
serializes pulses but does not deduplicate them. Initialization sets direction
before writing idle level. Release depends on worker execution; there is no
independent hardware pulse limit. Status reads the sense level without freshness
or debounce. Source comments explicitly call the hardware untested.

The authenticated POWER click route returns after queue admission; success is
not pulse completion or boot confirmation. The second channel/API is named RESET,
not EJECT, and shares short timing. Saved defaults alone do not make it an Xbox
EJECT feature. Firmware work remains: explicit EJECT semantics, independent timing,
safe initialization, bounded release policy, busy/dedup/operation status and
restart/timeout reconciliation. Keep long-hold disabled for the Xbox profile.

## Exactly what still needs hardware testing

| Stage | Test / evidence required | Owner |
| --- | --- | --- |
| Before permanent solder | Confirm actual front-board orientation and identify opposite POWER/EJECT electrical contacts with AC disconnected; photograph labels and continuity results | Installer |
| Before permanent solder | Verify physical ESP J1 orientation and confirm pins36/37 are available on this exact board/cabling; no output drive is needed for inspection | Owner/installer |
| Relay bench, detached from Xbox | On independent5V, verify3.3V input activation, NC unused, NO open at idle and each NO/COM closure; check correct pair labels | Owner |
| Relay bench | Measure actual GPIO/pulse with analyzer/scope using suitable test firmware; release duration and both channels independent | Owner |
| Relay fault bench | ESP boot/reset/off while relay supply is on; no accidental closure or backfeed. Test supply loading with both channels and stuck-active policy/hardware limiter | Owner |
| Console lead installation | Use appropriately sized wire, insulation, strain relief, keyed mates and safe case route; physical buttons/LEDs work after modification | Installer |
| Powered console | Measure button idle/pressed voltage and demonstrate short isolated closure starts from standby; qualify POWER/EJECT timing and Stellar boot behavior separately | Installer/owner |
| AC restoration | Measure/verify main-rail decay and standby readiness; after AC restore detect already-on versus off and press only when appropriate | Owner/installer |
| State sensing | Calibrate fresh metering bands or add isolated rail sense; UNKNOWN/disconnected sensor must not trigger repeated presses | Owner |
| Whole rig | Repeat recovery with capture/audio/DSI/touch/SD active and Ethernet/Wi-Fi paths; demonstrate KVM stays independently powered; test all four Xbox controller ports | Owner |
| Acceptance | Lost/duplicate requests, restarts during every phase, and supervised genuine-freeze recovery; no blindly repeated toggle | Owner |

The installer can identify/test button contacts without the finished ESP firmware.
Those passive leads do not require a working API. To avoid commissioning risk,
bench-test the relay/controller side detached from the Xbox before connecting the
finished leads. Do not call this unattended-ready until firmware and full hardware
acceptance pass. A normal successful startup alone does not prove freeze recovery.

## References

- [Current wiring and parts](../XBOX-POWER-RELAY-SETUP.md)
- [Installer scope: Xbox leads only](../XBOX-POWER-ADAPTER-BUILD-BRIEF.md)
- [XWiFi source review and integration plan](XBOX-XWIFI-BUTTONS.md)
- [Espressif P4X J1 mapping](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32p4/esp32-p4x-function-ev-board/user_guide.html)
- [Relay3-5V signal/power specifications](https://learn.adafruit.com/adafruit-stemma-non-latching-mini-relay/pinouts)
- Local source: `components/kvm_config/test/test_pin_conflict.c`,
  `components/kvm_board/board_reserved.c`, `components/kvm_atx/kvm_atx.c`,
  `components/kvm_web/http_server.c`, `components/video_pipeline/capture_dsi_preview.c`.
