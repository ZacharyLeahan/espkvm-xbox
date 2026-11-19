<!-- SPDX-License-Identifier: Apache-2.0 -->
# Entry point for agents developing Xbox games

This repository supplies shared hardware-test tools for other original Xbox
game repos. Keep the game's own build/deploy instructions authoritative for its
code, and use this entrypoint for the physical test rig. The published tooling
repo is [ZacharyLeahan/espkvm-xbox](https://github.com/ZacharyLeahan/espkvm-xbox).

## Identify the deployed mixed-USB firmware

The KVM web root returns an ETag with the firmware version and the first eight
hex digits of the ELF SHA-256. The deployed build reporting
`v.0.56.1-180-g4d170a7-dirty-f2b88a83` corresponds to the
[archived source snapshot](https://github.com/ZacharyLeahan/espkvm-xbox/commit/e34f216b51b49b122ec73ac4b3a07300ea0f06ce).
The version names the checkout used at build time. Subsequent commits retain a
larger LCD detail buffer. A rebuild from a clean commit has its own build ID.
The deployed profile enables mixed USB/S3 routing, the DSI status strip and
`CONFIG_KVM_DSI_PREVIEW_FORCE_FALLBACK=y`. Supply private Wi-Fi and controller
pairing settings locally.

## Fixed controller wiring and current test loop

Zach's standing wiring rule (2026-11-04): **the main ESP DEVICE USB connection
is always physical Xbox player 1**. When that cable is connected, use P1 without
asking which socket it occupies. It will never be moved to P2-P4. Optional Wi-Fi
S3 receivers retain their original P1-P4 labels, with direct USB as an alternative for P1. This is an
operator-maintained wiring contract; USB enumeration is not automatic socket
discovery. USB ON establishes configuration, and advancing Xbox completion
counts establish report delivery; neither alone proves game behavior.

Check the running firmware before multiplayer tests. Mixed mode enables
`KVM_USB_XBOX`, `KVM_XBOX_S3_PLAYER1` and `KVM_XBOX_DIRECT_PLAYER1`: direct P1
takes priority when USB-configured, and otherwise falls back to the original
P1 S3 automatically. Keep all four S3 identities paired; never reassign P1's
adapter to P2. The standby P1 S3 receives neutral reports while direct P1 is
active, preventing duplicated input. Plain direct and all-S3 modes remain supported. The LCD keeps all four link/USB status tiles and ESP LAN IPs visible;
per-controller IPs, report activity and counters open only after a tile tap;
unseen/stale telemetry is not a physical empty-port determination. On 2026-11-05 direct P1 and Wi-Fi P2-P4 simultaneously reported USB configured
and advancing Xbox completion counters. That verifies mixed transport, not a
complete game test; do not substitute earlier all-S3 gameplay results.

2026-11-09 update: P1/P2 adapters have firmware 0.2.2 installed; P3/P4 remain
on 0.2.1. All four Wi-Fi routes passed a 60-second transport sample with zero
new send errors and advancing Xbox completions. Bounded logical-P1 Wi-Fi and
physical direct-P1 XBMC navigation were visually reviewed. The physical P1
cable swap exposed a historical identity mismatch: current physical P1 uses
S3 d4:05:92:44:aa:94 (`.243`), and P2 uses ac:27:6e:b0:ce:a4 (`.242`). The
private pairing profile and running main firmware now use this observed wiring;
do not restore the old inventory mapping. Corrected mixed mode passed with
DIRECT P1 and Wi-Fi P2-P4 all advancing, with zero new errors in a 14.5-second
sample. Each P2-P4 also passed bounded XBMC navigation with 200 ms presses and
2 s capture settling. The operator also confirmed the return from direct P1 to Wi-Fi. See the [handoff verification](CONTROLLER-HANDOFF.md#hardware-verification-2026-11-09). These
are rig checks; Super Mario War still requires its own game qualification. Evidence: `observations/controller-ready-20261109/`.

Use a fresh video stream while reviewing input. The MJPEG still endpoint can
return an old published JPEG with no streaming viewer even while capture status
reports fresh raw frames. Open `/stream`, wait for new frames, then collect
`/api/v1/video/frame.jpg` evidence; status counters alone do not establish JPEG
freshness. Earlier unchanged stills on 2026-11-04 did not establish a console
freeze. With a stream active, direct P1 successfully navigated XBMC and launched
`Programs -> Apps -> Controller Test -> Gamepad.xbe` (Control Test Page).
That is a verified menu/control path, not a complete button/analog sweep.

For each game loop: build/deploy using that game's instructions, confirm the
running title and fresh screen, dry-run a bounded scenario, execute with explicit
releases, then review actual game behavior and save evidence. Current recovery
is controller-based only; manual recovery is required if a confirmed live view
and a bounded reset attempt establish that the console is unresponsive.

## Read only the references needed for the task

| Task | Reference |
| --- | --- |
| Choose controller/capture tests and understand the current rig | [Hardware and validated limits](XBOX-RIG-DETAILS.md) |
| Run repeatable controller sequences with saved evidence | [Scenario runner and outcomes](XBOX-SCENARIOS.md) |
| Check rig progress, colors and input delivery without OCR | [Standalone Xbox diagnostic title](../diagnostics/xbox/README.md) |
| Check exact player-one analog/button values | [164-state sweep](S3-PLAYER1-VALIDATION.md) |
| Check multiplayer routes or simultaneous inputs | [Four-player validation](S3-FOUR-PLAYER-VALIDATION.md) and [dongle tools](../s3-dongle/README.md) |
| Collect video/audio or recording evidence | [Observation](XBOX-TESTING.md), [audio](HDMI-AUDIO.md), [recording](SD-RECORDING.md) |

The scenario runner, visual diagnostic title and panel decoder have local build
and host/mock test results. Their physical-console validation is pending. Do not
confuse those new tools with the earlier, separately verified controller sweeps
and gameplay reports. Update this status only from actual hardware evidence.

## Work from a game repository

1. Locate the shared checkout using `ESPKVM_REPO` or the installed skill's real
   path. Run tooling commands with that directory as the working directory;
   continue building the game in its own checkout. Obtain the current device
   origin and connection state from established project/session context.
2. Pick a test appropriate to the change. Diagnostics can isolate rig/input
   problems, but a game change still needs the actual game tested. Launching the
   diagnostic XBE replaces the current game session; do it when appropriate to
   the user's task, not automatically on every build.
3. Validate/print the scenario, then execute it on the confirmed start screen
   within the task's existing authorization. Save new evidence and distinguish
   the Xbox program build from the running KVM firmware build.
4. Report what actually passed, what remains inconclusive and which failures
   require a person. Generic game scenarios never become a game PASS solely
   because video fields matched. `PASS_DIAGNOSTIC` qualifies only the requested
   checks of the standalone diagnostic program.

For example, from the tooling checkout:

```sh
node tools/run_xbox_scenario.mjs --scenario tools/scenarios/diagnostic-player1.json
```

That default dry run makes no device requests. Read the selected tool's guide
for live execution; do not apply the diagnostic plan to a game or dashboard.
Your current rig cannot perform physical power cycling. A verified controller
reset can be part of a game-specific plan, but cannot recover a hard freeze.
Do not assume future serial, XBDM or power-control upgrades are operational.

## Cross-repository discovery

The maintained skill is
[`agent-skills/xbox-hardware-testing/SKILL.md`](../agent-skills/xbox-hardware-testing/SKILL.md).
On Zach's current Mac it is linked into the user skill directories for Codex and
Claude Code. Short Xbox-only routing instructions in `~/.codex/AGENTS.md` and
`~/.claude/CLAUDE.md` point at it, so the game repo need not duplicate this guide.
New sessions should load those global instructions; an already-running agent
can be directed to read this entrypoint explicitly.

These local links/instructions do not travel with a Git clone or configure a
Windows laptop automatically. On another machine, install the skill from this
checkout into that agent's user skill directory and add an Xbox-specific pointer
to its global instruction file using the new local path. `ESPKVM_REPO` can
identify a relocated tooling checkout after the skill/instructions are installed.
See the official [Codex global instructions](https://learn.chatgpt.com/docs/agent-configuration/agents-md)
and [Claude Code user instructions](https://code.claude.com/docs/en/memory).
