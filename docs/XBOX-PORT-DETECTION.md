<!-- SPDX-License-Identifier: Apache-2.0 -->
# Physical Xbox controller port detection

## Standing wiring contract

As of 2026-11-04, Zach always plugs main ESP DEVICE USB into physical Xbox P1,
never P2-P4. Agents must use that P1 assignment whenever direct USB is connected
without asking for socket discovery. Optional Wi-Fi S3 adapters are intended for
all four original labelled player assignments, with S3 P1 as the direct cable's alternative. This operator-maintained mapping is sufficient to design mixed routing;
automatic topology discovery is not a prerequisite for this rig. Connection
state must still come from live telemetry.

Mixed firmware enables both `KVM_XBOX_S3_PLAYER1` and
`KVM_XBOX_DIRECT_PLAYER1`, preferring configured DEVICE USB for P1 and
automatically using the original P1 S3 otherwise. Never remap P1's S3 as P2. Plain direct and all-S3 modes remain available. The automatic-discovery discussion
below concerns arbitrary/swapped adapters, not uncertainty about fixed direct P1.

## Current evidence

### Current placement calibration (2026-11-09)

The operator confirmed moving only physical Xbox P1's cable from its S3 to main
DEVICE USB. The S3 at `.243` (`d4:05:92:44:aa:94`), then labelled logical P2,
disappeared, while `.242` (`ac:27:6e:b0:ce:a4`) stayed configured with advancing
Xbox completions. Main DEVICE USB configured as physical P1 under the standing
wiring contract. The private pairing profile was corrected to current physical
P1 = d4:05:92:44:aa:94 and P2 = ac:27:6e:b0:ce:a4. P3/P4 stayed unchanged.
Earlier inventory labels described a different placement and must not overwrite
this calibration. This is a controlled operator-confirmed cable observation,
not automatic arbitrary-socket detection from USB addresses.

P1/P2 both run S3 firmware 0.2.2 after verified USB flashing. P3/P4 remain 0.2.1.
The all-S3 60-second continuity sample passed with zero new send errors; direct
P1 XBMC Games/Movies/Games navigation passed fresh screenshot review. Final
corrected mixed-mode and return-to-Wi-Fi checks are tracked in
`observations/controller-ready-20261109/`.

The P4 pairs four configured station MAC addresses with logical input routes.
`xbox_s3_route.c` authenticates S3 HELLO messages containing MAC, boot nonce,
sequence and one USB configured/not-suspended bit. The message has no physical
Xbox port field. HTTP diagnostics add XID-open status and USB completion counts,
not Xbox topology. An S3 losing power stops announcing; four seconds without
fresh beacons or diagnostics produces stale status, not proof of an empty port.

The direct DEVICE USB connection is one XID device. `s_port` in `usb_xbox.c`
is TinyUSB's local root controller index, not an Xbox controller socket number.
USB configuration and transfers supply no physical-port mapping in these drivers.
In S3 routing mode, `xid_pump` returns after forwarding the four network routes:
plain all-S3 mode does not initialize direct USB. Automatic mixed routing uses
`KVM_XBOX_DIRECT_PLAYER1` to initialize local USB alongside all four network
routes, with direct USB priority and a neutral standby P1 S3.

The all-S3 LCD labels denote logical routes; USB/WAIT/OLD/OFF indicators are
adapter telemetry, not physical socket occupancy. Direct mode shows P1 DIRECT.
The generic PHYSICAL XBOX PORT UNKNOWN footer describes the lack of automatic
discovery; it does not override the operator's fixed direct-P1 wiring contract.

## What is needed for automatic P1-P4 status

No observed software input can distinguish two adapters swapped between Xbox
sockets. Adding a port field to a beacon alone would not discover its value.
These are potential new sources, not commissioned capabilities:

- Xbox-side software can enumerate controller topology, identify each ESP device
  with a stable identity/challenge, then publish a complete port snapshot with
  freshness/boot generation. This requires Xbox-side code and an identity exchange
  in both ESP XID drivers; current descriptors/protocol do not provide that link.
  A diagnostic title could prototype this, but only supplies status while running.
  A persistent Stellar service would require investigation of available interfaces.
- Hardware at each socket can supply a unique port identity or independently
  sensed occupancy. Adapter firmware and additional signaling/wiring would need
  design and validation. A fixed identity stored in a movable adapter is not a
  socket identity.

A future resolver should expire complete snapshots to Unknown; accept Empty only
from fresh authoritative absence; reject conflicting occupancy and multiple
Device claims; and invalidate old assignments on disconnect or generation change.
Unknown must remain valid when USB is connected but topology is unavailable.

## Incremental bench check

Build the P4 and S3 firmware first. After flashing the appropriate board, open
its serial monitor and move one identified S3 from P1 to P2. The route number
should stay fixed, USB configured transitions should be logged, and logs should
say physical Xbox port unknown. Unplug an Xbox-powered S3 and check that its
route becomes OLD after telemetry expires; reconnect and check rediscovery.
These checks validate telemetry only, not automatic physical port detection.

Use the fixed wiring contract for direct P1 when implementing mixed routing.
Automatic P2-P4 occupancy still needs an authoritative source. Direct P1
menu navigation and Controller Tool launch were verified on 2026-11-04; mixed
routing and automatic topology detection remain unvalidated.

## Port-by-port discovery experiment

For the direct cable experiment, compile with KVM_USB_XBOX enabled and
KVM_XBOX_S3_PLAYER1 disabled: the S3-only image does not initialize DEVICE USB.
The direct XID driver logs the host-assigned USB address at interface open and
class request fields. An address is an enumeration identifier, not a proven
physical socket identity. Compare P1-P4, then repeat ports in a different order
and across console restarts before treating any correlation as useful. Do not
turn a single pass into a persistent physical-port assignment.
