<!-- SPDX-License-Identifier: Apache-2.0 -->
# Proposed setup: agent-commanded Xbox POWER and EJECT

Selected build: **professionally soldered POWER/EJECT leads on the Xbox front
button board, routed to one four-pin sidewall socket and removable cable, then
two isolated relays controlled by our existing ESP-KVM**. Use two loose two-pin
disconnects only if the panel port cannot be fitted safely.

The goal is the simplest practical, low-cost setup centered on hardware we
already own: the ESP-KVM supplies networking and button commands. No additional
Pico, internal Wi-Fi MCU, XERC or IR emitter is needed. The sidewall connector
is a cable-management choice; the loose backup has the same electrical function.
This is a selected design, not a claim of measured lowest cost or completed
hardware qualification. [Button-technique source review](docs/XBOX-XWIFI-BUTTONS.md).

Both buttons are in scope. Codex can command the ESP to **press POWER** or
**press EJECT**, including deliberately starting the Xbox with either button.
"Agent-commanded" does not mean someone must physically press the console.
Firmware/API support is planned, not implemented or validated by this document.

![Two-channel POWER and EJECT wiring with real product photos](roadmap-assets/xbox-power-relay-setup.png)

## Connections and quantities

| Connection | Planned wiring |
| --- | --- |
| POWER control | ESP J1-36 / GPIO46 -> white SIG on POWER relay cable |
| EJECT control | ESP J1-37 / GPIO47 -> white SIG on EJECT relay cable |
| Control ground | ESP J1-39 -> both black relay wires and external supply negative |
| Relay power | One independent regulated 5 V supply -> both red V+ wires |
| POWER contacts | POWER relay NO/COM -> verified Xbox POWER signal/return |
| EJECT contacts | EJECT relay NO/COM -> verified Xbox EJECT signal/return |

Use **two #4409 relay boards and two #3894 control cables**. Prefer one
four-pin sidewall socket and matching external cable for the Xbox contact pairs;
keep two #2880 disconnect sets as the no-drilling backup. Label pairs POWER/EJECT. The builder makes insulated,
strain-relieved power/ground junctions; do not stack loose sockets on one pin.
The ESP end uses push-on female sockets. Existing audio GPIO22/53/27 stays wired.
The proposed GPIOs and header positions require final board/live pin verification.

Only relay NO/COM contacts go to the Xbox; leave NC unused. Do not add a direct
Xbox-button connection to ESP GPIO, control ground or 5 V. Existing HDMI/USB
may already join system grounds elsewhere; this is not whole-rig isolation.
The relay supply stays independent of switched Xbox AC. Verify supply polarity,
current headroom with both channels active, reset behavior and backfeed.

## Why it can start after Xbox AC returns

The KVM and one shared 5 V relay supply remain powered independently. The Xbox
PSU restores its own standby circuit after AC returns; that circuit responds to
POWER without the Xbox OS running. Wait for confirmed restoration and qualified
standby readiness, then pulse only if fresh evidence says the console is off.
The ESP supplies no Xbox power and draws no accessory current from Xbox standby.
A failed standby supply cannot be fixed by repeatedly pressing POWER.

## Selected Xbox installation: soldered leads and sidewall port

