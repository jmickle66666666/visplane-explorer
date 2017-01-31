//------------------------------------------------------------------------
//  Menu Bar
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

#include "headers.h"


class UI_AskLocation : public Fl_Double_Window
{
private:
	Fl_Int_Input *pos_x;
	Fl_Int_Input *pos_y;

	Fl_Button *ok_but;
	Fl_Button *cancel_but;

	bool want_close;

private:
	static void ok_callback(Fl_Widget *w, void *data)
	{
		UI_AskLocation * that = (UI_AskLocation *)data;

		int want_x = atoi(that->pos_x->value());
		int want_y = atoi(that->pos_y->value());

		want_x = CLAMP(-20000, want_x, 20000);
		want_y = CLAMP(-20000, want_y, 20000);

		main_win->canvas->MoveOrigin(want_x, want_y);

		// zoom all the way in
		for (int i = 0 ; i < 30 ; i++)
		{
			main_win->canvas->Zoom(+1);
		}

		that->want_close = true;
	}

	static void close_callback(Fl_Widget *w, void *data)
	{
		UI_AskLocation * that = (UI_AskLocation *)data;

		that->want_close = true;
	}

public:
	UI_AskLocation() :
		Fl_Double_Window(340, 205, "Location Dialog"),
		want_close(false)
	{
		Fl_Box *title = new Fl_Box(10, 11, w() - 20, 32, "Enter the new coordinates:");
		title->labelsize(16);
		title->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

		pos_x = new Fl_Int_Input(95, 55, 65, 25,  "x:");
		pos_y = new Fl_Int_Input(95, 85, 65, 25,  "y:");

		/// pos_x->value("0");
		/// pos_y->value("0");

		Fl_Group * grp = new Fl_Group(0, h() - 70, w(), 70);
		grp->box(FL_FLAT_BOX);
		grp->color(WINDOW_BG, WINDOW_BG);
		{
			cancel_but = new Fl_Button(30, grp->y() + 20, 95, 30, "Cancel");
			cancel_but->callback(close_callback, this);
		
			ok_but = new Fl_Button(225, grp->y() + 20, 95, 30, "Go!");
			ok_but->labelfont(FL_HELVETICA_BOLD);
			ok_but->callback(ok_callback, this);

			grp->end();
		}

		end();

		resizable(NULL);

		callback(close_callback, this);
	}

	virtual ~UI_AskLocation()
	{ }

	void Run()
	{
		set_modal();

		show();

		while (! want_close)
		{
			Fl::wait(0.2);
		}
	}
};


//------------------------------------------------------------------------
//  FILE MENU
//------------------------------------------------------------------------

static void file_do_quit(Fl_Widget *w, void * data)
{
	want_quit = true;
}

static void file_do_open(Fl_Widget *w, void * data)
{
	DLG_OpenMap();
}

static void file_do_reload(Fl_Widget *w, void * data)
{
	Main_ReloadMap();
}

static void file_do_next(Fl_Widget *w, void * data)
{
	Main_NextMap(+1);
}

static void file_do_prev(Fl_Widget *w, void * data)
{
	Main_NextMap(-1);
}

static void file_do_save(Fl_Widget *w, void * data)
{
	main_win->canvas->Screenshot();
}


//------------------------------------------------------------------------
//  VIEW MENU
//------------------------------------------------------------------------

static void view_do_heat_map(Fl_Widget *w, void * data)
{
	const Fl_Menu_Item *item = ((Fl_Menu_*)w)->mvalue();

	want_heat_map = item->value() ? true : false;

	M_CreatePalettes();
}

static void view_do_draw_map(Fl_Widget *w, void * data)
{
	main_win->canvas->ToggleMap();
}

static void view_do_seg_splits(Fl_Widget *w, void * data)
{
	main_win->canvas->ToggleSplits();
}

static void view_do_open_doors(Fl_Widget *w, void * data)
{
	const Fl_Menu_Item *item = ((Fl_Menu_*)w)->mvalue();

	// set this for future map loads
	want_open_doors = item->value() ? true : false;

	int dir = item->value() ? +1 : -1;

	if (map_is_loaded)
	{
		VPO_OpenDoorSectors(dir);

		// restart vis from scratch
		main_win->canvas->KillTiles();
		main_win->canvas->MapWasLoaded();
	}
}


static void view_do_paused(Fl_Widget *w, void * data)
{
	const Fl_Menu_Item *item = ((Fl_Menu_*)w)->mvalue();

	paused = item->value() ? true : false;

	main_win->UpdateProgress(-1);
}


static void view_do_zoom_in(Fl_Widget *w, void * data)
{
	main_win->canvas->Zoom(+1);
}

static void view_do_zoom_out(Fl_Widget *w, void * data)
{
	main_win->canvas->Zoom(-1);
}

static void view_do_whole_map(Fl_Widget *w, void * data)
{
	main_win->canvas->GoHome();
}

static void view_do_goto_loc(Fl_Widget *w, void * data)
{
	UI_AskLocation *dialog = new UI_AskLocation();

	dialog->Run();

	delete dialog;
}


//------------------------------------------------------------------------
//  MODE MENU
//------------------------------------------------------------------------

static void mode_do_visplanes(Fl_Widget *w, void * data)
{
	main_win->canvas->SetMode('v');
}

static void mode_do_drawsegs(Fl_Widget *w, void * data)
{
	main_win->canvas->SetMode('d');
}

