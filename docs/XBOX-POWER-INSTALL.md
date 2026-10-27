<!-- SPDX-License-Identifier: Apache-2.0 -->
# Xbox power-control installer worksheet

Preferred architecture: [existing ESP-KVM with isolated POWER/EJECT contacts](XBOX-XWIFI-BUTTONS.md).
Keep KVM and relay power independent of switched Xbox AC; no extra internal
microcontroller or IR receiver is required. This remains a proposed installation.

Current build scope: both POWER and EJECT, using independent isolated relay
channels. Plug-in or professionally soldered, strain-relieved button leads are
accepted. See the [current build brief](../XBOX-POWER-ADAPTER-BUILD-BRIEF.md) and
[two-channel wiring plan](../XBOX-POWER-RELAY-SETUP.md). The investigation below
retains the earlier POWER-only baseline; EJECT is available to Codex by explicit command, outside default recovery.

2026-10-20. **Proposed installation, not hardware-validated.** Read the
[investigation, limitations and recovery design](XBOX-POWER-CONTROL.md) first.
Do not enable this with the current firmware. Firmware changes and all gates
below must pass before unattended use. The owner identifies the Xbox as motherboard revision **1.4**. This is
owner-reported; the PSU revision and exact solder points remain unverified.

## Parts to procure

Exact orderable identifiers below specify the proposed design. Check current
manufacturer drawings, received markings and stock before ordering; substitutions
need review. No purchase was made. Generic assembly materials have no electrical
part-number dependency.

| Qty | Manufacturer / exact identifier | Purpose |
| --- | --- | --- |
| 1 | Adafruit **2895** | Selected non-latching relay breakout; do not buy latching 2923 |
| 1 | Vishay **VO617A-3** | DIP4 phototransistor optocoupler for rail sense |
| 2 | Vishay **MRS25000C1002FCT00** | 10k 1% axial, control pull-down and sensor pull-up |
| 1 | Vishay **MRS25000C7500FCT00** | 750 ohm 1% axial, measured switched 5 V sense only |
| 1 | Vishay **1N4148-TAP** | Antiparallel sense LED protection |
| 1 | KEMET **C315C103K5R5TA** | 10 nF ceramic, 50 V, sensor filter |
| 1 | Shelly **S4PL-00116US** | Proposed metered Xbox AC plug, US 120 V only |
| As needed | Insulated perfboard/carrier, heat shrink, strain relief, labeled twisted signal pairs, insulated enclosure and removable low-voltage connectors | Preserve separation and prevent wire fatigue/shorts |
| Existing | Independent KVM USB power supply and cables | Remain outside switched outlet; measure for backfeed |

Optional PhotoMOS carrier replaces 2895 (do not install both): Panasonic
**AQY212EH**, onsemi **2N3904BU**, Vishay **MRS25000C2400FCT00** (240 ohm LED),
**MRS25000C4701FCT00** (4.7k base), **MRS25000C1003FCT00** (100k base-emitter).
The relay GPIO pull-down remains useful. See the investigation for drive-current
corners and qualification. This alternative needs a built carrier and is not
simpler than the selected breakout for initial installation.

A hardware pulse limiter is **not** included. If an absolute pulse cap independent
of MCU execution is required, the assembly is incomplete until a separately
reviewed/tested one-shot is added. Neither a relay, PhotoMOS nor a pull-down
provides that guarantee on its own.

## Annotated low-voltage wiring diagram

Physical J1 pin numbers below come from the P4X guide. Confirm pin 1 and GPIO
silkscreen on the actual board. Do not infer numbering from this text's layout.
The previously installed audio ground can share J1-34 only with a secure branch
connection; do not stack loose jumpers.

