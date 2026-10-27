<!-- SPDX-License-Identifier: Apache-2.0 -->
# Roadmap: hard power-cycle recovery after an Xbox hard freeze

Updated 2026-10-27. Status: preferred approach is existing ESP-KVM with two
isolated button-contact channels. Hardware
procurement, custom harness verification, firmware work and acceptance remain.
This plan does not implement firmware or authorize live settings, flashing or AC
switching. Hardware qualification remains required.

## Purpose

Recover remotely from a genuine hard freeze: remove Xbox AC, wait for validated
power decay, restore AC, and briefly press POWER only if fresh evidence confirms
the console remains off. Then confirm a fresh boot and useful response. The KVM,
relay supply and network stay independently powered so recovery remains reachable.

This work is worthwhile because otherwise someone must physically unplug and
restart the Xbox before remote testing can resume. A reset command alone is not
a hard power cycle.

## Selected architecture and power

Reuse the XWiFi physical-button technique within ESP-KVM, through two isolated
relays. Do not install the EEPROM utility, extra Pico/ESP8266, XERC or IR emitter
for this approach. The KVM and one shared 5 V relay supply remain on unswitched
power. After Xbox AC returns, the Xbox PSU restores its own standby circuit;
then a brief relay closure emulates POWER. The ESP does not power the Xbox and
adds no load to its standby supply. Actual standby readiness must be verified.

[Source review, wiring and firmware implementation plan](docs/XBOX-XWIFI-BUTTONS.md).

## Current design and diagram

The [current two-channel wiring plan](XBOX-POWER-RELAY-SETUP.md) supports Codex
commands to press either POWER or EJECT, including deliberate startup with either.

![Current POWER and EJECT photo wiring diagram](roadmap-assets/xbox-power-relay-setup.png)

The permanent design is **one AC switch, two independent isolated relays and
professionally installed POWER/EJECT connections (plug-in or soldered)**. The external servo and shared
servo/relay port are retired from the permanent plan.

- [Current photo diagram, wiring notes and parts](XBOX-POWER-RELAY-SETUP.md).
- [Standalone Xbox adapter build brief to send a technician](XBOX-POWER-ADAPTER-BUILD-BRIEF.md).
- [Detailed firmware audit and recovery design](docs/XBOX-POWER-CONTROL.md).
- [Earlier installer research and candidate builders](docs/XBOX-POWER-INSTALL.md).

## Hardware decisions

| Area | Proposed decision | Remaining gate |
| --- | --- | --- |
| Xbox | Owner-reported revision 1.4, existing Stellar retained | Verify actual front-button connector and POWER continuity |
| Button actuation | Two Adafruit #4409 non-latching dry-contact relays | Boot/reset, release, separate-supply sequencing and low-current contact tests |
| Xbox connection | Passive inline adapter or insulated soldered leads; separate POWER/EJECT contacts | Confirm connector family, mirrored mating views and fit before fabrication |
| Remote disconnect | Two Adafruit #2880 keyed two-wire pairs, labeled by button | Final cable mapping, secure contact termination, case exit route |
| ESP control | GPIO46 / J1-36 POWER, GPIO47 / J1-37 EJECT, GND / J1-39; two #3894 cables | Verify live pin claims, physical board identity and startup state |
| Relay power | Independent regulated 5 V, #276/#368 proposed | Availability, polarity, secure ground branch and backfeed test |
| Xbox AC | Shelly Plug US Gen4 S4PL-00116US; Xbox cord only | Confirm actual firmware/control and mains/load suitability |
| Initial power-state evidence | Calibrated Shelly off/on bands, ambiguous/stale = UNKNOWN | Prove separation and fresh downstream AC evidence; add sensor if inadequate |
| Optional rail sense | GPIO48 reserved | Select/verify isolated switched-rail sensor only if needed |

