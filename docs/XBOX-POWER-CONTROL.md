<!-- SPDX-License-Identifier: Apache-2.0 -->
# Original Xbox power recovery investigation

Current build scope: both POWER and EJECT, using independent isolated relay
channels. Plug-in or professionally soldered, strain-relieved button leads are
accepted. See the [current build brief](../XBOX-POWER-ADAPTER-BUILD-BRIEF.md) and
[two-channel wiring plan](../XBOX-POWER-RELAY-SETUP.md). The investigation below
retains the earlier POWER-only baseline; EJECT is available to Codex by explicit command, outside default recovery.

Investigated 2026-10-20 against source commit
`61f43235c024d9738d9a37342d334512dc74a82c`.
Documentation and proposed design only. No firmware, saved settings, hardware,
or remote repositories were changed. Installer checklist: [XBOX-POWER-INSTALL.md](XBOX-POWER-INSTALL.md).

## Recommendation and readiness

Reuse the P4 KVM. Proposed first installation: Adafruit **2895 non-latching
Mini Relay FeatherWing**, driven by GPIO46, with its COM/NO contacts across the
physical Xbox power switch. Sense a measured switched low-voltage Xbox rail
with a Vishay **VO617A-3** into GPIO48. Leave GPIO47 and Xbox reset unwired.
The Shelly controls only the Xbox AC cord; the KVM and network stay independently
powered. No additional Pico or KVM is required.

This is **not ready for unattended recovery**. Firmware needs the changes below;
actual saved pins, board schematic connectivity, Xbox revision/solder points,
button current, rail voltage, and reset/pulse behavior remain installation gates.
A relay or PhotoMOS alone does not impose an absolute pulse deadline if the MCU
hangs while driving high. The proposed first installation relies on a separately
serviced release deadline plus watchdog reset; this must be fault-tested. If a
hard electrical maximum under arbitrary MCU failure is required, add a tested
non-retriggerable hardware one-shot before authorizing unattended operation.
That limiter is not designed or included in this parts list. Do not describe a
pull-down as protection against a stuck-high driven output.

Evidence labels used here:

- **S**: source verified, including manufacturer documents and repository code.
- **L**: existing local observation/report; not repeated in this investigation.
- **P**: proposed engineering choice; must pass acceptance tests.
- **M**: measurement or identification still required.

## Xbox identification update

The owner reports an **Xbox motherboard revision 1.4**. Use that revision when
researching the front-button connection and switched rail. PSU revision, actual
connector orientation, switch contacts, rail locations and Stellar firmware
remain unverified; this report does not yet provide revision-specific solder
instructions.

## Identity, profiles, and evidence limits

**L:** `HDMI-AUDIO.md` records PCB **v1.6**, chip **3.2**, a supplied board photo,
and working PCM on 22/53/27. `XBOX-RIG-DETAILS.md` agrees on P4 3.2 + C6 + C790 +
Waveshare 3.5 DSI (E). `boards/funcev_audio.defaults` explicitly records v1.6.
**S:** `funcev_p4.defaults` still labels the PCB v1.5.2 in its comment, while
selecting chip >=3.0. `AGENTS.md` and `HARDWARE-NOTES.md` describe an older
Waveshare P4-ETH rev 1.3 rig. Those are not evidence for this Xbox rig's pins.

**S:** Espressif's P4X guide says P4X uses silicon v3.1 or later; it does not
establish that this particular unit is 3.2 or PCB v1.6. Confirm silkscreen/photo
and read-only system information or an existing boot log; record both separately.
Do not flash a rev<3 image to determine identity.

**S:** Plain defaults target the old board. Intended overlay order is
`sdkconfig.defaults;boards/funcev_p4.defaults;boards/funcev_xbox.defaults;boards/funcev_dsi_live.defaults;boards/funcev_audio.defaults`.
Private credentials overlays are separate and must not be copied into reports.
`funcev_p4_rev1`, `funcev_c5_p4`, DSI probe/BIST/PLL profiles are not interchangeable
with this C6 rev3 live configuration. `funcev_dsi_triple` is now a compatibility
alias; `funcev_lcd_record` additionally limits capture to 720p.

