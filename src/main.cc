//------------------------------------------------------------------------
//  MAIN PROGRAM
//------------------------------------------------------------------------
//
//  Visplane Explorer
//
//  Copyright (C) 2012-2014 Andrew Apted
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//------------------------------------------------------------------------

#define AJ_ARG_LIB_IMPLEMENTATION

#include "headers.h"


// use Sean Barrett's PNG writing code
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


bool want_quit;

const char * current_file;
const char * current_map;

time_t  current_file_time;

game_kind_e  current_game;

// toggled by a menu option
bool want_open_doors = false;
bool want_heat_map = false;

bool paused = false;


bool file_has_been_modified;

static bool map_is_valid;


bool new_color_scheme = true;

#define REFRESH_TIME  1.0  // seconds


static const int angles_to_check[8] =
{
	0, 90, 180, 270,   /* four main compass directions: E, N, W, S */

	45, 135, 225, 315  /* the other compass dirs: NE, NW, SW, SE */

	/* 22, 67, 112, 157, 202, 247, 292, 337 */
};


int limit_of_visplanes = 128;
int limit_of_drawsegs  = 256;
int limit_of_openings  = 320 * 64;
int limit_of_solidsegs = 32;


#define VIEWHEIGHT  41
#define MAXBOB       8


// test a spot using current settings.
// returns a packed datum_t value (or a special DATUM_XXX value),
// but never DATUM_UNSET.
datum_t Main_TestSpot(int x, int y)
{
	int height = VIEWHEIGHT + MAXBOB;

	int num_visplanes = 0;
	int num_drawsegs  = 0;
	int num_openings  = 0;
	int num_solidsegs = 0;

	int result = VPO_TestSpot(x, y, height, angles_to_check[0],
	                          &num_visplanes, &num_drawsegs,
							  &num_openings,  &num_solidsegs);

	if (result == RESULT_IN_VOID)
		return DATUM_VOID;

	// closed door (or low window) -- use black for this
	if (result == RESULT_BAD_Z)
		return DATUM_CLOSED;

	if (result == RESULT_OVERFLOW)
		return DATUM_OVERFLOW;

	for (int a = 1 ; a < 8 ; a++)
	{
		result = VPO_TestSpot(x, y, height, angles_to_check[a],
							  &num_visplanes, &num_drawsegs,
							  &num_openings,  &num_solidsegs);

		if (result == RESULT_OVERFLOW)
			return DATUM_OVERFLOW;
	}

	// clamp values and pack into a datum_t
	num_visplanes = MIN(250, 1 + num_visplanes);
	num_drawsegs  = MIN(250, 1 + (num_drawsegs + 1) / 2);
	num_openings  = MIN(250, 1 + (num_openings + 159) / 160);
	num_solidsegs = MIN(250, 1 + num_solidsegs * 4);

	datum_t d =  num_visplanes |
	            (num_drawsegs  << 8)  |
	            (num_openings  << 16) |
				(num_solidsegs << 24);

	return d;
}


static time_t QueryFileTime(const char *filename)
{
	// NOTE: under Win32 the modification time of a file is NOT updated
	//       until the file handle is CLOSED (as mentioned in the MSDN
	//       article about file times, and confirmed for Windows XP).
	//       
	//       But editor programs, including Eureka, may keep the file
	//       open all the time.  Hence we CANNOT detect when the editor
	//       has modified the wad file (via this method).

	struct stat  info;

	if (stat(filename, &info) != 0)
		return (time_t) -1;

	return info.st_mtime;
}


void Main_Shutdown(bool error)
{
	if (main_win)
	{
		delete main_win;

		main_win = NULL;
	}

	current_file = NULL;
	current_map  = NULL;

	map_is_valid = false;
}


