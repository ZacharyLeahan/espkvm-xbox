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