**S:** Existing ignored `build.audio-xbox/sdkconfig` was inspected only for
relevant nonsecret symbols: FUNCEV, rev3-style profile, audio 22/53/27, three
DSI buffers, Ethernet/Wi-Fi failover, USB Serial/JTAG console, ATX defaults
46/47/-1. This is build evidence, **not proof of what is running**. There is
no root `sdkconfig`. Other build directories exist and may differ.

**M:** A read-only HTTP attempt to `esp.local` failed DNS resolution. Live
settings and running image were not verified. No subnet scan or serial reset
was attempted. Later use the operator/router-confirmed address and read
`GET /api/v1/system/info`, `/api/v1/pins`, `/api/v1/settings`, and
`/api/v1/settings/schema`; retain only nonsecret relevant values. The attempted
`/api/v1/status` URL was not a verified endpoint; DNS failed before any HTTP.
The source exposes ATX status in `/api/v1/system/info`.

## Pin and peripheral audit

**S:** The current official **P4X** J1 table independently confirms the proposed
numbers; do not count from an arbitrary photo orientation. GPIO labels and
physical positions are different numbers.

| Use | GPIO / J1 physical pin | Disposition |
| --- | --- | --- |
| Power switch driver | 46 / 36 | P: use; no claim in audited peripheral config |
| Reset default | 47 / 37 | P: leave unused, set -1 after firmware fix |
| Isolated rail sense | 48 / 33 | P: use; no claim in audited peripheral config |
| KVM ground | GND / 34 | S: header ground, already used by audio; share via secure distribution |
| Driver/sense supply | 3V3 / 1 or 17 | P: use KVM side only |
| C790 BCLK, WS, DIN | 22 / 12, 53 / 35, 27 / 38 | L: working audio; keep |
| Capture + touch/control I2C | 7 / 3, 8 / 5 | S: shared bus; keep |
| C790 RESETN | 23 / 7 | S: profile default; keep |
| microSD | 43,44,39,40,41,42 | S: CLK,CMD,D0..D3; keep all |
| SD power net | 45 / 40 (NC label) | S: attached SD_PWRn even with configured -1; do not use |
| C6 SDIO | 18,19,14,15,16,17 | S: CLK,CMD,D0..D3; reserve even on Ethernet |
| C6 reset / wake | 54 / 32, 6 / 15 | S: reserve for fallback radio |
| RMII data/control | 49,34,35,28,29,30 | S: TX_EN,TXD0,TXD1,CRS_DV,RXD0,RXD1; keep |
| PHY clock/management/reset | 50,31,52,51 | S: REFCLK,MDC,MDIO,reset; keep |
| BOOT | 35 | S: also Ethernet TXD1; never load externally |
| Console/debug | 24,25; 37,38 | S: preserve USB Serial/JTAG and UART/strap alternatives |
| Crystal / unavailable | 0,1; J1 27,28 | S: crystal resistor changes required for 0/1; NC positions are not GPIO27/28 |

DSI and CSI lanes use dedicated interfaces; they are not free generic header
GPIOs. The implemented Waveshare DSI preview polls GT911 at 0x5d/0x14 on the
capture I2C bus and the control MCU at 0x45; it does not claim 46/47/48.
The official Espressif LCD adapter instructions use **GPIO27 reset / GPIO26 PWM**;
do not add that vendor adapter wiring to this working Waveshare setup, since
27 is already audio DIN. Audit physical extra wires as well as source.

Possible alternate header GPIOs are not approved spares: 2/3/4/5 may have
onboard codec or control connections; 20/21/22/26/33 are round-display defaults;
32/33 and 36/37/38 can serve debugging/straps. Prefer the specifically audited
46/48 pair and keep 47 spare. ESP32-P4 strap pins include 34..38. Do not apply
ESP32 classic restrictions (e.g. its GPIO46 rules) to a P4. P4 GPIO46/48 support
ordinary digital I/O; use only 3.3 V domain signals and verify rail/pad levels.