void Main_FatalError(const char *msg, ...)
{
	static char buffer[MSG_BUF_LEN];

	static bool shown_error = false;

	va_list arg_pt;

	va_start(arg_pt, msg);
	vsnprintf(buffer, MSG_BUF_LEN-1, msg, arg_pt);
	va_end(arg_pt);

	buffer[MSG_BUF_LEN-2] = 0;

	// show an error dialog, but guard against infinite error loops
	if (! shown_error)
	{
		shown_error = true;

		fl_alert("Fatal Error: %s", buffer);
	}

	fprintf(stderr, "ERROR: %s\n", buffer);

	Main_Shutdown(true);

	exit(9);
}


/* this is only to prevent ESCAPE key from quitting */
int Main_key_handler(int event)
{
	if (event != FL_SHORTCUT)
		return 0;

	if (Fl::event_key() == FL_Escape)
	{
		fl_beep();
		return 1;
	}

	return 0;
}


void Main_OpenWindow()
{
	Fl::visual(FL_RGB);

	if (new_color_scheme)
	{
		Fl::background(236, 232, 228);
		Fl::background2(255, 255, 255);
		Fl::foreground(0, 0, 0);

		Fl::scheme("plastic");
	}

#if 0  // debug
	int screen_w = Fl::w();
	int screen_h = Fl::h();

	fprintf(stderr, "Screen dimensions = %dx%d\n", screen_w, screen_h);
#endif

	fl_message_font(FL_HELVETICA /* _BOLD */, 18);

	// load icons for file chooser
#ifndef WIN32
	Fl_File_Icon::load_system_icons();
#endif


	// create the FLTK window

	main_win = new UI_MainWin();

	// show the window (pass some dummy arguments)
	{
		char *argv[2];

		argv[0] = strdup("visplane-explorer.exe");
		argv[1] = NULL;

		main_win->show(1 /* argc */, argv);
	}

    // kill the stupid bright background of the "plastic" scheme
	if (new_color_scheme)
    {
		delete Fl::scheme_bg_;
		Fl::scheme_bg_ = NULL;

		main_win->image(NULL);
    }

	main_win->UpdateMemory(0);
	main_win->UpdateProgress(0);

	Fl::add_handler(Main_key_handler);

	// run GUI for half a sec
	Fl::wait(0.1); Fl::wait(0.1);
	Fl::wait(0.1); Fl::wait(0.1);
}


void Main_SaveImage()
{
	Fl_Native_File_Chooser  chooser;

	chooser.title("Pick file to save to");
	chooser.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
	chooser.filter("PNG images\t*.png");

	switch (chooser.show())
	{
		case -1:
			fl_alert("Unable to save the image:\n\n%s", chooser.errmsg());
			return;

		case 1:  // cancelled
			return;

		default:  // OK
			break;
	}

	static char filename[FL_PATH_MAX + 10];

	strcpy(filename, chooser.filename());

	// add extension if missing
	char *pos = (char *)fl_filename_ext(filename);
	if (! *pos)
		strcat(filename, ".png");

	// check if can write
	FILE *fp = fopen(filename, "wb");
	if (! fp)
	{
		int err_num = errno;
		fl_alert("Unable to save the image:\n\n%s", strerror(err_num));
		return;
	}
	fclose(fp);

	// write the PNG file
	int image_w, image_h;

	byte *data = main_win->canvas->getScreenData(&image_w, &image_h);

	stbi_write_png(filename, image_w, image_h, 3 /* RGB */, data, image_w * 3);
}


void Main_ProcessTiles()
{
	main_win->canvas->ProcessOneTile();
}


void Main_Refresh(void *)
{
	if (map_is_valid)
	{
		main_win->canvas->Refresh();

		int kbytes = main_win->canvas->calcTileMemory();

		main_win->UpdateMemory(kbytes);

		float prog = main_win->canvas->calcProgress();

		main_win->UpdateProgress(prog);
	}

	if (map_is_loaded)
	{
		// check if file has been modified
		if (QueryFileTime(current_file) != current_file_time)
			file_has_been_modified = true;
	}

	Fl::repeat_timeout(REFRESH_TIME, Main_Refresh);
}


