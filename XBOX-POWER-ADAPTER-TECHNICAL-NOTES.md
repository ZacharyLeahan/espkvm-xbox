<!-- SPDX-License-Identifier: Apache-2.0 -->
# Adapter technical notes: preliminary verification checklist

Scope update: professionally soldered, insulated button leads with strain relief
and a keyed disconnect are also accepted in place of the passive interposer.
Verify the actual switch contacts and preserve factory operation. The original
plug-only restrictions below apply only to the interposer option.

Scope update: the [short build request](XBOX-POWER-ADAPTER-BUILD-BRIEF.md) now
asks for optional independent EJECT control. POWER-only restrictions below describe
the original baseline; an agreed EJECT extension needs its own verified contact
path and relay channel, with the same isolation and pass-through requirements.

Revision 1 - 2026-10-24. Request for a quote, connector verification, fabrication
and testing. This is a functional specification, **not a verified production
pinout**. Please close the verification items below before building the final
assembly. Retained as supporting research; use the short build request for initial discussions.

## What I want to buy

One fully assembled, insulated and tested **passive inline front-panel adapter**
for an original Xbox motherboard revision **1.4** (owner-reported), plus its
mating two-wire relay lead. I want to install it by unplugging/reconnecting
connectors, without cutting, crimping or soldering any original Xbox wiring.

My goal is remote recovery from a genuine hard freeze using an AC power cycle,
followed by a short physical POWER-button equivalent if the console remains off.
AC switching and controller firmware are separate from this adapter build.

The adapter must preserve the original power/eject buttons and front LEDs,
while exposing only the **POWER switch signal and its ground return** to an
external isolated normally-open relay. The Xbox has Stellar installed; leave
it unchanged and verify normal POWER/EJECT behavior with it present if testing
on my console. Do not use EJECT as the remote power input.

## Physical arrangement

```text
Xbox motherboard front-panel connector (reported J2G2)
                  |
                  | A: new mating cable plug
                  v
       +----------------------------------+
       | NEW passive inline adapter       |
       | Original circuits pass through   |
       | unchanged; no active electronics |
       +----------------------------------+
                  |                    |
                  | B: header matching | C: POWER + Xbox return only
                  | the OEM cable      |    keyed two-pin disconnect
                  v                    v
       Existing front-panel cable    Mating two-wire relay lead
                  |                    |
                  v                    +-- POWER -> relay NO
       Original POWER / EJECT         +-- return -> relay COM
       buttons and LEDs                   relay NC unused
```

The relay closes C's two conductors in parallel with the original POWER switch.
It must not interrupt the original switch path or require the relay/controller
to be powered for the factory controls to work.

A short cable pigtail and small insulated carrier PCB are acceptable; please
choose the mechanical arrangement that fits the actual console safely. Retain
all originally populated circuits and cavity positions, including unused lines
where physically present. Do not populate a blocked/keyed cavity merely to make
an assumed ten-wire count. No wires may be crossed because opposing connector
views look mirrored.

## Electrical boundary: this adapter connects to contacts only

The intended external relay is **Adafruit #4409 STEMMA non-latching mini relay**.
Only its **NO and COM contact terminals** connect to this Xbox-side harness.
Its power, signal and control-side ground do not enter the Xbox adapter.

- Do not add a connection to ESP ground, ESP GPIO, USB power or relay V+.
- Do not connect either tap to a PSU rail or to the mains side of the console.
- Do not break out EJECT, LED signals or other circuits on the remote two-pin port.
- Do not add a pull-up, LED indicator, resistor, transistor or protection network
  to the Xbox button nets without proposing and documenting that change first.
- The external relay provides the isolation; the passive adapter itself does not.

The research supplied to me reports a standby-logic POWER input, approximately
3.3 V idle with a 10k pull-up (about 0.33 mA nominal). Treat this as an expectation
to verify, not a measured specification for my console. Confirm actual voltage
and that an isolated closure reproduces a normal short POWER press.

## Mandatory verification before connector procurement/final assembly

Please supply photographs or a marked drawing establishing all of the following:

1. Actual motherboard revision and front-panel connector designation.
2. Connector pitch, row spacing, shroud/key/retention geometry, populated
   positions and circuit-1 marking on both mating ends.
3. A consistent numbering convention: label views explicitly as **mating face**,
   **wire entry** or **PCB component side**. Show orientation and cavity 1.
4. With AC removed, continuity from the proposed POWER tap to the actual POWER
   switch and from return to its matching Xbox ground. Confirm EJECT separately.
5. Whether the proposed commercial connectors fit the OEM hardware fully and
   securely without force, bending contacts or removing a key.

### Research leads, NOT an authorized fabrication pinout

| Item | Candidate from supplied research | Required confirmation |
| --- | --- | --- |
| Front-panel connector | Xbox J2G2 | Check actual PCB and harness |
| POWER switch signal | J2G2 circuit 2 | Continuity to physical POWER switch |
| Switch return | J2G2 circuit 1 | Ground / other side of POWER switch |
| EJECT | J2G2 circuit 4 | Identify and leave pass-through only |
| Cable housing | JST PHDR-10VS, PHD family, 2.0 mm dual row | Physical fit to Xbox header is unproven |
| Header accepting OEM cable | JST B10B-PHDSS; side-entry S10B-PHDSS possible | Match fit, orientation and enclosure clearance |
| Crimp terminals | JST SPHD-001T-P0.5 or SPHD-002T-P0.5 | Select by actual conductor and insulation dimensions |

JST lists those PHD components, but I do not have an OEM Microsoft BOM proving
that the Xbox uses them. Do not order a finished harness solely from this table.
If fit is wrong, propose the correct family or an appropriate OEM donor harness.
A compatible verified alternative is preferable to modifying the Xbox connector.

