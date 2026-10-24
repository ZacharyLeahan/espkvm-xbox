<!-- SPDX-License-Identifier: Apache-2.0 -->
# Roadmap: hard power-cycle recovery after an Xbox hard freeze

Updated 2026-10-24. Status: proposed dedicated relay installation. Hardware
procurement, custom harness verification, firmware work and acceptance remain.
No firmware flashes, live setting changes, AC switching or commits are authorized
by this documentation update.

## Purpose

Recover remotely from a genuine hard freeze: remove Xbox AC, wait for validated
power decay, restore AC, and briefly press POWER only if fresh evidence confirms
the console remains off. Then confirm a fresh boot and useful response. The KVM,
relay supply and network stay independently powered so recovery remains reachable.

This work is worthwhile because otherwise someone must physically unplug and
restart the Xbox before remote testing can resume. A reset command alone is not
a hard power cycle.

## Current design and diagram

![Dedicated relay and plug-in Xbox button adapter](roadmap-assets/xbox-power-relay-setup.png)

The permanent design is **one AC switch, one dedicated isolated relay and one
reversible passive Xbox front-button interposer**. The external servo and shared
servo/relay port are retired from the permanent plan.

- [Current photo diagram, wiring notes and parts](XBOX-POWER-RELAY-SETUP.md).
- [Standalone Xbox adapter build brief to send a technician](XBOX-POWER-ADAPTER-BUILD-BRIEF.md).
- [Detailed firmware audit and recovery design](docs/XBOX-POWER-CONTROL.md).
- [Earlier installer research and candidate builders](docs/XBOX-POWER-INSTALL.md).

## Hardware decisions

| Area | Proposed decision | Remaining gate |
| --- | --- | --- |
| Xbox | Owner-reported revision 1.4, existing Stellar retained | Verify actual front-button connector and POWER continuity |
| Button actuation | Adafruit #4409 non-latching dry-contact relay | Boot/reset, release, separate-supply sequencing and low-current contact tests |
| Xbox connection | Technician-built passive inline adapter, original circuits preserved; POWER/return tap only | Confirm connector family, mirrored mating views and fit before fabrication |
| Remote disconnect | Adafruit #2880 keyed two-wire pair | Final cable mapping, secure contact termination, case exit route |
| ESP control | GPIO46 / J1-36 and GND / J1-39; builder-adapted #3894 cable | Verify live pin claims, physical board identity and startup state |
| Relay power | Independent regulated 5 V, #276/#368 proposed | Availability, polarity, secure ground branch and backfeed test |
| Xbox AC | Shelly Plug US Gen4 S4PL-00116US; Xbox cord only | Confirm actual firmware/control and mains/load suitability |
| Initial power-state evidence | Calibrated Shelly off/on bands, ambiguous/stale = UNKNOWN | Prove separation and fresh downstream AC evidence; add sensor if inadequate |
| Optional rail sense | GPIO48 reserved | Select/verify isolated switched-rail sensor only if needed |

Working audio GPIO22/53/27 and its existing ground remain installed. Capture,
DSI/touch, SD, Ethernet, Wi-Fi fallback and debugging must remain functional.
The current pins look clear in the audited build, not yet verified live.

## Optional manual EJECT control

Request an independently wired EJECT contact as well if practical. This requires
a second isolated relay channel, a validated spare ESP output (GPIO47 proposed)
and separate firmware commands. Preserve factory controls, verify the shared
Xbox return if used, and keep EJECT outside automatic hard-freeze recovery.
The current photo diagram depicts the POWER-only baseline.

## Xbox adapter commissioning

Send the standalone build brief for a quote. The technician must verify the
reported J2G2 POWER/ground circuits and candidate JST PHD connector fit; the
research is not a released fabrication pinout. Require a reversible plug-only
installation, both disconnect halves, labeled relay lead, insulation/mounting,
continuity sheet and revision-appropriate installation photos. No owner soldering
or original-wire cutting is intended. No builder has been contacted or agreed
to make the kit yet.

## Implementation and acceptance sequence

1. Obtain a quote and verified connector/continuity findings before ordering a
   finished harness. Record final BOM and cable exit route.
2. Complete the adapter's passive/functional acceptance tests in the build brief.
   Test the relay under reset, separate power sequencing and stuck-active faults.
3. Implement optional reset=-1, full pin validation, safe output initialization,
   bounded release, one operation at a time, duplicate IDs and operation status.
4. Implement explicit Shelly OFF/ON, confirmed restoration, fresh power checks,
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

The owner also accepts professionally soldered leads across verified POWER and,
if practical, EJECT switch contacts, preferably on the front button board. Use
insulated joints, strain relief and a keyed disconnect; preserve factory controls.
This can replace the custom plug-in interposer shown in the diagram. Removing
soldered leads requires desoldering. The relay isolation and separate control
channels remain the same. See the [build request](XBOX-POWER-ADAPTER-BUILD-BRIEF.md).
