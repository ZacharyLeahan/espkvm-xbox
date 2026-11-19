---
name: xbox-hardware-testing
description: Use Zach's ESP-KVM rig when developing, debugging, or planning real-hardware validation for original Xbox games, nxdk homebrew, XBE programs, or multiplayer controller behavior across repositories. Locate the shared tooling and choose deterministic scenarios, capture evidence, and interpret diagnostic results. Excludes Xbox 360/One/Series development.
---

# Original Xbox hardware testing

The shared test infrastructure lives in the ESP-KVM repository, separately from
the game being developed. Consult its maintained instructions before choosing
hardware-specific tests or claiming real-console results.

## Locate the shared context

Use `ESPKVM_REPO` if supplied. Otherwise resolve this installed skill directory's
symlink and use its `../..` directory, which contains `tools/` and `docs/`.
On Zach's current Mac the checkout is:
`/Users/zach/Developer/github.com/ZacharyLeahan/ip-kvm/espkvm`.
If the path is unavailable on another machine, locate the existing
`ZacharyLeahan/espkvm-xbox` checkout or ask for its location; do not substitute an
unrelated upstream clone or pretend the physical rig is reachable.

Read [the agent testing entrypoint](../../docs/AGENT-XBOX-TESTING.md) from that
checkout. Resolve relative references against the actual source skill directory,
not `~/.codex/skills`, `~/.agents/skills` or `~/.claude/skills` symlink locations.
Read the current game's instructions for its build, assets, deployment and reset
behavior. ESP-KVM firmware build instructions are not game build instructions.

## Use the existing tools

Prefer a named scenario and saved evidence over recreating individual commands
for each repetition. Dry-run the proposed scenario before executing it. Keep
start-state and end-state assumptions explicit, including whether reset/launch
steps are actually verified for this game. Host access to a skill is not evidence
that the hardware is connected or a diagnostic title is already running.

- For a game: use `tools/run_xbox_scenario.mjs` with a reviewed game-specific
  plan, screenshots/status and explicit Xbox/KVM build identifiers. The generic
  result remains `INCONCLUSIVE` until the game behavior is reviewed.
- For rig/input checks: use the standalone `diagnostics/xbox` title and the
  diagnostic plans only when that title is actually loaded. Its XKD1 panel is
  not present in an ordinary game. `PASS_DIAGNOSTIC` covers that title's bounded
  progress, reference-color and requested input/release checks, not the game.
- For exact analog or simultaneous multiplayer tests, follow the dedicated
  player-one sweep and four-player validation references in the entrypoint.

Use the user's current task authorization for live testing/deployment; consult
existing session permissions rather than requesting them again. This skill
itself neither grants hardware access nor requires a new approval ritual.
Current documented reset capability is controller-based only; stop for manual
recovery after a hard freeze. Incoming serial/debug BIOS/XBDM/power hardware is
not commissioned merely because it was planned. Never infer game liveness from
HDMI lock, a responding KVM, USB completion counts or a successful build alone.

Store private run evidence under the tooling repo's ignored `observations/`,
with new directories per attempt, and summarize actual results back in the game
repo when relevant. Preserve the game checkout's working state. Change shared
infrastructure only when required by the task, and retain OXOS attribution when
adapting the shared runner/diagnostic approach.
