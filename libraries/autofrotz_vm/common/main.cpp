/* main.c - Frotz V2.40 main function
 *	Copyright (c) 1995-1997 Stefan Jokisch
 *
 * This file is part of Frotz.
 *
 * Frotz is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Frotz is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

/*
 * This is an interpreter for Infocom V1 to V6 games. It also supports
 * the recently defined V7 and V8 games. Please report bugs to
 *
 *    s.jokisch@avu.de
 *
 */

#include "frotz.h"

#ifndef MSDOS_16BIT
#define cdecl
#endif

extern void interpret (void);
extern void init_memory (void);
extern void init_undo (void);
extern void init_buffer (void);
extern void init_process (void);
extern void init_sound (void);
extern void reset_memory (void);

#ifdef AUTOFROTZ
vmlocal autofrotz::vmlink::VmLink *vmLink = nullptr;
#endif

/* Story file name, id number and size */

vmlocal const char *story_name = 0;

vmlocal enum story story_id = UNKNOWN;
vmlocal long story_size = 0;

/* Story file header data */

vmlocal zbyte h_version = 0;
vmlocal zbyte h_config = 0;
vmlocal zword h_release = 0;
vmlocal zword h_resident_size = 0;
vmlocal zword h_start_pc = 0;
vmlocal zword h_dictionary = 0;
vmlocal zword h_objects = 0;
vmlocal zword h_globals = 0;
vmlocal zword h_dynamic_size = 0;
vmlocal zword h_flags = 0;
vmlocal zbyte h_serial[6] = { 0, 0, 0, 0, 0, 0 };
vmlocal zword h_abbreviations = 0;
vmlocal zword h_file_size = 0;
vmlocal zword h_checksum = 0;
vmlocal zbyte h_interpreter_number = 0;
vmlocal zbyte h_interpreter_version = 0;
vmlocal zbyte h_screen_rows = 0;
vmlocal zbyte h_screen_cols = 0;
vmlocal zword h_screen_width = 0;
vmlocal zword h_screen_height = 0;
vmlocal zbyte h_font_height = 1;
vmlocal zbyte h_font_width = 1;
vmlocal zword h_functions_offset = 0;
vmlocal zword h_strings_offset = 0;
vmlocal zbyte h_default_background = 0;
vmlocal zbyte h_default_foreground = 0;
vmlocal zword h_terminating_keys = 0;
vmlocal zword h_line_width = 0;
vmlocal zbyte h_standard_high = 1;
vmlocal zbyte h_standard_low = 0;
vmlocal zword h_alphabet = 0;
vmlocal zword h_extension_table = 0;
vmlocal zbyte h_user_name[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

vmlocal zword hx_table_size = 0;
vmlocal zword hx_mouse_x = 0;
vmlocal zword hx_mouse_y = 0;
vmlocal zword hx_unicode_table = 0;

/* Stack data */

vmlocal zword stack[STACK_SIZE];
vmlocal zword *sp = 0;
vmlocal zword *fp = 0;
vmlocal zword frame_count = 0;

/* IO streams */

vmlocal bool ostream_screen = TRUE;
vmlocal bool ostream_script = FALSE;
vmlocal bool ostream_memory = FALSE;
vmlocal bool ostream_record = FALSE;
vmlocal bool istream_replay = FALSE;
vmlocal bool message = FALSE;

/* Current window and mouse data */

vmlocal int cwin = 0;
vmlocal int mwin = 0;

vmlocal int mouse_y = 0;
vmlocal int mouse_x = 0;

/* Window attributes */

vmlocal bool enable_wrapping = FALSE;
vmlocal bool enable_scripting = FALSE;
vmlocal bool enable_scrolling = FALSE;
vmlocal bool enable_buffering = FALSE;

/* User options */

/*
vmlocal int option_attribute_assignment = 0;
vmlocal int option_attribute_testing = 0;
vmlocal int option_context_lines = 0;
vmlocal int option_object_locating = 0;
vmlocal int option_object_movement = 0;
vmlocal int option_left_margin = 0;
vmlocal int option_right_margin = 0;
vmlocal int option_ignore_errors = 0;
vmlocal int option_piracy = 0;
vmlocal int option_undo_slots = MAX_UNDO_SLOTS;
vmlocal int option_expand_abbreviations = 0;
vmlocal int option_script_cols = 80;
vmlocal int option_save_quetzal = 1;
*/

vmlocal int option_sound = 1;
vmlocal char *option_zcode_path;


/* Size of memory to reserve (in bytes) */

vmlocal long reserve_mem = 0;

/*
 * z_piracy, branch if the story file is a legal copy.
 *
 *	no zargs used
 *
 */

void z_piracy (void)
{

    branch (!f_setup.piracy);

}/* z_piracy */

/*
 * main
 *
 * Prepare and run the game.
 *
 */

#ifdef AUTOFROTZ
int common_main (autofrotz::vmlink::VmLink *vmLink)
#else
int cdecl main (int argc, char *argv[])
#endif
{

#ifdef AUTOFROTZ
    DPRE(!::vmLink, "a VM has already been created (and more than one is not supported)");
    ::vmLink = vmLink;

    int argc = 0;
    char **argv = nullptr;
#endif

    os_init_setup ();

    os_process_arguments (argc, argv);

    init_buffer ();

    init_err ();

    init_memory ();

    init_process ();

    init_sound ();

    os_init_screen ();

    init_undo ();

    z_restart ();

    interpret ();

    reset_memory ();

    os_reset_screen ();

    return 0;

}/* main */
