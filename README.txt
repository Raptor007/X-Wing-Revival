----------------------------------------
|            X-Wing Revival            |
|       0.1.9 Alpha (2021-05-04)       |
|     by Raptor007 (Blair Sherman)     |
| http://raptor007.com/x-wing-revival/ |
----------------------------------------


Welcome to X-Wing Revival!  I started making this because I wanted to have an X-Wing LAN
party in 2011, and found that X-Wing Alliance (released in 2002) didn't run correctly on
modern PCs.  This project has fallen lower in my list of priorities, but I still enjoy
playing it and releasing a little update every May the Fourth.

There's finally a new official X-Wing game!  Check out Star Wars Squadrons:
  https://store.steampowered.com/app/1222730/STAR_WARS_Squadrons/


== GETTING STARTED ==

The quickest way to start is to click Play, then Host / Solo, adjust the game settings,
and click Fly when you're ready to launch.


== TROUBLESHOOTING / FAQ ==

Textures are blank.  Everything looks like white boxes!

  Windows: This is a directory permissions problem.  This often happens on Windows 10 if
    you attempt to unzip X-Wing Revival in your Downloads folder or on the desktop.

    The solution is to move the zip file to a different directory like "C:\Program Files"
	before extracting it.

  Mac: This is usually caused by the Quarantine file system attribute, which prevents a
    downloaded program from reading files, even if they were extracted from the same zip.

	The solution is to open Utilities/Terminal and remove the Quarantine flag from the
	directory you extracted X-Wing Revival to:

	  xattr -dr com.apple.quarantine "/Applications/X-Wing Revival"

My Mac won't allow a downloaded or unsigned application to run.

  This is because Apple wants you to buy everything through their app store, and they've
  deliberately made it difficult to distribute software by any other mechanism.

  The solution is to enable "allow apps downloaded from anywhere" in the Terminal:

      sudo spctl --master-disable


== CONTROLS ==

  Joy X-Axis: Roll
  Joy Y-Axis: Pitch
  Joy Twist/Pedals: Yaw
  Joy Throttle: Throttle
  Joy Hat: Look Around
  Joy Button 0: Fire
  Joy Button 1: Change Weapon
  Joy Button 2: Target Ahead
  Joy Button 3: Target Nearest Enemy
  Joy Button 4: Change Firing Mode
  Joy Button 5: Look Center / VR Head Center
  Joy Button 6: Change Shield Direction
  Joy Button 7: Target Attacker
  Joy Button 8: Target Previous
  Joy Button 9: Target Next
  Joy Button 10: Target Previous Friendly
  Joy Button 11: Target Next Friendly
  Joy Button 12: Target Previous Enemy
  Joy Button 13: Target Next Enemy
  Joy Button 19: Target Prev Friendly
  Joy Button 20: Target Next Enemy
  Joy Button 21: Target Next Friendly
  Joy Button 22: Target Prev Enemy
  Joy Button 23: Target Prev Friendly
  Joy Button 24: Target Next Enemy
  Joy Button 25: Target Next Friendly
  Joy Button 26: Target Prev Enemy
  Joy Button 30: Target Incoming Warhead

  Xbox Left Stick: Yaw/Pitch
  Xbox Right Stick: Look Around
  Xbox Triggers: Roll
  Xbox Left Bumper: Target Center
  Xbox Right Bumper: Fire
  Xbox A: Throttle Decrease
  Xbox B: Change Firing Mode
  Xbox X: Throttle Increase
  Xbox Y: Change Weapon
  Xbox Back: Change Shield Direction
  Xbox Start: Change Shield Direction
  Xbox D-Pad Up: Target Attacker
  Xbox D-Pad Down: Target Nearest Enemy
  Xbox D-Pad Left: Target Previous Enemy
  Xbox D-Pad Right: Target Next Enemy

  Mouse Left-Click: Fire
  Mouse Right-Click: Target Center
  Mouse Middle-Click: Change Weapon
  Mouse Scroll Up: Target Previous
  Mouse Scroll Down: Target Next
  Mouse Thumb-Button Big (Back): Throttle Decrease
  Mouse Thumb-Button Little (Forward): Throttle Increase

  Arrows: Yaw/Pitch
  D: Roll Left
  F: Roll Right
  Backslash: 0% Throttle
  Left Bracket: 33% Throttle / Spectate Previous
  Right Bracket: 67% Throttle / Spectate Next
  Backspace: 100% Throttle
  Plus: Throttle Increase
  A: Throttle Increase
  Minus: Throttle Decrease
  Z: Throttle Decrease
  Numeric Keypad: Look Around
  Numeric Keypad 5: Look Center / VR Head Center
  Space: Fire
  W: Change Weapon
  X: Change Firing Mode
  S: Shield Direction
  Ctrl: Target Center
  E: Target Attacker
  R: Target Nearest Enemy
  T: Target Next
  Y: Target Previous
  F1: Target Next Friendly
  F3: Target Next Enemy
  Q: Target None
  Tab: Show Scores
  Return/Enter: Toggle/Send Chat
  Backtick/Tilde: Toggle Console