**S/M schematic limit:** The P4X guide links a reference-design ZIP. During this
investigation that URL returned an HTML documentation application rather than
an archive; an alternate `dl.espressif.com/dl/schematics/` path returned an error
document. Neither is a reviewed v1.6 schematic. The official J1 table is verified,
but internal net/strap/power-on coupling to 46/48 still needs the actual PCB
v1.6 schematic and continuity check. Do not substitute the old v1.4/v1.5.2 drawing
and mark that gap resolved. This is an explicit remaining audit limitation.

### Runtime claims are incomplete

`board_reserved.c` covers capture, SD, Ethernet, BOOT, selected UART pins, and
C6 transport/wake. It does **not** reserve the compiled HDMI audio pins or all
physical USB/crystal/codec connections. `board_header.c` even presents 45 as an
ordinary IO when SD power is -1, despite the physical SD_PWRn connection;
`board_reserved.c` only includes its extra 45 guard under Wi-Fi compilation.

`kvm_settings.c::check_pins` builds runtime KVM_SF_PIN claims, including visible
display and ATX settings. Audio uses CONFIG_KVM_AUDIO_* directly, so those pins
are absent from this dynamic table. `pin_conflict.c` rejects conflicts involving
changed pins, deliberately tolerating pre-existing saved conflicts. NVS loading
checks scalar ranges but does not establish full physical safety. ATX apply()
itself checks only nonnegative power/reset values; no valid-output macro,
uniqueness or reserved-pin validation. A UI suggestion is not pin clearance.

## Switch implementation comparison

| Choice | Electrical behavior | Assembly / failure behavior | Decision |
| --- | --- | --- | --- |
| Adafruit 2895 non-latching breakout | Transistor-driven coil, flyback circuit, dry COM/NO contacts; 3.3 V control; about 50 mA supply while active; signal does not supply coil current | Supported schematic/guide, terminal block; floating/low input releases, loss of coil supply opens; mechanical wear/contact bounce and minimum-load behavior need testing | P: simplest first installation |
| Panasonic AQY212EH PhotoMOS | DIP4 normally open bidirectional MOS contacts, 60 V absolute / 48 V recommended load limit, 0.55 A, 2.5 ohm max on resistance, 1 uA max off leakage, 5 kVrms component isolation | Fewer moving parts, silent, compact; requires a carrier and intentional LED driver; leakage/resistance must suit button circuit | P: good alternative when building a custom carrier |

Manufacturer-supported means the breakout has published design/support, not
that Adafruit or ESP-KVM has validated it with Xbox. Never use a latching relay:
a reboot must not retain a closed switch. Never drive a bare coil from a GPIO.
For 2895 wire its **3V power pad** to KVM 3V3, GND to KVM GND, control pad to
GPIO46, and select COM/NO. Confirm received board schematic/pad labels and
measured supply current. Add an external 10k pull-down on control.
Manufacturer guide rates the relay family up to 2 A at 30 VDC resistive loads;
our application is a low-current button, not AC switching. Require measured
button voltage <=5 V and closure current <=10 mA for this proposed installation.
Measure stable switching at that low load: a large maximum rating alone is not
a low-current reliability guarantee. Keep all mains switching in the enclosed plug.

PhotoMOS alternative, **not fitted together with the relay**:
AQY212EH pins 3/4 across button terminals, either direction. LED pin1 fed from
KVM 3V3 via 330 ohm; pin2 to collector of onsemi **2N3904BU**, emitter KVM GND;
GPIO46 through 4.7k to base, base to GND through 100k. At 3.3 V, Vf 1.25 V and
Vce 0.1 V, LED current ~5.9 mA; at 3.0 V, Vf 1.5 V, Vce 0.2 V ~3.9 mA.
The latter exceeds 3 mA maximum operate threshold but falls below Panasonic's
5 mA recommended drive: qualify actual supply/temperature or reduce to 240 ohm
(Vishay MRS25000C2400FCT00) for ~5.4 mA at that corner, ~8.1 mA nominal.
**Use 240 ohm in an installed alternative.** Base drive ~0.5 mA, use conservative
forced beta and measure saturation. Check actual transistor pinout. External
bias releases on high impedance; it cannot defeat a driven-high GPIO. Keep
button circuit isolated from the KVM supply and ground in both alternatives.