The installer solders two wires across each verified physical switch, preferably
on the front button board, keeping the factory buttons and LEDs connected.
Four internal wires terminate at the [#5319 panel socket](https://www.adafruit.com/product/5319).
The [#5318 mating plug](https://www.adafruit.com/product/5318) connects a removable
four-conductor cable, whose labeled POWER and EJECT pairs go to their respective
relay NO/COM terminals. The owner connects the relays and ESP; the installer
assembles both the Xbox-side port and external button cable.

Check case clearance before drilling. If mounting is impractical, use two
[#2880 loose disconnect sets](https://www.adafruit.com/product/2880) and a protected,
strain-relieved cable exit. Exact solder pads and four-pin assignment require
continuity verification and an as-built photo. No supply voltage goes through
the port. Removal of soldered leads requires desoldering.

The [installer brief and linked shopping list](XBOX-POWER-ADAPTER-BUILD-BRIEF.md)
is the current build request. The historical Xecuter inline adapter, servo,
Pico and XERC/IR investigations remain background alternatives, not purchases
or parallel installation tasks.

## Agent commands and freeze recovery

The planned API must expose separate bounded **PRESS POWER** and **PRESS EJECT**
operations, usable by Codex. These emulate buttons, not "set console ON" commands:
check fresh state first, because pressing an already-on console can have another
effect. Eject-start may select a different Stellar boot mode; qualify both paths.

The default automatic hard-freeze sequence uses Shelly explicit OFF/ON, confirmed
restoration and fresh power evidence, then one POWER pulse only if still off.
EJECT remains available for deliberate agent requests; exclude it from that
default sequence. Never blindly replay either button after a timeout.

Firmware must validate both pins, idle both outputs safely, bound release,
serialize button operations, deduplicate requests and report accepted versus
completed status. Add explicit EJECT semantics rather than assuming PC RESET or
long-hold behavior. Reconcile state after restarts. Hardware release guarantees
and stuck-active behavior still need qualification.

Calibrate Shelly standby/running bands before using watts as power evidence;
stale or ambiguous means UNKNOWN and inhibits a recovery press. Plug output ON
alone does not prove downstream AC. Reserve GPIO48 for isolated rail sensing if
needed. Require a fresh boot progression and useful response to verify liveness;
watts, LEDs, HDMI lock or ping alone are insufficient.

[Detailed investigation and implementation plan](docs/XBOX-POWER-CONTROL.md).

## Xbox leads, sidewall port and backup shopping list

The [installer shopping list](XBOX-POWER-ADAPTER-BUILD-BRIEF.md#parts-to-send-the-installer)
contains quantities, purchase links and prices checked 2026-10-28: internal
#3892 wire ($1.95), #5319 panel socket ($9.95), #5318 mating plug ($9.95),
five feet of Alpha Wire 1174C SL013 ($7.29), and two #2880 backup sets ($1.50).
**Total $30.64 before shipping/tax and labor.** The installer assembles the
external cable; the owner connects its two labeled pairs to the relay NO/COM
screw terminals. Port clearance and actual cable fit remain hardware checks.

The diagram shows the selected soldered/sidewall route and identifies loose
disconnects as the fallback. Supplier photos show the relays and backup connectors;
the sidewall route is labeled schematically, not pictured as a finished assembly.

## Parts and photo sources

| Item | Exact proposed product / source |
| --- | --- |
| ESP board | Existing P4X; [local reference render](docs/esp32-p4x-function-ev-board-isometric_v1.6.png) |
| Control cables (2) | [Adafruit #3894](https://www.adafruit.com/product/3894) |
| Relays (2) | [Adafruit #4409](https://www.adafruit.com/product/4409) |
| Backup contact disconnects (2 pairs) | [Adafruit #2880](https://www.adafruit.com/product/2880) |
| External supply | [Adafruit #276](https://www.adafruit.com/product/276), proposed pending availability |
| DC jack termination | [Adafruit #368](https://www.adafruit.com/product/368) |
| AC switch | [Shelly Plug US Gen4](https://us.shelly.com/products/shelly-plug-us-gen4-white), S4PL-00116US |
| Historical adapter picture | [X3 CE guide page 12, photo 5](https://connortumbleson.com/uploads/2024/03/x3_ce_v10_v16.pdf#page=12) |
| Xbox internal wire | [Adafruit #3892](https://www.adafruit.com/product/3892), four-wire 26-AWG ribbon |
| Sidewall socket / plug | [Adafruit #5319](https://www.adafruit.com/product/5319) / [#5318](https://www.adafruit.com/product/5318) |
| External button cable | [Alpha Wire 1174C SL013](https://www.digikey.com/en/products/detail/alpha-wire/1174C-SL013/12471121), five feet |

Real supplier photographs remain credited to their respective owners. The ESP
image is a reference render. The Xbox image is a historical installation crop.
No photo of a proposed custom assembly is fabricated. Images are not to scale.
Original product-image URLs are recorded in
[the photo manifest](roadmap-assets/product-photos/sources.json).

[Diagram layout source](roadmap-assets/draw_xbox_relay_setup.py) ·
[Roadmap](XBOX-POWER-ROADMAP.md) ·
[Previous interchangeable concept, historical](XBOX-POWER-PARTS.md)

## Live configuration validation

[2026-10-27 read-only validation and remaining hardware tests](docs/XBOX-BUTTON-VALIDATION-2026-10-27.md):
the running KVM saves button GPIO46/47 with ATX disabled, and reports neither
in its fixed peripheral reservations. Pin choice is supported; electrical behavior,
physical switch points and complete recovery still require bench qualification.