== GAME TYPES ==

  Battle of Yavin: Rebels must fly down the trench and try to hit the exhaust port
    with a proton torpedo.  Imperials must try to prevent the Rebels from doing this
    until the time limit runs out.

  Capital Ship Hunt: The attacking team must destroy the defending team's capital ship
    before the time limit runs out.

  Defend/Destroy: Each team tries to destroy the enemy capital ship while defending
    their own.  The last team with a surviving capital ship wins.

  Team Elimination: The last team with any ships remaining wins.  Players can control
    AI ships when the player's ship has been destroyed, but no new ships will be given
    to either team once the match has started.

  Team Deathmatch: The team to hit the kill limit first wins.  Respawn always enabled.

  FFA Elimination: Last man standing wins, and there's no respawn whatsoever.

  FFA Deathmatch: The player to hit the kill limit first wins.  Respawn always enabled.


== PREFERENCES ==

--Graphics--

  Fullscreen: Toggle fullscreen, and set the resolution for fullscreen mode.
    NOTE: 0 x 0 means use desktop resolution (may require re-launch).
    Windowed resolution can be changed by dragging the lower-right corner.

  FSAA (Antialiasing): Use multiple samples to reduce jagged edges.  Can be GPU-heavy,
    especially when VR is enabled.

  Vertical Synchronization (VSync): Eliminate tearing by only drawing frames when the
    monitor is ready to refresh.  Recommended for fast systems, but turning this off is
	an easy way to improve framerate and reduce stuttering.

  Texture Quality: Limit the maximum texture resolution; downsample anything too large.
    This may improve performance with very limited VRAM and is required on some ancient
    video cards, but if your PC was made in the last 20 years you can leave this High.

  AF (Anisotropic Filtering): Improve texture clarity and detail.  Slightly GPU-heavy.
    Trilinear mode is the standard non-anisotropic mode.  Linear mode uses bilinear
    filtering without mipmapping, which looks sharp but sparkles at longer distances.

  Framebuffer Textures: Use framebuffer objects as live textures for the cockpit.
    This shouldn't hurt performance much, but some old video cards don't support it.
	Framebuffers are also used for the VR eyes, so this is required for VR.

  Draw With Shaders: Use GLSL vertex and fragment shaders.  Recommended for performance
    and better appearance, but certain old motherboards perform better with this off
	and some ancient video cards require this off.

  Light Quality: Per-pixel can be GPU-heavy, especially in VR, but looks much better on
    large surfaces like capital ships and the Death Star trench.
    NOTE: This only works if "Draw With Shaders" is enabled.

  Dynamic Lights Per Object: Number of point light sources (lasers, torpedos, etc) to
    use for lighting each object.  Higher values use a bit more CPU and GPU power.
    NOTE: This only works if "Draw With Shaders" is enabled.

  Death Star Trench Details: Add detail to the Death Star trench walls and floor.  Can
    be somewhat GPU-heavy and CPU-heavy.
    NOTE: This is even more CPU-heavy when shaders are disabled!

  Virtual Reality (OpenVR/SteamVR): Enable VR HMD output and seated head tracking.
    Tested with the HTC Vive, and it should work for the Oculus Rift or any other HMD.
	You might need to tweak vr_fov and vr_offset in the console.  VERY GPU-HEAVY!