Isolation here prevents a new DC control connection. HDMI and USB may already
join grounds, so do not claim that the entire rig is galvanically isolated.
Do not use these hobby assemblies as a mains isolation barrier.

## Rail sensing, electrical design, and state meaning

**P:** Choose a low-voltage **switched 5 V** rail only after identifying the Xbox
motherboard/PSU revision. Xbox-side rail -> 750 ohm -> VO617A-3 pin1 anode;
pin2 cathode -> Xbox low-voltage return. Add Vishay **1N4148-TAP** antiparallel
to LED (cathode at pin1) for reverse protection. KVM side: pin4 collector to
GPIO48 and 10k pull-up to KVM 3V3, pin3 emitter to KVM GND. 10 nF collector to
KVM GND suppresses brief noise; software supplies the main debounce. Active-low
means powered. Diagram and exact parts are in the installer sheet.

At 5 V / Vf ~1.2 V, current ~5.1 mA; resistor power ~19 mW. At 4.75 V / Vf 1.65 V
and +1% R, ~4.1 mA. The VO617A-3 CTR bin is 100..200% **at 5 mA**; don't extend
that minimum blindly across temperature/current. Only ~0.33 mA is needed to pull
10k low; verify output <0.4 V at minimum rail/current, hot/cold installation
conditions, and after settling. VO617A has 80 V VCEO, 50 mA collector and 60 mA
LED absolute limits, and 5.3 kVrms component isolation; use those as limits,
not operating targets. At 12 V this resistor would dissipate ~0.16 W and drive
~14 mA: **do not reuse the 5 V design on a different rail**. For a verified
3.3 V rail, recalculate and qualify instead of guessing.

**M:** No Xbox solder location, PSU connector number, wire color, or motherboard
revision is asserted here. Identify switch contacts by continuity with AC
unplugged; identify switched rail by measurements OFF-with-AC, ON, frozen,
AC-removed and AC-restored. Keep clear of PSU primary/mains and do not open the
plug. A rail that stays present in standby is unsuitable. A rail that disappears
during the freeze is insufficient alone. Record Stellar firmware and observed
AC-return/button behavior; do not assume PC ATX long-press semantics.

Rail ON proves power presence, not application liveness. Rail OFF also cannot
distinguish a disconnected sensor from a powered-down target. Until a successful
ON/OFF calibration, treat it as unknown. Use rail, calibrated metered-load bands,
and sensor diagnostics together; contradictory evidence locks recovery out.
Front LED is programmable; HDMI may stay locked with a frozen frame; ping may
succeed in a dead application. None is a definitive freeze detector. A fresh
application nonce/response or operator confirmation is needed to call it alive.
An advancing game-side heartbeat plus responding input is preferable. Without
such instrumentation label outcome POWERED / BOOT VISUALLY CONFIRMED, not ALIVE.

## Firmware reliability audit and focused implementation plan

**S:** `components/kvm_atx/kvm_atx.c` and its header explicitly state blind,
untested optocoupler bring-up. This investigation did not test that hardware.

