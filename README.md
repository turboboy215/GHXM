# GHXM
GHX (Shin'en GBC) to XM converter


This tool converts music from Game Boy Color games using GHX, Shin'en's GBC sound engine to XM (FastTracker II) format. It is named after GAXM (https://github.com/byvar/gaxm), which has essentially the same function for Game Boy Advance games using its GBA counterpart, GAX. Like that program, I have decided to create a converter to a common tracker module format rather than MIDI, since GHX is a tracker-based sound engine and is much more similar in structure to tracker modules.

It works with ROM images. To use it, you must specify the name of the ROM followed by the number of the bank containing the sound data (in hex).
For games that contain multiple banks of music (usually 2; Astérix: Search for Dogmatix has 3), you must run the program multiple times specifying where each different bank is located. However, in order to prevent files from being overwritten, the XM files from the previous bank must either be moved to a separate folder or renamed.

The program will also search for a table containing the internal titles of each song, which is present in some games. Note that for many games, there are "empty" tracks (usually the last track). This is normal.

Examples:
* GHXM "Jimmy White's Cueball (E) [C][!].gbc" 11
* GHXM "Tomb Raider (UE) (M5) [C][!].gbc" 7F
* GHXM "Spider-Man (U) [C][!].gbc" 39
* GHXM "Spider-Man (U) [C][!].gbc" 3A

This tool was based on my own reverse-engineering of the sound engine, with little disassembly involved. The resulting XM files don't sound "great" since all the instruments are represented by the same sine waveform. The few known effects supported by the driver are converted; however, portamento/pitch slides are inaccurate, since the sound engine seems to set it to an absolute note value, which is difficult to convert to a traditional portamento value. If anyone is willing to work on a better method, please let me know.

Also included is another program, GHX2TXT, which prints out information about the song data from each game. This is essentially a prototype of GHXM, similar to MC2TXT. Do not delete the xmdata.dat file! It is necessary for XM conversions.

Supported games:
  * AMF Bowling
  * Astérix: Search for Dogmatix
  * Barbie: Magic Genie Adventure
  * Barbie: Shelly Club
  * Beach'n Ball
  * Blue's Clues: Blue's Alphabet Book
  * Carnivale
  * Dave Mirra Freestyle BMX
  * Fix & Foxi Episode 1: Lupo
  * Hoyle Card Games
  * Jimmy White's Cueball
  * Käpt'n Blaubär: Die verrückte Schatzsuche
  * Le Mans 24 Hours
  * NASCAR Heat
  * NBA Jam 2001
  * Planet of the Apes
  * Prince Naseem Boxing
  * Pro Darts
  * Rescue Heroes: Fire Frenzy
  * Sea-Doo Hydrocross
  * SnowCross
  * Spider-Man
  * Spider-Man 2: The Sinister Six
  * SpongeBob SquarePants: Legend of the Lost Spatula
  * Supercross Freestyle
  * Tomb Raider
  * Tomb Raider: Curse of the Sword
  * Tyrannosaurus Tex (prototype)
  * VR Sports Powerboat Racing
  * Wacky Races
  * The Wild Thornberrys: Rambler

## To do:
  * Better portamento support
  * GBS file support