void Main_SetGame(game_kind_e kind, bool update_menu)
{
	current_game = kind;

	switch (current_game)
	{
		case GAME_Doom:
			limit_of_visplanes = 128;
			break;

		case GAME_Hexen:
			limit_of_visplanes = 160;
			break;

		case GAME_Strife:
			limit_of_visplanes = 200;
			break;
	}

	M_CreatePalettes();

	if (update_menu)
		Menu_SetGame(current_game);
}


bool Main_TryLoadWAD(const char *filename)
{
	VPO_FreeWAD();

	current_file = NULL;
	current_map  = NULL;

	map_is_valid = false;
	file_has_been_modified = false;

	main_win->canvas->KillTiles();
	main_win->canvas->ClearMessage();

	if (VPO_LoadWAD(filename) != 0)
	{
		fl_alert("%s", VPO_GetError());
		return false;
	}

	if (! VPO_GetMapName(0))
	{
		fl_alert("No maps found in this WAD.");
		return false;
	}

	// can now load a map
	current_file = strdup(filename);

	main_win->UpdateTitle();

	// get modification time
	current_file_time = QueryFileTime(current_file);

	return true;
}


bool Main_TrySetMap(const char *map_name, bool go_home)
{
	VPO_CloseMap();

	current_map = strdup(map_name);

	main_win->canvas->KillTiles();
	main_win->canvas->ClearMessage();

	bool is_hexen;

	if (VPO_OpenMap(map_name, &is_hexen) != 0)
	{
		main_win->canvas->SetMessage(VPO_GetError());
	}
	else
	{
		map_is_valid = true;

		if (go_home)
			main_win->canvas->GoHome();

		// we cannot distinguish between DOOM and STRIFE, but we can distinguish
		// between HEXEN and the other two, so switch game if Hexen-ness changes.

		if (is_hexen && current_game != GAME_Hexen)
			Main_SetGame(GAME_Hexen);
		else if (!is_hexen && current_game == GAME_Hexen)
			Main_SetGame(GAME_Doom);

		if (want_open_doors)
			VPO_OpenDoorSectors(+1);
	}

	main_win->canvas->MapWasLoaded();

	main_win->UpdateTitle();


	return true;
}


// convert a plain number like "3" into a map name like "MAP03".
// requires a WAD file to be loaded already.
// returns NULL if conversion fails.

const char * Main_ConvertMapNumber(const char *num_str, const char *num_2)
{
	static char buffer[32];

	int number = atoi(num_str);

	if (number < 1)
		return NULL;

	// check first if wad uses MAPxx or ExMx style
	const char *first_map = VPO_GetMapName(0);

	if (first_map && strcmp(first_map, "TITLEMAP") == 0)
		first_map = VPO_GetMapName(1);

	if (first_map && toupper(first_map[0]) == 'E')
	{
		// this logic matches vanilla DOOM 1

		int episode = number;
		if (episode > 9)
			return NULL;

		if (num_2)
			number = atoi(num_2);
		else
			number = 1;

		if (number < 1 || number > 99)
			return NULL;

		sprintf(buffer, "E%dM%d", episode, number % 10);
		return buffer;
	}

	// DOOM 2 mode : any second number is ignored

	if (number > 999)
		return NULL;
	
	sprintf(buffer, "MAP%02d", number);
	return buffer;
}


bool Main_ReloadMap()
{
	if (! current_file)
	{
		fl_beep();
		return false;
	}

	const char *last_map = current_map;

	if (! Main_TryLoadWAD(current_file))
		return false;
	
	return Main_TrySetMap(last_map, false /* go_home */); 
}


// look for next map (if dir > 0) or previous map (dir < 0), load it if exists
void Main_NextMap(int dir)
{
	if (! current_file || ! current_map)
	{
		fl_beep();
		return;
	}

	int num_levels;
	int found_idx = -1;

	for (num_levels = 0 ; ; num_levels++)
	{
		const char *name = VPO_GetMapName(num_levels);

		if (! name)
			break;

		if (strcasecmp(name, current_map) == 0)
			found_idx = num_levels;
	}

	// not found ?? (this should never happen)
	if (found_idx < 0)
	{
		fl_beep();
		return;
	}

	int new_idx = found_idx + ((dir > 0) ? 1 : -1);

	if (new_idx < 0 || new_idx >= num_levels)
	{
		fl_beep();
		return;
	}

	Main_TrySetMap(VPO_GetMapName(new_idx));
}