| Area | Current source | Focused change / required evidence |
| --- | --- | --- |
| Power-only | apply requires pwr>=0 AND rst>=0 | Require valid power only; reset=-1 supported; reset action returns unavailable without driving anything |
| Initialization | gpio_config enables output before setting idle; gpio_set_level errors ignored | Preload inactive latch, then enable output; external bias; check every error and release on partial failure; scope boot and brownout |
| Release | Worker asserts, vTaskDelay, releases; config mutex separated from operation mutex | Keep serialization; independent monotonic release deadline, bounded short-only Xbox pulse, release on every error; verify under stalled worker |
| Watchdog | No ATX task-specific watchdog enrollment or release deadline | Add explicit supervision with a tested reset path; ordinary idle/task watchdog is not proof an individual worker cannot stall |
| Reconfiguration | Waits for whole pulse/150 ms gap; old pins reset to inputs | Reject or defer changes while busy, drain obsolete work, validate complete effective pin set on startup and enable |
| Queue | Four pending command slots plus possibly one executing | Atomic single in-flight admission; no future toggles queue; busy covers admission through release and cooldown |
| Duplicates | Enum-only queue has no identity or deadline | Caller operation ID, parameter hash, state precondition and boot ID; duplicate returns original status, never another press |
| Busy HTTP | 429 only when queue full; says action in progress | Return explicit busy while any op reserved; preserve existing errors or version new contract |
| HTTP success | 200 {status:ok} after enqueue | Distinguish accepted/started/released/completed/failed/uncertain; success means electrical release, not boot success |
| Status | system/info atx.enabled/known/on; known means sense pin exists | Timestamped sample, sample sequence, boot ID, debounce, health/validity, last transition, operation state; unknown on restart/stale/error |
| Sense | gpio_get_level once per status read, internal bias towards OFF | Periodic sampler independent of HTTP; external pull-up, active-low; stable 100 ms and freshness <=1 s proposed |
| Bounds | Settings permit short 50..2000 ms, long 1000..15000 ms | Xbox mode short nominal 200 ms, hard cap 500 ms; disable hold/reset routes in this target mode |

Also add compiled audio claims to the central pin registry, USB/crystal and board
physical exclusions, correct GPIO45 presentation, and boot-time validation of
stored settings. Use `GPIO_IS_VALID_GPIO` / `GPIO_IS_VALID_OUTPUT_GPIO`, enforce
all distinct pins and correct board restrictions. Invalid existing NVS should
leave ATX disabled with a specific reason, not reclaim another peripheral.

Pin settings have KVM_SF_REBOOT metadata, but `main.c` calls apply for atx_ changes;
the module rereads settings on each apply. Do not rely on a UI reboot badge for
safe sequencing. Inspect/update setters and callbacks as part of the patch.
Document when effective versus saved configuration changes.

Proposed operation contract (new API, not currently callable): accept a unique
operation ID, expected KVM boot ID, and fresh debounced OFF sample sequence.
Atomically reserve, recheck OFF and cooldown immediately before assert, schedule
release, then assert. Record started before driving and record released after
successful deassert. A bounded in-memory operation ledger deduplicates within a
boot. After reboot all old boot IDs are rejected; outputs start inactive, no
commands replay, previous ambiguous operation requires external reconciliation.
An alternative durable journal must mark intent before assert and never replay
unresolved intent. Do not promise exactly-once physical effects across power loss.
Rate-limit distinct IDs too; dedup alone doesn't stop repeated toggles.

API changes require updates to web submodule, Xbox skills fork and MCP companion
per AGENTS.md. No companion edits are made by this documentation-only task.
Keep existing routes backward compatible or explicitly version/deprecate them;
a timeout of the current click API has no operation lookup and is ambiguous.

### Meaningful tests for the future patch

- Unit/state tests: power-only works; reset disabled; invalid/reserved/duplicate
  pins and stored conflicts rejected; 22/53/27 protected with audio compiled;
  boot state unknown; bouncing/stale/disconnected inputs never authorize a click.
- Concurrency: two simultaneous IDs admit one; same ID repeated before/after
  completion gives one pulse; queue saturation cannot defer toggles; reconfigure
  during press safely waits/rejects; an old generation never runs with new pins.
- Timing/error injection: assertion/release GPIO failures, timer allocation
  failure, task creation failure and memory exhaustion fail inactive; pause the
  worker while asserted and measure independent release/reset behavior. Test
  interrupt starvation separately; a software timer is not a hardware guarantee.
- HTTP: drop request before/after admission, drop response after completion,
  disconnect client, retry same ID, reboot KVM, expire ledger. Verify an old
  boot ID cannot produce a new pulse and current legacy timeout is never retried.
