<!-- SPDX-License-Identifier: Apache-2.0 -->
# Current proposed setup: dedicated Xbox power-button relay

Updated 2026-10-24. This supersedes the interchangeable servo/relay concept as
the proposed permanent installation. No servo or universal accessory connector
is included. No hardware purchases, firmware changes, or live actions are made
by this documentation task.

![Dedicated relay setup with real product photos and numbered connections](roadmap-assets/xbox-power-relay-setup.png)

## Optional EJECT control

If practical, ask the adapter builder to expose EJECT as well as POWER, with a
separate labeled contact pair (or a verified shared Xbox return). This extends
the single-POWER diagram above: EJECT needs a second independent isolated relay
channel and ESP output. GPIO47 / J1-37 is a candidate pending final pin validation
and firmware support. The original buttons and LEDs must keep working. Keep
EJECT manual and outside automatic freeze recovery; Stellar eject-start behavior
must be tested. See the [updated build request](XBOX-POWER-ADAPTER-BUILD-BRIEF.md).

## Follow the numbered connections

1. **ESP header:** female sockets on J1 physical pin 36 (GPIO46) and J1 physical
   pin 39 (GND). This leaves the existing audio wiring in place. Confirm physical
   identity and effective pin claims before installing; the board render is not
   an exact pin-location annotation.
2. **Dedicated cable:** Adafruit #3894 PH-to-female-socket cable. White/control
   goes to GPIO46; black goes to ESP ground and external supply negative. Its
   red lead goes to the independent 5 V supply, never ESP 3V3/5 V. A builder
   assembles and insulates the power/ground branch; three stock female sockets
   do not all plug into the ESP.
3. **Relay:** the PH end of that cable plugs into Adafruit #4409. Its driver
   activates an isolated normally-open switch. NO and COM are the only contact
   terminals connected toward the Xbox; NC stays unused. An actual hardware
   release bound and safe boot behavior remain to be qualified.
4. **Relay power:** proposed Adafruit #276 regulated 5 V/2 A adapter into #368
   barrel-to-terminal adapter, then into the red/black cable circuits. A securely
   assembled junction connects the external supply negative to ESP/control
   ground. Check supply availability and polarity before purchase; no current
   stock claim is made. Do not add a positive connection to the ESP supply.
5. **Contact disconnect:** proposed Adafruit #2880 two-pin JST-SM cable pair.
   Builder terminates one side at NO/COM and the other at the Xbox interposer's
   verified power-button signal and return. Wire colors are not authoritative.
   Secure contact leads to prevent cable strain and verify terminal acceptance
   of the chosen conductor size/ferrules; do not assume all loose leads fit.
6. **Xbox plug-in adapter:** with AC removed, unplug the stock front-button
   cable, insert the tested new adapter into the motherboard connection, and
   plug the original cable into the adapter. The builder passes all original
   circuits through unchanged and taps only the POWER switch circuit in parallel.
   The owner should not cut or solder original Xbox wiring. Builder assembly
   and testing are still necessary; this is not an available finished kit.
7. **AC path:** wall outlet to Shelly Plug US Gen4 S4PL-00116US to Xbox AC cord.
   KVM, relay supply and network equipment stay on unswitched power. The purpose
   is a hard power cycle after a genuine hard freeze, followed by a short button
   press only if fresh evidence says the Xbox remains off after AC restoration.

## Xbox adapter build brief

Send [XBOX-POWER-ADAPTER-BUILD-BRIEF.md](XBOX-POWER-ADAPTER-BUILD-BRIEF.md) to a
technician for a quote. It is a short illustrated request covering the plug-in
adapter, two-wire relay lead and basic verification. Supporting technical notes
are linked separately. The brief covers the Xbox side; ESP firmware
and AC automation remain separate work.

## Xbox connection remains an installation gate

The historical X3 adapter photo demonstrates the inline arrangement; it is not
a photo of our unbuilt passive harness. Its small red lead was intended for an
X3, not automatically compatible with our proposed relay cable.

The supplied research identifies **J2G2 circuit 1 as ground and circuit 2 as
POWER_SW**, with JST PHD components as candidate mating parts. Those claims
remain provisional in this repository until a builder verifies the actual
connector family, orientation and continuity against the physical POWER switch.
No exact solder-pad or header-corner instruction is provided by this diagram.

Do not add a new Xbox-ground-to-ESP-ground wire. The relay contact pair stays
separate from the control supply/ground. Existing HDMI/USB may already join
system grounds elsewhere; this does not claim complete rig galvanic isolation.

## Power state and firmware

Initially evaluate Shelly load measurements as a possible main-power indicator;
use them only if calibrated standby and running bands are clearly separated.
Stale or ambiguous readings mean UNKNOWN and must inhibit a button press.
Plug output=ON alone is not proof of downstream AC. Confirm restore using
appropriate fresh corroborating evidence. GPIO48 remains reserved for an
optional isolated sensor if needed; its circuit is not shown here.

Neither watts, a relay pulse, HDMI lock nor a single frame proves application
liveness. Require a fresh post-cycle boot sequence and useful response to input,
or stronger application instrumentation, before declaring recovery successful.

The existing ATX worker still needs the focused reliability work documented in
[the investigation](docs/XBOX-POWER-CONTROL.md): optional reset,
initialization and bounded release, duplicate admission, busy/completed status,
fresh sensing/evidence, and reconciliation after timeouts/restarts. Never blindly
retry a power-button event or assume PC long-hold behavior applies to Xbox.

## Parts and photo sources

| Item | Exact proposed product / source |
| --- | --- |
| ESP board | Existing P4X; [local reference render](docs/esp32-p4x-function-ev-board-isometric_v1.6.png) |
| Control cable | [Adafruit #3894](https://www.adafruit.com/product/3894) |
| Relay | [Adafruit #4409](https://www.adafruit.com/product/4409) |
| Contact disconnect | [Adafruit #2880](https://www.adafruit.com/product/2880) |
| External supply | [Adafruit #276](https://www.adafruit.com/product/276), proposed pending availability |
| DC jack termination | [Adafruit #368](https://www.adafruit.com/product/368) |
| AC switch | [Shelly Plug US Gen4](https://us.shelly.com/products/shelly-plug-us-gen4-white), S4PL-00116US |
| Historical adapter picture | [X3 CE guide page 12, photo 5](https://connortumbleson.com/uploads/2024/03/x3_ce_v10_v16.pdf#page=12) |
| Xbox interposer | Custom; connector parts and as-built wiring require verification |

Real supplier photographs remain credited to their respective owners. The ESP
image is a reference render. The Xbox image is a historical installation crop.
No photo of a proposed custom assembly is fabricated. Images are not to scale.
Original product-image URLs are recorded in
[the photo manifest](roadmap-assets/product-photos/sources.json).

[Diagram layout source](roadmap-assets/draw_xbox_relay_setup.py) ·
[Roadmap](XBOX-POWER-ROADMAP.md) ·
[Previous interchangeable concept, historical](XBOX-POWER-PARTS.md)

## Accepted alternative: professional soldered installation

The owner also accepts professionally soldered leads across verified POWER and,
if practical, EJECT switch contacts, preferably on the front button board. Use
insulated joints, strain relief and a keyed disconnect; preserve factory controls.
This can replace the custom plug-in interposer shown in the diagram. Removing
soldered leads requires desoldering. The relay isolation and separate control
channels remain the same. See the [build request](XBOX-POWER-ADAPTER-BUILD-BRIEF.md).
