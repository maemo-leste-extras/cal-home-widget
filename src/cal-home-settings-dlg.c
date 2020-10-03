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

#include <libintl.h>
#include <locale.h>
#include <CMulticalendar.h>
#include <hildon/hildon.h>
#include "cal-home-settings-dlg.h"
#include "cal-home-calendar-util.h"
#include "cal-home-load-and-store.h"

void
cal_widget_resize_for_cal_size(CalHomePlugin* desktop_plugin);

static GtkWidget*
create_calendar_colour_icon_image(CalendarColour colour, GtkIconSize icon_size)
{
  switch(colour)
  {
  case COLOUR_DARKBLUE:
    return gtk_image_new_from_icon_name("calendar_colors_darkblue", icon_size);
  case COLOUR_DARKGREEN:
    return gtk_image_new_from_icon_name("calendar_colors_darkgreen", icon_size);
  case COLOUR_DARKRED:
    return gtk_image_new_from_icon_name("calendar_colors_darkred", icon_size);
  case COLOUR_ORANGE:
    return gtk_image_new_from_icon_name("calendar_colors_orange", icon_size);
  case COLOUR_VIOLET:
    return gtk_image_new_from_icon_name("calendar_colors_violet", icon_size);
  case COLOUR_YELLOW:
    return gtk_image_new_from_icon_name("calendar_colors_yellow", icon_size);
  case COLOUR_BLUE:
    return gtk_image_new_from_icon_name("calendar_colors_blue", icon_size);
  case COLOUR_RED:
    return gtk_image_new_from_icon_name("calendar_colors_red", icon_size);
  case COLOUR_GREEN:
    return gtk_image_new_from_icon_name("calendar_colors_green", icon_size);
  default:
    return gtk_image_new_from_icon_name("calendar_colors_white", icon_size);
  }
}

void
show_visible_calendars_dlg(GtkButton* button, gpointer data)
{
  cal_settings_data_t* settings_data = (cal_settings_data_t*)data;
  GtkWidget *dialog;

  dialog = gtk_dialog_new_with_buttons("Settings",
				       NULL,
				       (GtkDialogFlags)0,
				       dgettext("hildon-libs", "wdgt_bd_done"),
				       GTK_RESPONSE_ACCEPT,
				       NULL);

  vector<CCalendar*> calendars = CMulticalendar::MCInstance()->getListCalFromMc();

  GtkWidget* content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
  GtkWidget* list_box = gtk_vbox_new(TRUE, 0);
  GtkWidget* check_buttons[calendars.size()];
  vector<CCalendar*>::iterator calendars_iter = calendars.begin();
  vector<CCalendar*>::iterator calendars_iter_end = calendars.end();
  int i = 0;
  while(calendars_iter!=calendars_iter_end)
  {
    GtkWidget* check_button = hildon_gtk_toggle_button_new(HILDON_SIZE_FINGER_HEIGHT);

    check_buttons[i] = check_button;      

    for(int k = 0;k<settings_data->number_visible_cals;++k)
    {
      if(settings_data->visible_cals[k]==(*calendars_iter)->getCalendarId())
      {
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button),
				     TRUE);
      }
    }
    const gchar* calendar_name = dgettext("calendar", (*calendars_iter)->getCalendarName().c_str());
    gtk_button_set_label(GTK_BUTTON(check_button),
			 calendar_name);

    GtkWidget* image = create_calendar_colour_icon_image((*calendars_iter)->getCalendarColor(),
							 HILDON_ICON_SIZE_FINGER);
    gtk_button_set_image(GTK_BUTTON(check_button), image);
    gtk_button_set_alignment(GTK_BUTTON(check_button),0 ,0);
    gtk_box_pack_start(GTK_BOX(list_box), check_button, TRUE, FALSE, 0);

    ++calendars_iter;
    ++i;
  }

  GtkWidget* pan = hildon_pannable_area_new();
  hildon_pannable_area_add_with_viewport(HILDON_PANNABLE_AREA(pan), list_box);
  gtk_box_pack_start(GTK_BOX(content), pan, TRUE, TRUE, 0);
  gtk_window_set_default_size(GTK_WINDOW(dialog), -1, 325);  
  gtk_widget_show_all(dialog);

  if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
  {
    int visible_count = 0;
    for(int i=0;i<calendars.size();++i)
    {
      if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_buttons[i])))
	++visible_count;
    }
    g_free(settings_data->visible_cals);
    settings_data->number_visible_cals = visible_count;
    settings_data->visible_cals = g_new0(gint, visible_count);
    visible_count = 0;
    for(int i=0;i<calendars.size();++i)
    {
      if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_buttons[i])))
      {
	settings_data->visible_cals[visible_count] = calendars[i]->getCalendarId();
	++visible_count;
      }
    }
    gchar* button_value = g_strdup_printf("(%d)", settings_data->number_visible_cals); 
    hildon_button_set_value(HILDON_BUTTON(button),
			    button_value);
    g_free(button_value);

  }
  CMulticalendar::MCInstance()->releaseListCalendars(calendars);
  gtk_widget_destroy(dialog);
}

