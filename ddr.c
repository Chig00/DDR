#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <SDL.h>

// Meta Constants
#define ARROWS 4 // The number of arrow directions in the game
#define SCORES 2 // The number of scores stored in a player
#define WINDOW_WIDTH 600 // The width of the window
#define WINDOW_HEIGHT 600 // The height of the window
#define ARROW_WIDTH 50 // The width of an arrow
#define ARROW_HEIGHT 50 // The height of an arrow

// Player type
typedef struct {
	SDL_Surface* arrows[ARROWS]; // The player has an SDL_Surface* for each arrow
	SDL_Rect rects[ARROWS]; // The player has an SDL_Rect for each arrow
	int score[SCORES]; // The player stores the obtained and maximum possible score
	bool press[ARROWS]; // Checks for the player holding down a button
} Player;

// Target arrow type
typedef struct {
	SDL_Rect rect; // Each target arrow has an SDL_Rect for comparison to the player arrows
	double pos; // Each target has a position, which shows how far away it is from its spawn
	double last_move; // Each target arrow stores the time where it last moved, so the distance it has to travel can be calculated
} Target;

// Target arrow container type
typedef struct {
	SDL_Surface* arrows[ARROWS]; // An SDL_Surface* for each arrow direction
	Target* targets[ARROWS]; // An array of pointers to dynamically allocated memory blocks to store target arrows organised by direction
	int counts[ARROWS]; // An array of integers to count the number of target arrows in each direction
} Targets;

// Constants
const int MAX_LEVEL = 5; // Maximum length of level string
const int MAX_DATA = 20; // Maximum length of data's filename
const int MAX_SONG = 20; // Maximum length of song's filename
const int MAX_SONG_TITLE = 50; // Maximum length of the song title
const int MAX_TITLE = 100; // Maximum length of the window title

const char* const TITLE = "DDR by Chigozie Agomo: ";
const int STALL_UNIT = 1000; // Fraction of a second for arrow spawn stalling
const int NO_STALL = -1; // Constant to represent a request for a new stall value
const int LEVELS = 3; // Number of levels available
const int BACKGROUND[] = {0, 0, 0}; // Background colour in RGB
const char PERCENT = '%';

const int QUIT = SDL_SCANCODE_ESCAPE; // Key for quitting the game is Esc
const int PAUSE = SDL_SCANCODE_P; // Key for pausing the game is P

const int MAX_SCORE = 100; // The maximum score possible for a round
const char* const PLAYER_ARROWS[] = {
	"data/p0.bmp", // Up
	"data/p1.bmp", // Left
	"data/p2.bmp", // Down
	"data/p3.bmp" // Right
}; // The filenames for the player's arrow to be loaded
const int PLAYER_RECT_DATA[][2] = {
	{(WINDOW_WIDTH - ARROW_WIDTH) / 2, (WINDOW_HEIGHT - 3 * ARROW_WIDTH) / 2}, // Up
	{(WINDOW_WIDTH - 3 * ARROW_WIDTH) / 2, (WINDOW_HEIGHT - ARROW_WIDTH) / 2}, // Left
	{(WINDOW_WIDTH - ARROW_WIDTH) / 2, (WINDOW_HEIGHT + ARROW_WIDTH) / 2}, // Down
	{(WINDOW_WIDTH + ARROW_WIDTH) / 2, (WINDOW_HEIGHT - ARROW_WIDTH) / 2} // Right
}; // The rect coordinated for the player's arrows
const int PLAYER_KEYS[] = {
	SDL_SCANCODE_UP, // Up
	SDL_SCANCODE_LEFT, // Left
	SDL_SCANCODE_DOWN, // Down
	SDL_SCANCODE_RIGHT // Right
}; // The keys for player input
const int PLAYER_ALT_KEYS[] = {
	SDL_SCANCODE_W,
	SDL_SCANCODE_A,
	SDL_SCANCODE_S,
	SDL_SCANCODE_D
}; // The alternative keys for player input (for player used to local multiplayer)

const int ARROW_SPEED = WINDOW_WIDTH / 4; // The number of pixels travelled by a target arrow in a second
const char* const TARGET_ARROWS[] = {
	"data/t0.bmp", // Up
	"data/t1.bmp", // Left
	"data/t2.bmp", // Down
	"data/t3.bmp" // Right
}; // The filenames for the targets' arrows to be loaded

