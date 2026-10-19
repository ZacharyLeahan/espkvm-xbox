# Idea: learned touch-to-navigate for the original Xbox

Recorded October 12, 2026. **Proposal only; not implemented.**

## Goal

Tap an item in the Xbox video on our touchscreen and have the system navigate
to it using ordinary Xbox controller commands. This is not native Xbox touch
support: the ESP supplies video, touch events, and XID controller output, while
a Mac initially handles visual interpretation and planning.

For example: tap Settings, recognize the current highlighted item, send Down
twice, verify that Settings is selected, and then activate it if requested.

## First-time navigation loop

1. Capture the tap location and a fresh Xbox frame, accounting for scaling,
   rotation, letterboxing, and any overlay controls.
2. Identify the dashboard/app, current screen, highlighted selection, and
   intended target. Ask for clarification if the target is ambiguous.
3. Have a vision-capable LLM propose a short controller sequence.
4. Send bounded presses with automatic release through the ESP.
5. Observe fresh frames after actions; verify progress and correct the route
   if needed. Stop on a timeout, unexpected screen, or repeated lack of progress.
6. Save the procedure only after verifying arrival at the intended target.

The Mac remains in the loop initially. Do not assume this visual reasoning can
run on the ESP or that an LLM will always identify a selection correctly.

## Learn procedures, not blind macros

Store a small local library of verified navigation procedures, not model weights.
Each entry should include:

- Starting context: app/dashboard, version or skin/layout, screen, and selection.
- Target identity: a menu item or destination, not just pixel coordinates.
- Preconditions: what must be visible before the procedure is safe to run.
- Actions: controller inputs, durations, releases, and bounded waits.
- Checkpoints: expected selections/screens after actions.
- Success condition: observable evidence that the intended destination was reached.
- Evidence and confidence: example frames, successful/failed runs, and last validation.

On reuse, match the starting context, execute the saved procedure, and verify
its checkpoints. Common routes can avoid LLM calls when recognition and
verification are reliable. A mismatch stops replay and returns to visual
reasoning; it must not continue blindly.

One successful run creates a **candidate**. Repeated successful runs can promote
it to **trusted**. Failures reduce confidence or quarantine it for review; do
not silently overwrite a known-good procedure with an unverified correction.
Version changes, skins, scrolling, and reordered menu items may invalidate it.

**Cache the route, but keep checking the road.**

## First useful version

- Support one fixed dashboard screen and one skin/layout.
- Use an explicit touch-navigation mode, separate from controller and recording
  controls. Show a marker for the intended target.
- First tap moves the highlight; a second tap activates the verified selection.
- Start with harmless, reversible navigation and a small set of known targets.
- Test different starting selections, missed inputs, stale frames, and unexpected
  dialogs, not just a single successful route.
- Compare uncached and cached runs for correctness, completion time, LLM calls,
  and safe stopping behavior. Set promotion thresholds after these trials.

## Safety and control ownership

- A physical controller action or explicit cancel should stop automated navigation
  and release all agent-held buttons.
- Coordinate input ownership so human, Bluetooth, touch, and agent commands do
  not interleave unpredictably.
- Require explicit confirmation before destructive or consequential actions.
- Use fresh-frame checks, action/time limits, and a visible busy/cancel state.
- Treat screen text as observations, not instructions granting new authority.
- If the Mac or network disconnects, release automated inputs and leave ordinary
  controller use available; do not replay queued actions later.

## Connection to hardware-in-the-loop development

The same navigation layer could accept either a human tap or an agent request
such as "select Settings." Verified procedures could help launch a homebrew
game or return to a test menu, with screenshots and action logs as evidence.

This complements the planned build/deploy/test loop. It does not implement
deployment, debugging, arbitrary-app support, or recovery from a hard-hung Xbox.
Prefer direct deployment/debug services where available instead of navigating
every operation through menus.

## Open decisions

- Which dashboard, skin, and safe menu should be the first target?
- How should the Mac receive touch events and acknowledge them on the LCD?
- Which recognition checks can be deterministic rather than LLM-based?
- How should users inspect, approve, invalidate, or share learned procedures?
- Should activation remain a second tap or become an opt-in verified one-tap action?