void
cal_show_settings_dialog(GtkWidget* widget, gpointer data)
{
  CalHomePlugin *desktop_plugin = CAL_HOME_PLUGIN(widget);

  GtkWidget *dialog;

  dialog = gtk_dialog_new_with_buttons("Settings",
				       NULL,
				       (GtkDialogFlags)0,
				       dgettext("hildon-libs", "wdgt_bd_done"),
				       GTK_RESPONSE_ACCEPT,
				       NULL);
  gtk_window_set_default_size(GTK_WINDOW(dialog), -1, 325);
  GtkWidget *cal_time_selector = hildon_touch_selector_new_text();
  GtkWidget *launch_view_selector = hildon_touch_selector_new_text();
  GtkWidget *cal_size_selector = hildon_touch_selector_new_text();
  GtkWidget *show_all_recurrence = hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT);
  GtkWidget *show_old_undone_tasks = hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT);
  GtkWidget *show_done_tasks = hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT);
  GtkWidget* toggle_event_visible = hildon_gtk_radio_button_new(HILDON_SIZE_FINGER_HEIGHT,
							    NULL);
  GtkWidget* toggle_task_visible = hildon_gtk_radio_button_new_from_widget(HILDON_SIZE_FINGER_HEIGHT,
									   GTK_RADIO_BUTTON(toggle_event_visible));
  GtkWidget* toggle_event_and_task_visible = hildon_gtk_radio_button_new_from_widget(HILDON_SIZE_FINGER_HEIGHT,
										     GTK_RADIO_BUTTON(toggle_event_visible));
  
  gtk_button_set_label(GTK_BUTTON(toggle_event_visible),
		       "Events");
  gtk_button_set_label(GTK_BUTTON(toggle_task_visible),
		       "Tasks");
  gtk_button_set_label(GTK_BUTTON(toggle_event_and_task_visible),
		       "Events/Tasks");
  if(desktop_plugin->visible_entry_mode == VISIBLE_ENTRY_EVENT_AND_TASK)
  {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle_event_and_task_visible), TRUE);
  }
  else if(desktop_plugin->visible_entry_mode == VISIBLE_ENTRY_EVENT)
  {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle_event_visible), TRUE);
  }
  else if(desktop_plugin->visible_entry_mode == VISIBLE_ENTRY_TASK)
  {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle_task_visible), TRUE);
  }
  gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(toggle_event_visible), FALSE);
  gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(toggle_task_visible), FALSE);
  gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(toggle_event_and_task_visible), FALSE);
  
  hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(launch_view_selector), 
				    dgettext("calendar", "cal_me_toggle_month"));
  hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(launch_view_selector), 
				    dgettext("calendar", "cal_me_toggle_week"));
  hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(launch_view_selector), 
				    dgettext("calendar", "cal_me_toggle_agenda"));

  hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(cal_size_selector),
				    "Auto");
  hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(cal_size_selector),
				    "1");
  hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(cal_size_selector),
				    "2");
  hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(cal_size_selector),
				    "3");
  hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(cal_size_selector),
				    "4");
  hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(cal_size_selector),
				    "5");
  hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(cal_size_selector),
				    "6");
  hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(cal_size_selector),
				    "7");
  hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(cal_size_selector),
				    "8");
  hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(cal_size_selector),
				    "9");

  hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(cal_time_selector),
				    "1 Week");
  hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(cal_time_selector),
				    "2 Weeks");
  hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(cal_time_selector),
				    "3 Weeks");
  hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(cal_time_selector),
				    "1 Month");
  hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(cal_time_selector),
				    "2 Month");
  hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(cal_time_selector),
				    "3 Month");
  hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(cal_time_selector),
				    "6 Month");
  hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(cal_time_selector),
				    "1 Year");


  GtkWidget* open_mode_button = hildon_picker_button_new(HILDON_SIZE_FINGER_HEIGHT,
							 HILDON_BUTTON_ARRANGEMENT_HORIZONTAL);
  
  GtkWidget* cal_size_button = hildon_picker_button_new(HILDON_SIZE_FINGER_HEIGHT,
							HILDON_BUTTON_ARRANGEMENT_HORIZONTAL);
  
  GtkWidget* cal_time_button = hildon_picker_button_new(HILDON_SIZE_FINGER_HEIGHT,
							HILDON_BUTTON_ARRANGEMENT_HORIZONTAL);
  
  GtkWidget* visible_calendar = hildon_button_new(HILDON_SIZE_FINGER_HEIGHT,
						  HILDON_BUTTON_ARRANGEMENT_HORIZONTAL);

  GtkWidget* show_age = hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT);
  GtkWidget* show_week_day_name = hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT);

  hildon_check_button_set_active(HILDON_CHECK_BUTTON(show_age),
				 desktop_plugin->show_age);
  hildon_check_button_set_active(HILDON_CHECK_BUTTON(show_all_recurrence), 
				 desktop_plugin->show_all_recurrences);
  hildon_check_button_set_active(HILDON_CHECK_BUTTON(show_old_undone_tasks), 
				 desktop_plugin->show_old_undone_tasks);
  hildon_check_button_set_active(HILDON_CHECK_BUTTON(show_done_tasks), 
				 desktop_plugin->show_done_tasks);

  gtk_button_set_label(GTK_BUTTON(show_all_recurrence),
		       "Show all Recurrences");
  gtk_button_set_label(GTK_BUTTON(show_old_undone_tasks),
		       "Show old undone tasks");
  gtk_button_set_label(GTK_BUTTON(show_done_tasks),
		       "Show done tasks");

  gtk_button_set_label(GTK_BUTTON(show_age),
		       "Show persons age");

  gtk_button_set_label(GTK_BUTTON(show_week_day_name),
		       "Show weekday name");

  GtkWidget* update_if_visible = hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT);

  hildon_check_button_set_active(HILDON_CHECK_BUTTON(update_if_visible),
				 desktop_plugin->update_if_visible);

  hildon_check_button_set_active(HILDON_CHECK_BUTTON(show_week_day_name),
				 desktop_plugin->show_week_day_name);

  gtk_button_set_label(GTK_BUTTON(update_if_visible),
		       "Update when switch to the desktop");

  hildon_button_set_style(HILDON_BUTTON(visible_calendar),
			  HILDON_BUTTON_STYLE_PICKER);

  hildon_button_set_title(HILDON_BUTTON(cal_size_button),
			  dgettext("hildon-libs", "ckdg_va_sort_size"));

  hildon_button_set_title(HILDON_BUTTON(cal_time_button),
			  "Visible Events");

  hildon_button_set_value(HILDON_BUTTON(visible_calendar),
			  g_strdup_printf("(%d)", desktop_plugin->number_visible_cals));

  hildon_button_set_title(HILDON_BUTTON(visible_calendar),
			  dgettext("calendar", "cal_fi_visible"));

  hildon_button_set_title(HILDON_BUTTON(open_mode_button),
			  dgettext("calendar", "cal_ia_wizard_typetext"));


  
  hildon_touch_selector_set_active(HILDON_TOUCH_SELECTOR(launch_view_selector),
				   0, desktop_plugin->launch_view -1);

  hildon_picker_button_set_selector(HILDON_PICKER_BUTTON(open_mode_button),
				    HILDON_TOUCH_SELECTOR(launch_view_selector));
  if(desktop_plugin->cal_row_count==-1)
    hildon_touch_selector_set_active(HILDON_TOUCH_SELECTOR(cal_size_selector),
				     0, 0);
  else
    hildon_touch_selector_set_active(HILDON_TOUCH_SELECTOR(cal_size_selector),
				     0, desktop_plugin->cal_row_count);
  

  hildon_picker_button_set_selector(HILDON_PICKER_BUTTON(cal_size_button),
				    HILDON_TOUCH_SELECTOR(cal_size_selector));
  

  hildon_touch_selector_set_active(HILDON_TOUCH_SELECTOR(cal_time_selector),
				   0, desktop_plugin->cal_time_period);

  hildon_picker_button_set_selector(HILDON_PICKER_BUTTON(cal_time_button),
				    HILDON_TOUCH_SELECTOR(cal_time_selector));
  
  cal_settings_data_t* settings_data = g_new0(cal_settings_data_t, 1);
  settings_data->number_visible_cals = desktop_plugin->number_visible_cals;
  settings_data->visible_cals = (gint*)g_memdup(desktop_plugin->visible_cals, 
					sizeof(gint)*desktop_plugin->number_visible_cals);
  
  g_signal_connect(G_OBJECT(visible_calendar), "clicked", 
		   G_CALLBACK(show_visible_calendars_dlg), settings_data);
  
  GtkWidget* pan = hildon_pannable_area_new();
  GtkWidget* pan_content = gtk_vbox_new(TRUE, 3);
  GtkWidget* content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
  gtk_box_pack_start(GTK_BOX(pan_content), cal_time_button, FALSE, FALSE, 3);
  gtk_box_pack_start(GTK_BOX(pan_content), cal_size_button, FALSE, FALSE, 3);
  gtk_box_pack_start(GTK_BOX(pan_content), open_mode_button, FALSE, FALSE, 3);
  gtk_box_pack_start(GTK_BOX(pan_content), visible_calendar, FALSE, FALSE, 3);
  gtk_box_pack_start(GTK_BOX(pan_content), show_age, FALSE, FALSE, 3);
  gtk_box_pack_start(GTK_BOX(pan_content), update_if_visible, FALSE, FALSE, 3);
  gtk_box_pack_start(GTK_BOX(pan_content), show_all_recurrence, FALSE, FALSE, 3);
  gtk_box_pack_start(GTK_BOX(pan_content), show_week_day_name, FALSE, FALSE, 3);
  GtkWidget* visible_event_mode_box = gtk_hbox_new(TRUE, 0);
  gtk_box_pack_start(GTK_BOX(visible_event_mode_box), toggle_event_visible, TRUE, TRUE, 3);
  gtk_box_pack_start(GTK_BOX(visible_event_mode_box), toggle_task_visible, TRUE, TRUE, 3);
  gtk_box_pack_start(GTK_BOX(visible_event_mode_box), toggle_event_and_task_visible, TRUE, TRUE, 3);
  gtk_box_pack_start(GTK_BOX(pan_content), visible_event_mode_box, FALSE, FALSE, 3);
  gtk_box_pack_start(GTK_BOX(pan_content), show_old_undone_tasks, FALSE, FALSE, 3);
  gtk_box_pack_start(GTK_BOX(pan_content), show_done_tasks, FALSE, FALSE, 3);

  hildon_pannable_area_add_with_viewport(HILDON_PANNABLE_AREA(pan), pan_content);
  gtk_container_add(GTK_CONTAINER(content), pan);
  gtk_widget_show_all(dialog);

  if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
  {
    desktop_plugin->number_visible_cals = settings_data->number_visible_cals;
    desktop_plugin->visible_cals = (gint*)g_memdup(settings_data->visible_cals, 
						   sizeof(gint)*settings_data->number_visible_cals);
    desktop_plugin->launch_view = hildon_touch_selector_get_active(HILDON_TOUCH_SELECTOR(launch_view_selector), 0) + 1;
    int row_count = hildon_touch_selector_get_active(HILDON_TOUCH_SELECTOR(cal_size_selector), 0);
    if(row_count == 0)
      desktop_plugin->cal_row_count = -1;
    else
      desktop_plugin->cal_row_count = row_count;
    desktop_plugin->cal_time_period = CalTimePeriod(hildon_touch_selector_get_active(HILDON_TOUCH_SELECTOR(cal_time_selector), 0));
    desktop_plugin->show_age = hildon_check_button_get_active(HILDON_CHECK_BUTTON(show_age));
    desktop_plugin->update_if_visible = hildon_check_button_get_active(HILDON_CHECK_BUTTON(update_if_visible));
    desktop_plugin->show_all_recurrences = hildon_check_button_get_active(HILDON_CHECK_BUTTON(show_all_recurrence));
    desktop_plugin->show_old_undone_tasks = hildon_check_button_get_active(HILDON_CHECK_BUTTON(show_old_undone_tasks));
    desktop_plugin->show_done_tasks = hildon_check_button_get_active(HILDON_CHECK_BUTTON(show_done_tasks));
    desktop_plugin->show_week_day_name = hildon_check_button_get_active(HILDON_CHECK_BUTTON(show_week_day_name));
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle_event_and_task_visible)))
    {
      desktop_plugin->visible_entry_mode = VISIBLE_ENTRY_EVENT_AND_TASK;
    }else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle_event_visible)))
    {
      desktop_plugin->visible_entry_mode = VISIBLE_ENTRY_EVENT;
    }else
    {
      desktop_plugin->visible_entry_mode = VISIBLE_ENTRY_TASK;
    }
    cal_home_save_settings(desktop_plugin);
    cal_home_load_db_events(desktop_plugin);
    cal_widget_resize_for_cal_size(desktop_plugin);
  }
  g_free(settings_data);
  gtk_widget_destroy (dialog);
}
