<!-- SPDX-License-Identifier: Apache-2.0 -->
# Four-player controller status

The web header and touchscreen stats view show a compact P1-P4 row. It appears
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

## Data path and compatibility

Signed S3 discovery supplies per-player identity and USB configuration. A
separate low-priority P4 task reads each already discovered receiver's existing
plain-HTTP `/status`, using persistent connections, bounded requests and retry
backoff. It samples USB counters without blocking the input worker. Responses
from a previous receiver boot/address are discarded. HTTP diagnostics are
supplemental trusted-LAN observations; they do not authenticate or authorize
control. Fresh signed discovery takes precedence over diagnostic USB state.
No dongle firmware update or REST endpoint change is required.

The existing authenticated `/ws` status reply gains a second binary message:
`0x85` followed by UTF-8 JSON containing four objects in player order. Fields:

| Field | Meaning |
| --- | --- |
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
  all four LCD states. Physical panel/tap confirmation was requested from the
  operator and is pending; a rendered preview is not a panel photograph.