const int BIT_TESTS[] = {
	0x1000, // Up
	0x0100, // Left
	0x0010, // Down
	0x0001 // Right
}; // Constants used with bitwise AND operator to test for type of target arrow to be spawned

// Returns a random integer, x, in the range min <= x <= max
int rand_int(int min, int max) {
	return min + (max - min + 1) * rand() / (RAND_MAX + 1);
}

// Returns a time in seconds relative to the last call of the function
double get_time() {
	return (double) clock() / CLOCKS_PER_SEC;
}

// Returns the delay required for the first target arrow to be perfectly erased at the song's start (syncs the song to the arrows)
double get_delay() {
	return (double) (WINDOW_WIDTH - ARROW_WIDTH) / (2 * ARROW_SPEED);
}

// Creates a new arrow using the data file for spawn location information
void spawn(Targets* targets, FILE* data) {
	// The bitmask allows information from the data file for all arrow directions to stored in a single integer
	int bitmask;
	fscanf(data, "%x", &bitmask);
	
	for (int i = 0; i < ARROWS; i++) {
		// The bitmask is tested with the bitwise AND to see whether a target arrow should be spawned in a position
		if (bitmask & BIT_TESTS[i]) {
			// The target container's relevant container is extended for the target
			targets->targets[i] = realloc(targets->targets[i], sizeof(Target) * (targets->counts[i] + 1));
			// These variables are identical for all initialised targets and are stored in the last Target memory block
			targets->targets[i][targets->counts[i]].pos = -ARROW_WIDTH / 2;
			targets->targets[i][targets->counts[i]].rect.w = ARROW_WIDTH;
			targets->targets[i][targets->counts[i]].rect.h = ARROW_HEIGHT;
			
			// The target's constant coordinate is set
			switch (i % 2) {
				// If the target arrow is up or down, its x coordinate is constant
				case 0:
					targets->targets[i][targets->counts[i]].rect.x = (WINDOW_WIDTH - ARROW_WIDTH) / 2;
					break;
					
				// If the target arrow is left or right, its y coordinate is constant
				case 1:
					targets->targets[i][targets->counts[i]].rect.y = (WINDOW_HEIGHT - ARROW_HEIGHT) / 2;
					break;
			}
			
			// The target's counter for the direction is incremented and its last_move is set
			targets->targets[i][targets->counts[i]++].last_move = get_time();
		}
	}
}