--Sound--

  Volume: Master volume that affects all playback.

  Effects: Volume of sound effects.

  Music: Volume of music.

  Menu Music: Play music in the menus and lobby.

  Game Music: Play music while flying.

--Joystick--

  Deadzone: Ignore some stick/twist motion near the center to prevent drift.

  Swap Yaw/Roll: Normally X = roll and twist/pedals = yaw.  This option swaps those
    controls so the joystick points your ship and twist/pedals change your ship's roll,
    which is how classic X-Wing games were controlled.

  X / Y / Twist: Linear is most responsive while Smoothest is least twitchy.

--Controller--

  Deadzone: Ignore some thumbstick motion near the center to prevent drift.

  Sticks: Linear is most responsive while Smoothest is least twitchy.

--Mouse--

  Mode: Disabled: Disable mouse input in game.
  Mode: Yaw/Pitch: Mouse controls yaw and pitch in flight.
  Mode: Roll/Pitch: Mouse controls roll and pitch.  Reassigns keys D and F to control yaw.
  Mode: Freelook: Use mouse to look around the cockpit.

  Invert: This makes mouse flying feel more like using a joystick (up = pitch down).

  Input: Linear is most responsive while Smoothest is least twitchy.


== CONSOLE COMMANDS ==

X-Wing Revival features a Quake-style console which allows power users to tweak the
game parameters.  Press backtick (tilde) to toggle the console.  If you mess anything
up, quit and delete settings.cfg to restore defaults.

Here are the commands currently implemented:

  status: Show the status of various game subsystems, including framerate and ping.
  show: Show all variables.
  set <name> <value>: Change a variable's value.
  exec <file>: Load a config file.
  export <file>: Save a config file.
  g_restart: Apply video settings and restart video.
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
  sv state ++: When a game is in progress, you can use this to end the round early.
  sv netrate <num>: Adjust the network update rate for all clients.  Default: 30
  sv maxfps <num>: Adjust the simulation update rate.  Default: 60
  sv port <num>: Change the TCP port number (after restart).  Default: 7000
  sv restart: Restart the server.

From a dedicated server console, the sv commands are used without the "sv" keyword.


== COMMAND-LINE OPTIONS ==

You can control X-Wing Revival's behavior with these command-line options:

  -host: Start a new game immediately.
  -connect <host>: Join a game immediately by IP/hostname.
  -dedicated: Host a dedicated server console instead of playing.
  -set <name> <value>: Change a variable's value.
  -safe: Use old fixed pipeline OpenGL at 640x480 with minimal extensions.
  -screensaver: Load in screensaver mode.  Moving the mouse will quit.


== NETWORKING AND PORT-MAPPING ==

By default, X-Wing Revival servers run on TCP port 7000.  If you wish to host a game
online, you'll need to map this port from your firewall to your PC.  You can override
the server port by changing the "sv_port" variable.

Local games are announced by sending LAN broadcasts on UDP port 7000.  If you have a
software firewall enabled (such as Windows Firewall) make sure this port is allowed so
you can discover local games.


== VIRTUAL REALITY ==

This version of X-Wing Revival has OpenVR (SteamVR) support!  It has only been tested
on the HTC Vive, but it should work on other headsets like the Oculus Rift.

I quickly hacked this in without following all the best practices, so it may require a
little bit of tweaking to look perfect, especially if you aren't using the HTC Vive.
Here are the relevant console variables:

  vr_fov: Field of view in VR.  Negative values specify vertical FOV.  Defaut: -111
  vr_offset: Eye centering fix in pixels.  Small values exaggerate depth.  Default: 87
  vr_separation: Distance between in-game eyes in meters.  Default: 0.0625

When playing in VR, there's no radar yet, but the ability to freely look around more
than makes up for this.  Messages won't display by default, but you can show them by
pressing return to toggle chat.


== ABOUT THE SCREENSAVER ==

To use X-Wing Revival as your screensaver, simply copy the RaptorEngine.scr file into
C:\Windows and select "RaptorEngine" in the Screen Saver settings.  Before you can
preview or activate it, you'll need to click "Settings...", browse to your copy of
"X-Wing Revival.exe" (or x64), and click "OK".

The RaptorEngine screensaver wrapper is only currently available for Windows.


== VERSION HISTORY ==

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