Working audio GPIO22/53/27 and its existing ground remain installed. Capture,
DSI/touch, SD, Ethernet, Wi-Fi fallback and debugging must remain functional.
The current pins look clear in the audited build, not yet verified live.

## Both buttons are in scope

Build independent POWER and EJECT channels. EJECT is an explicit agent-commanded short press,
kept outside automatic hard-freeze recovery. Verify both GPIOs, use separate
relay contacts, preserve factory controls and test each button independently.
Firmware must add EJECT semantics rather than assuming the existing PC RESET
command is suitable; serialize both channels and qualify safe release on both.

## Xbox adapter commissioning

Send the standalone build brief for a quote. The technician must verify the
reported J2G2 POWER/EJECT/ground circuits and candidate JST PHD connector fit; the
research is not a released fabrication pinout. Accept a plug-in adapter or professionally soldered, strain-relieved leads.
Require both disconnect halves, labeled POWER/EJECT leads, insulation/mounting,
continuity sheet and revision-appropriate installation photos. No owner soldering
or original-wire cutting is intended. Inquiries have been sent to two Etsy builders; no build agreement is recorded.

## Implementation and acceptance sequence

1. Obtain a quote and verified connector/continuity findings before ordering a
   finished harness. Record final BOM and cable exit route.
2. Complete the adapter's passive/functional acceptance tests in the build brief.
   Test the relay under reset, separate power sequencing and stuck-active faults.
3. Add explicit POWER/EJECT semantics and per-button timing, full pin validation,
   optional unused channels, safe output initialization,
   bounded release, one operation at a time, duplicate IDs and operation status.
4. Implement explicit Shelly OFF/ON, confirmed restoration and standby readiness,
   fresh power checks,
   bounded retries, cooldowns and persistent restart reconciliation. Never blindly
   retry a non-idempotent button press after a timeout.
5. Calibrate power evidence; do not infer OFF from a stale meter reading or ON
   from plug output state alone. Add isolated rail sensing if necessary.
6. Qualify with audio/video, display/touch, storage and network load, then conduct
   supervised AC-recovery trials before approving unattended operation.

Do not use PC-style long-hold shutdown as the Xbox recovery method. A pull-down
cannot release an actively stuck-high GPIO. A hardware pulse limiter is needed
if the accepted failure guarantee requires release independent of MCU execution.
Power draw, front LED state, HDMI lock and ping alone do not prove boot/liveness.

## Design history

[Earlier interchangeable-accessory roadmap](XBOX-POWER-ROADMAP-HISTORY.md) and
[its photographed parts study](XBOX-POWER-PARTS.md) remain as history. They are
not the purchasing or installation plan for the dedicated relay setup.

## Accepted alternative: professional soldered installation

The owner also accepts professionally soldered leads across verified POWER and
EJECT switch contacts, preferably on the front button board. Use
insulated joints, strain relief and a keyed disconnect; preserve factory controls.
This can replace the custom plug-in interposer shown in the diagram. Removing
soldered leads requires desoldering. The relay isolation and separate control
channels remain the same. See the [build request](XBOX-POWER-ADAPTER-BUILD-BRIEF.md).

## Alternative under evaluation: XERC infrared

[IR feasibility and integration investigation](docs/XBOX-XERC-IR.md): use one
ESP RMT output and an IR emitter to control an installer-fitted XERC 2 XE. This
may replace external relays while preserving all controller ports. Exact codes,
standby behavior, optical reliability and live GPIO availability need qualification;
the independently powered direct-contact plan above is now preferred. IR is
retained as an unselected alternative, not a required part of the build.

## Direct-button reference: XWiFi technique

The [XWiFi-to-ESP-KVM investigation](docs/XBOX-XWIFI-BUTTONS.md) explains how
our independently powered KVM and two isolated contact channels can emulate
POWER/EJECT after Xbox standby returns. It reuses the button technique, not the
EEPROM application; no separate Xbox-powered Wi-Fi MCU is required.