```text
 INDEPENDENT KVM SIDE                   ISOLATED XBOX CONTROL SIDE

 J1-1 or 17 (3V3) ---- relay 2895 3V
 J1-34 (GND) --------- relay GND
 J1-36 (GPIO46) ------ relay control
                  |
                 10k
                  |
 KVM GND ---------+
                                        relay COM ---- switch terminal A [M]
                                        relay NO ----- switch terminal B [M]
                                        relay NC      NO CONNECTION
                                        Original button remains in parallel.
                      contacts create closure only; never feed 3V3 into Xbox

 J1-1/17 (3V3) --10k--+-- J1-33 (GPIO48)
                     |
                     +-- VO617A-3 pin4 collector   || pin1 anode --750R-- +5V_SW [M]
                     +-- 10nF -- KVM GND           || pin2 cathode ------ Xbox return [M]
 KVM GND --------------- VO617A-3 pin3 emitter     ||
                              optical barrier     || 1N4148 across pins1/2:
                                                  || cathode to1, anode to2

 GPIO48 LOW = switched rail present. HIGH = rail absent OR sensor disconnected.
 J1-37 (GPIO47) UNUSED. No Xbox reset connection.
 GPIO22/53/27 audio wires stay exactly as installed.

 AC outlet ---- Shelly S4PL-00116US ---- Xbox AC cord only
 Separate supply --------------------- KVM USB-Serial/JTAG power
 Separate power ---------------------- router / automation host / display as needed
```

[M] terminals and rail locations are **unverified**, not solder instructions.
Map the actual motherboard/PSU and front-button assembly first. The relay
contact pair is not tied to either ground by this design. Isolated sense input
uses Xbox return; isolated output uses KVM return. Existing HDMI/USB may connect
system grounds elsewhere. Keep all new wiring on the low-voltage side.

## Proposed firmware settings after implementation

Record saved and effective values before making any changes. The following is
an installer reference, **not a command to apply now**:

| Setting | Proposed value | Condition |
| --- | --- | --- |
| atx_enable | false during assembly; true only after tests | Current default false |
| atx_pwr_gpio | 46 | J1 physical 36 |
| atx_rst_gpio | -1 | Requires optional-reset firmware patch; current firmware disables whole module |
| atx_led_gpio | 48 | Name retained for compatibility; connected to switched rail, not LED |
| atx_active_high | true | Relay or transistor-driven PhotoMOS |
| atx_led_ah | false | Collector goes low when rail is present |
| atx_short_ms | 200 initially | Measure response; proposed Xbox mode max 500 ms |
| atx_long_ms | unused | Disable hold route in Xbox mode; value alone does not disable it |
| HDMI audio | BCLK22 / WS53 / DIN27 | Preserve compiled and effective setup |
| Network | Ethernet primary / Wi-Fi fallback | Preserve C6 pins even while unused |
| Round SPI display | disabled | Default CS22 conflicts with audio |

Firmware must expose independent reset availability, busy/operation completion,
freshness/validity and target mode. Those fields/APIs do not exist yet. Do not
work around the mandatory reset bug by temporarily assigning another real pin.
Preserve DSI live three-buffer configuration. Confirm all runtime display/ATX
claims, not just defaults. Authentication is checked by power handlers, but the
Xbox build overlay disables HTTPS/auth at build time: verify deployed policy and
limit power control to a trusted network/VPN until authenticated access is enabled.

## Installation gates and acceptance tests

Record date, operator, firmware commit/build ID, KVM boot ID, board photo,
Xbox/Stellar revision, instrument and measured results. Blank or contradictory
results mean **not accepted**. Electrical work is with AC removed; observe
low-voltage waveforms only after leads are secured. No PSU-primary probing.

