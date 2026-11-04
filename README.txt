ISMAIL: THE FIVE FORBIDDEN TEMPLES — 3D PSP PROTOTYPE
Version 0.2 — PSP homebrew source project

WHAT'S INCLUDED
---------------
This is a first 3D-rendered PSP prototype source project. It uses PSP GU/GUM
hardware rendering and simple low-poly placeholder geometry:
- Third-person camera following Ismail
- 3D stone temple room with pillars and doorway
- Stylized warrior placeholder model and sword
- Temple guardian enemy
- Movement, jumping, quick attack, heavy attack, special attack
- Circle grab finisher and health chest
- Health values and basic enemy damage logic

This is an early prototype, not a finished commercial-quality 3D adventure.
The character and enemy are assembled from simple 3D shapes; custom models,
textures, detailed animations, sound, menus, save data, cinematic sequences,
and all five levels are future development work.

CONTROLS
--------
Analog stick / D-pad : Move
X                    : Jump
Triangle             : Quick attack
Square               : Heavy attack
Circle               : Grab / finisher or open health chest
L                    : Block (hold)
R                    : Special attack
START                : Exit

BUILD REQUIREMENTS
------------------
A computer with PSPSDK installed and configured is required. This source has
not been compiled or tested in this environment.

1. Install PSPSDK for your operating system.
2. Open a terminal in this folder.
3. Run: make
4. The build should produce EBOOT.PBP if compilation succeeds.

INSTALL IN PPSSPP
-----------------
Create:
  PSP/GAME/ISMAIL/

Copy the compiled EBOOT.PBP into that folder. Open PPSSPP, browse to Games,
and launch the ISMAIL homebrew entry.

ANDROID / IPHONE
----------------
PPSSPP runs PSP homebrew on supported platforms, but installation procedures
vary by device and operating-system version. This project creates PSP-format
homebrew, not an Android APK or native iOS app.

STORY ROADMAP
-------------
Chapter 1: Temple of Sharada
Chapter 2: Temple of Gidan Mariya
Chapter 3: Temple of Gidan Shanu
Chapter 4: Temple of Dorayi
Chapter 5: Temple of Ramin Jigawa
Final boss: Babbaka

This is an original fan-made project inspired by the broad genre of 3D action
adventure games. It is not affiliated with or endorsed by God of War's owners
and contains no God of War assets.

ANDROID-ONLY BUILD OPTION (CLOUD BUILD)
---------------------------------------
You can use GitHub Actions from your Android browser without installing PSPSDK
on the phone:
1. Create a GitHub account and a NEW PUBLIC repository named Ismail-Five-Temples.
2. Upload the contents of this project folder to the repository's main branch.
   Include the hidden .github/workflows/build.yml file and keep its folders.
3. Open the repository's Actions tab.
4. Select "Build Ismail PSP game", then choose "Run workflow".
5. When the run finishes successfully, open the run and download the artifact
   named Ismail-PSP-EBOOT. Extract EBOOT.PBP from the downloaded artifact.
6. Copy EBOOT.PBP to PSP/GAME/ISMAIL/ on your phone and launch it in PPSSPP.

This cloud workflow uses the PSPDEV project's published PSPSDK container image.
If the workflow fails, open the failed run and share the error text so the
build can be corrected. This is not guaranteed until a successful build occurs.