//------------------------------------------------------------------------


void Main_Loop()
{
	// run the GUI until the user quits
	while (! want_quit)
	{
		if (! paused)
			Main_ProcessTiles();

		Fl::wait((paused || ! map_is_valid) ? 0.2 : 0);

		if (main_win->canvas->getScreenData())
		{
			Main_SaveImage();
			main_win->canvas->clearScreenData();
		}
	}
}


static void ShowHelp()
{
	printf(
		"*\n"
		"**\n"
		"*** " VISEXP_TITLE " " VISEXP_VERSION " (C) 2009-2014 Andrew Apted\n"
		"**\n"
		"*\n"
	);

	printf(
		"This program is free software, under the terms of the GNU General\n"
		"Public License (GPL), and comes with ABSOLUTELY NO WARRANTY.\n"
		"\n"
		"Home page: http://vis-explorer.sourceforge.net/\n"
		"\n"
	);

	printf(
		"USAGE: visplane-explorer [options...] [FILE]\n"
		"\n"

		"Available options:\n"
		"  -f  -file   <wad>    specify wad file to load\n"
		"  -w  -warp   <map>    specify map to visualize\n"
		"  -s  -strife          set game to Strife\n"
		"\n"
	);
}


static void ParseArgs(const char *first_arg)
{
	int index;
	int num_param;

	const char * filename = NULL;
	const char * map_name = NULL;
	const char * map_2    = NULL;

	// very first argument can be a filename (drag'n'drop).
	// but -file option will override.

	index = aj_arg_Find('f', "file", &num_param);

	if (index >= 0)
	{
		if (num_param == 0)
		{
			fl_alert("Missing filename after -file\n");
			return;
		}
		
		filename = aj_arg_Param(index, 0);
	}
	else if (first_arg && first_arg[0] != '-')
	{
		filename = first_arg;
	}

	index = aj_arg_Find('w', "warp", &num_param);

	if (index >= 0)
	{
		if (num_param == 0)
		{
			fl_alert("Missing map after -warp\n");
			return;
		}

		if (! filename)
		{
			fl_alert("-warp used without a specified file\n");
			return;
		}

		map_name = aj_arg_Param(index, 0);

		if (num_param >= 2)
			map_2 = aj_arg_Param(index, 1);
	}

	// try to load the specified file...

	if (filename)
	{
		if (! Main_TryLoadWAD(filename))
			return;

		if (! map_name)
		{
			map_name = VPO_GetMapName(0);

			// this cannot happen (already checked)
			if (! map_name)
				return;
		}

		if (isdigit(map_name[0]))
		{
			///  const char *orig = map_name;

			map_name = Main_ConvertMapNumber(map_name, map_2);

			if (! map_name)
			{
				fl_alert("Invalid map number after -warp\n");
				return;
			}
		}

		Main_TrySetMap(map_name);
	}

	// args for controlling the game kind.
	// we only really need something for Strife, since the default is Doom and
	// Hexen can be automatically detected.

	if (aj_arg_Exists('s', "strife"))
	{
		Main_SetGame(GAME_Strife);
	}
}


int main(int argc, char **argv)
{
	aj_arg_Init(argc-1, (const char **) (argv+1));

	if (aj_arg_Exists('h', "help") ||
		aj_arg_Exists(0,   "version"))
	{
		ShowHelp();
		return 0;
	}

	Main_OpenWindow();

	ParseArgs(argv[1]);

	// ensure we have a valid palette
	Main_SetGame(current_game, false /* update_menu */);

	Fl::add_timeout(REFRESH_TIME, Main_Refresh);

	if (! map_is_loaded)
		DLG_OpenMap();

	Main_Loop();

	return 0;
}


//--- editor settings ---
// vi:ts=4:sw=4:noexpandtab
