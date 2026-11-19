<!-- SPDX-License-Identifier: Apache-2.0 -->
# Four-player controller status

The touchscreen default keeps a compact row for P1 DIRECT USB and Wi-Fi P2-P4,
with the ESP Ethernet/Wi-Fi addresses below it. The game remains visible beneath
this header. Tap a controller tile for its IP, USB state, report counters,
activity and held inputs; tap again or tap the detail panel to close it. ESP LAN
addresses remain visible while details are open. Unseen/stale adapters remain
in the row; unknown USB state is not an empty-port claim. Direct DEVICE USB is always physical P1 under the standing
operator wiring contract. Enable both `CONFIG_KVM_XBOX_S3_PLAYER1` and
`CONFIG_KVM_XBOX_DIRECT_PLAYER1` for automatic P1 selection alongside Wi-Fi P2-P4.
P1 uses configured DEVICE USB when available and its original paired S3 otherwise.
The standby S3 remains discovered with neutral input. Disconnecting/resetting
DEVICE USB cannot drain the logical P1 input queue or release P2-P4. The LCD
changes P1 DIRECT/WIFI with the selected transport. See [current wiring and modes](AGENT-XBOX-TESTING.md#fixed-controller-wiring-and-current-test-loop).

In all-S3 mode, the web header and touchscreen stats view show a compact P1-P4 row. It appears
only on firmware with S3 routing enabled. Click a web indicator, or tap the
LCD row, for details; tap the same indicator or the LCD details panel to close.
The LCD overlay is drawn only on its own framebuffer, not the captured game.

- Gray: unpaired/not yet discovered, or web status unavailable.
- Amber: Wi-Fi is reachable but Xbox USB is waiting, or all status is stale.
- Green: the receiver reports configured, unsuspended Xbox USB.
- Blue activity mark: the main ESP recently sent non-neutral input for that
  player. This is not an acknowledgement from the receiver or game.
- `!`: discovery is stale. The learned unicast route can still work.

Details separate the main ESP's sent state/counters from receiver USB submitted
and Xbox host-completed report counts. Each sample has an age; old counts remain
visible with their age instead of becoming a false disconnection or a fresh
success. USB completion proves the host consumed a report, not that a particular
game acted on it. Inspecting web details releases this browser's captured input
without taking ownership from another control session.

## Automatic P1 transport selection

Enable `KVM_XBOX_S3_PLAYER1` and `KVM_XBOX_DIRECT_PLAYER1` together. Keep all
four physically verified S3 assignments: configured main DEVICE USB takes P1,
and its paired S3 takes over when the cable is removed. Never change player
assignments merely to accommodate the direct cable. A disconnected standby S3
is expected while direct P1 is active; discovery remains available and the
standby receives neutral reports when present. P2-P4 retain independent routes.

The LCD P1 DIRECT/WIFI label and browser details follow the selected transport.
The compact header keeps all four controller tiles and LAN addresses visible;
tap a tile for USB counts and report freshness. USB ON means enumeration, not
that a particular game handled an input. See [verified handoff and current
physical assignments](CONTROLLER-HANDOFF.md).

## Data path and compatibility

Signed S3 discovery supplies per-player identity and USB configuration. A
separate low-priority P4 task reads each already discovered receiver's existing
plain-HTTP `/status`, using persistent connections, bounded requests and retry
backoff. It samples USB counters without blocking the input worker. Responses
from a previous receiver boot/address are discarded. HTTP diagnostics are
supplemental trusted-LAN observations; they do not authenticate or authorize
control. Fresh signed discovery takes precedence over diagnostic USB state.
No dongle firmware update or REST endpoint change is required.

Controller UDP output and receiver HTTP diagnostics explicitly follow the
default live network interface. Discovery remains unbound so announcements can
arrive on either LAN link. With Ethernet and Wi-Fi on the same subnet, lwIP's
first subnet match can otherwise bypass Ethernet's higher route priority.
The output socket and persistent diagnostic clients follow interface changes;
receiver identities, authentication and input-expiry rules stay the same.

The existing authenticated `/ws` status reply gains a second binary message:
`0x85` followed by UTF-8 JSON containing four objects in player order. Fields:

| Field | Meaning |
| --- | --- |
| `transport` | Selected `direct` or `wifi` output; optional for older firmware |
| `player`, `configured`, `seen`, `ip` | Slot 1-4, pairing present, authenticated discovery observed, learned address |
| `connection` | 0 not seen/unpaired; 1 reachable, USB waiting; 2 USB configured; 3 status stale |
| `held` | Main ESP's latest sent input label; very small stick values use `ANALOG` |
| `beaconAgeMs`, `diagnosticAgeMs` | Age of signed discovery and receiver HTTP sample; -1 means unknown |
| `activityAgeMs`, `completionAgeMs` | Age of last non-neutral send and last observed USB completion-counter change; -1 means unknown |
| `sent`, `sendErrors` | Successful and failed local UDP send attempts, not receiver acknowledgements |
| `submitted`, `consumed` | Last receiver USB counts; omitted until a valid sample is available |

Clients learn this extension through the existing ping/status exchange. The new
console polls status every 500 ms only after recognizing it. Older clients
ignore the extra message; older firmware never enables the strip. Polls do not
claim controller ownership. Web status is marked unavailable after a dropped
connection or three seconds without a fresh status message. The input pulse
lasts up to one second; reachability samples expire after four seconds.

## Verification

### Physical placement and mixed-mode check (2026-11-09)

After restoring power, all four S3s configured and advanced Xbox USB completions.
A 60-second sample showed 2,067 new sends per route, zero new send errors, and
about 6,000 Xbox completions per player. Initial logical-P1 Wi-Fi XBMC navigation
passed fresh screenshot review; that alone did not prove a physical socket.

The operator's physical P1 cable swap exposed different placement from the old
inventory map. `.243` (d4:05:92:44:aa:94) disappeared, while `.242`
(ac:27:6e:b0:ce:a4) stayed USB-active. The saved profile and installed main
firmware now assign physical P1 to `.243` and P2 to `.242`, leaving P3/P4 as
before. See [placement evidence](XBOX-PORT-DETECTION.md#current-placement-calibration-2026-11-09).
An absent standby P1 is no longer continuously retried while direct P1 is active;
fresh signed discovery resumes its neutral refresh.

Corrected mixed mode passed a 14.5-second sample: direct P1 completed 1,445 USB
reports; Wi-Fi P2/P3/P4 advanced by 1,401/1,575/1,574 completions, with zero new
send errors and fresh telemetry throughout. Physical direct P1 and logical
P2-P4 each navigated XBMC Games/Movies/Games in bounded, released-input tests.
The P2-P4 reviewed repeat used 200 ms presses and 2 s capture settling; the
shorter visually inconsistent attempt remains inconclusive. Earlier direct
screenshots without an active stream were excluded as stale.

Current state: direct P1, Wi-Fi P2-P4, Games selected, all input released.
The operator's swap back to the P1 S3 and its overlay recovery are still pending.
These results do not qualify a game, latency, analog sweeps or long-term
wireless reliability. Evidence: `observations/controller-ready-20261109/`.

### Automatic P1 selection (2026-11-06)

Main KVM firmware is installed with the four original receiver identities and
automatic P1 selection. Live status reports Wi-Fi P1 at `.242`, P2 at `.243`,
P3 at `.244`, P4 at `.246`; every route advances network sends. P1/P2 currently
report USB unconfigured, while P3/P4 advance Xbox USB completions. The overlay
now consumes the resolved transport instead of overwriting P1 as DIRECT.

All 85 browser tests and the production browser build passed. ASan/UBSan host
tests passed P1 source selection, unchanged P2-P4 overlay tiles through P1
switches, USB retry timing and suspended/configured-host protection. Main and
S3 firmware builds passed; no new warnings in changed sources. The full main
rebuild retains an existing unused-function warning in ESP-Hosted.

Main USB recovery is installed and its first delayed enumeration retry is
visible in the boot log. S3 0.2.2 is built but requires physical USB flashing;
the running 0.2.1 receivers cannot accept a network update or USB-reset command.
P1 Wi-Fi/direct hot switching, recovery of P1/P2 and game input are not yet
hardware-qualified. Private evidence: `observations/p1-auto-switch-20261106/`.

USB flashing follow-up: the connected adapter was positively identified as
original P2 (`d4:05:92:44:aa:94`), backed up in full, then flashed with 0.2.2.
All three written regions passed hash verification. Its HTTP status confirmed
0.2.2 booted and rejoined Wi-Fi at `.243`. The USB host for that sample was the
Mac, so this does not verify Xbox enumeration or input. P1 still needs flashing;
the main KVM was unreachable during this physical cable swap. Evidence:
`observations/p1-auto-switch-20261106/p2-flash/`.

P1 flashing follow-up: original P1 (`ac:27:6e:b0:ce:a4`) was also backed up and
flashed with 0.2.2; all three regions passed hash verification. Esptool reported
a serial-port error during the final watchdog reset, but HTTP independently
confirmed 0.2.2 booted at `.242`. That USB host was still the Mac. P2, returned
to Xbox P2 by the operator, reported configured USB with completion counts
advancing from 2,480 to 6,193. The main KVM remains unreachable during the cable
swap; four-route network qualification and physical P1 handoff await restored
main power and P1's return to the Xbox. P3/P4 remain on 0.2.1. Evidence:
`observations/p1-auto-switch-20261106/p1-flash/`.

### Dual-link routing repair (2026-11-06)

On the mixed P1/P2-P4 rig, all three S3 receivers were discoverable and
USB-configured but their routing leases expired. Main-ESP sends stopped
advancing after about 275 reports per receiver, and send errors kept growing.
A restart reproduced the failure when Wi-Fi acquired its IPv4 address.
Temporary diagnostics confirmed successful HMAC signing and `sendto()` failing
with `ENOMEM`; the allocation-failure hook recorded no failed heap allocations.
That errno alone was not evidence that the general heap was exhausted.

Explicit default-interface binding restored delivery through Ethernet (`en1`)
with both LAN interfaces active. The firmware build passed without compiler
warnings and was installed through the verified OTA path. The first 60-second
startup sample showed all receiver leases active at both endpoints and advancing
USB completions, with brief send-queue congestion followed by automatic recovery.
In the second 60-second run, P1 completed 5,891 reports; P2/P3/P4 sent
2,093/2,092/2,077 packets and their USB completion counts advanced by
5,950/5,775/5,775. All receiver leases were active at both endpoints and final
diagnostic ages were below 1.7 seconds. The strict zero-send-error check failed:
P2/P3/P4 recorded 8/9/24 transient errors, followed by continued successful sends.
This restores the formerly stuck routes; it does not establish zero congestion.
This is transport evidence, not four-player game qualification. Physical network
failover and standalone Wi-Fi reliability were not exercised in this repair.
Private logs, firmware hash and status samples are under
`observations/controller-route-repair-20261106/`.

### Earlier status checks

- Console typechecking, production build, and all 84 console tests passed.
- Host tests cover never-seen state, stale discovery with fresh diagnostics,
  USB waiting/suspended state, signed-state precedence, malformed/overflowing
  diagnostic counters, LCD hit targets and framebuffer guard bytes.
- The P4 build passed and was installed for live web/LCD checks. The live web
  view showed all four players, receiver sample ages and Xbox completion counts;
  player-2 activity was observed without taking over the controlling session.
- A passive live viewer collected 64 status messages per player. All 256
  player samples reported USB-ready, and all four receiver completion counters
  advanced. Individual HTTP diagnostic samples could be delayed (up to about
  21 seconds during startup for player 4); these were visibly aged while fresh
  signed discovery continued to report USB configuration.
- Restarting the main ESP changed all four web indicators to status-unavailable
  and hid old detail counts until fresh messages returned. The viewer did not
  take ownership from the existing controlling session.
- The host renderer passed bounds/hit-target tests and produced a preview of
  all four LCD states. On 2026-10-25, the operator confirmed that the physical
  touchscreen shows the P1–P4 row and tapping a player opens its details.
  The saved rendered preview is not a panel photograph.
