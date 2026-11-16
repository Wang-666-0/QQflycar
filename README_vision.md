# QQflycar vision module

The guide line is white tape on dark cloth. `project/code/vision.c` and
`vision.h` provide a pure C processor for a 188 x 120 MT9V03X grayscale frame.
No file under `libraries/` was changed.

## What is connected

- `examples/vision_main.c` is a complete alternative car entry point. It
  processes each completed camera frame, displays it on the TFT, and converts
  the signed pixel error to left/right motor duty. The repository's existing
  `project/user/src/main.c` is left exactly as it was.
- A 3 x 3 majority filter and Otsu threshold reject isolated bright and dark
  spots. The binary image is packed to 2.9 KiB to save MCU RAM.
- The row tracker follows the white tape, exposes a center point per row,
  estimates a signed steering error, and marks bends, right-angle turns,
  crosses, forks, and side-branch ring candidates.
- At a cross, the incoming center is retained for straight travel. A fork
  alternates left and right choices after its branches disappear for four
  frames. Ring entry chooses the side on which a sustained branch appears.
- The car example stops the motors on line loss or low confidence.

The algorithm is a starting point for tuning on the actual course. In
particular, the ring classifier uses a geometric hint and has **not** been
validated to complete a full hexagonal lap and exit in the correct place.
The fork counter assumes one recognizable fork per lap. Reset it with
`vision_init` at the starting line.

Tune the constants at the top of `project/code/vision.c` and the duty/gain
values at the top of `examples/vision_main.c`. Positive `error` means the
line is to the right of the camera center. The current motor signs assume
the left wheel speeds up for a right turn; swap signs if your wiring differs.

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

The new `project/code/vision.c` is picked up by the existing MounRiver
project's linked `code` source folder. The default firmware still starts in
its original `main.c`. To run the vision-controlled car, make a copy of the
MounRiver project, use `examples/vision_main.c` as its only `main.c`, then
build and flash. Keep the original project intact. Run the car with wheels
raised first to check motor sign and line-loss stop behavior.

## Checks performed

- The C smoke test covers straight lines, bends, right-angle turns, crosses,
  isolated noise, lost line, fork direction, and a ring-side hint.
- The algorithm processed all 109 BMPs shipped with the simulator reference
  repository without a lost-line result. A broad gray obstacle in group 6
  is treated as straight travel rather than a crossing.
- The simulator adapter and the alternative car entry point passed a C
  compilation check. The full Windows simulator and the CH32V307 firmware
  were not built here, so lap completion and MCU timing still need testing.
