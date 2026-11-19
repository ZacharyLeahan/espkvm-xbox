<!-- SPDX-License-Identifier: Apache-2.0 -->
# Repeatable Xbox scenarios

`tools/run_xbox_scenario.mjs` lets an agent execute a reviewed sequence without
thinking through every button press again. It uses Node 22+ and the existing
main KVM `/ws`, `/api/v1/video/status` and `/api/v1/video/frame.jpg` interfaces.
No firmware change, Modxo, Xbox serial connection or new API is required.

## Borrowed technique and attribution

Inspired by [kooscode/OXOS](https://github.com/kooscode/oxos), specifically
[`tools/modxo/bootrun.py`](https://github.com/kooscode/oxos/blob/bfec2af8a33528c835cc9dada484142ecaf58f21/tools/modxo/bootrun.py)
and its [Modxo rig workflow](https://github.com/kooscode/oxos/blob/bfec2af8a33528c835cc9dada484142ecaf58f21/tools/modxo/README.md).
OXOS supplied the repeat-run, bounded observation, result classification and
last-progress-evidence approach. Our implementation is new Apache-2.0 code;
no OXOS source code (GPL-2.0) was copied. The
[diagnostic title](../diagnostics/xbox/README.md) additionally credits OXOS's
image/controller-test techniques and adds visual progress/input assertions.
Its serial-based boot verdicts, power
switching and kernel netboot are not implemented by this tool.

## Review first, then execute

This example assumes Xbox Controller Tool's Control Test Page is already open.
It briefly presses and releases player 1's A button; it does not launch the tool.
Both screenshots are taken with inputs released, so they are baseline/release
evidence, not proof the Xbox received A. The existing player-1 sweep remains the
more detailed input correctness test.

From the firmware repository root:

```sh
# Default mode validates and prints the exact plan; no network or controller access.
node tools/run_xbox_scenario.mjs --scenario tools/scenarios/controller-tool-a.json --runs 2

# Takes controller ownership and performs the reviewed inputs on your device.
node tools/run_xbox_scenario.mjs --scenario tools/scenarios/controller-tool-a.json \
  --execute --url http://esp.local --output observations/controller-a-NEW \
  --build-id controller-tool-reference --kvm-build-id YOUR_KVM_COMMIT \
  --runs 2 --timeout-seconds 120
```

Use a new output directory. `--build-id` names the Xbox program, and
`--kvm-build-id` names the actual KVM firmware (use an explicit unknown/reference
label if necessary). Neither identifier is discovered or verified by the runner.
This targets the existing no-login local profile. Authenticated deployments need
separate client support; this tool does not bypass login or disabled agent APIs.
HTTPS uses Node's normal certificate verification. Close competing controllers;
execution explicitly takes control of the KVM session.

## Scenario format

Top-level fields are `name`, `start_state` and ordered `steps`. `start_state` is
an operator/agent precondition, not automatic screen recognition. Before a batch,
verify the Xbox is on that screen and each iteration ends where the next starts.
Blindly repeating menu navigation after an unexpected screen is not reliable.

Each step has a `name` and one or more of:

- `buttons`: simultaneous named buttons, `player` (1..4, default 1), `hold_ms`
  (50..2000). Supported: up/down/left/right, start/back, ls/rs clicks,
  a/b/x/y/black/white, lt/rt. Pressure buttons/triggers use full pressure.
  Reports refresh every 100 ms; release-all follows every hold.
- `wait_ms`: 0..120000, after releasing input.
- `expect`: exact video status checks for `signal`, `width` and/or `height`.
  Checks happen once after the wait; choose a settling time for the scenario.
- `diagnostic: true`: verify the dedicated XKD1 diagnostic title via screenshot
  panels before/after the step: advancing counters and coarse reference colors;
  input steps additionally require a new matching input/release acknowledgment
  on the selected player and unchanged counters on other ports. Needs Python 3
  and Pillow. See the [diagnostic setup](../diagnostics/xbox/README.md).
- `screenshot: true`: save a JPEG after input release/wait and record video
  status. The temporary capture consumer may affect LCD performance. A returned
  JPEG or matching resolution is not proof of fresh game behavior.

For your current reset workflow, write the verified reset chord as one `buttons`
step, then a settling wait and dashboard checkpoint, then the known launch
sequence. No reset chord or game-specific menu path is assumed by the supplied
example. Console software determines whether a chord resets anything. Generic scenarios
cannot establish dashboard/game identity from signal/resolution; the agent must
review the screen evidence. For unattended navigation, stronger game-specific
assertions are still needed. All players are released between holds; use the
existing four-player test for independent simultaneous held states.

## Results and recovery

`plan.json` preserves the exact sequence. `events.jsonl` journals run/step starts,
input release, selected video diagnostics (including frames/frameAgeMs when
available), screenshots, checks and errors, with UTC and monotonic elapsed times.
These are host observation times, not exact USB execution times. `summary.json`
contains each attempted run and its outcome:

| Outcome | Meaning |
| --- | --- |
| `PASS_DIAGNOSTIC` | Every step has XKD1 assertions and they passed. Covers only the diagnostic program, not game qualification. |
| `INCONCLUSIVE` | Sequence completed; video-field checks, if requested, matched. Application/screen behavior still needs review. |
| `FAILED_CHECK` | A requested video field did not match. Batch stopped. |
| `NEEDS_RECOVERY` | Connection/ownership, evidence request or deadline failed, or execution was interrupted. Inspect evidence before restarting. |

Exit 0 means diagnostic assertions passed; exit 2 means completed evidence
awaiting review; exit 1 means validation/error, failed check or interrupted
batch. There is deliberately no automatic game PASS.
If live checks prove something only about capture, the summary does not upgrade
that to a game-health claim. No attempt is made to detect freezes from a static
picture, zero encoded FPS or missing HDMI alone.

The session deadline covers the entire batch (default 120 seconds, maximum one
hour); each HTTP request also has a five-second timeout. First failure stops the
batch, with no automatic reconnect, takeover retry or replay. Ctrl+C requests
release and closes the connection. Release is best effort; lost transport relies
on the device's existing disconnect/stale-input release. Abrupt process kill or
host failure cannot guarantee delivery. Timings use host timers and Wi-Fi, not
frame-accurate scheduling.

There is **no physical power control, deployment or automatic hard-freeze
recovery**. A controller reset is the best available reset path today. A frozen
Xbox may retain HDMI signal and accept USB reports without progressing; even a
completed batch can therefore remain inconclusive. Stop and recover manually
when the displayed state is wrong. Future XBDM/serial/power integration is
separate work.

## Host validation

```sh
node tools/test_run_xbox_scenario.mjs
python3 tools/test_xbox_diagnostic.py
node tools/test_xbox_diagnostic_runner.mjs
```

A local mock KVM checks repetition/evidence, mismatched resolution, interrupted
control, lost ownership, deadlines, missing takeover acknowledgment, malformed
JPEG and release behavior. This establishes host-tool behavior only. No real
Xbox input or reset was performed as part of implementing this tool.