| Gate/test | Pass criterion | Result |
| --- | --- | --- |
| Board identity | PCB marking/photo v1.6 and silicon revision independently recorded; correct C6 profile | Pending |
| Schematic | Actual matching schematic inspected, 46/48 nets checked; guide's broken archive link alone insufficient | Pending |
| Runtime claims | Saved/effective pins captured; 46/48 clear of all active functions; USB/debug retained | Pending |
| Button identification | Continuity identifies only the physical power contacts; no eject/reset confusion | Pending |
| Button electrical limits | Open voltage <=5 V; closure current <=10 mA; measured safely without shorting a supply | Pending |
| Rail identification | Measured OFF-with-AC / ON / frozen / AC-removed / restored levels and decay; switched 5 V confirmed | Pending |
| Sensor loading | ~5 mA available without rail disturbance; low <0.4 V and valid high at GPIO48 | Pending |
| Sensor fault cases | Open cable, shorted input and conflicting meter states inhibit autonomous clicks; false OFF never treated as proof | Pending |
| KVM independent power | Xbox AC/USB loss does not reset KVM; no unintended backpower through USB or HDMI | Pending |
| Driver bench | Dummy continuity load releases on KVM power loss/reset; no boot/reset/brownout closure on scope, 20 cycles | Pending |
| Release bound | 200 ms nominal pulse measured; <=500 ms operational release including loaded firmware; stopped-worker/watchdog test recorded | Pending |
| Strong failure guarantee | Explicitly accept software-supervised bound or qualify hardware limiter for arbitrary stuck-high failure | Pending |
| Manual function | Original physical switch still works; one short commanded closure boots from standby, no unintended second closure | Pending |
| Button cycles | 20 supervised cold-start trials, exactly one pulse when rail OFF; already-ON trials produce zero pulses | Pending |
| Input quality | Debounce rejects <100 ms chatter, stale >1 s unknown; OFF/ON transition deadlines satisfied | Pending |
| Plug identity/config | S4PL-00116US label, firmware/RPC mode, limits, AC default ON and local restore timer verified | Pending |
| AC evidence | Meter standby/ON bands recorded; restoration detectable downstream; otherwise certified sensor/operator gate | Pending |
| Timeout/duplicate | Lost HTTP responses, concurrent IDs and retransmissions cause <=1 admitted physical press | Pending |
| Restart matrix | Automation restarted at each state; KVM reboot during/after press; no replay/toggle; budgets survive | Pending |
| Network loss | Ethernet removal/fallback, plug unreachable and stale status all fail closed for button admission | Pending |
| Peripheral regression | Capture/audio/DSI/touch/SD/debug work before/during/after operation under load | Pending |
| Recovery end-to-end | Supervised genuine freeze or controlled test: AC OFF dwell, confirmed restore, one press, fresh boot response | Pending |
| Boot failure | Rail ON with no application response locks out without a second toggle | Pending |
| Limits | Two attempts max/recovery, one AC cycle/recovery, cooldown/rate budget enforced and journal retained | Pending |

Freeze detection and post-boot confirmation must be defined for the application
under test. A unique fresh nonce or game-side heartbeat is preferred. If only a
person can confirm boot, keep that step manual and label it accurately. Never
call LED color, HDMI presence, static frames, ping or metered watts a definitive
liveness signal.

## Measurement sheet

- Board PCB / chip / running build:
- Xbox motherboard: **1.4 (owner-reported)** / PSU revision: unverified / Stellar firmware: unverified
- Switch terminals photo reference / open voltage / closure current:
- Switched rail location photo / OFF-with-AC V / ON V / frozen V:
- AC removal rail decay time / AC restore standby settling:
- Sense LED current / GPIO48 low-high / load impact:
- Pulse nominal/min/max under load / reset and stall trace references:
- Standby watts band / ON watts band / meter resolution / AC evidence method:
- Manual button duration needed / any AC-return auto-start behavior:
- Boot evidence method / typical and maximum boot time:
- Firmware/API tests passed / unresolved limits:
- Acceptance decision and operator:

## Plug-in adapter research (2026-10-20 follow-up)

The owner's preference is to avoid soldering small Xbox board points. Earlier
chat descriptions of four soldered wires were a possible approach, not a
requirement. Prefer removable harnesses if their pinout can be verified.

**Source verified:** the original Xecuter 3 CE installation guide covers Xbox
v1.0-v1.6 and describes a **Power/Eject No Solder Adapter**, installed between
the existing yellow front-button cable and its motherboard connection (page 12).
This includes revision 1.4 mechanically in the documented installation scope.
It was intended as an X3 button-signal breakout, not an ESP-KVM controller:
verify its circuit, exposed signals/ground and connector orientation before
connecting relay contacts. Do not connect it directly to ESP GPIO or replace
Stellar with an X3. A suitably wired mating lead could move any assembly work
off the Xbox board, but no ready-to-use ESP mating harness was verified.

