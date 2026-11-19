<!-- SPDX-License-Identifier: Apache-2.0 -->
# Xbox P1 transport handoff

Enable `KVM_USB_XBOX`, `KVM_XBOX_S3_PLAYER1` and `KVM_XBOX_DIRECT_PLAYER1`.
Main DEVICE USB always belongs to physical Xbox P1. When that USB output is
configured it takes P1; when removed, P1 automatically returns to its paired
Wi-Fi S3. P2-P4 keep independent queues and expiry timers. Keep all four S3
identities configured, including the standby P1 adapter. A fresh but still
unconfigured direct cable cannot hide a working Wi-Fi P1.

Logical P1 input is owned by the input worker and copied under a lock for USB.
USB reset/disconnect cannot drain the Wi-Fi queue. While direct P1 is active,
the standby S3 receives neutral reports; an absent standby is not continuously
retried. Signed discovery remains unbound, while UDP output and HTTP diagnostics
follow the preferred live interface so same-subnet Wi-Fi cannot bypass the
Ethernet preference. The LCD and optional WebSocket `transport` field describe
the selected output; legacy clients can ignore the additive field.

Main and S3 firmware 0.2.2 also retry unconfigured, unsuspended USB enumeration:
wait ten seconds, detach for one second, reconnect, then back off to 20/40/60
seconds. Configured/suspended hosts are left alone. Existing S3 0.2.1 has no
network updater or USB-reset endpoint; install 0.2.2 through USB. Provisioning,
pairing keys and player assignments are preserved. Never publish private
sdkconfigs, credential-bearing firmware images or flash backups.

## Current physical map

A controlled operator-confirmed swap on 2026-11-09 identified the S3 removed
from physical P1. The historical inventory map did not match current placement.
The private profile now uses:

| Xbox socket | Paired S3 MAC | Observed LAN address |
| --- | --- | --- |
| P1 | d4:05:92:44:aa:94 | 192.168.1.243 |
| P2 | ac:27:6e:b0:ce:a4 | 192.168.1.242 |
| P3 | a0:f2:62:eb:f7:60 | 192.168.1.244 |
| P4 | ac:27:6e:b0:d5:d0 | 192.168.1.246 |

DHCP addresses are observations, not identities. A MAC identifies an adapter,
not its physical socket. Do not restore the old inventory map or infer socket
numbers from USB addresses. Arbitrarily moving adapters needs an updated
operator-maintained map. P1/P2 have verified 0.2.2 flashes; P3/P4 remain 0.2.1.

## Hardware verification, 2026-11-09

- All four S3 routes: a 60-second passive sample had 2,067 new sends per route,
  zero new send errors and approximately 6,000 Xbox USB completions per player.
- The operator moved only physical P1 from its S3 to DEVICE USB. P1 selected
  DIRECT automatically. After correcting the physical map, a 14.5-second mixed
  sample advanced P1/P2/P3/P4 USB completions by 1,445/1,401/1,575/1,574, with
  zero new send errors and fresh status.
- Direct P1 and each Wi-Fi P2-P4 navigated XBMC Games/Movies/Games with explicit
  input release and reviewed fresh screenshots. P2-P4 used 200 ms holds and
  2 s settling; shorter inconsistent captures were not counted as passes.
- The operator confirmed returning the cable to the P1 S3 restored WIFI / USB ON.
  A follow-up sample confirmed Wi-Fi P1 at `.243`, P2 at `.242`, and all four
  completion counts advancing. It recorded one transient send failure per route
  followed by continued successful sends; this is not a zero-loss claim.

Activate `/stream` before reviewing JPEG evidence: idle stills can be cached.
The generic scenario runner's automatic INCONCLUSIVE result is retained;
bounded dashboard navigation was reviewed separately. These checks establish
rig transport and menu input, not game qualification, latency, exact analog
values or long-term wireless reliability. Local evidence and private firmware
hashes remain ignored under `observations/controller-ready-20261109/`.

The live working-tree build also contained pre-existing LCD experiments and an
ESP-Hosted dependency update. Those are excluded from this focused repair;
clean-review-checkout build and host checks are recorded separately in the PR.
