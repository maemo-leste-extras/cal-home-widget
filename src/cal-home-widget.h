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
#ifndef CAL_HOME_PLUGIN_H
#define CAL_HOME_PLUGIN_H


#include <glib-object.h>
#include <libhildondesktop/libhildondesktop.h>

G_BEGIN_DECLS

typedef struct _CalHomePlugin CalHomePlugin;
typedef struct _CalHomePluginClass CalHomePluginClass;

#define CAL_HOME_TYPE_HOME_PLUGIN (cal_home_plugin_get_type())

#define CAL_HOME_PLUGIN(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), CAL_HOME_TYPE_HOME_PLUGIN, CalHomePlugin))

#define CAL_HOME_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), CAL_HOME_TYPE_HOME_PLUGIN, CalHomePluginClass))

#define CAL_HOME_IS_HOME_PLUGIN(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CAL_HOME_TYPE_HOME_PLUGIN))

#define CAL_HOME_IS_HOME_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CAL_HOME_TYPE_HOME_PLUGIN))

#define CAL_HOME_PLUGIN_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), CAL_HOME_TYPE_HOME_PLUGIN, CalHomePluginClass))

const int MEDIUM_ROW_COUNT = 5;

enum CalTimePeriod
{
  CAL_TIME_PERIOD_WEEK = 0,
  CAL_TIME_PERIOD_2WEEK,
  CAL_TIME_PERIOD_3WEEK,
  CAL_TIME_PERIOD_MONTH,
  CAL_TIME_PERIOD_2MONTH,
  CAL_TIME_PERIOD_3MONTH,
  CAL_TIME_PERIOD_6MONTH,
  CAL_TIME_PERIOD_12MONTH
};

enum VisibleEntryMode
{
  VISIBLE_ENTRY_EVENT = 0,
  VISIBLE_ENTRY_TASK,
  VISIBLE_ENTRY_EVENT_AND_TASK
};

struct _CalHomePlugin
{
  HDHomePluginItem hitem;
  gchar* iD;
  DBusGConnection* dbus_system_conn;
  DBusGProxy* dbus_clockd_proxy;
  DBusGConnection* dbus_session_conn;
  DBusGProxy* dbus_calendar_proxy;

  guint time_out_handler;
  guint gconf_notify_handler;  

  gsize number_visible_cals;
  gint* visible_cals;
  gint launch_view;
  gboolean show_age;
  gboolean has_24_time_format;
  gboolean update_if_visible;
  gboolean show_week_day_name;
  gboolean show_old_undone_tasks;
  gboolean show_done_tasks;
  struct tm date_today;
  gboolean update_view;
  gboolean show_all_recurrences;
  gint cal_row_count;
  gint today_events;
  time_t time_to_update;
  gint press_time;
  CalTimePeriod cal_time_period;
  VisibleEntryMode visible_entry_mode;
  GtkListStore* event_list;
  GtkWidget* day_label;
  GtkWidget* month_label;
  GtkWidget* day_image;
  GtkWidget* event_list_container;
  GdkPixbuf* applet_bg;
};

struct _CalHomePluginClass
{
  HDHomePluginItemClass parent_class;

};

GType cal_home_plugin_get_type(void);

G_END_DECLS
gboolean
cal_update_time(gpointer data);


#endif