// Updates the game
bool update(Player* player, Targets* targets, FILE* data, SDL_Surface* display) {
	// Static variables must be initialised with non-variables, so the first time the function runs, they are set after initialisation
	static bool first_update = true;
	static int stall;
	static double last_spawn;
	
	// The target arrow container is checked to see if it empty
	bool no_targets = true;
	
	for (int i = 0; i < ARROWS; i++) {
		if (targets->counts[i]) {
			no_targets = false;
		}
	}
	
	// The game ends when the end of the data file is reached, the game isn't stalling, and all targets have been destroyed
	if (feof(data) && stall == NO_STALL && no_targets || SDL_GetKeyboardState(NULL)[QUIT]) {
		return true;
	}
	
	// The first run of this function sets the static variables, as initialisation with these values is a compilation error
	if (first_update) {
		stall = NO_STALL;
		last_spawn = get_time();
		first_update = false;
	}
	
	// New stall data is obtained if the stall variable is set to having no stall value
	if (stall == NO_STALL) {
		fscanf(data, "%d", &stall);
	}
	
	// No arrows will spawn until the stall time (in milliseconds) has passed
	else if (get_time() > last_spawn + (double) stall / STALL_UNIT) {
		stall = NO_STALL;
		spawn(targets, data);
		last_spawn = get_time();
	}
	
	// All target arrows are moved
	for (int i = 0; i < ARROWS; i++) {
		// Target arrows are first separated by direction and then looped through
		for (int j = 0; j < targets->counts[i]; j++) {
			// The position of a target arrow is incremented by its speed multiplied by the time since the last move
			targets->targets[i][j].pos += ARROW_SPEED * (get_time() - targets->targets[i][j].last_move);
			// The target arrow's last move is updated
			targets->targets[i][j].last_move = get_time();
			
			// If the target moved beyond the centre it is moved to the centre
			if (targets->targets[i][j].pos > (WINDOW_WIDTH + ARROW_WIDTH) / 2) {
				targets->targets[i][j].pos = (WINDOW_WIDTH + ARROW_WIDTH) / 2;
			}
			
			// The target's inconstant rect coordinated is set (the setting is dependent on the direction)
			switch (i) {
				case 0:
					targets->targets[i][j].rect.y = targets->targets[i][j].pos - ARROW_HEIGHT / 2;
					break;
					
				case 1:
					targets->targets[i][j].rect.x = targets->targets[i][j].pos - ARROW_WIDTH / 2;
					break;
					
				case 2:
					targets->targets[i][j].rect.y = WINDOW_HEIGHT - (targets->targets[i][j].pos + ARROW_HEIGHT / 2);
					break;
					
				case 3:
					targets->targets[i][j].rect.x = WINDOW_WIDTH - (targets->targets[i][j].pos + ARROW_WIDTH / 2);
					break;
			}
		}
	}
	
	// Target arrows are checked for their eligibility for destruction
	for (int i = 0; i < ARROWS; i++) {
		// Only directions with target arrows are checked
		if (targets->counts[i]) {
			// The first target arrow for each direction is checked for a player miss
			if (targets->targets[i][0].pos == (WINDOW_WIDTH + ARROW_WIDTH) / 2) {
				// If the player missed the target arrow, the maximum score possible is incremented
				player->score[1] += ARROW_WIDTH * ARROW_HEIGHT;
				
				// The target arrow is deleted
				for (int j = 0; j < targets->counts[i] - 1; j++) {
					targets->targets[i][j] = targets->targets[i][j + 1];
				}
				
				// The target buffer is reallocated a smaller portion of memeory and the count is decremented
				targets->targets[i] = realloc(targets->targets[i], sizeof(Target) * --targets->counts[i]);
			}
				
			// If the player pressed a key, the arrow target is checked
			else if (SDL_GetKeyboardState(NULL)[PLAYER_KEYS[i]] || SDL_GetKeyboardState(NULL)[PLAYER_ALT_KEYS[i]]) {
				// If the player wasn't holding down the button from a previous loop
				if (!player->press[i]) {
					// The button is marked as being pressed down, so the player doesn't multi input
					player->press[i] = true;
					// The player's score is incremented by the portion of overlap between their arrow and the target's
					SDL_Rect result;
					
					if (SDL_IntersectRect(&player->rects[i], &targets->targets[i][0].rect, &result)) {
						player->score[0] += result.w * result.h;
					}
					
					// The maximum possible score is incremented
					player->score[1] += ARROW_WIDTH * ARROW_HEIGHT;
					
					// The target arrow is deleted
					for (int j = 0; j < targets->counts[i] - 1; j++) {
						targets->targets[i][j] = targets->targets[i][j + 1];
					}
					
					// The target buffer is reallocated a smaller portion of memeory and the count is decremented
					targets->targets[i] = realloc(targets->targets[i], sizeof(Target) * --targets->counts[i]);
				}
			}
			
			// If the player isn't pressing the button any longer it is noted
			else if (player->press[i]) {
				player->press[i] = false;
			}
		}
	}
	
	// The player and then target arrows are blitted to the display
	for (int i = 0; i < ARROWS; i++) {
		SDL_BlitSurface(player->arrows[i], NULL, display, &player->rects[i]);
		
		for (int j = 0; j < targets->counts[i]; j++) {
			SDL_BlitSurface(targets->arrows[i], NULL, display, &targets->targets[i][j].rect);
		}
	}
	
	// The game doesn't end under normal circumstances
	return false;
}

