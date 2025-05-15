----------------------------------------
|            X-Wing Revival            |
|       0.5.1 Alpha (2025-05-15)       |
|     by Raptor007 (Blair Sherman)     |
| http://raptor007.com/x-wing-revival/ |
----------------------------------------


Welcome to X-Wing Revival!  I started making this because I wanted to have an X-Wing LAN
party in 2011, and found that X-Wing Alliance (released in 2002) didn't run correctly on
modern PCs.  We've come a long way since then!  The goal is still to recreate the feeling
of a classic LucasArts X-Wing game, but X-Wing Revival has developed into its own unique
take on the genre, with a focus on fast dogfighting, epic battles, multiplayer balance,
and staying true to classic Star Wars ship designs.  (TIE Fighters don't have missiles!)

Basically: a 90's combat flight sim with early 2000's graphics and modern internet play.

If you're looking for the official X-Wing title of 2020, check out Star Wars Squadrons:
  https://store.steampowered.com/app/1222730/STAR_WARS_Squadrons/


== CREDITS AND THANKS ==

Ship models were shamelessly pilfered from X-Wing Alliance asset files, including some
beautiful high-res models from the X-Wing Alliance Upgrade Project.
  https://xwaupgrade.com/

The campaign uses voice clips from the Star Wars trilogy, Dark Forces, Indiana Jones,
Rainbow Six, Rogue Spear, Halo, and the CollegeHumor sketch "Stormtroopers' 9/11".

Special thanks to NoJoe, Peter, and Sawyer for all the test flights, and to The Rebel
Alliance for helping me bring balance to the forces.
  https://therebelalliance.zyp.mx/


== GAME TYPES ==

  Fleet Battle: Each team tries to destroy the enemy capital ships while defending
    their own.  The last team with any surviving capital ship wins.  Cruisers and
    battleships will respawn if their entire group is destroyed before the flagship,
    but it takes time to replace a large group of capital ships.

  Battle of Yavin: Rebels must fly down the trench and try to hit the exhaust port
    with a pair of proton torpedoes.  Imperials must try to prevent the Rebels from
    doing this before the time limit runs out.  Harder AI skill effects turrets too!

  Flagship Hunt: The attacking team must destroy the defending team's flagship before
    the time limit runs out.  (This mode was previously called "Capital Ship Hunt".)

  Team Deathmatch: The team to hit the kill limit first wins.  Respawn always enabled.

  Team Elimination: The last team with any ships remaining wins.  Players can control
    AI ships when the player's ship has been destroyed, but no new ships will be given
    to either team once the match has started.

  Team Kessel Run: Race through the asteroid field, gaining points for your team with
    each checkpoint reached.  First team at the score limit (or last team alive) wins.

  FFA Deathmatch: The player to hit the kill limit first wins.  Respawn always enabled.

  FFA Elimination: Last man standing wins, and there's no respawn whatsoever.

  FFA Kessel Run: Race through the checkpoints in the asteroid field.  First player to
    reach the score limit (or last player alive) wins.  Basically Carmageddon in space.

  Mission: Play a campaign level or custom scripted mission.


== TROUBLESHOOTING / FAQ ==

Textures are blank.  Everything looks like white boxes!

  Windows: This is a directory permissions problem.  This often happens on Windows 10/11
    if you attempt to unzip X-Wing Revival in your Downloads folder or on the desktop.

    The solution is to move the zip file to a different directory like "C:\Program Files"
    before extracting it.

  Mac: This is usually caused by the Quarantine file system attribute, which prevents a
    downloaded program from reading files, even if they were extracted from the same zip.

    The solution is to open Utilities/Terminal and remove the Quarantine flag from the
    directory you extracted X-Wing Revival to:

      xattr -dr com.apple.quarantine "/Applications/X-Wing Revival"

My OS warns about or won't allow a downloaded or unsigned application to run.

  Apple and Microsoft have made it disappointingly onerous to distribute software for
  their platforms without paying them for code-signing.  Fortunately for those of us
  who still value running whatever we want on our own computers, there are workarounds.

  Windows: In the SmartScreen popup, click "More Info" at bottom, then "Run Anyway".
    The permanent fix is in Windows Security, "App & Browser Control", "Reputation-based
    protection settings".  Turn off "Check apps and files".

  Mac: You might be able to just right-click to run an unsigned app.  (Unconfirmed.)
    The permanent fix to allow apps downloaded from anywhere is done in the Terminal:

      sudo spctl --global-disable

  I recommend the permanent fixes for power users only, or anyone who misses the 90's.

Should I run "X-Wing Revival.exe" or "X-Wing Revival Legacy.exe" on Windows?

  You should try "X-Wing Revival.exe" first.  This is the 64-bit version using SDL 2.0.
    If you are on a 32-bit system or having problems that did not occur in old versions
    of X-Wing Revival, you can try "X-Wing Revival Legacy.exe" which is 32-bit SDL 1.2.
    NOTE: SDL 1.2 and 2.0 often have different joystick axis mappings, so you may
    need to alter control binds or restore defaults when switching between them.
    NOTE: There is currently no microphone support in the Legacy exe.

Linux asks what application to open files of type "executable" with.

  Click Cancel, then right-click "X-Wing Revival.elf", select "Properties", then go to
    the "Permissions" tab, and check "Allow this file to run as a program".  Close this
    window.  Now double-clicking "X-Wing Revival.elf" should run it.

MacOS says the application "X-Wing Revival.app" cannot be opened.

  Zipping/unzipping sometimes clobbers file permissions.  Easy fix in Terminal:

    chmod +x "X-Wing Revival.app/Contents/MacOS/X-Wing Revival"


== VIDEO PREFERENCES ==

--Graphics--

  Fullscreen: Toggle fullscreen, and set the resolution for fullscreen mode.
    Windowed resolution can be changed by dragging the lower-right corner.

  FOV: Set horizontal field of view in degrees.  Default "auto" uses vFOV 60.

  MSAA (Antialiasing): Use multiple samples to reduce jagged edges.  GPU-heavy at high
    res, in VR, or with bump-mapping.  (Renamed from "FSAA" to avoid FXAA confusion.)

  VSync (Vertical Synchronization): Eliminate tearing by only drawing frames when the
    monitor is ready to refresh.  Recommended for fast systems, but turning this off is
    an easy way to improve framerate and reduce stuttering.

  Framebuffer Textures: Use framebuffer objects as live textures for the cockpit.
    This shouldn't hurt performance much, but some old video cards don't support it.
    Framebuffers are also used for the VR eyes, so this is required for VR.

  Texture Quality: Limit the maximum texture resolution; downsample anything too large.
    This may improve performance with very limited VRAM and is required on some ancient
    video cards, but if your PC was made in the last 20 years you can leave this High.

  AF (Anisotropic Filtering): Improve texture clarity and detail.  Slightly GPU-heavy.
    Trilinear mode is the standard non-anisotropic mode.  Linear mode uses bilinear
    filtering without mipmapping, which looks sharp but sparkles at longer distances.

  Shaders: Use GLSL vertex and fragment shaders instead of old fixed-pipeline OpenGL.
    Recommended for performance and better appearance, but a few old motherboards may
    perform better with this off, and some ancient video cards require this off.

  Lighting: Quality of shader-based lighting.  Lower quality levels perform more work
    per vertex rather than per pixel.  Highest quality levels use bump-mapping to add
    more detailed lighting to some surfaces, which can be GPU-heavy.

  Dynamic Lights: Number of point light sources (lasers, torpedos, explosions, etc) to
    use for lighting each object.  Higher values use a bit more CPU and GPU power.

  Blastpoints: Number of damage blastpoints to draw per object.  Higher values can be
    very GPU-heavy, especially at higher quality.  Requires shaders.

  Blastpoint Quality: Controls shader complexity used to draw blastpoints.  Low does
    most work per vertex.  Medium and High operate per pixel, which can be quite
    GPU-heavy when combined with high blastpoint count, anti-aliasing, and/or VR.

  Effects: Adjust the number of sprites per explosion.  Lowest removes shot impact
    effects entirely.  Most should leave this High, but lower settings may reduce
    the CPU load of dynamic lights and/or the GPU load of layered transparencies.

  Engine Glow: Draw engine glow effects.  Very little performance impact on most GPUs.

  Debris: Scatter tiny rocks throughout space to help visualize your speed.

  Sway: Tilt the 3D cockpit to simulate motion.

  Asteroid Level of Detail: Controls how far away asteroids render in higher detail.
    Can be somewhat CPU-heavy and GPU-heavy with many asteroids, especially in VR.
    NOTE: This is even more CPU-heavy when shaders are disabled!

  Death Star Trench/Surface: Draw 3D details over the flat surfaces of the Death Star.
    Can be somewhat GPU-heavy and CPU-heavy, especially in VR.
    NOTE: This is even more CPU-heavy when shaders are disabled!

--UI--

  Target Box: Color of the box around selected enemy targets.  Default: White

  Friendly: Color of the box around selected friendly targets.  Default: Green

  Thickness: Line width of target box.  Default: Medium

  Style: Classic Rect fits target into a box aligned to camera up/right vectors.
    Modern fits a close rect matching the taret's angles.  Squared is similar
    but makes a diamond shape with equal length sides.  Default: Modern

  Classic Target Info: Vague target health instead of hull and shield percent.

  Rotate Lobby Ship: Controls if the ship model spins or just faces forward.

  Show Framerate: Show graphics FPS (and physics if hosting) in bottom right.

  Cinematic Mode: Hide UI elements that look bad in recordings.

--Virtual Reality--

  VR Mode: Enable VR HMD output and seated head tracking, using OpenVR/SteamVR.
    Tested with the HTC Vive, and it should work for the Oculus Rift, Valve Index,
    or any other HMD.  Some headsets may require view tweaks below.  VERY GPU-HEAVY!

  Start in VR: Enable VR mode at launch.  Equivalent to "-vr" command-line parameter.

  Sway: Enable cockpit motion in VR.  More immersive, but can be nauseating.

  FOVW / FOVH: Field of view in degrees; negative means vertical FOV.  Default: -111

  Eye Separation: Spacing in meters between the two eyes.  Default: 0.0625

  Center Offset: Offset in pixels from screen center to eye center.  Default: 87

--Networking--

  Rate: Send updates to the server at this rate.  Default: 30

  Predict: Shots fire immediately instead of waiting for server.  Default: All My Shots


== AUDIO PREFERENCES ==

--Volumes--

  Master: Master volume that affects all playback.

  Effects: Volume of sound effects.

  Engines: Engine sound volume, relative to effect volume.

  Comlink: Player voice volume.

  Music: Volume of all music, including end-of-round victory/defeat music.

  Music in Menus: Play background music in the menus and lobby.

  Music During Flight: Play background music while flying.

  Shield Alarm: Play alarm klaxon when your shields drop.  Default: All Ships

--Mixing Quality--

  Output Channels: Stereo, quadraphonic, or 5.1/7.1 surround.  Default: Stereo

  Sample Depth: 16-bit/24-bit integer or 32-bit floating point.  Default: 16-Bit

  Rate: Samples per second.  Default: 44.1KHz

--Commlink--

  Microphone Input Gain: Recording volume of your microphone.  Default: Auto

  Maximum Playback Gain: Loudest allowed incoming voice.  Default: +24 dB

  Always Use Automatic Gain: Ignore gain settings of incoming voice.

  Positional Team Voice: Hear teammate voices from the direction of their ships.

  Music Scale for Comms: Reduce music volume when receiving voice.  Default: 30%


== CONTROLS ==

  Swap Yaw/Roll: Normally X = roll and twist/pedals = yaw.  This option swaps those
    controls so the joystick points your ship and twist/pedals change your ship's roll,
    which is how classic X-Wing games were controlled.

  Invert Turrets: Normally turrets pitch the same way ships do.  This reverses them.

--Joystick--

  Deadzone: Ignore some stick/twist motion near the center to prevent drift.
    NOTE: This setting is not used for calibrated devices!

  X / Y / Twist: Sharpest is most responsive while Smoothest is least twitchy.

--Controller--

  Deadzone: Ignore some thumbstick motion near the center to prevent drift.
    NOTE: This setting is not used for calibrated devices!

  Sticks: Sharpest is most responsive while Smoothest is least twitchy.

  Triggers: Sharpest is most responsive while Smoothest is least twitchy.

--Mouse--

  Mode: Disabled: Disable mouse input in game.
  Mode: Turret Aim: Mouse does not control flight, but does aim for turret gunners.
  Mode: Yaw/Pitch: Mouse controls yaw and pitch in flight, and aims turrets.
  Mode: Roll/Pitch: Mouse controls roll and pitch in flight, and aims turrets.
  Mode: Freelook: Use mouse to look around the cockpit.

  Invert Pitch: Make mouse flying feel more like using a joystick (up = pitch down).

  Input: Sharpest is most responsive while Smoothest is least twitchy.

--Binds--

  Show Only Connected Devices: Hide control binds for devices that were not found.

  Refresh: Search again for connected joysticks.

  Default Binds:

   Joy X-Axis: Roll
   Joy Y-Axis: Pitch
   Joy Twist/Pedals: Yaw
   Joy Throttle: Throttle
   Joy Hat: Look Around
   Joy Button 1 (Trigger): Fire
   Joy Button 2: Change Weapon
   Joy Button 3: Change Firing Mode
   Joy Button 4: Target Nearest Enemy
   Joy Button 5: Look Center / VR Head Center
   Joy Button 6: Target Ahead
   Joy Button 7: Target Attacker
   Joy Button 8: Target Incoming Warhead
   Joy Button 9: Recall Stored Target 1
   Joy Button 10: Recall Stored Target 2
   Joy Button 11: Change Shield Direction
   Joy Button 12: Show Scores

   X52 Button 1 (Trigger): Fire
   X52 Button 2 (Fire): Change Weapon
   X52 Button 3 (A): Target Ahead
   X52 Button 4 (B): Target Nearest Enemy
   X52 Button 5 (C): Change Firing Mode
   X52 Button 6 (Pinkie): Look Center / VR Head Center
   X52 Button 7 (D): Change Shield Direction
   X52 Button 8 (E): Target Attacker
   X52 Button 9 (T1): Target Incoming Warhead
   X52 Button 10 (T2): Target Data-Link
   X52 Button 11 (T3): Cockpit Seat
   X52 Button 12 (T4): Toggle Co-Pilot (Chewie)
   X52 Button 13 (T5): Gunner Seat 1
   X52 Button 14 (T6): Gunner Seat 2
   X52 Button 20 (Hat 2 Up): Recall Stored Target 1
   X52 Button 21 (Hat 2 Right): Recall Stored Target 2
   X52 Button 22 (Hat 2 Down): Recall Stored Target 3
   X52 Button 23 (Hat 2 Left): Recall Stored Target 4
   X52 Button 24 (Hat 3 Up): Hold to Store Target
   X52 Button 25 (Hat 3 Right): Target Next
   X52 Button 26 (Hat 3 Down): Show Scores
   X52 Button 27 (Hat 3 Left): Target Previous
   X52 Button 31 (Clutch): Target Incoming Warhead

   SideWinder Button 1 (Trigger): Fire
   SideWinder Button 2: Change Weapon
   SideWinder Button 3: Target Ahead
   SideWinder Button 4: Change Firing Mode
   SideWinder Button 5 (A): Target Nearest Enemy
   SideWinder Button 6 (B): Target Attacker
   SideWinder Button 7 (C): Target Incoming Warhead
   SideWinder Button 8 (D): Change Shield Direction
   SideWinder Button 9 (Shift): Look Center / VR Head Center

   Xbox Left Stick: Yaw/Pitch
   Xbox Right Stick: Look Around
   Xbox Right Stick Click: VR Head Center
   Xbox Triggers: Roll
   Xbox Left Bumper: Target Center
   Xbox Right Bumper: Fire
   Xbox A: Throttle Decrease
   Xbox B: Change Firing Mode
   Xbox X: Throttle Increase
   Xbox Y: Change Weapon
   Xbox Back: Show Scores
   Xbox Start: Change Shield Direction
   Xbox D-Pad Up: Target Nearest Enemy
   Xbox D-Pad Down: Target Attacker
   Xbox D-Pad Left: Target Previous Enemy / Spectate Previous
   Xbox D-Pad Right: Target Next Enemy / Spectate Next

   Mouse Left-Click: Fire
   Mouse Right-Click: Target Center
   Mouse Middle-Click: Change Weapon
   Mouse Scroll Up: Target Previous
   Mouse Scroll Down: Target Next
   Mouse Thumb-Button Big (Back): Throttle Decrease
   Mouse Thumb-Button Little (Forward): Throttle Increase

   Arrows: Yaw/Pitch
   Backslash: 0% Throttle
   Left Bracket: 33% Throttle / Spectate Previous
   Right Bracket: 67% Throttle / Spectate Next
   Backspace: 100% Throttle
   Plus: Throttle Increase
   Minus: Throttle Decrease
   Numeric Keypad: Look Around
   Numeric Keypad 5: Look Center / VR Head Center
   Space: Fire
   Ctrl: Target Center
   Q: Target None
   W: Change Weapon
   E: Target Attacker
   R: Target Nearest Enemy
   T: Target Next
   Y: Target Previous
   U: Target Newest
   I: Target Incoming
   O: Target Objective
   P: Target Next Player
   A: Target Current Target's Attacker
   S: Shield Direction
   D: Roll Left
   F: Roll Right
   G: Target Groupmate
   H: Target Dockable
   K: Eject
   X: Change Firing Mode
   V/CapsLock: Target Data-Link
   F1: Cockpit Seat
   F2: Gunner Seat 1
   F3: Gunner Seat 2
   F4: Toggle Co-Pilot (Chewie)
   F5: Recall Stored Target 1
   F6: Recall Stored Target 2
   F7: Recall Stored Target 3
   F8: Recall Stored Target 4
   Shift: Hold to Store Target
   1: View Cockpit
   2: View Crosshair
   3: View Chase
   4: View Drop Camera
   5: View Cinema
   6: View Fixed
   7: View Selfie
   8: View Gunner
   9: View Cycle
   0: View Instruments
   Tab: Show Scores
   Return: Toggle/Send Chat
   Keypad Enter: Toggle/Send Team Chat
   Esc/F10: Menu
   Backtick/Tilde: Toggle Console

  To rebind an analog axis, click the box and then move your joystick or controller
  axis in the positive direction (pitch up, yaw right, roll right, throttle up, etc).
  If the result is reversed from what you want, rebind it in the other direction.

  To rebind a digital control, click the box and then press the desired button or key.

  To unbind a control entirely, right-click its box; all of its binds will be cleared.


== CALIBRATION ==

  1. Select the device type to calibrate.
  2. Sweep every axis to its full extents, then release to center.
  3. Check "Deadzone", then gently wiggle stick and twist within the deadzone.
  4. Click "Apply" when satisfied or "Clear" to start over.


== CONSOLE COMMANDS ==

X-Wing Revival features a Quake-style console which allows power users to tweak the
game parameters.  Press backtick (tilde) to toggle the console.  If you mess anything
up, quit and delete settings.cfg to restore defaults.

Here are some of the commands currently implemented:

  status: Show the status of various game subsystems, including framerate and ping.
  show: Show all variables.
  set <name> <value>: Change a variable's value.
  exec <file>: Load a config file.
  export <file>: Save a config file.
  g_restart: Apply video settings and restart video.
  joy_refresh: Look for new joysticks that have been connected.
  joy_clone <from> <to>: Define a new joystick category and copy binds to it.
  who: Show list of players connected.
  say <message>: Send chat message.
  echo <text>: Display text in the local console.
  host: Start a new game.
  connect <host>: Join a game by IP/hostname.
  reconnect: Rejoin the IP/hostname last connected to.
  disconnect: Leave the current game.
  quit: Exit to desktop.

When hosting, you can also use the "sv" command to control the server:

  sv show: List game variables.
  sv set <name> <value>: Change a game variable's value.
  sv defaults: Restore all game properties to default settings.
  sv state ++: When a game is in progress, you can use this to end the round early.
  sv netrate <num>: Adjust the network update rate for all clients.  Default: 30
  sv maxfps <num>: Adjust the simulation update rate.  Default: 60
  sv threads <num>: Use extra threads for capital ship collisions.  Default: 0
  sv announce <true/false>: Send UDP broadcasts for LAN discovery.  Default: true
  sv port <num>: Change the TCP port number (after restart).  Default: 7000
  sv restart: Restart the server.

From a dedicated server console, the sv commands are used without the "sv" keyword.


== COMMAND-LINE OPTIONS ==

You can control X-Wing Revival's behavior with these command-line options:

  -host: Start a new game immediately.
  -connect <host>: Join a game immediately by IP/hostname.
  -name <name>: Set player name.
  -dedicated: Host a dedicated server console instead of playing.
  -set <name> <value>: Change a variable's value.
  -windowed: Start windowed, not fullscreen.
  -vr: Start in VR mode.
  -safe: Use old fixed-pipeline OpenGL at 640x480 with minimal extensions.
  -screensaver: Load in screensaver mode.  Moving the mouse will quit.


== NETWORKING AND PORT-MAPPING ==

By default, X-Wing Revival servers run on TCP port 7000.  If you wish to host a game
online, you'll need to map this port from your firewall to your PC.  You can override
the server port by changing the "sv_port" variable.

Local games are announced by sending LAN broadcasts on UDP port 7000.  If you have a
software firewall enabled (such as Windows Firewall) and want to play over LAN, make
sure this port is allowed so you can discover local games.


== VIRTUAL REALITY ==

This version of X-Wing Revival has OpenVR (SteamVR) support!  I've only tested this
on the HTC Vive, but it should work on other headsets with SteamVR support, such as
Oculus Rift, Valve Index, etc.  See the Preferences for VR view tweaks.

When playing in VR, there's no radar yet, so pick up your visual scanning!  Messages
won't display by default, but you can show them by pressing return to toggle chat or
holding tab to show scores.


== ABOUT THE SCREENSAVER ==

To use X-Wing Revival as your screensaver, simply copy the RaptorEngine.scr file into
C:\Windows and select "RaptorEngine" in the Screen Saver settings.  Before you can
preview or activate it, you'll need to click "Settings...", browse to your copy of
"X-Wing Revival.exe", and click "OK".

The RaptorEngine screensaver wrapper is only currently available for Windows.  You can
also launch the game with "-screensaver" to watch screensaver mode on any platform.

You can customize the screensaver by creating a text file "screensaver.cfg" with
console commands to execute, such as "sv gametype yavin" or "sv rebel_fighter A/W".


== VERSION HISTORY ==

Alpha 0.5.1 (2025-05-15):
 * Fixed crash when pressing throttle up/down while not flying a ship.
 * Fixed "Target Objective" using mission objectives after leaving mission.
 * Improved modern targeting box boundaries and added squared option.
 * Lobby shows ship 3D model and can adjust Respawn Time and Preferences.
 * Added environment background "Blue Nebula" (and renamed "Red Nebula").
 * Nebulon-B Frigate engines are orange to match the movies.
 * Respawn is allowed in Fleet Battle even after flagship is lost.
 * Added team chat (default: keypad enter).  Fixed voice overlay in VR.

Alpha 0.5 (2025-05-04):
 * Multiplayer voice commlink with optional positional audio.
 * More progress on both campaigns.  Respawn is allowed when playing coop.
 * Improved graphics, such as bump-mapped lighting and new shot effects.
 * Surround sound and hi-def audio modes added.  Improved explosion sounds.
 * Increased speed of turbolaser bolts.  Turrets pick targets a bit smarter.
 * Y-Wings now go nearly as fast as X-Wings, though X-Wings maneuver better.
 * Targeting computer shows hull/shield damage.  HUD box fits target shape.
 * Target Crosshair should no longer stutter in large fleet battles.
 * Improved AI avoidance of friendly fire and collisions (somewhat).
 * Fixed missile/torpedo rubber-banding and other shot prediction bugs.
 * Fixed fullscreen resolution change in SDL2.
 * Fixed shield alarm not working in double-front/double-rear modes.
 * Added more Kessel Run options and fixed Team Kessel Run victory check.
 * Death Star exhaust port can only be locked onto from within the trench.
 * Lobby background image and player name colors now represent chosen team.

Alpha 0.4.5 (2024-08-28):
 * Fixed collision detection bugs and improved performance.
 * Fixed missing Lamba shuttle textures on case-sensitive Linux.
 * Added more detail to turret and starfighter laser firing animations.
 * Campaign progress is now saved separately from settings.

Alpha 0.4.4 (2024-07-27):
 * Joystick and controller deadzones can be precisely calibrated.
 * Reworked collision detection for improved accuracy and performance.
 * Alarm sound plays when deflector shields are lost.
 * Turret firing mode is retained when changing seats.
 * Fixed getting stuck spectating after trying to switch to gunner role.
 * Fixed shader compile errors on Mac OS X.

Alpha 0.4.3 (2024-06-12):
 * Textboxes read unicode text input events instead of raw key down/up.
 * Moving from turret to another seat retains current target.
 * Campaign now remembers selected difficulty.

Alpha 0.4.2 (2024-06-06):
 * Fixed buggy missile and torpedo behavior when shot prediction is enabled.
 * Added "Allow Team Change" option in pre-game lobby.
 * Replaced "Frigates" category with "Battleships" in Fleet Battle.
 * Added Victory-class Star Destroyer as the default Imperial Battleship.
 * Increased TIE Bomber health.

Alpha 0.4.1 (2024-05-15):
 * Fixed "Change Ship" showing all ships when team change is not allowed.
 * Fixed crash when attempting to play as turret gunner on Battle of Yavin.
 * Client-predicted shots are now synchronized with the server.
 * Corvette turrets can be destroyed.  Other minor balance tweaks.
 * Added Target Next/Previous Player controls.
 * Added another ship category "Frigates" to Fleet Battle mode.
 * Added "Reset to Defaults" button in lobby.
 * Bumping into a capital ship now damages from the correct direction.
 * Criusers now respawn spread out.

Alpha 0.4 (2024-05-04):
 * First proof-of-concept demo of singleplayer campaign.
 * Overhauled sound effects for improved dynamic range and variety.
 * Improved Linux Xbox controller support, and hopefully fixed missing libs.
 * Docking bays now rearm torpedoes and missiles.  (This also drops shields!)
 * Radar provides a bit more detail.
 * Flight groups jump in staggered instead of all at once.
 * Higher skill AI can angle deflectors towards incoming shots.
 * Weapon firing modes are now retained when respawning.
 * Added target store/recall, target target's attacker, and view mode controls.
 * Target nearest attacker no longer changes target if no fighter is attacking.
 * Reworked ship and weapon stats.  Every YT-1300 has Han Solo's upgrades.
 * Player shots can now be predicted client-side to reduce perceived lag.

Alpha 0.3.5 (2024-02-24):
 * Fixed VR bug that prevented objects from moving.
 * Fixed "Shields Down" status on targets with full shields double-front/rear.
 * Improved turret gunner targeting hologram and info, especially in VR mode.
 * Cleaned up weapon placements on starfighters.
 * Target Data-Link can no longer be used to target your own ship.

Alpha 0.3.4 (2024-02-07):
 * Ships and turrets show charring around recent blastpoints.
 * Improved rendering of shots and engine glow.
 * Improved shot synchronization in netcode.
 * Low-speed collisions are sometimes survivable.
 * Ships can now hover in docking bays to repair.  (Shields drop for repairs!)
 * Cockpit target display shows impact and explosion effects.
 * Control binds in console are no longer case-sensitive.

Alpha 0.3.3 (2023-12-16):
 * YT-1300 turrets deal more damage, but must be manned.  Cockpit has its own laser.
 * Player turret shots no longer appear out of sync at certain firing angles.
 * Capital ship turrets should no longer shoot through their own ship.
 * Ships reducing speed no longer jerk backwards as suddenly.
 * Improved engine glow effects.
 * Missiles are now able to strike the ship that fired them (if extremely unlucky).

Alpha 0.3.2 (2023-12-02):
 * Fixed several bugs related to networking and threading, especially on server.
 * Engines now glow at full throttle.
 * Slightly improved AI checkpoint chasing in Kessel Run.

Alpha 0.3.1 (2023-11-22):
 * Fixed players losing sync when playing over the internet.
 * Fixed cockpit seat being unavailable to turret gunners after pilot disconnects.
 * Fixed "resync" causing client and/or server to crash.  (Two separate bugs!)
 * Fixed collision model sometimes not updating when player changes ship.
 * Slightly improved netcode prediction accuracy.
 * Turrets no longer aim at Kessel Run checkpoints.  Gunners hear checkpoints too.
 * Scoreboard now highlights the winning team.
 * Added Eject button (default K): hold for 3 seconds to self-destruct.

Alpha 0.3 (2023-11-07):
 * Now includes ELF64 binary for Linux.
 * Updated to SDL2 on Windows/Linux.  This fixes some joystick device mappings.
 * Improved netcode fixes shot sync and reduces ship rubber-banding when stopping.
 * Added new race game modes: Team Kessel Run and FFA Kessel Run.
 * Added new playable Imperial ship: Assault Gunboat.
 * Improved gameplay with respawn disabled.  Round ends when the last player dies.
 * AI ships work harder to complete main objectives, especially without respawn.
 * Expanded AI skill levels: Bantha Fodder, Rookie, Veteran, Ace, Jedi, and Sith.
 * Subsystems on Star Destroyer can now be targeted and locked on at close range.
 * Y-Wings and B-Wings now have working ion cannons to take down shields quickly.
 * All ship turrets animate movement.  YT-1300 turrets reworked with firing modes.
 * Improved explosions, shots, and dynamic lights.  Effects quality now adjustable.
 * Reduced z-fighting flicker and improved performance by culling model backfaces.
 * Improved TIE cockpit visibility and moved VR viewport into correct head position.
 * Added VR tweaks to prefs, including new "vr_sway" (default off to reduce nausea).
 * Reworked sounds: damage received, enemy lock, missile launch, and rebel turrets.
 * More variety in asteroid size and health.  Improved scatter and reduced count.
 * Target Incoming Warhead now always targets the most recently launched.
 * Joystick hat now defaults to Glance views, similar to controller thumbstick look.
 * Increased spacing between cruiser arrival points to reduce friendly collisions.
 * If the rebels do not destroy the Death Star in time, it fires its superlaser.
 * Fixed auto-assign moving you to the winning team if you died after your flagship.
 * Fixed flyby sounds playing too frequently when closely following another ship.
 * Fixed position of Customize Fleet menu in VR.
 * Added pause button for single-player games.

Alpha 0.2 (2023-05-04):
 * AI ships now dodge obstacles, avoid friendly fire, and have selectable skill level.
 * Improved capital ship collision detection to maintain performance in large fleets.
 * Replaced "Defend/Destroy" with "Fleet Battle"; all AI ship types are customizable!
 * Ship and weapon rebalancing, mostly of capital ships.  Added Interdictor Cruiser.
 * Improved netcode makes other ships move more smoothly and corrects for jitter.
 * Players can control turrets, change seats, and let an AI co-pilot fly the YT-1300.
 * In-flight menu allows selecting a different ship or group for next respawn.
 * Improved graphics.  Respawn jumps in from hyperspace.  Engines glow with throttle.
 * Improved some sound effects.  Accelerating or decelerating makes engine sounds.
 * Weapons remember mode.  Each action has a unique beep.  Lasers beep when on target.
 * Deflector shield angled double-front or double-rear protects only that direction.
 * If your target crashes after you shoot them, you get the kill.  TKs subtract score.
 * Improved input curves, especially controllers.  Xbox 360 controllers now work great!
 * Joystick, game controller, keyboard, and mouse binds are now fully customizable.
 * Updated RaptorEngine.scr to keep multiple paths, such as X-Wing Revival and BTTT.

Alpha 0.1.9 (2021-05-04):
 * Ships are now less twitchy, especially at full speed.  Slow down for tighter turns.
 * Balance and damage tweaks.  It now takes a pair of torpedoes to kill the Death Star!
 * Improved lighting effects, and fixed lighting artifacts along polygon edges.
 * Fixed anti-aliasing on Windows by using 24-bit depth buffer when not in VR mode.
 * Can toggle VSync (default off), and default maxfps on Windows is the refresh rate.
 * Fixed some issues with capital ship turret spawning and tracking angles.
 * Fixed a bug that caused players to regain health when someone joined late.

Alpha 0.1.8 (2020-05-04):
 * Added a bunch of new ships to play with, and did some re-balancing.
 * Disables Windows DPI scaling; the screen should no longer stretch and crop.
 * Dramatically reduced network jitter.  Formation flying should now be possible.
 * Improved framerate smoothness.  The optimal maxfps is now your video refresh rate.
 * Improved graphics, especially lighting.  The Death Star horizon looks much better.
 * The cockpit sways with yaw/pitch/roll input for a better feeling of ship motion.
 * Instruments flash for enemy lock-on, and your droid warns about incoming missiles.
 * Added support for some joystick and pedal devices with unusual axis mapping.

Alpha 0.1.7 (2019-05-04):
 * Torpedos must now acquire a lock to track, but are better at hitting their targets.
 * Improved targeting computer display, especially for locking onto the exhaust port.
 * Trench obstacles protrude a bit less, so flying the trench is a little easier.
 * Detailed geometry in the trench has been reworked to reduce z-fighting flicker.
 * VR is more comfortable, with better default vr_fov and a more stable spectator view.
 * The cockpit shakes when hit, and moves slightly for acceleration and deceleration.
 * Fixed the cockpit orientation when using view rotation controls (keys/thumbstick/hat).
 * If the screensaver engages while the PC is running a server, it spectates the game.

Alpha 0.1.6 (2018-05-04):
 * Fixed cockpit jitter when you fly far from the map center, especially in VR mode.
 * Menus now have a 3D background in VR, making them infinitely less nauseating.
 * Automatically recenters VR head position when a mission begins.
 * Roll/pitch/yaw have a maximum rate of change, which smooths out keyboard flying.
 * AI ships fly a little smarter; there should be fewer Porkins suicide runs.
 * Fixed texture loading on new versions of Mac OS X by downgrading to SDL_image 1.2.10.
 * Fixed library not found crash in Mac OS X if OpenVR is not installed (dylib bundled).

Alpha 0.1.5 (2017-05-04):
 * Added OpenVR/SteamVR support for HTC Vive (and probably Oculus Rift).
 * Chat beep sound, because in VR messages don't display unless you press return.
 * Controller thumbstick look now intelligently toggles itself.
 * Fixed font and mouse cursor artifacts with antialiasing.
 * More customization options for texture resolution and filtering.
 * Safe mode is now so safe, it works on the Voodoo5 (with 3rd-party XP drivers).

Alpha 0.1.4 (2016-05-04):
 * Improved lighting quality.
 * Game engine and net-code cleanup.
 * You can now disable thumbstick look (it prevents other look methods when active).
 * Improved spectator view.  Players waiting to respawn now watch their teammates.
 * Added safe-mode for systems with strange GPUs or outdated video drivers.
 * Officially added the silly sounds easter-egg.  See if you can find it!

Alpha 0.1.3 (2015-05-04):
 * Fighters and lasers are all a little faster (the Y-Wing especially needed this).
 * Added high-quality cockpits.
 * Other minor graphical improvements.
 * Host can now end the round early without quitting the server.
 * Fixed some rare issues with working directory and DLL/resource paths.

Alpha 0.1.2 (2014-05-28):
 * Added "Defend/Destroy" game mode.
 * Merged the two hunt modes into one customizable "Capital Ship Hunt" game mode.
 * Added Nebulon B Frigate and Mon Calamari Cruiser.
 * Some capital ships now have destructible attached turrets.
 * Large capital ships will shoot asteroids out of their way.
 * Star Destroyer shield towers now explode correctly.
 * Improved capital ship performance, especially on the server.
 * Fixed compatibility issues with recent Mac OS X versions, including 10.9 Mavericks.

Alpha 0.1.1 (2014-05-03):
 * Added "Star Destroyer Hunt" and "Rebel Corvette Hunt" game modes.
 * Added group spawning option to encourage teamwork.
 * Added Saitek DirectOutput support for X52 Pro and Flight Instrument Panel (FIP).
 * No longer ignores analog axes on joystick when button pads are present.
 * Reduced default sv_maxfps from 120 to 60 to reduce CPU load.

Alpha 0.1 (2013-08-29):
 * First semi-public release.
