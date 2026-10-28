<!-- SPDX-License-Identifier: Apache-2.0 -->
# Installer request: soldered Xbox button leads and sidewall port

Please install **Xbox-side leads only** on my original Xbox (revision 1.4).
**I will handle the relays, external power supply, ESP-KVM wiring and firmware.**
The selected build is **soldered leads plus a four-pin sidewall port**.
Please also assemble the removable external button cable described below.
Use loose two-pin disconnects only if the port cannot be mounted safely.
The goal is remote button presses, including turning the Xbox back on after an
AC power cycle. You do not need to build or configure the external control system.

## What I need installed

- Solder one pair of wires in parallel across the verified **POWER switch contacts**
  and another pair across **EJECT**, preferably on the front button board.
- Keep both physical buttons, front LEDs and all four controller ports working.
- Insulate the joints and add strain relief so cable pulls cannot load solder pads.
- Route the leads safely out of the console without pinching or exposed metal.
  Prefer the sidewall port below if clearance permits; use the loose-connector
  backup if drilling or mounting is impractical.
- Fit the four-pin sidewall socket and matching external cable, or two keyed
  two-pin disconnects labeled **POWER** and **EJECT**. Include all mating halves
  and labeled free wire ends for my relay connections.
- Quote about **0.5 m of usable cable**, with final length/routing agreed to fit.
  Label each conductor's button-signal/return function and provide a wiring photo.

```text
                 INSTALLER                         I WILL HANDLE
Xbox POWER contacts -> secured pair -> disconnect -> POWER relay -> ESP-KVM
Xbox EJECT contacts -> secured pair -> disconnect -> EJECT relay -> ESP-KVM
```

Two separate two-wire pairs are preferred. Never join POWER and EJECT signals.
Do not connect accessory power, ESP GPIO or ESP/control ground to these leads;
they will connect only to isolated relay contacts. No extra Pico or XERC is needed.

## Wire and solder locations

Use small, flexible **stranded insulated hookup wire**, approximately **26-28 AWG**
as a starting point. Choose the final size to fit the actual solder pads and
connector contacts; use larger wire only through a properly supported transition
if needed. Keep each pair together and label both connector halves.

Prefer soldering on the **front POWER/EJECT button board**, with one wire on each
of the two electrical sides of the corresponding switch. Verify the pads with
AC disconnected: the two selected points should become connected when that
button is pressed. On a four-leg tactile switch, some legs may already be
internally connected; do not mistake two legs on the same electrical side for
the two switch contacts. Leave the original switch and its wiring in place.

**Exact solder-pad locations and board orientation are not verified here.**
Identify them on the actual board by continuity testing and provide a marked
photo. Do not solder to an LED connection or rely on wire color/assumed pinout.

## Why two wires per button?

Pressing a button joins two electrical points. The relay must join those same
two points, so one wire goes to each side. Its contact is wired **in parallel**
with the physical switch, allowing either to operate the button.

```text
                   Original button stays installed
                 +--------[ POWER switch ]--------+
                 |                                |
Xbox POWER side A o                                o Xbox POWER side B
                 |                                |
               wire 1                           wire 2
                 |                                |
           keyed disconnect                 keyed disconnect
                 |                                |
                 +---- NO [relay contact] COM -----+
                       (I connect this relay)

Each pair uses two pins of the four-pin port, or its own two-pin disconnect.
Repeat the same arrangement separately for EJECT.
```

**Four wires total: two for POWER and two for EJECT.** A return may be electrically
common inside the Xbox, but please supply two labeled pairs rather than combining
them. These are button signal/return leads, not a positive/negative power supply.

## Parts to send the installer

USD prices checked 2026-10-28; exclude shipping, tax and installation labor.
Buy all five rows to provide both the preferred port and the backup option.

| Qty | Buy | Price for this quantity | Use |
| --- | --- | --- | --- |
| 1 | [Adafruit #3892: four-wire 26-AWG flexible ribbon, 1 m](https://www.adafruit.com/product/3892) | $1.95 | Internal leads: two wires per button |
| 1 | [Adafruit #5319: four-pin panel socket](https://www.adafruit.com/product/5319) | $9.95 | Preferred Xbox sidewall port |
| 1 | [Adafruit #5318: matching four-pin cable plug](https://www.adafruit.com/product/5318) | $9.95 | Removable external cable end; requires solder assembly |
| 5 feet | [Alpha Wire 1174C SL013: round four-conductor 22-AWG cable](https://www.digikey.com/en/products/detail/alpha-wire/1174C-SL013/12471121) | $7.29 ($1.458/ft at 5 ft) | External cable; enter order quantity **5** for five feet |
| 2 sets | [Adafruit #2880: two-pin plug/receptacle cable set](https://www.adafruit.com/product/2880) | $1.50 ($0.75/set) | Backup loose disconnects, one set per button |

**Parts total: $30.64 before shipping/tax.** Prices and availability can change.
Installer supplies solder, heat-shrink, labels, mounting and strain-relief materials.
The owner keeps the relay boards, their control cables and shared 5 V supply.

```text
PREFERRED
POWER contacts -- 2 internal wires --+                         +-- POWER pair -> relay NO/COM
                                    +-- socket <-> plug -- cable
EJECT contacts -- 2 internal wires --+                         +-- EJECT pair -> relay NO/COM

BACKUP (no panel socket)
POWER contacts -- secured pair -- two-pin disconnect -- labeled leads -> relay NO/COM
EJECT contacts -- secured pair -- two-pin disconnect -- labeled leads -> relay NO/COM
```

The port is panel-mounted with a slightly protruding rim, not fully recessed.
Check case thickness, internal clearance, shielding and cable routing before
making the hole; the product's “9 mm” label is not a drilling instruction.
The plug requires **4.3-5.2 mm outside-diameter round cable** for its strain relief.
The proposed Alpha cable is nominally **4.70 mm**; confirm actual fit during assembly.
Use the thin ribbon for internal solder leads, not the external plug's cable clamp.

Assign and document four contacts: POWER A/B and EJECT A/B. No supply voltage
or ESP ground goes through this port. Provide a labeled mating-face photo and
continuity sheet; opposite connector views can be mirrored. Label the port
**BUTTON CONTACTS ONLY**. Return the assembled external cable with two labeled
pairs of free ends for the owner's relay screw terminals, using suitable stripped
length and secure termination. In the backup installation, return both matching
connector halves and ensure the case exit has a grommet or equivalent protection.

## Verification before handoff

Identify the actual switch contacts by continuity, rather than assumed pin numbers
or wire colors. With suitable test equipment, briefly close each installed pair
independently to confirm it emulates its button without operating the other one.
Verify the original buttons and LEDs still work, and record the test pulse used.
Test POWER startup from standby, including after a normal AC disconnect/restore;
retain the existing Stellar setup and note EJECT-start behavior. No firmware work
or deliberate freeze testing is required from you.

## Reference diagram

The **[full wiring plan and photo diagram](XBOX-POWER-RELAY-SETUP.md)** shows the
complete system for context. Its relays, supply and ESP wiring are **my work**;
your scope includes the sidewall port or backup disconnects and assembled
external button cable; I connect its labeled free ends to my relays.

Historical alternative only; a plug-in interposer is not requested for this build.
The historical **[Xecuter X3 Power/Eject No Solder Adapter, page 12](https://connortumbleson.com/uploads/2024/03/x3_ce_v10_v16.pdf#page=12)**
illustrates that alternative; its connector pinout is not a fabrication instruction.

Please quote the Xbox-side installation, connectors and testing only.
