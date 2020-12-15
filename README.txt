Dance Dance Revolution by Chigozie Agomo

This is a DDR style game made by myself using the Windows port of the GNU C compiler, MinGW, and SDL2, a graphics, audio, and input library.
Feel free to edit the data files, the song files, or even the source code, but (even though you probably won't) please give credit to me with a
note in a README if you are to share your edits. A specific level (even one that the game wouldn't normally play) can be picked using
command line arguments and putting the level's dat.ddr and song.wav files inside the corresponding subdirectory (see "how to mod the game"
-> "how to play a custom level").

This game was last updated on: 2018/10/07 14:34

Controls:
QUIT -> ESCAPE
UP -> UP / W
LEFT -> LEFT / A
DOWN -> DOWN / S
RIGHT -> RIGHT / D

How to play:
Black arrows are falling to the centre of the screen. Press the correct button with the right timing to get points. The greater the overlap
between black and white arrows, the more points you get! The percentage of points you attained out of the maximum number of points possible in
the stage is your final score. Try (and probably fail) to get 100%! Try to listen to the music that is being played! The button timings (should)
match up to the music!

How to mod the game:
Adding your own levels to the game is quite simple (depending on what you want to do).

	How to change a level's music:
	If my music isn't your cup of tea, get better taste. But if you really want to change a level's music, you should enter the level's subdirectory
	(marked by a number inside data) and replace the song.wav file with another song called song.wav (this song must be of the WAVE file format,
	if your song is another format, like .mp3, you can convert it in audacity or online).
	
	How to change a level's format:
	If the level is too easy or too hard for your liking you can modify its format (at your own risk). The level's subdirectory should be accessed
	(data/x) and the dat.ddr file can be edited for an instant change in level format. The first line should be a maximum of 50 characters and
	contains the song's title. The next lines should contain alternating decimal and hexadecimal integers (as seen in the official dat.ddr files).
	The decimal integers show the stall (wait) time until the next set of arrows is spawned (in milliseconds (ms)). The hexadecimal integers show
	the arrows that will be spawned after the stall. The hexadecimal integers are tested with a bitmask, so they should only contain 1 and 0.
	The format for hexadecimals is WXYZ where W, X, Y, Z are 1 or 0, and they represent whether the up (W), left (X), down (Y), or right (Z)
	arrows should spawn at that time or not. Using the same format that the official dat.ddr files use will make levels much easier to read and
	is recommened.
	
	How to make a custom level:
	If you want to make a completely new level, I pity your efforts to match the arrow spawns to the music. However, if you are really determined
	to make a new level it is simple. In the data directory, make a new folder and name it with any number (or word) that is 1 - 5 digits long.
	Then read the previous 2 guides and apply them to your own level.
	
	How to play a custom level:
	The level you made being unofficial will require you to do some hacking (not really). Open cmd.exe and type in "cd x" without the quotations
	where x is the directory address of ddr.exe. Then type in "ddr y" where y is the level directory's name (don't inlcude the "data/").
	For myself I use "cd documents/c/ddr" followed by "ddr 0" to play Galaga Medley. If you prefer your own level to my own you are surely
	mistaken, but if you are confident in your beliefs you can rename your level directory to replace one of the official level directories, so the
	game includes your level in its random level selection.
	
	How to completely redo the game:
	If you are a C programmer and know SDL2 well, you can mod the game however you want with ease. The code is fully documented with
	comments all throughout and my coding style is quite readable, so you should have a good and easy time modding this game. If you aren't
	and want to fully mod the game, that's too bad. If you learn C, SDL2, and get a compiler with SDL2 binaries and libraries, then you may
	be able to mod the game without completely breaking it, but I wouldn't recommend modding the game until you can make your own games. 