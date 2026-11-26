# QQflycar vision module

中文的赛道适配、算法推导与调参顺序见 [VISION_RACE_GUIDE.md](VISION_RACE_GUIDE.md)。

The original guide line is white tape on dark cloth; a dark-line mode is now
available through `vision_set_line_polarity`. `project/code/vision.c` and
`vision.h` provide a pure C processor for a 188 x 120 MT9V03X grayscale frame.
No file under `libraries/` was changed.

## What is connected

- `project/code/vision_main.c` is the car entry point selected by the
  MounRiver source configuration. It processes each completed camera frame,
  displays it on the TFT, and converts the signed pixel error to left/right
  speed targets for the existing PI controller. The original `project/user/src/main.c` is kept unchanged and
  excluded from this build. `examples/vision_main.c` remains a reference copy.
- A 3 x 3 majority filter and Otsu threshold reject isolated bright and dark
  spots. The binary image is packed to 2.9 KiB to save MCU RAM.
- The row tracker follows the white tape, exposes a center point per row,
  estimates a signed steering error, and marks bends, right-angle turns,
  crosses, forks, and side-branch ring candidates.
- Three image bands (near, middle and far) produce a pixel-space heading and
  curvature proxy. A three-point interpolation selects a preview target. The
  look-ahead shortens slightly on low-confidence frames. Curvature alone no
  longer shortens it: a filmed chamfer-like bend previously reduced preview
  from 60 to 24 and canceled the steering target. This is not a physical
  curvature radius or a speed-dependent preview controller.
- If tracking from the previous frame finds too few rows, the tracker retries
  once from the image centre. This recovers sudden shifts in the reference
  image sequences without weakening normal tracking.
- A right-angle turn needs one dominant horizontal arm. A cross needs two
  substantial arms and a continuing line above the junction. This prevents a
  small camera offset at a cross from causing a false hard turn. The confidence
  denominator covers the lower three quarters of the image, where tracking is used.
- The current car entry point reads a physically inverted camera as a 180
  degree rotated frame, without adding a second full-size image buffer.
  Thresholding and tracking use the full frame. It selects left at the first
  fork and right at the second.
- Ring handling has entry, circulation, exit-ready, and cooldown states. For
  the mapped left-entry/right-exit route, it arms the opposite-side exit after
  recognizing the left entry a second time, with eight image bends as fallback.
  Each hexagon corner changes travel direction by about 60 degrees; no
  greater-than-180-degree steering maneuver is assumed.
- The car example stops on line loss or low confidence, except for up to 12
  low-speed camera frames immediately after a confirmed right-angle turn.
  It then stops if the line is still absent. Bend speed and steering response
  are tuned separately from straight-line damping.

The algorithm is a starting point for tuning on the actual course. In
particular, ring exit and fork events have **not** been validated on camera
frames from the physical car. The current entry point has no cumulative
encoder distance input for vision, so ring exit currently uses the less
reliable corner-count estimate. A ring
timeout stops the car after 500 active frames if it cannot find an exit.
The fork counter assumes one recognizable fork per lap. Reset it with
`vision_init` at the starting line.

Tune `FLYCAR_PREVIEW_PERCENT` (15-90, initial 60) and the speed/gain values at
the top of `project/code/vision_main.c`, plus the thresholds at the top of
`project/code/vision.c`. `vision_set_preview` also allows runtime tuning.
Positive `error` means the
line is to the right of the camera center. The current motor signs assume
the left wheel speeds up for a right turn; swap signs if your wiring differs.
The existing motor controller uses encoder feedback for wheel speed. Vision's
optional `vision_update_odometry` input is not connected in this build; ring
distance calibration would require the team's cumulative encoder data. The
TFT displays ring phase (`P`), current side-line hint (`H`), short corner
recovery frame count (`T`), completed fork count (`N`), left/right wheel
speeds, pixel error, feature, and valid-row percentage.

## Computer simulator

Use the Liumian simulator linked in the assessment sheet. Its `code/camera.c`
is an example, so keep a backup and replace only that file with
`simulator/camera.c`. Copy `project/code/vision.c` and `vision.h` into its
`code/` directory. The adapter shows the binary image and the left/right/
center tracks, and logs error, confidence, threshold, feature and fork count.
The simulator's build scripts collect new C files. In Dev-C++, run
`tools/rebuild.bat` once after adding the two files, then F11; Visual Studio
and command-line builds rescan automatically. Select the provided test image
groups in the simulator and press R for its repeated-call timing check.

The MounRiver `.project` links `code` to `project/code`, and `.cproject` lists
that folder as a source path while excluding `user_c/main.c`. The selected
sources are `code/vision.c` and `code/vision_main.c`; the original main is
preserved for reference. After opening the project in MounRiver Studio,
refresh it and run Clean/Rebuild so the IDE regenerates the makefiles. The
checked-in `project/mrs/obj` makefiles are old outputs with absolute paths
from a different computer; do not build them directly. Check the new build
log for `vision.c` and `vision_main.c`, and verify `user/src/main.c` is absent.
Run the car with wheels raised first to check motor sign and line-loss stop
behavior; then tune at low speed. A successful build does not establish that
the car can complete the course.

## Checks performed

- The C smoke test covers straight lines, bends, right-angle turns, crosses,
  isolated noise, lost line, fork direction, a ring-side hint, adjustable
  look-ahead, a curved path and sudden line reacquisition.
- The algorithm processed all 109 BMPs shipped with the simulator reference
  repository without a lost-line result after adding the centre retry. The
  previous version lost six frames. A broad gray obstacle in group 6 is
  treated as straight travel rather than a crossing. Image labels and steering
  directions have not been verified for every frame.
- The active car entry point and `vision.c` passed host C compiler checks.
  The full MounRiver CH32V307 firmware has not been built here; lap completion
  and MCU timing still need testing.
