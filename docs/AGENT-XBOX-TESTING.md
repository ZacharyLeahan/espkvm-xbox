<!-- SPDX-License-Identifier: Apache-2.0 -->
# Entry point for agents developing Xbox games

This repository supplies the ESP-KVM hardware and tools used to test original
Xbox games in other repositories. Keep the game's build/deploy instructions
authoritative; use this document for rig assumptions and the physical test loop.

## Fixed controller wiring and current test loop

Zach's standing wiring rule (2026-11-04): **main ESP DEVICE USB always connects
to physical Xbox player 1**, never P2-P4. When connected, assume P1 without
asking which socket it occupies. Optional Wi-Fi S3 receivers are intended for
P2, P3 and P4 alongside direct P1. This is an operator-maintained contract,
not automatic USB topology discovery.

Check the firmware mode before multiplayer tests. Direct mode uses
`CONFIG_KVM_USB_XBOX=y` with `CONFIG_KVM_XBOX_S3_PLAYER1` disabled and drives P1
only. The existing all-S3 mode uses four Wi-Fi receivers and does not initialize
DEVICE USB. **Mixed direct P1 plus Wi-Fi P2-P4 is intended but not implemented
or validated.** Do not silently switch to all-S3 or reassign direct P1 to fit
the old routing architecture.

USB configuration establishes enumeration; advancing host completion counts
establish report transport, not game response. On 2026-11-04, direct P1 inputs
visibly navigated XBMC through Programs -> Apps -> Controller Test and launched
Gamepad.xbe (Control Test Page). This verifies navigation and launch, not an
exhaustive button/analog sweep. Local private evidence is retained under
`observations/direct-usb-20261104/`; it is not part of the public repository.

## Capture freshness

Keep a video stream active while reviewing inputs. With MJPEG and no streaming
viewer, `/api/v1/video/frame.jpg` can return an old published JPEG even while raw
capture frame counters advance. Open `/stream`, wait for new frames, then save
stills. Raw frame age alone does not establish JPEG freshness. Earlier unchanged
stills on 2026-11-04 were not evidence of a frozen console; a fresh stream showed
XBMC and successful direct input navigation.

## Work from a game repository

1. Locate this shared checkout, using `ESPKVM_REPO` when supplied. Read
   [hardware and validated limits](XBOX-RIG-DETAILS.md) and the game's own build,
   deployment and reset instructions. Confirm the running KVM firmware mode.
2. Build and deploy with the game's tools. Confirm the actual title/start screen
   from fresh video before sending input. Launching Controller Tool replaces
   the current application; do so when appropriate to the task.
3. Review/dry-run any scenario before live execution. Use bounded inputs with
   explicit releases and recorded start/end assumptions. Existing
   [input setup and mappings](XBOX-REMOTE-CONTROLLER.md),
   [observation guidance](XBOX-TESTING.md), and
   [S3 tools](../s3-dongle/README.md) describe the available interfaces.
4. Save fresh screenshots/status and Xbox/KVM build identifiers in a new ignored
   `observations/` directory for each attempt. Review actual game behavior;
   USB completions, HDMI lock and successful builds alone do not prove liveness
   or game correctness. Preserve the game repository's working state.
5. Recovery is controller-based only. After a failed bounded reset attempt and
   review of genuinely fresh video, stop for manual recovery from a hard freeze.
   Planned serial, XBDM and physical power hardware are not commissioned merely
   because they are documented.

For exact analog and multiplayer evidence, consult the existing
[player-one sweep](S3-PLAYER1-VALIDATION.md) and
[four-player validation](S3-FOUR-PLAYER-VALIDATION.md). Those historical all-S3
results do not validate mixed routing. Audio and recording have separate limits
in [HDMI-AUDIO.md](HDMI-AUDIO.md) and [SD-RECORDING.md](SD-RECORDING.md).
