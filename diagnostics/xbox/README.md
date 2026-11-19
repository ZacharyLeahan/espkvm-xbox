<!-- SPDX-License-Identifier: Apache-2.0 -->
# Xbox visual diagnostic title

A standalone nxdk XBE for the current ESP-KVM rig. It shows advancing time and
render-loop frame counters, available memory pages, and P1-P4 presence, held
buttons, last non-neutral button combination, press transitions and releases.
The program never rumbles, changes BIOS/settings, writes saves or initiates a
reset. It can be launched using the current dashboard; no debug BIOS, XBDM,
serial board, Modxo or new KVM firmware is needed.

A magenta-bordered black/white panel carries the same state with an `XKD1`
signature and CRC32. The host reads it from ordinary ESP-KVM JPEG screenshots.
Fixed red/green/blue/white/black/gray/yellow/cyan bars below it provide a coarse
color and positioning reference. This avoids OCR and gives the script concrete
application-side evidence instead of inferring game health from HDMI lock.

## Build

Use an existing [nxdk](https://github.com/XboxDev/nxdk) toolchain with SDL2 and
pbkit built, activate its compiler wrappers, then:

```sh
# Run from the firmware repository root, with NXDK_DIR set to your SDK.
export PATH="$NXDK_DIR/bin:$PATH"
make -C diagnostics/xbox
```

The output is `diagnostics/xbox/bin/default.xbe`. Install that XBE in its own
folder using your usual dashboard FTP workflow, then launch it. There are no
additional runtime assets. This task builds the executable but does not deploy
it or send inputs to the physical console. Keep the known reset/return-to-
dashboard method available; no automatic power recovery is added.

## Run deterministic checks

The host needs Node 22+, Python 3 and Pillow (`python3 -m pip install Pillow`
in your chosen Python environment). The same `python3` must be on the runner's
PATH. Dry-run validation works without Pillow.

After launching the diagnostic title and verifying the visible P1-P4 identities,
run from the firmware repository root:

```sh
node tools/run_xbox_scenario.mjs \
  --scenario tools/scenarios/diagnostic-player1.json

node tools/run_xbox_scenario.mjs \
  --scenario tools/scenarios/diagnostic-player1.json \
  --execute --url http://esp.local --output observations/diagnostic-p1-NEW \
  --build-id YOUR_DIAGNOSTIC_BUILD --kvm-build-id YOUR_RUNNING_KVM_BUILD \
  --runs 2 --timeout-seconds 120
```

`diagnostic-four-players.json` tests each player's A/B/Up/LT/RT separately with
isolation checks against the other three ports. Allow 300 seconds for repeated
four-player runs on slower links. It verifies sequential port independence;
simultaneous four-player states are covered by the existing routing/gameplay tests.
The plans finish neutral and stay in this title, so they can be repeated without
resetting or navigating menus. They exercise a representative subset; edit a
reviewed plan to add other supported buttons or chords.

Each `diagnostic: true` step:

1. Captures two readable panels and requires advancing frame/time counters,
   reference colors and neutral controls before sending input. A button step
   also requires the requested physical player to be present.
2. Sends/refreshes the bounded input, releases all players, then reads panels
   again. All captured diagnostic frames are saved with the journal.
3. Requires advancing counters and matching color references. For input steps,
   the target's last mask must match the requested chord, its press and release
   counts must each increase by exactly one, and it must be neutral afterward.
   Other players must retain their presence and unchanged input counters.

The runner polls up to five seconds per observation phase within the overall
session deadline. CRC failures from torn/compressed frames are retried and
journaled; absent/incorrect/ambiguous panels never become a pass. It stops on
failed checks, transport loss or loss of controller ownership, with no reconnect
or automatic reset. Exit 0 / `PASS_DIAGNOSTIC` is possible only when every step
has these diagnostic assertions; generic game scenarios still need review.

## What the checks mean

- Frame count counts render-loop iterations synchronized with vertical blank,
  not a calibrated game frame-time or capture-latency measurement.
- Input counters describe application-observed transitions. SDL digitalizes
  pressure-sensitive face buttons; LT/RT are treated as pressed above roughly
  three-quarter pressure. The runner sends full pressure. Exact analog values,
  sticks, controller polling frequency, rumble and physical latency remain
  covered by other tests or future work.
- Player numbers use nxdk SDL's physical-port player index (1..4); manually
  confirm the display mapping during first hardware commissioning. Missing or
  ambiguous indices are not silently assigned by enumeration order.
- Signature/CRC validate a readable snapshot, not cryptographic freshness.
  Two advancing panels plus new input/release counters provide bounded progress
  evidence. Counter wrap/restart stops for review. No assertion establishes that
  a whole game is correct, and a failed heartbeat cannot alone distinguish a
  frozen Xbox from a frozen capture path.
- The decoder locates the magenta border to tolerate screenshot scaling and
  HDMI padding. It expects an uncropped, upright image, sufficient cell contrast
  and color fidelity. Unknown transforms/overscan, missing reference bars or
  heavy compression fail rather than guessing. RGB tolerance is a coarse smoke
  check, not color calibration or complete geometry/aspect-ratio certification.
- The title is fixed at 640x480, with progressive/interlaced output chosen by
  nxdk and console settings. It does not yet exercise resolution switching.

## Credit and validation

Inspired by [kooscode/OXOS](https://github.com/kooscode/oxos)'s
[image-test title](https://github.com/kooscode/oxos/tree/bfec2af8a33528c835cc9dada484142ecaf58f21/tests/image-test-title)
(visible live counters and a known image) and
[controller test](https://github.com/kooscode/oxos/blob/bfec2af8a33528c835cc9dada484142ecaf58f21/tests/controller_test.py)
(check what the application received). This is a new Apache-2.0 implementation;
no GPL-2.0 OXOS source or picture was copied. Its emulator memory-reading approach
is replaced here by a CRC-protected visual panel that works with current capture
hardware.

Validation so far: local nxdk build at SDK commit
`14d5ee97e73347c973f1f57b68b79ec08c9e77f2` (the installed linker retains its existing
`.edata` merge warning), host C serialization/transition checks, scaled/padded
JPEG decoding and corrupted/wrong-screen/color rejection, plus mock-KVM runner
checks for success, wrong-player delivery, stuck input and frozen/wrong screens.
**Physical-console validation is pending.** Synthetic fixtures establish tool
behavior, not real HDMI/SDL controller behavior on the Xbox.