- [Original X3 CE guide, page 12](https://consolemods.org/wiki/images/5/5e/X3ce.pdf).
- [Helder's OSH Park X3 Reset/Power Adapter](https://oshpark.com/shared_projects/q30n1XCY)
  describes BlueMeanie23's replacement design. This is a **bare PCB fabrication
  project**, not a confirmed assembled plug-in product. Connectors and assembly
  would still be needed; the displayed board price is not a complete-kit price.

**Availability limit:** searches did not establish a current in-stock assembled
X3 adapter with its mating cable. Historical existence is confirmed; a buy-now
recommendation is not. XIR Easy Kit has historical solderless reports, but no
current manufacturer-supported supply/ESP interface was established. XERC is
not established as a solderless solution. Kratos is an active front-panel
replacement project with additional features, but no verified simple ESP relay
connection was found; do not buy it as a passive adapter for this design.

**Proposed sensing alternative:** use a removable branch in the hard-drive's
4-pin peripheral power cable instead of motherboard solder points. A standard
LP4 splitter such as [StarTech PYO2L](https://www.startech.com/en-us/cables/pyo2l)
provides a candidate harness; a mating sense lead would feed only the existing
isolated sensor's Xbox-side 5 V input/return. This is an engineering proposal,
not a tested revision-1.4 assembly. Confirm actual drive/adapter arrangement,
connector fit, pinout and switched 5 V behavior in standby/ON/freeze; never route
12 V into the 5 V sensor. A preassembled sensor lead could avoid Xbox soldering
entirely. No exact ready-made isolated LP4 sensor product was verified.

Practical next step: source or have a technician assemble and continuity-test
(1) an inline front-button breakout plus relay lead and (2) an LP4 sense lead.
Installation can then consist of opening the case and inserting connectors,
with assembly soldering performed on removable parts outside the console.
Until those assemblies are verified, the no-solder installation remains feasible
in principle rather than a complete purchasable kit.

## Builder shortlist and quote request (2026-10-20)

No current assembled X3 power/eject adapter listing was verified. The following
are **candidate builders**, not confirmed suppliers of this particular kit.
No contact, order or payment was made. Price, willingness to make a cables-only
kit, matching connectors, and delivery time all require a direct response.

1. **TK's Custom Super Modz / Anthony (Tony) Kuberka**, Covington, WA.
   Best first inquiry based on relevant assembly work: shop's XERC 2 XE listing
   describes building and functional-testing Xbox power-control hardware with a
   custom test adapter. Its cable/parts catalog also includes Xbox-specific
   cables and Molex adapters. The XERC product itself requires soldering and is
   not the requested solution. Some catalog requests redirected to a password
   page; do not assume current ordering availability.
   - Email: `TKsCustomSuperModz@gmail.com`.
   - [Official contact](https://tkscustomsupermodz.com/policies/contact-information).
   - [Relevant power-control assembly experience](https://tkscustomsupermodz.com/products/xerc-2-xe).
   - [Molex adapter product context](https://tkscustomsupermodz.com/products/led-ring-of-light).
2. **ModzvilleUSA**, Fullerton, CA.
   Candidate based on Xbox PCB/harness design work and offered console services.
   The Modxo Ultra page credits PCB design and an X3-style wiring harness; that
   product is shelved and sold out, not a purchase recommendation or a source
   for a currently available harness.
   - Email: `modzvilleusa@proton.me`.
   - [Official service inquiries](https://modzvilleusa.com/pages/send-in-services).
   - [Contact](https://modzvilleusa.com/pages/contact).
   - [Harness/design experience](https://modzvilleusa.com/products/xbox-modxo-ultra).

### Unsent inquiry text

> I have an original Xbox motherboard revision 1.4 with a Stellar modchip and
> an existing ESP32-P4 KVM. Can you supply a fully assembled and tested plug-in
> cable/interface kit so I do not have to solder or cut any original Xbox wires?
>
> I need an inline adapter for the original power/eject cable, with an isolated
> normally-open relay across the power-button signal and return, preserving the
> existing buttons and Stellar. An X3-style power/eject breakout may be useful,
> but I do not want to replace my modchip.
>
> I also need an isolated power-state sensor fed from switched 5 V and return,
> preferably via a hard-drive LP4/Molex pass-through or splitter, rather than
> motherboard solder points. The KVM side should use labeled 2.54 mm female
> sockets for GPIO46 (active-high button control), GPIO48 (active-low sensing),
> 3.3 V and GND. No Xbox voltage should connect directly to an ESP GPIO.
>
> Please quote the complete assembled interface, all mating cables/connectors,
> insulation/enclosure and continuity/function testing, with installation photos
> for a revision 1.4. Can you test against your own Xbox, or would you need mine?
> Please identify any remaining soldering, original-wire cutting or case changes,
> lead time and total cost. I can provide a proposed schematic/BOM for review;
> firmware support is being handled separately.

This is a request for design/assembly review, not a manufacturing-ready pinout.
The builder must confirm exact Xbox connector wiring, switched rail behavior,
interface ratings and low-current button switching. Ask for the **complete
interface**, not only loose cables: relay/sensor assembly soldering must also
be done by the builder to meet the owner's plug-only requirement.
