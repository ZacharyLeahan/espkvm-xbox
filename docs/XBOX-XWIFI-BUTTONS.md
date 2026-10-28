<!-- SPDX-License-Identifier: Apache-2.0 -->
# Reusing XWiFi's button technique with ESP-KVM

Investigated 2026-10-27. Recommendation: reuse the physical-button technique,
not the complete EEPROM application. The existing ESP-KVM plus two isolated
contact channels can perform it. No additional Pico, internal Wi-Fi module or
IR receiver is required. Hardware behavior still needs installer qualification.
No firmware, settings, flashing or console power was changed in this investigation.

## The power arrangement

```text
UNSWITCHED POWER                         SWITCHED XBOX POWER
  |
  +-> ESP-KVM supply -> ESP32-P4          Smart plug -> Xbox AC supply
  |                       |                               |
  +-> one 5 V supply      |                         Xbox standby rail
         |                |                               |
         +-> 2 relay      +-> GPIO46/47               Xbox SMC
             drivers          control signals             |
                 |                                        |
           isolated NO/COM contacts ----------------> POWER / EJECT inputs
```

Each NO/COM pair goes across one verified physical button's contacts. Closing
it emulates the button; opening it releases the button. Preserve the original
buttons/LEDs. Professionally soldered, insulated leads with strain relief and
a four-pin sidewall port are the selected installation. Use two loose keyed
disconnects only if port mounting is impractical. The passive inline adapter
remains a historical alternative. See the [current linked shopping list](../XBOX-POWER-ADAPTER-BUILD-BRIEF.md#parts-to-send-the-installer).

The ESP and both relay drivers remain powered when Xbox AC is absent. They do
not power the Xbox. After AC restoration, the Xbox PSU supplies its own standby
circuit, which can respond to the physical POWER input without Xbox software
running. A healthy standby supply and button controller are prerequisites.
This is cold startup from standby after AC restoration, not startup with no AC.
No accessory current is drawn from Xbox standby in this recommended arrangement.

Do not add an ESP-ground-to-Xbox-ground wire or drive the button with ESP 3.3 V.
The two contact pairs are the only new electrical interface to the Xbox.
Existing HDMI/USB grounds may already be common elsewhere; do not claim whole-rig
isolation. Test detached contact/control assemblies when checking isolation.

## What the linked project establishes

The XWiFi section of Xbox-EEPROM-Utility defines POWER/EJECT GPIO actions and a
separate power-state input. Its source drives a button low, waits 120 ms (POWER)
or 500 ms (EJECT), briefly drives high, then changes the pin to input. Its
`pwr` handler always issues a pulse; it is not an idempotent ON command.
XWiFi builds explicitly reject ESP32 as untested. Its GPIO numbers and direct
wiring must not be copied onto the P4. These source timings are starting points
for qualification, not Xbox timing specifications.

Reimplement assert/release through isolated contacts instead. Do not reproduce
the active-high release transient, raw GPIO-to-Xbox connection or EEPROM/SMBus
features. No SDA/SCL, HDD-key, LED-control or controller-port wiring is needed.

## Wiring and parts decision

| Part / connection | Proposed implementation |
| --- | --- |
| POWER channel | GPIO46 / J1-36 -> #3894 SIG -> Adafruit #4409 input |
| EJECT channel | GPIO47 / J1-37 -> second #3894 SIG -> second #4409 input |
| Control ground | J1-39 -> both relay grounds and external supply negative |
| Relay power | One regulated independent 5 V supply, branched to both boards |
| Xbox side | Each relay NO/COM across its verified POWER or EJECT contacts; NC unused |
| Disconnects | #5319 sidewall socket + #5318 plug; two #2880 sets as backup |
| State evidence | Fresh calibrated plug metering initially; isolated switched-rail sensing if inadequate |

These are the same two channels in the [current photo wiring plan](../XBOX-POWER-RELAY-SETUP.md).
The #4409 has an onboard driver suitable for 3 V/5 V control, so GPIO does not
supply the coil current. Verify total supply loading, startup idle, power-off
backfeed and pulse release. GPIO46/47 remain candidates, not live-verified free
pins: runtime ATX/display claims and all build peripherals must be checked.
Audio22/53/27 and capture/display/touch/network/SD/debugging must remain intact.

A technician-built PhotoMOS/optocoupler interface can replace the mechanical
relays and may avoid the separate 5 V supply. That is a different circuit requiring
LED-current, leakage, output polarity and off-state tests. A bare GPIO solution
requires common grounds and careful unpowered-device analysis; it is not our
recommended interface for an independently powered KVM and AC-cycled Xbox.
Do not attach both an XERC and new contact controller without reviewing interaction.

## Firmware: mostly an existing worker, with reliability work remaining

`components/kvm_atx/kvm_atx.c` already serializes pulses; the HTTP layer already
has authenticated `POST /api/v1/power/click`. However:

- The second channel is currently called RESET, not EJECT. Add explicit Xbox
  button semantics/API and per-button pulse settings; keep PC long-hold operations
  disabled for the Xbox profile. Do not silently relabel `/power/reset`.
- Four queue slots plus an active action allow repeated physical presses. Add
  operation IDs, deduplication, busy admission and cooldowns shared across buttons.
- Current `status:ok` means queued, not released/completed. Provide operation
  status and distinguish pulse completion from observed console ON or booted.
- Configure outputs inactive before enabling drive, validate every pin, and test
  reset/reconfigure/error exits. A task delay is not an independent release bound;
  use a hardware limiter if that guarantee is required for MCU hangs.
- Power evidence needs debounce, freshness and UNKNOWN state. Never infer Xbox
  state from its programmable ring LED alone. `ensure_on` must observe first.
- Update skill/MCP/console API documentation with implementation. No external
  XWiFi Arduino server or EEPROM endpoints need to be imported.

## AC restoration and acceptance

1. Smart plug explicitly OFF; confirm Xbox main power disappeared and wait the
   measured drain interval. Keep ESP/network/relay supply alive.
2. Smart plug explicitly ON; confirm actual restoration, then wait measured
   standby readiness. Plug relay state alone does not prove downstream AC.
3. Read fresh main-power evidence. If ON, skip the button. If OFF with confirmed
   restored standby, send one bounded POWER pulse. UNKNOWN means stop/reconcile.
4. Verify a new boot sequence and useful response via KVM. A completed pulse,
   HDMI lock, watts or ping alone does not prove recovery.
5. On timeout/restart, query actual state before any retry. Never replay a toggle
   blindly. Use bounded retries and cooldowns. EJECT remains available to Codex by
   explicit command, including deliberate eject-start; test Stellar behavior.

Installer acceptance: continuity-map real switch contacts; measure standby absent
with AC off and present with AC restored/console off; verify physical and remote
POWER/EJECT independently; qualify pulse widths; test ESP resets and independent
supply sequencing for unintended presses/backfeed; prove all four controller ports
work; repeat AC restore/startup trials and a supervised genuine-freeze recovery.
Record exact contact locations, observed readiness/dwell times and power thresholds.
An unplugged sensor or failed PSU must not cause repeated speculative button presses.

## Sources

- [Project README/XWiFi](https://github.com/dx4m/Xbox-EEPROM-Utility#xwifi)
- [Source: button routines and network handler](https://github.com/dx4m/Xbox-EEPROM-Utility/blob/master/xbox_eeprom_utility/xbox_eeprom_utility.ino), functions `toggleConsoleOnOff`, `toggleEject`, `isConsoleOn`, `handleXWiFi`
- [Microsoft hardware design specification](https://consolemods.org/wiki/images/a/ae/Xbox_Hardware_Design_Specification_1.02.pdf): standby-powered controller and active-low POWER input; older architecture reference, not a revision-1.4 solder map
- [Microsoft debug guide, including Xblade SMC](https://consolemods.org/wiki/images/2/27/Microsoft_PowerPoint_-_Debug_Guide_for_the_xbox_Tuscany.pdf): standby and switch signals
- [Adafruit #4409](https://www.adafruit.com/product/4409) and [pinout](https://learn.adafruit.com/adafruit-stemma-non-latching-mini-relay/pinouts)
- [Existing installer brief](../XBOX-POWER-ADAPTER-BUILD-BRIEF.md) and [firmware investigation](XBOX-POWER-CONTROL.md)