## Requested remote connector and cable

Proposed disconnect: **Adafruit #2880 two-pin JST-SM-type plug/receptacle cable
set**, or a documented equivalent keyed, retained two-pin pair. Include both
halves. It must not be confusable with an accessible power-input connector in
the finished installation.

For this NEW custom connector, define and label:

| Custom port circuit | Function | Relay termination |
| --- | --- | --- |
| C1 | Verified Xbox POWER-switch return / ground | COM |
| C2 | Verified Xbox POWER-switch signal | NO |

C1/C2 are our custom labels, not a claim about the commercial connector's cavity
numbers or wire colors. Show their exact mating-face positions in the as-built
photo. Continuity to these labels must be maintained through both cable halves.

Please quote approximately **0.5 m of usable relay lead**, with final length and
exit route agreed after fit assessment. Use appropriately sized insulated wire
(24 AWG is a proposed starting point), secure the two conductors together and
provide strain relief. The #4409 contact clamp is documented for 24-18 AWG;
confirm the supplied lead is accepted securely, with suitable termination if
needed. Do not tin stranded ends intended for a pressure terminal or substitute
an unsuitable ferrule without checking the terminal manufacturer's guidance.
Label the relay ends **COM / XBOX RETURN** and **NO / XBOX POWER**.

If routing outside the case, document the exit path. Protect against sharp
shield edges and lid/drive-tray pinching. Do not require drilling or irreversible
case changes without discussing the concrete proposed location first.

## Mechanical and workmanship requirements

- Insulate the carrier, solder joints and unused contacts from the metal chassis.
  A fitted cover/enclosure or durable insulating mount is preferred to loose tape.
- Provide secure mounting and strain relief; the OEM header must not support
  external cable tension. Keep clear of fan blades, heat sinks and PSU primary.
- Use correct contacts, wire sizes and crimp tooling. No loose wire stuffed into
  connector cavities, friction-only improvised pin adapters or exposed splices.
- Keep the adapter removable so the original cable can reconnect directly.
- Supply only this Xbox-side adapter/relay lead in the base quote. Quote the
  #4409 relay, enclosure/mounting and complete ESP/control harness separately
  if you can supply them too.

## Acceptance tests and evidence to deliver

Perform passive tests with the harness disconnected and console AC removed.
Powered tests require a verified matching console and secured test leads.
Never test insulation with high voltage through the installed Xbox electronics.

| Test | Requested pass condition / evidence |
| --- | --- |
| Connector fit | Both ends fully seat and retain without forcing; clear pin-1 photos |
| Pass-through map | Every originally present circuit goes to its corresponding circuit; record all positions, including empty/keyed cavities |
| Low resistance | End-to-end resistance below 1 ohm after accounting for meter leads; record instrument/method and actual results |
| Cross-short check | No unintended connections between independent nets; account for intentionally common ground nets in the verified schematic |
| Tap map | C1 reaches only intended return net; C2 reaches POWER; no accidental EJECT/LED tap |
| Open remote port | Leaving C disconnected does not affect any original control or LED behavior |
| Factory function | POWER, EJECT and front LEDs work before/after installing adapter; document test console revision and configuration |
| Remote function | At least 50 supervised short contact-closure trials across OFF-to-ON and ON behavior; one event per closure, EJECT not triggered |
| Relay termination | COM/NO attached securely; NC unused; open contact at rest and repeatable closure/release |
| Control isolation, if relay included | With disconnected relay assembly, no DC continuity from either Xbox contact lead to control power/ground/SIG; not a whole-rig ground-isolation test |
| Mechanical fit | Lid/drive trays close, wires protected, mounting secure, no cable tension at Xbox header |
| Reversibility | Adapter removes cleanly and OEM cable reconnects without repair |

For remote trials, begin around a 200-250 ms closure and record the actual test
pulse. Do not assume long-hold shutdown works like PC ATX. These tests do not
require 50 AC cuts and do not authorize software/firmware changes. If you cannot
perform powered tests without my console, state that explicitly in the quote
and separate bench-only results from final installation acceptance.

Deliver:

- Complete assembled adapter plus both halves of remote disconnect and relay lead.
- Final BOM, connector identifiers, wire gauges and measured lengths.
- Pin-by-pin map and photographs of both connector views with cavity 1 marked.
- Test record with measured values, test console revision and any limitations.
- Simple owner installation/removal instructions and a routing/mounting photo.

## Quote questions

Can you verify the connector fit and test on your own revision-1.4-compatible
hardware, or do you need my console/cable? What is the complete parts/labor cost,
lead time, shipping and remedy if the assembled adapter does not fit? Identify
any proposed owner soldering, original-wire cutting or case alteration before
accepting the build. My intended installation is plug-only.

## Reference links

- Historical inline topology: [Xecuter X3 CE installation guide, page 12](https://connortumbleson.com/uploads/2024/03/x3_ce_v10_v16.pdf#page=12).
  The old adapter is an example of placement, not a verified pinout or required
  component. Its X3 auxiliary cable must not be assumed to match our two-wire port.
- Candidate connector dimensions and contacts: [JST PHD manufacturer datasheet](https://www.jst-mfg.com/product/pdf/eng/ePHD.pdf).
- Intended external relay: [Adafruit #4409](https://www.adafruit.com/product/4409).
- Relay wiring/terminal guidance: [Adafruit relay guide](https://learn.adafruit.com/adafruit-stemma-non-latching-mini-relay?view=all).
- Proposed two-wire disconnect: [Adafruit #2880](https://www.adafruit.com/product/2880).

No finished adapter, confirmed OEM connector identity, measured Xbox pinout or
validated unattended recovery capability is claimed by this brief.