static void mode_do_solidsegs(Fl_Widget *w, void * data)
{
	main_win->canvas->SetMode('s');
}


//------------------------------------------------------------------------
//  GAME MENU
//------------------------------------------------------------------------

static void game_do_Doom(Fl_Widget *w, void * data)
{
	Main_SetGame(GAME_Doom, false /* update_menu */);
}

static void game_do_Hexen(Fl_Widget *w, void * data)
{
	Main_SetGame(GAME_Hexen, false);
}

static void game_do_Strife(Fl_Widget *w, void * data)
{
	Main_SetGame(GAME_Strife, false);
}


//------------------------------------------------------------------------
//  HELP MENU
//------------------------------------------------------------------------

void help_do_about(Fl_Widget *w, void * data)
{
	DLG_AboutText();
}


void help_do_cheatsheet(Fl_Widget *w, void * data)
{
	DLG_CheatSheet();
}


//------------------------------------------------------------------------

#undef  FCAL
#define FCAL  (Fl_Callback *)

static Fl_Menu_Item menu_items[] = 
{
	{ "&File", 0, 0, 0, FL_SUBMENU },

		{ "&Open Map",       FL_COMMAND + 'o', FCAL file_do_open },
		{ "&Reload Map",                  'r', FCAL file_do_reload },
		{ "", 0, 0, 0, FL_MENU_DIVIDER|FL_MENU_INACTIVE },

		{ "&Next Map",          FL_COMMAND + 'n', FCAL file_do_next },
		{ "&Previous Map   ",   FL_COMMAND + 'p', FCAL file_do_prev },
		{ "", 0, 0, 0, FL_MENU_DIVIDER|FL_MENU_INACTIVE },

		{ "&Save Image   ",  FL_COMMAND + 's', FCAL file_do_save },
		{ "", 0, 0, 0, FL_MENU_DIVIDER|FL_MENU_INACTIVE },

		{ "&Quit",      FL_COMMAND + 'q', FCAL file_do_quit },
		{ 0 },

	{ "&View", 0, 0, 0, FL_SUBMENU },

		{ "&Visplanes",   'v', FCAL mode_do_visplanes, 0, FL_MENU_RADIO|FL_MENU_VALUE },
		{ "&Draw-segs",   'd', FCAL mode_do_drawsegs,  0, FL_MENU_RADIO },
		{ "&Solid-segs",  's', FCAL mode_do_solidsegs, 0, FL_MENU_RADIO },

		{ "", 0, 0, 0, FL_MENU_DIVIDER|FL_MENU_INACTIVE },

		{ "Paused",      FL_Pause, FCAL view_do_paused, 0, FL_MENU_TOGGLE },

		{ "", 0, 0, 0, FL_MENU_DIVIDER|FL_MENU_INACTIVE },

		{ "2D &Map",      'm', FCAL view_do_draw_map,   0, FL_MENU_TOGGLE|FL_MENU_VALUE },
		{ "Seg s&plits",  'p', FCAL view_do_seg_splits, 0, FL_MENU_TOGGLE },
		{ "&Heat colors", 'h', FCAL view_do_heat_map,   0, FL_MENU_TOGGLE },
		{ "&Open doors",  'o', FCAL view_do_open_doors, 0, FL_MENU_TOGGLE },

		{ "", 0, 0, 0, FL_MENU_DIVIDER|FL_MENU_INACTIVE },

		{ "Zoom In",     '+', FCAL view_do_zoom_in },
		{ "Zoom Out",    '-', FCAL view_do_zoom_out },
		{ "&Whole Map",  FL_Home, FCAL view_do_whole_map },

		{ "", 0, 0, 0, FL_MENU_DIVIDER|FL_MENU_INACTIVE },

		{ "&Go to Location...  ", 'g', FCAL view_do_goto_loc },
		{ 0 },

	{ "&Game", 0, 0, 0, FL_SUBMENU },
		{ "&Doom    ",     0,  FCAL game_do_Doom,   0, FL_MENU_RADIO|FL_MENU_VALUE },
		{ "&Hexen    ",    0,  FCAL game_do_Hexen,  0, FL_MENU_RADIO },
		{ "&Strife    ",   0,  FCAL game_do_Strife, 0, FL_MENU_RADIO },
		{ 0 },

	{ "&Help", 0, 0, 0, FL_SUBMENU },
		{ "&About...  ",           0,  FCAL help_do_about },
		{ "&Cheat Sheet   ",  FL_F+1,  FCAL help_do_cheatsheet },
		{ 0 },

	{ 0 }
};


Fl_Sys_Menu_Bar * Menu_Create(int x, int y, int w, int h)
{
	Fl_Sys_Menu_Bar *bar = new Fl_Sys_Menu_Bar(x, y, w, h);
	bar->menu(menu_items);
	return bar;
}


void Menu_SetGame(game_kind_e kind)
{
	const Fl_Menu_Item * item = NULL;
	
	switch (kind)
	{
		case GAME_Doom:
			item = main_win->menu->find_item(FCAL game_do_Doom);
			break;

		case GAME_Hexen:
			item = main_win->menu->find_item(FCAL game_do_Hexen);
			break;

		case GAME_Strife:
			item = main_win->menu->find_item(FCAL game_do_Strife);
			break;
	}

	// this should not happen
	if (! item)
		return;

	((Fl_Menu_Item *)item)->setonly();
}


//--- editor settings ---
// vi:ts=4:sw=4:noexpandtab
