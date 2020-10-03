/*
 *  calendar home widget for the maemo desktop.
 *  Copyright (C) 2010 Nicolai Hess
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <gtk/gtk.h>
#include "cal-home-load-and-store.h"

#define CAL_HP_KEY_VISIBLE_CALS "visible_calendars"
#define CAL_HP_KEY_LAUNCH_VIEW "launch_view"
#define CAL_HP_KEY_CAL_ROW_COUNT "cal_row_count"
#define CAL_HP_KEY_CAL_TIME_PERIOD "cal_time_period"
#define CAL_HP_KEY_CAL_SHOW_AGE "cal_show_age"
#define CAL_HP_KEY_CAL_UPDATE_IF_VISIBLE "update_if_visible"
#define CAL_HP_KEY_CAL_SHOW_ALL_RECURRENCES "show_all_recurrences"
#define CAL_HP_KEY_CAL_SHOW_OLD_UNDONE_TASKS "show_old_undone_tasks"
#define CAL_HP_KEY_CAL_SHOW_DONE_TASKS "show_done_tasks"
#define CAL_HP_KEY_CAL_SHOW_WEEK_DAY_NAME "show_week_day_name"
#define CAL_HP_KEY_CAL_VISIBLE_ENTRY_MODE "visible_entry_mode"
#define CAL_HOME_PLUGIN_SETTINGS_FILE "/.cal_home_widget"

void 
cal_home_read_settings(CalHomePlugin *desktop_plugin)
{
  GKeyFile *keyFile;
  gchar* fileName;
  gboolean fileExists;

  keyFile = g_key_file_new();

  fileName = g_strconcat(g_get_home_dir(), CAL_HOME_PLUGIN_SETTINGS_FILE, NULL);

  fileExists = g_key_file_load_from_file (keyFile, fileName, G_KEY_FILE_KEEP_COMMENTS, NULL);

  if(fileExists)
  {
    if(g_key_file_has_key(keyFile, desktop_plugin->iD, CAL_HP_KEY_VISIBLE_CALS, NULL))
    {
      g_free(desktop_plugin->visible_cals);
      desktop_plugin->visible_cals = g_key_file_get_integer_list(keyFile, 
								 desktop_plugin->iD, 
								 CAL_HP_KEY_VISIBLE_CALS, 
								 &(desktop_plugin->number_visible_cals), NULL);
    }
    if(g_key_file_has_key(keyFile, desktop_plugin->iD, CAL_HP_KEY_LAUNCH_VIEW, NULL))
    {
      desktop_plugin->launch_view = g_key_file_get_integer(keyFile, desktop_plugin->iD, CAL_HP_KEY_LAUNCH_VIEW, NULL);
      
    }
    if(g_key_file_has_key(keyFile, desktop_plugin->iD, CAL_HP_KEY_CAL_ROW_COUNT, NULL))
    {
      desktop_plugin->cal_row_count = g_key_file_get_integer(keyFile, desktop_plugin->iD, CAL_HP_KEY_CAL_ROW_COUNT, NULL);
      
    }
    if(g_key_file_has_key(keyFile, desktop_plugin->iD, CAL_HP_KEY_CAL_TIME_PERIOD, NULL))
    {
      desktop_plugin->cal_time_period = CalTimePeriod(g_key_file_get_integer(keyFile, 
									     desktop_plugin->iD, 
									     CAL_HP_KEY_CAL_TIME_PERIOD, 
									     NULL));
      
    }
    if(g_key_file_has_key(keyFile, desktop_plugin->iD, CAL_HP_KEY_CAL_SHOW_AGE, NULL))
    {
      desktop_plugin->show_age = g_key_file_get_boolean(keyFile, 
							desktop_plugin->iD, 
							CAL_HP_KEY_CAL_SHOW_AGE,
							NULL);
    }
    if(g_key_file_has_key(keyFile, desktop_plugin->iD, CAL_HP_KEY_CAL_UPDATE_IF_VISIBLE, NULL))
    {
      desktop_plugin->update_if_visible = g_key_file_get_boolean(keyFile, 
								 desktop_plugin->iD, 
								 CAL_HP_KEY_CAL_UPDATE_IF_VISIBLE,
								 NULL);
    }
    if(g_key_file_has_key(keyFile, desktop_plugin->iD, CAL_HP_KEY_CAL_SHOW_ALL_RECURRENCES, NULL))
    {
      desktop_plugin->show_all_recurrences = g_key_file_get_boolean(keyFile, 
								    desktop_plugin->iD, 
								    CAL_HP_KEY_CAL_SHOW_ALL_RECURRENCES,
								    NULL);
    }
    if(g_key_file_has_key(keyFile, desktop_plugin->iD, CAL_HP_KEY_CAL_SHOW_OLD_UNDONE_TASKS, NULL))
    {
      desktop_plugin->show_old_undone_tasks = g_key_file_get_boolean(keyFile, 
								     desktop_plugin->iD, 
								     CAL_HP_KEY_CAL_SHOW_OLD_UNDONE_TASKS,
								     NULL);
    }
    if(g_key_file_has_key(keyFile, desktop_plugin->iD, CAL_HP_KEY_CAL_SHOW_DONE_TASKS, NULL))
    {
      desktop_plugin->show_done_tasks = g_key_file_get_boolean(keyFile, 
							       desktop_plugin->iD, 
							       CAL_HP_KEY_CAL_SHOW_DONE_TASKS,
							       NULL);
    }
    if(g_key_file_has_key(keyFile, desktop_plugin->iD, CAL_HP_KEY_CAL_SHOW_WEEK_DAY_NAME, NULL))
    {
      desktop_plugin->show_week_day_name = g_key_file_get_boolean(keyFile, 
								  desktop_plugin->iD, 
								  CAL_HP_KEY_CAL_SHOW_WEEK_DAY_NAME,
								  NULL);
    }
    if(g_key_file_has_key(keyFile, desktop_plugin->iD, CAL_HP_KEY_CAL_VISIBLE_ENTRY_MODE, NULL))
    {
      desktop_plugin->visible_entry_mode = VisibleEntryMode(g_key_file_get_integer(keyFile, 
										desktop_plugin->iD, 
										CAL_HP_KEY_CAL_VISIBLE_ENTRY_MODE,
										NULL));
      
    }
  }
  g_key_file_free(keyFile);
  g_free(fileName);
}

void
cal_home_save_settings(CalHomePlugin* desktop_plugin)
{
  GKeyFile *keyFile;
  gchar *fileData;
  FILE *iniFile;
  gsize size;
  gchar *filename;

  keyFile = g_key_file_new();
  filename = g_strconcat (g_get_home_dir(), CAL_HOME_PLUGIN_SETTINGS_FILE, NULL);
  g_key_file_load_from_file (keyFile, filename, G_KEY_FILE_KEEP_COMMENTS, NULL);

  g_key_file_set_integer_list(keyFile, desktop_plugin->iD, CAL_HP_KEY_VISIBLE_CALS, 
			      desktop_plugin->visible_cals, 
			      desktop_plugin->number_visible_cals);
  g_key_file_set_integer (keyFile, desktop_plugin->iD, CAL_HP_KEY_LAUNCH_VIEW, desktop_plugin->launch_view);
  g_key_file_set_integer (keyFile, desktop_plugin->iD, CAL_HP_KEY_CAL_ROW_COUNT, desktop_plugin->cal_row_count);
  g_key_file_set_integer (keyFile, desktop_plugin->iD, CAL_HP_KEY_CAL_TIME_PERIOD, desktop_plugin->cal_time_period);
  g_key_file_set_boolean (keyFile, desktop_plugin->iD, CAL_HP_KEY_CAL_SHOW_AGE, desktop_plugin->show_age);
  g_key_file_set_boolean (keyFile, desktop_plugin->iD, CAL_HP_KEY_CAL_UPDATE_IF_VISIBLE, desktop_plugin->update_if_visible);
  g_key_file_set_boolean (keyFile, desktop_plugin->iD, CAL_HP_KEY_CAL_SHOW_ALL_RECURRENCES, desktop_plugin->show_all_recurrences);
  g_key_file_set_boolean (keyFile, desktop_plugin->iD, CAL_HP_KEY_CAL_SHOW_OLD_UNDONE_TASKS, desktop_plugin->show_old_undone_tasks);
  g_key_file_set_boolean (keyFile, desktop_plugin->iD, CAL_HP_KEY_CAL_SHOW_DONE_TASKS, desktop_plugin->show_done_tasks);

  g_key_file_set_boolean (keyFile, desktop_plugin->iD, CAL_HP_KEY_CAL_SHOW_WEEK_DAY_NAME, desktop_plugin->show_week_day_name);
  g_key_file_set_integer (keyFile, desktop_plugin->iD, CAL_HP_KEY_CAL_VISIBLE_ENTRY_MODE, desktop_plugin->visible_entry_mode);

  fileData = g_key_file_to_data (keyFile, &size, NULL);
  g_file_set_contents(filename, fileData, size, NULL);

  g_key_file_free (keyFile); 
  g_free(filename); 
  g_free(fileData); 
}
