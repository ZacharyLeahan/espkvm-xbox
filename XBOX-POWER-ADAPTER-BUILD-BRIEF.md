<!-- SPDX-License-Identifier: Apache-2.0 -->
# Build request: Xbox remote button connections

I would like to pay someone to add remote button connections to my original
Xbox (revision 1.4). My goal is to turn it back on remotely after a hard freeze
and an AC power cycle. I am open to either a plug-in adapter or professionally
soldered connections; I do not want to do the soldering myself.

## Two acceptable installation options

- **Plug-in adapter:** insert a pass-through between the motherboard and original
  front-panel cable, as pictured below.
- **Professionally soldered leads:** attach wires in parallel across the verified
  POWER and EJECT switch contacts, preferably on the front button board. Insulate the joints, provide strain relief so cable pulls cannot
  load the solder pads, and route the wires to a keyed removable connector.
  Preserve the original buttons, LEDs and wiring; removal would require desoldering.

Please recommend and quote whichever is simpler and dependable for one console.
The electrical function is the same for either option:

```text
Verified POWER contacts -> strain-relieved lead -> keyed disconnect -> POWER relay
Verified EJECT contacts -> strain-relieved lead -> keyed disconnect -> EJECT relay
```

## Plug-in adapter example

```text
Xbox motherboard       NEW plug-in adapter       Original front-panel cable
     connector  ------> [pass-through] ---------> POWER / EJECT / LEDs
                              |
                              +---- POWER + return ---> POWER relay (NO + COM)
                              |
                              +---- EJECT + return ---> EJECT relay (NO + COM)
```

The adapter plugs between the motherboard and the existing front-panel cable.
The original buttons and LEDs keep working. Each extra two-wire pair lets its
relay briefly close the same circuit as the corresponding physical button.

## Example: the old Xecuter adapter

Something like the **[Xecuter X3 Power/Eject No Solder Adapter, shown on page 12
of this guide](https://connortumbleson.com/uploads/2024/03/x3_ce_v10_v16.pdf#page=12)**,
but with separate POWER and EJECT connections for two independent relays.

![Historical Xecuter adapter installed between the Xbox and front-panel cable](roadmap-assets/x3-power-eject-adapter-closeup.png)

This is a historical example, not a photo or verified pinout of the new adapter.

## What I need you to supply

- Either a fully assembled, insulated plug-in adapter or professionally
  installed soldered leads with strain relief. Preserve all original controls.
- A labeled two-wire POWER/return lead with a keyed disconnect, including both
  connector halves. Please quote about 0.5 m of usable cable.
- **Include both independent POWER and EJECT control in the build.** Two
  labeled two-wire pairs are preferred; a keyed three-wire POWER/EJECT/shared
  Xbox return cable is acceptable only after verifying the common return.
- Label each relay pair **NO / POWER** or **NO / EJECT**, and **COM / Xbox return**.
  Include both mating connector halves for every disconnect.
- A checked wiring map, clear connector-orientation photos and simple
  installation instructions.

Please identify the button contacts/returns by continuity to the actual switches
before soldering. For the plug-in option, also verify the connector fit. The
reported J2G2 pin numbers and candidate JST parts are still unverified; do not fabricate from those assumptions alone.
Test that the original controls still work and that a short relay closure
operates POWER without triggering EJECT. Verify EJECT independently and confirm
neither remote input triggers the other button.

Only the relay's switch contacts connect to this harness. Do not connect ESP
GPIO, control-side ground or 5 V directly to the Xbox button wires.

## Where it connects downstream

**Xbox button connections -> isolated relay contact(s) -> ESP32 KVM controller.**

EJECT needs its own relay channel and ESP output; never join POWER and EJECT
together. GPIO46 (POWER) and GPIO47 (EJECT) are candidates pending the final pin
audit and firmware support. Codex must be able to request PRESS POWER or PRESS EJECT, including startup
with either button. EJECT is deliberately agent-commanded, not physical-only.
Default freeze recovery uses POWER; eject-start can select a different Stellar
boot mode. Both need fresh state checks and separate firmware/API support.

The ESP controls the relay; a separate smart plug cycles Xbox AC. Those parts
are outside this adapter request. See the **[full setup plan and photo wiring
diagram](XBOX-POWER-RELAY-SETUP.md)** for context. Additional preliminary
[technical notes](XBOX-POWER-ADAPTER-TECHNICAL-NOTES.md) are available if needed.

**Could you build and test this?** Please quote the complete installation and cable,
lead time, and whether you need my console or an original cable to confirm fit.