int main(int argc, char* argv[]) {
	// The PRNG is seeded
	srand(time(NULL));
	// The first numer is discarded to give randomness in close time periods
	rand();
	
	// The player's constant variables are set
	Player player = {
		.arrows = {
			SDL_LoadBMP(PLAYER_ARROWS[0]),
			SDL_LoadBMP(PLAYER_ARROWS[1]),
			SDL_LoadBMP(PLAYER_ARROWS[2]),
			SDL_LoadBMP(PLAYER_ARROWS[3])
		},
		.rects = {
			{
				.x = PLAYER_RECT_DATA[0][0],
				.y = PLAYER_RECT_DATA[0][1],
				.w = ARROW_WIDTH,
				.h = ARROW_HEIGHT
			},
			{
				.x = PLAYER_RECT_DATA[1][0],
				.y = PLAYER_RECT_DATA[1][1],
				.w = ARROW_WIDTH,
				.h = ARROW_HEIGHT
			},
			{
				.x = PLAYER_RECT_DATA[2][0],
				.y = PLAYER_RECT_DATA[2][1],
				.w = ARROW_WIDTH,
				.h = ARROW_HEIGHT
			},
			{
				.x = PLAYER_RECT_DATA[3][0],
				.y = PLAYER_RECT_DATA[3][1],
				.w = ARROW_WIDTH,
				.h = ARROW_HEIGHT
			}
		}
	};
	
	// The target arrow container's constant varialbes are set
	Targets targets = {
		.arrows = {
			SDL_LoadBMP(TARGET_ARROWS[0]),
			SDL_LoadBMP(TARGET_ARROWS[1]),
			SDL_LoadBMP(TARGET_ARROWS[2]),
			SDL_LoadBMP(TARGET_ARROWS[3])
		}
	};
	
	// The player score and maximum score are initialised to zero
	for (int i = 0; i < SCORES; i++) {
		player.score[i] = 0;
	}
	
	// The player and targets's directional variables are set to zero
	for (int i = 0; i < ARROWS; i++) {
		player.press[i] = false; // The player isn't pressing any keys
		targets.targets[i] = NULL; // The target arrow conatiner has no targets
		targets.counts[i] = 0; // The target arrow container has no targets
	}
	
	// The level is stored in a string
	char* level = malloc(MAX_LEVEL);
	
	// If a command line argument is passed, the specific song is loaded
	if (argc > 1) {
		strcpy(level, argv[1]);
	}
	
	// Otherwise, a random level is loaded
	else {
		sprintf(level, "%d", rand_int(0, LEVELS - 1));
	}
	
	// The filenames for the song and data files are stored in a buffer
	char* data_name = malloc(MAX_DATA);
	char* song_name = malloc(MAX_SONG);
	
	// The filenames are filled with the correct subfdirectory according to the level
	strcpy(data_name, "data/");
	strcat(data_name, level);
	free(level);
	strcpy(song_name, data_name);
	strcat(data_name, "/dat.ddr");
	strcat(song_name, "/song.wav");
	
	// The data file is opened and the buffer used to open it is freed
	FILE* data = fopen(data_name, "r");
	free(data_name);
	
	// The song file is loaded and the buffer used to load it is freed
	SDL_AudioSpec song;
	Uint8* song_buffer;
	Uint32 song_length;
	SDL_LoadWAV(song_name, &song, &song_buffer, &song_length);
	free(song_name);
	SDL_OpenAudio(&song, NULL);
	SDL_QueueAudio(1, song_buffer, song_length);
	
	// The song title is obtained from the first line of the data file
	char* song_title = malloc(MAX_SONG_TITLE);
	fgets(song_title, MAX_SONG_TITLE, data);
	// The window's title is the game's name combined with the song's title
	char* title = malloc(MAX_TITLE);
	strcpy(title, TITLE);
	strcat(title, song_title);
	free(song_title);
	// The window is created with the song's data file title and the buffer for the title is freed
	SDL_Window* window = SDL_CreateWindow(
		title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN
	);
	free(title);
	// Tne window's display surface is stored
	SDL_Surface* display = SDL_GetWindowSurface(window);
	
	// The start time is recorded and the music starts off paused
	double start = get_time();
	bool paused = true;
	
	while (true) {
		// The background is filled in black
		SDL_FillRect(display, NULL, SDL_MapRGB(display->format, BACKGROUND[0], BACKGROUND[1], BACKGROUND[2]));
		
		// The game is updated and ended at the the appropiate point
		// If the player wants to quit they can with escape
		if (update(&player, &targets, data, display)) {
			break;
		}
		
		// If enough time has elapsed for the arrows and song to be in sync, the song starts to play
		if (paused && get_time() > start + get_delay()) {
			SDL_PauseAudio(false);
			paused = false;
		}
		
		// The window's display and keyboard input are updated
		SDL_UpdateWindowSurface(window);
		SDL_PumpEvents();
	}
	
	if (!player.score[1]) {
		player.score[1]++;
	}
	
	// The score is displayed as a percentage
	printf("Score: %f%c\n", (double) MAX_SCORE * player.score[0] / player.score[1], PERCENT);
	
	while (!SDL_GetKeyboardState(NULL)[QUIT]) {
		SDL_PumpEvents();
	}
}