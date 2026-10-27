<!-- SPDX-License-Identifier: Apache-2.0 -->
# Installer request: Xbox POWER and EJECT button leads

Please install **Xbox-side leads only** on my original Xbox (revision 1.4).
**I will handle the relays, external power supply, ESP-KVM wiring and firmware.**
The goal is remote button presses, including turning the Xbox back on after an
AC power cycle. You do not need to build or configure the external control system.

## What I need installed

- Solder one pair of wires in parallel across the verified **POWER switch contacts**
  and another pair across **EJECT**, preferably on the front button board.
- Keep both physical buttons, front LEDs and all four controller ports working.
- Insulate the joints and add strain relief so cable pulls cannot load solder pads.
- Route the leads safely out of the console without pinching or exposed metal.
  Agree any required case alteration with me first.
- Fit keyed removable connectors, clearly labeled **POWER** and **EJECT**.
  Include the matching connector halves with free leads for my relay cables.
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

Repeat the same arrangement separately for EJECT.
```

**Four wires total: two for POWER and two for EJECT.** A return may be electrically
common inside the Xbox, but please supply two labeled pairs rather than combining
them. These are button signal/return leads, not a positive/negative power supply.

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
your scope ends at the labeled removable button leads and their matching halves.

If a plug-in adapter is easier, that is acceptable instead of soldered leads.
The historical **[Xecuter X3 Power/Eject No Solder Adapter, page 12](https://connortumbleson.com/uploads/2024/03/x3_ce_v10_v16.pdf#page=12)**
illustrates that alternative; its connector pinout is not a fabrication instruction.

Please quote the Xbox-side installation, connectors and testing only.