- Integration: run capture, audio, DSI/touch, SD writes, Ethernet then Wi-Fi
  failover under peak load and record pulse timing and sensor age. No flash is
  authorized by this investigation; these tests are for the implementation stage.

## Recovery controller design

**P:** Implement on the existing external automation host, with local network
access to plug and KVM. Only one authority may control recovery; lease/lock and
persistent journal are mandatory. No automation was installed here.

Use Shelly explicit `Switch.Set` ON/OFF, never Toggle. Query device identity,
firmware, configuration and `Switch.GetStatus` capabilities first. Proposed
model is **S4PL-00116US** (120 V/60 Hz, max 15 A; published 1800 W is resistive).
Qualify Xbox PSU inrush rather than applying the resistive rating blindly.
Use local Wi-Fi RPC in a supported mode; do not assume Zigbee mode exposes the
same RPC. Reserve its address; preserve protection limits. Proposed power-up
state ON, no energy-saving auto-off/schedules, with a tested local auto-on
safety timer during deliberate OFF (e.g. 60 s). Verify supported fields on the
received firmware before configuring later. The KVM is never on this outlet.

Switch output=true is relay status, and reported voltage may be **input** voltage;
it does not alone prove Xbox outlet AC. Commission a known-good standby-load
band and ON band with this Xbox. Restore confirmation requires fresh output ON,
no protection errors, valid mains telemetry, and repeatable downstream standby
consumption evidence. If standby is below meter resolution, or load behavior
cannot distinguish a failed-open contact, add a certified downstream AC-presence
sensor or require operator confirmation; remain AC_UNKNOWN meanwhile. Do not
attach mains to an optocoupler perfboard. Meter power supports diagnosis but is
not an application heartbeat.

Proposed starting timings below require measured tuning. Each wait has a deadline.
A failed deadline enters LOCKOUT with a reason, rather than falling through.

| State | Entry/action | Required transition |
| --- | --- | --- |
| IDLE | Collect fresh health, retain history | Operator-confirmed freeze or failed fresh application challenge repeatedly over >=30 s; not HDMI/ping alone |
| PRECHECK | Acquire exclusive lease; journal recovery ID and limits; check independent KVM power, sensor calibration, fresh plug/KVM IDs | All evidence healthy, no conflicting session/operation; otherwise lockout |
| AC_OFF_INTENT | Journal intent before explicit OFF | Reconcile actual output OFF, load near calibrated zero and switched rail OFF stable >=2 s within 10 s |
| DISCHARGE | Keep confirmed OFF for initially 15 s; observe rail decay | Entire validated off dwell elapsed; timeout/contradiction locks out |
| AC_ON_INTENT | Journal before explicit ON | Fresh ON plus downstream evidence above within 15 s; explicit ON may be retried twice because it is idempotent |
| SETTLE | Observe rails; initial 5 s standby stabilization | Fresh debounced sensor; if ON already, skip button entirely and observe boot |
| PRESS_INTENT | Journal operation ID, boot ID and OFF sample; submit once | Only if AC confirmed and OFF fresh, no busy/cooldown; never issue a toggle based on old OFF |
| WAIT_RELEASE | Poll operation status using same ID | Released/completed within 2 s; any timeout/unknown enters reconciliation, not resend |
| WAIT_POWER | Observe fresh sensor >=500 ms stable ON | Within 10 s; otherwise reconcile known release, plug and sensor before considering bounded retry |
| WAIT_BOOT | Fresh application challenge or operator boot observation | Within initially 120 s; rail ON without liveness is powered-but-unconfirmed |
| COOLDOWN | Persist outcome and suppress new recovery >=5 min | New recovery requires new evidence and budget |
| LOCKOUT | Persist cause, inhibit all automatic button commands | Operator review; if this recovery left AC off, bounded explicit ON restoration is allowed unless plug protection fault forbids it |

Retry policy: at most **two button attempts per recovery**, and at most **one AC
cycle per recovery**, initially **two recovery cycles per hour** across restarts.
A second button attempt is permitted only after the previous operation is
positively released/completed, its entire power-observation window elapsed,
rail still freshly OFF, sensor/AC valid, and >=10 s cooldown. Use a new ID for
that deliberate second attempt. With the current API, completion is unknowable:
a lost click response locks out, even if the rail still reads OFF. If rail is ON
but boot fails, do not click again (it could turn it off); lock out rather than
looping AC cycles. Never use `/power/hold` as an Xbox hard-off assumption.

On automation restart, read the journal and acquire the lease before acting.
Re-query actual plug state, KVM boot ID, operations and fresh rail state; never
resume by replaying the last action. Unknown button intent means observe/reconcile
and lock out if ambiguous. An AC_OFF/DISCHARGE restart restarts a full off dwell
from a new confirmed observation (bounded by auto-on timer), or restores ON and
locks out if the timer already fired. KVM reboot invalidates old samples and IDs.
If AC_ON is uncertain retry explicit ON only, then verify it. Clock rollback
cannot clear budgets: use monotonic time within a run and conservatively preserve
cooldowns after reboot. Persist attempt budget before external effects, log
intent/admission/release/rail transitions separately, and redact credentials.

## Sources and verification record

Local source paths are relative to repository root; use the recorded commit to
recover this audit even after code changes:

- `components/kvm_atx/{kvm_atx.c,include/kvm_atx.h}`: apply, queue, worker, status.
- `components/kvm_config/{kvm_settings.c,kvm_settings_table.c,pin_conflict.c}`:
  NVS precedence, settings flags, bounds and changed-only conflicts.
- `components/kvm_board/{board_reserved.c,board_header.c}`: physical map/claims.
- `components/kvm_web/http_server.c`: atx_action around line 1294; system info
  ATX fields around 874; route registrations around 5060.
- `main/main.c`, `main/Kconfig.projbuild`, `boards/funcev*.defaults`,
  `components/kvm_audio/kvm_audio.c`,
  `components/video_pipeline/capture_dsi_preview.c`.
- [HDMI audio](HDMI-AUDIO.md), [DSI preview](DSI-PREVIEW.md),
  [rig](XBOX-RIG-DETAILS.md), [historical hardware](HARDWARE-NOTES.md).

Primary external references accessed 2026-10-20:

1. [Espressif P4X guide and J1 table](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32p4/esp32-p4x-function-ev-board/user_guide.html).
2. [P4X reference-design link](https://documentation.espressif.com/ESP32-P4X-Function-EV-Board-EN.zip)
   (returned HTML, schematic review unresolved); [guide source](https://github.com/espressif/esp-dev-kits/blob/master/docs/en/esp32-p4x-function-ev-board/user_guide.rst).
3. [P4 GPIO restrictions](https://docs.espressif.com/projects/esp-idf/en/stable/esp32p4/api-reference/peripherals/gpio.html)
   and [P4 datasheet](https://documentation.espressif.com/esp32-p4_datasheet_en.html).
4. [Adafruit 2895](https://www.adafruit.com/product/2895),
   [control pin behavior](https://learn.adafruit.com/mini-relay-featherwings/pinouts),
   [coil/load guidance](https://learn.adafruit.com/mini-relay-featherwings/overview),
   [schematic and relay datasheet links](https://learn.adafruit.com/mini-relay-featherwings/downloads).
5. [Panasonic AQY212EH specifications](https://industry.panasonic.com/global/en/products/control/relay/photomos/number/aqy212eh).
6. [Vishay VO617A datasheet, revision 2.8](https://www.vishay.com/docs/83430/vo617a.pdf).
7. [Shelly Plug US Gen4 identity/specifications](https://kb.shelly.cloud/knowledge-base/shelly-plug-us-gen4)
   and [Switch RPC/config/status](https://shelly-api-docs.shelly.cloud/gen2/ComponentsAndServices/Switch/).

Documentation QA only: local source cross-checks and relative-link checks. No
build or hardware test is claimed; no implementation exists for proposed APIs.
