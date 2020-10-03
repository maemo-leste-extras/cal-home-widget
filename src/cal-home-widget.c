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
#include <libintl.h>
#include <locale.h>
#include <string.h>
#include <ctime>
#include <clockd/libtime.h>
#include <dbus/dbus-glib-lowlevel.h>


#include <hildon/hildon.h>
#include <libhildondesktop/libhildondesktop.h>
#include <gconf/gconf-client.h>	

#include <CMulticalendar.h>
#include <CalInterface.h>
#include <CComponent.h>

#include "cal-home-widget.h"	
#include "cal-home-load-and-store.h"	
#include "cal-home-calendar-util.h"	
#include "cal-home-settings-dlg.h"	


extern "C" {
  HD_DEFINE_PLUGIN_MODULE(CalHomePlugin, cal_home_plugin, HD_TYPE_HOME_PLUGIN_ITEM)
}


#define CAL_BG_IMAGE_PATH "/usr/share/icons/hicolor/64x64/hildon/calendar_applet.png"
#define GCONF_TIME_FORMAT_PATH "/apps/clock"

static const char * 	CALENDAR_DBUS_LISTENER_SIGNAL = "dbChange";

static const int CAL_WIDGET_ENTRY_ROW_HEIGHT = 36;
static const int CAL_WIDGET_TITLE_HEIGHT = 64;
static const int CAL_WIDGET_BOTTON_HEIGHT = 8;
static const int CAL_WIDGET_WIDTH = 352;

void
cal_widget_resize_for_cal_size(CalHomePlugin* desktop_plugin)
{
  gint cal_row_count = desktop_plugin->cal_row_count;
  gint row_count = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(desktop_plugin->event_list), NULL);
  if(cal_row_count == -1)
    cal_row_count = row_count;
  gtk_widget_set_size_request(desktop_plugin->event_list_container, 
			      CAL_WIDGET_WIDTH-8,
			      CAL_WIDGET_ENTRY_ROW_HEIGHT*row_count);
  gtk_window_resize(GTK_WINDOW (desktop_plugin), 
		    CAL_WIDGET_WIDTH, 
		    (CAL_WIDGET_ENTRY_ROW_HEIGHT*cal_row_count)+CAL_WIDGET_TITLE_HEIGHT+CAL_WIDGET_BOTTON_HEIGHT);
}

static gboolean
cal_24_time_format()
{
  GConfClient* client = NULL;
  client = gconf_client_get_default();
  if(!GCONF_IS_CLIENT(client)) 
    return TRUE; 
  
  gboolean is24Format = TRUE;
  is24Format = gconf_client_get_bool(client, GCONF_TIME_FORMAT_PATH "/time-format", NULL);
  g_object_unref(client);
  return is24Format;
}

static GdkPixbuf*
load_calendar_applet_icon()
{
  gchar* appleticon;
  GKeyFile *keyFile;
  keyFile = g_key_file_new();
  const gchar* fileName = "/etc/hildon/theme/index.theme";
  g_key_file_load_from_file (keyFile, fileName, G_KEY_FILE_KEEP_COMMENTS, NULL);
  appleticon = g_key_file_get_string(keyFile, "X-Hildon-Metatheme", "IconTheme", NULL);
  g_key_file_free(keyFile);
  gchar* Aicon = g_strconcat("/usr/share/icons/", appleticon, "/64x64/hildon/calendar_applet.png", NULL);

  GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(Aicon, NULL);
  if(pixbuf==NULL)
    pixbuf = gdk_pixbuf_new_from_file(CAL_BG_IMAGE_PATH, NULL);
  g_free(appleticon);
  g_free(Aicon);
  return pixbuf;
}

static void
render_day_on_title(CalHomePlugin* desktop_plugin)
{
  cairo_t* cr;
  GdkPixbuf* pixbuf = load_calendar_applet_icon();

  if(!pixbuf)
  {
    return;
  }
  int img_width = gdk_pixbuf_get_width(pixbuf);
  int img_height = gdk_pixbuf_get_height(pixbuf);

  cairo_surface_t *surface
    = cairo_image_surface_create_for_data(gdk_pixbuf_get_pixels(pixbuf),
                                          CAIRO_FORMAT_RGB24,
                                          img_width,
                                          img_height,
                                          gdk_pixbuf_get_rowstride(pixbuf));

  cr = cairo_create(surface);

  PangoLayout* layout;
  PangoFontDescription* desc;
  
  layout = pango_cairo_create_layout(cr);

  struct tm time;
  int  ret = time_get_local(&time);
  char buf[255];

  strftime(buf, 255,"%A", &time);
  gtk_label_set_text(GTK_LABEL(desktop_plugin->day_label), buf);

  strftime(buf, 255,"%B", &time);
  gtk_label_set_text(GTK_LABEL(desktop_plugin->month_label), buf);

  sprintf(buf,"%d", time.tm_mday);
  pango_layout_set_text(layout, buf, -1);
  desc = pango_font_description_from_string("Sans Bold 19");
  pango_layout_set_font_description(layout ,desc);
  pango_font_description_free(desc);
  int width = 0;
  int height = 0;
  pango_layout_get_pixel_size(layout, &width, &height);

  cairo_move_to(cr, 32-(width/2.0),33-(height/2.0));
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0); 
  cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
  pango_cairo_show_layout(cr, layout);
  gtk_image_set_from_pixbuf(GTK_IMAGE(desktop_plugin->day_image),
 			    pixbuf); 
  g_object_unref(pixbuf); 
  g_object_unref(layout); 
  cairo_surface_destroy(surface);
  cairo_destroy(cr);
}


static void
register_time_out_handler(CalHomePlugin* desktop_plugin)
{
  if(desktop_plugin->time_out_handler)
  {
    g_source_remove(desktop_plugin->time_out_handler);
    desktop_plugin->time_out_handler = 0;
  }
  time_t now = time_get_time();
  int delay = desktop_plugin->time_to_update - now;
  desktop_plugin->time_out_handler = g_timeout_add_seconds(delay,
							   cal_update_time, 
							   (gpointer) desktop_plugin);
}

static void
calendar_update_view(CalHomePlugin* desktop_plugin)
{
  render_day_on_title(desktop_plugin);
  cal_home_load_db_events(desktop_plugin);
  cal_widget_resize_for_cal_size(desktop_plugin);
  register_time_out_handler(desktop_plugin);
}

static void
cal_update_if_visible_and_date_changed(CalHomePlugin* desktop_plugin);

gboolean
cal_update_time(gpointer data)
{
  if(CAL_HOME_IS_HOME_PLUGIN(data))
  {
    CAL_HOME_PLUGIN(data)->update_view =  TRUE;
    gdk_threads_enter();
    cal_update_if_visible_and_date_changed(CAL_HOME_PLUGIN(data));
    gdk_threads_leave();
    register_time_out_handler((CalHomePlugin*)data);
  }
  return FALSE;
}

static void 
handle_time_format_changed(GConfClient* client,
			   guint cnxn_id,
			   GConfEntry* entry,
			   gpointer user_data)
{
  g_return_if_fail(CAL_HOME_IS_HOME_PLUGIN(user_data));
  CalHomePlugin* desktop_plugin = CAL_HOME_PLUGIN(user_data);
  desktop_plugin->has_24_time_format = gconf_client_get_bool(client, GCONF_TIME_FORMAT_PATH "/time-format", NULL);
  desktop_plugin->update_view = TRUE;
  gboolean on;
  g_object_get(GTK_WIDGET(desktop_plugin), "is-on-current-desktop", &on, NULL);
  if(on)
  {
    gdk_threads_enter();
    calendar_update_view(desktop_plugin);
    gdk_threads_leave();
  }
}

static gboolean 
time_to_update(CalHomePlugin* desktop_plugin)
{
  time_t now = time_get_time();
  if((desktop_plugin->time_to_update - now)<=0)
    return TRUE;
  else
    return FALSE;
}

static void
cal_update_if_visible_and_date_changed(CalHomePlugin* desktop_plugin)
{
  if(!time_to_update(desktop_plugin))
  {
    return;
  }
  gboolean on;
  g_object_get(GTK_WIDGET(desktop_plugin), "is-on-current-desktop", &on, NULL);

  if(on)
  {
    calendar_update_view(desktop_plugin);
  }
  else
  {
    desktop_plugin->update_view = TRUE;
  }
}

static void
handle_time_changed(DBusGProxy *object, gint time, CalHomePlugin* desktop_plugin)
{
  gboolean on;
  g_object_get(desktop_plugin, "is-on-current-desktop", &on, NULL);
  if(on)
  {
    gdk_threads_enter();
    calendar_update_view(desktop_plugin);
    gdk_threads_leave();
  }
  else
  {
    CAL_HOME_PLUGIN(desktop_plugin)->update_view = TRUE;
  }
}

static DBusHandlerResult
handle_db_change(DBusConnection *bus, 
		 DBusMessage *msg, 
		 gpointer data)
{
  if(CAL_HOME_IS_HOME_PLUGIN(data))
  {
    CalHomePlugin* desktop_plugin = CAL_HOME_PLUGIN(data);
    if(dbus_message_is_signal(msg,CALENDAR_SERVICE, CALENDAR_DBUS_LISTENER_SIGNAL))
    {  
      gboolean on;
      g_object_get(desktop_plugin, "is-on-current-desktop", &on, NULL);
      if(on)
      {
	gdk_threads_enter();
	calendar_update_view(desktop_plugin);
	gdk_threads_leave();
      }
      else
      {
	CAL_HOME_PLUGIN(desktop_plugin)->update_view = TRUE;
      }
    }
  }
  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void
cal_on_current_desktop(GtkWidget* widget,
		       gpointer data)
{
  CalHomePlugin* desktop_plugin = CAL_HOME_PLUGIN(widget);

  gboolean on;
  g_object_get(widget, "is-on-current-desktop", &on, NULL);
  if(on)
  {
    if(time_to_update(desktop_plugin) ||
       desktop_plugin->update_if_visible || 
       desktop_plugin->update_view)
    {
      calendar_update_view(desktop_plugin);
    }
    else if(!desktop_plugin->time_out_handler)
    {
      register_time_out_handler(desktop_plugin);
    }
  }
  else if(desktop_plugin->time_out_handler)
  {
    g_source_remove(desktop_plugin->time_out_handler);
    desktop_plugin->time_out_handler = 0;
  }
}

static void
cal_home_show_calendar(DBusGProxy* proxy,
		       int type)
{

  time_t tt = time(NULL); 
  int msgID = tt; 
  const char* text = "DUMMY";  
  if(proxy)
  {
    dbus_g_proxy_call_no_reply(proxy, 
			       CALENDAR_LAUNCH_METHOD,
			       G_TYPE_UINT, type, 
			       G_TYPE_INT, msgID, 
			       G_TYPE_STRING, text,
			       DBUS_TYPE_INVALID); 
  }  
}

static gboolean
cal_home_button_press(GtkWidget* widget, GdkEventButton *event, CalHomePlugin* desktop_plugin)
{
  desktop_plugin->press_time = time_get_time();
  return TRUE;
}

static gboolean
cal_home_button_release(GtkWidget* widget, GdkEventButton *event, CalHomePlugin* desktop_plugin)
{
  cal_home_show_calendar(desktop_plugin->dbus_calendar_proxy,
			 desktop_plugin->launch_view);
  return TRUE;
}

static gboolean
cal_home_button_release_show_agenda(GtkWidget* widget, GdkEventButton *event, CalHomePlugin* desktop_plugin)
{
  cal_home_show_calendar(desktop_plugin->dbus_calendar_proxy,
			 3);
  return TRUE;
}

static gboolean
cal_home_button_release_show_week_view(GtkWidget* widget, GdkEventButton *event, CalHomePlugin* desktop_plugin)
{
  int t = time_get_time() - desktop_plugin->press_time;

  if(t<1)
    cal_home_show_calendar(desktop_plugin->dbus_calendar_proxy,
			   2);
  else
  {
    calendar_update_view(desktop_plugin);
    hildon_banner_show_information(NULL, NULL, "Calendar Widget updated");    
  }
  return TRUE;
}

static gboolean
cal_home_button_release_show_month_view(GtkWidget* widget, GdkEventButton *event, CalHomePlugin* desktop_plugin)
{
  cal_home_show_calendar(desktop_plugin->dbus_calendar_proxy,
			 1);
  return TRUE;
}

static GtkWidget*
build_ui(CalHomePlugin *desktop_plugin)
{
  GtkWidget *contents = gtk_event_box_new();
  GtkWidget* box  = gtk_vbox_new(FALSE, 0);

  gtk_event_box_set_visible_window(GTK_EVENT_BOX(contents), FALSE);
  gtk_container_set_border_width(GTK_CONTAINER(contents), 0);

  gint row_count = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(desktop_plugin->event_list), NULL);
  desktop_plugin->event_list_container  = gtk_table_new(1, 4, row_count);

  GtkWidget* title = gtk_hbox_new(FALSE, 0);
  GdkPixbuf* icon = load_calendar_applet_icon();
  desktop_plugin->day_image = gtk_image_new_from_pixbuf(icon);
  g_object_unref(icon);

  desktop_plugin->day_label = gtk_label_new("");
  desktop_plugin->month_label = gtk_label_new("");

  hildon_helper_set_logical_font(desktop_plugin->day_label,
				 "SystemFont");
  hildon_helper_set_logical_color(desktop_plugin->day_label,
				  GTK_RC_FG,
				  GTK_STATE_NORMAL,
				  "DefaultTextColor");

  gtk_label_set_ellipsize(GTK_LABEL(desktop_plugin->month_label), PANGO_ELLIPSIZE_END);
  hildon_helper_set_logical_color (desktop_plugin->month_label, GTK_RC_FG, GTK_STATE_NORMAL, "ActiveTextColor");
  hildon_helper_set_logical_font(desktop_plugin->month_label,
				 "SmallSystemFont");
  
  GtkWidget* day_box = gtk_event_box_new();
  GtkWidget* week_day_box = gtk_event_box_new();
  GtkWidget* month_box = gtk_event_box_new();

  gtk_event_box_set_visible_window(GTK_EVENT_BOX(day_box), FALSE);
  gtk_event_box_set_visible_window(GTK_EVENT_BOX(week_day_box), FALSE);
  gtk_event_box_set_visible_window(GTK_EVENT_BOX(month_box), FALSE);
  gtk_container_set_border_width(GTK_CONTAINER(day_box), 0);
  gtk_container_set_border_width(GTK_CONTAINER(week_day_box), 0);
  gtk_container_set_border_width(GTK_CONTAINER(month_box), 0);

  gtk_container_add(GTK_CONTAINER(day_box), desktop_plugin->day_image);
  gtk_container_add(GTK_CONTAINER(week_day_box), desktop_plugin->day_label);
  gtk_container_add(GTK_CONTAINER(month_box), desktop_plugin->month_label);

  gtk_misc_set_alignment (GTK_MISC (desktop_plugin->day_label), 0.5, 0.8);
  gtk_misc_set_alignment (GTK_MISC (desktop_plugin->month_label), 0.8, 1.0);
  
  gtk_box_pack_start(GTK_BOX(title), day_box, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(title), week_day_box, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(title), month_box, TRUE, TRUE, 3);

  g_signal_connect(GTK_CONTAINER(day_box), 
		   "button-release-event", 
		   G_CALLBACK(cal_home_button_release_show_agenda), 
		   desktop_plugin);

  g_signal_connect(GTK_CONTAINER(week_day_box), 
		   "button-release-event", 
		   G_CALLBACK(cal_home_button_release_show_week_view), 
		   desktop_plugin);

  g_signal_connect(GTK_CONTAINER(week_day_box), 
		   "button-press-event", 
		   G_CALLBACK(cal_home_button_press), 
		   desktop_plugin);

  g_signal_connect(GTK_CONTAINER(month_box), "button-release-event", 
		   G_CALLBACK(cal_home_button_release_show_month_view), 
		   desktop_plugin);

  gtk_box_pack_start(GTK_BOX(box), title, FALSE, FALSE, 0);
  GtkWidget* box2 = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_end(GTK_BOX(box2), desktop_plugin->event_list_container, FALSE, FALSE, 0); 
  gtk_box_pack_start(GTK_BOX(box), box2, FALSE, FALSE, 0);
  gtk_container_add(GTK_CONTAINER(contents), box);
  gtk_widget_show_all(contents);
  return GTK_WIDGET(contents);
}

static void
cal_realize(GtkWidget* widget)
{
  GdkScreen *screen = gtk_widget_get_screen(widget);
  gtk_widget_set_colormap(widget, gdk_screen_get_rgba_colormap(screen));
  gtk_widget_set_app_paintable(widget, TRUE);
  CalHomePlugin* desktop_plugin = CAL_HOME_PLUGIN(widget);
  desktop_plugin->iD = hd_home_plugin_item_get_applet_id (HD_HOME_PLUGIN_ITEM (widget));
  cal_home_read_settings(desktop_plugin);
  calendar_update_view(desktop_plugin);
  GTK_WIDGET_CLASS(cal_home_plugin_parent_class)->realize(widget);
}

static void
cal_render_background_auto(cairo_t* cr, CalHomePlugin* desktop_plugin)
{
  gint row_count = desktop_plugin->cal_row_count;
  if(row_count == -1)
    row_count = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(desktop_plugin->event_list), NULL); 
  if(row_count>5)
  {
    int height = (CAL_WIDGET_ENTRY_ROW_HEIGHT*row_count)+CAL_WIDGET_TITLE_HEIGHT+CAL_WIDGET_BOTTON_HEIGHT;
    cairo_translate(cr, 0, height-(5*36+8));
    gdk_cairo_set_source_pixbuf(cr, desktop_plugin->applet_bg, 0, -64);
    cairo_rectangle(cr, 0, 0, CAL_WIDGET_WIDTH, 5*36+8);
    cairo_fill(cr);
    cairo_translate(cr, 0, -(height-(5*36+8)));


    cairo_translate(cr, 0, 64); 
    gdk_cairo_set_source_pixbuf(cr, desktop_plugin->applet_bg, 0, -64 -(9-row_count)*36);
    cairo_rectangle(cr, 0, 0, CAL_WIDGET_WIDTH, (row_count-5)*36); 
    cairo_fill(cr); 
    cairo_translate(cr, 0, -64); 
  }
  else
  {
    cairo_translate(cr, 0, 64);
    gdk_cairo_set_source_pixbuf(cr, desktop_plugin->applet_bg, 0, -64 - (5-row_count)*36);
    cairo_rectangle(cr, 0, 0, CAL_WIDGET_WIDTH, row_count*36+8);
    cairo_fill(cr);
    cairo_translate(cr, 0, -64);
  }
  if(desktop_plugin->today_events > 0 &&
     desktop_plugin->today_events < row_count)
  {
    int today_line_y = desktop_plugin->today_events*CAL_WIDGET_ENTRY_ROW_HEIGHT+CAL_WIDGET_TITLE_HEIGHT;
    cairo_set_line_width(cr, 1.5);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    cairo_move_to(cr, 8, today_line_y);
    cairo_line_to(cr, CAL_WIDGET_WIDTH-8, today_line_y);
    cairo_stroke(cr);
  }
}

static gboolean
cal_expose(GtkWidget* widget, GdkEventExpose *event)
{
  CalHomePlugin* desktop_plugin = CAL_HOME_PLUGIN(widget);
  cairo_t* cr;
  cr = gdk_cairo_create(GDK_DRAWABLE(widget->window));
  gdk_cairo_set_source_pixbuf(cr, desktop_plugin->applet_bg, 0, 0);
  cairo_rectangle(cr, 0, 0, CAL_WIDGET_WIDTH, 64);
  cairo_fill(cr);
  cal_render_background_auto(cr, desktop_plugin);
  cairo_destroy(cr);
  return GTK_WIDGET_CLASS(cal_home_plugin_parent_class)->expose_event(widget, event);
}

static void
register_gconf_notify_on_timeformat_changed(CalHomePlugin* desktop_plugin)
{
  GConfClient* client = NULL;
  client = gconf_client_get_default();
  if(!GCONF_IS_CLIENT(client))
  {
    desktop_plugin->gconf_notify_handler = 0;
    return;
  }
  gconf_client_add_dir(client, GCONF_TIME_FORMAT_PATH,
		       GCONF_CLIENT_PRELOAD_NONE,
		       NULL);
  desktop_plugin->gconf_notify_handler = gconf_client_notify_add(client, GCONF_TIME_FORMAT_PATH,
								 handle_time_format_changed, 
								 desktop_plugin, NULL, NULL);
  g_object_unref(client);
}

static void
register_dbus_signal_on_time_changed(CalHomePlugin *desktop_plugin)
{
  desktop_plugin->dbus_system_conn = hd_home_plugin_item_get_dbus_g_connection(&desktop_plugin->hitem, 
									       DBUS_BUS_SYSTEM, 
									       NULL );

  if(desktop_plugin->dbus_system_conn)
  {
    desktop_plugin->dbus_clockd_proxy = dbus_g_proxy_new_for_name(desktop_plugin->dbus_system_conn,
								  CLOCKD_SERVICE,
								  CLOCKD_PATH,
								  CLOCKD_INTERFACE);
    dbus_g_proxy_add_signal(desktop_plugin->dbus_clockd_proxy,
			    CLOCKD_TIME_CHANGED,
			    G_TYPE_INT, G_TYPE_INVALID);

    dbus_g_proxy_connect_signal(desktop_plugin->dbus_clockd_proxy,
				CLOCKD_TIME_CHANGED,
				G_CALLBACK(handle_time_changed), desktop_plugin, NULL);
  }
}

static void
register_dbus_signal_on_db_changed(CalHomePlugin *desktop_plugin)
{
  desktop_plugin->dbus_session_conn = hd_home_plugin_item_get_dbus_g_connection(&desktop_plugin->hitem,  
 										DBUS_BUS_SESSION,  
 										NULL ); 
  if(desktop_plugin->dbus_session_conn) 
  { 
    desktop_plugin->dbus_calendar_proxy = dbus_g_proxy_new_for_name(desktop_plugin->dbus_session_conn, 
								    CALENDAR_SERVICE, 
								    CALENDAR_PATH, 
								    CALENDAR_INTERFACE); 
    
  } 
  dbus_bus_add_match(dbus_g_connection_get_connection(desktop_plugin->dbus_session_conn),   
  		     "type='signal',interface='com.nokia.calendar'",   
  		     NULL);  
  
  dbus_connection_add_filter(dbus_g_connection_get_connection(desktop_plugin->dbus_session_conn),   
  			     handle_db_change,   
  			     desktop_plugin,   
  			     NULL);  
}

static void
cal_home_init_default_calendar_list_setting(CalHomePlugin *desktop_plugin)
{
  desktop_plugin->number_visible_cals = CMulticalendar::MCInstance()->getNoofCalendars();
  desktop_plugin->visible_cals = g_new0(gint, desktop_plugin->number_visible_cals);
  vector<CCalendar*> calendars = CMulticalendar::MCInstance()->getListCalFromMc();
  for(int i=0;i<desktop_plugin->number_visible_cals;++i)
  {
    desktop_plugin->visible_cals[i] = calendars[i]->getCalendarId();
  }
  CMulticalendar::MCInstance()->releaseListCalendars(calendars);
}

static void
cal_load_original_bg(CalHomePlugin* desktop_plugin)
{
  desktop_plugin->applet_bg = NULL;
  GdkPixbuf* bg = gdk_pixbuf_new_from_file("/etc/hildon/theme/images/CalendarAppletBackground.png", NULL);
  if(bg == NULL)
    return;
  GdkPixbuf* stride = gdk_pixbuf_new_subpixbuf(bg, 0, 0, CAL_WIDGET_WIDTH, 252);
  desktop_plugin->applet_bg = gdk_pixbuf_copy(stride);
  g_object_unref(bg);
}

static void
cal_home_plugin_init(CalHomePlugin *desktop_plugin)
{
  struct tm update_date;
  cal_home_time_today_midnight(&update_date);
  update_date.tm_mday++;
  
  cal_load_original_bg(desktop_plugin);
  
  desktop_plugin->event_list = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, GDK_TYPE_PIXBUF);
  desktop_plugin->dbus_system_conn = NULL;
  desktop_plugin->dbus_clockd_proxy = NULL;
  desktop_plugin->dbus_session_conn = NULL;
  desktop_plugin->dbus_calendar_proxy = NULL;
  desktop_plugin->launch_view = 1;
  desktop_plugin->cal_time_period = CAL_TIME_PERIOD_12MONTH;
  desktop_plugin->cal_row_count = MEDIUM_ROW_COUNT;
  desktop_plugin->today_events = 0;
  desktop_plugin->time_to_update = mktime(&update_date);
  desktop_plugin->press_time = 0;
  desktop_plugin->update_view = TRUE;
  desktop_plugin->show_week_day_name = FALSE;
  desktop_plugin->update_if_visible = FALSE;
  desktop_plugin->has_24_time_format = cal_24_time_format();
  desktop_plugin->show_all_recurrences = TRUE;
  desktop_plugin->show_age = FALSE;
  desktop_plugin->show_old_undone_tasks = TRUE;
  desktop_plugin->show_done_tasks = TRUE;
  desktop_plugin->visible_entry_mode = VISIBLE_ENTRY_EVENT;
  GtkWidget *contents = build_ui(desktop_plugin);

  gtk_window_set_default_size (GTK_WINDOW (desktop_plugin), CAL_WIDGET_WIDTH, 252);

  cal_home_init_default_calendar_list_setting(desktop_plugin);

  gtk_container_add(GTK_CONTAINER(desktop_plugin), contents);

  hd_home_plugin_item_set_settings (HD_HOME_PLUGIN_ITEM (desktop_plugin), TRUE);

  g_signal_connect(desktop_plugin, "show-settings", G_CALLBACK(cal_show_settings_dialog), NULL);
  g_signal_connect(desktop_plugin, "notify::is-on-current-desktop", G_CALLBACK(cal_on_current_desktop), NULL);
  g_signal_connect(GTK_CONTAINER(contents), "button-release-event", G_CALLBACK(cal_home_button_release), desktop_plugin);
  register_gconf_notify_on_timeformat_changed(desktop_plugin);
  register_dbus_signal_on_time_changed(desktop_plugin);
  register_dbus_signal_on_db_changed(desktop_plugin);
}

static void
cal_plugin_finalize(GObject *object)
{
  CalHomePlugin *desktop_plugin = CAL_HOME_PLUGIN(object);

  if(desktop_plugin->visible_cals)
    g_free(desktop_plugin->visible_cals);

  if(desktop_plugin->time_out_handler)
  {
    g_source_remove(desktop_plugin->time_out_handler);
    desktop_plugin->time_out_handler = 0;
  }

  if(desktop_plugin->gconf_notify_handler)
  {
    GConfClient* client = gconf_client_get_default();
    if(GCONF_IS_CLIENT(client))
    {
      gconf_client_notify_remove(client,
				 desktop_plugin->gconf_notify_handler);
      desktop_plugin->gconf_notify_handler = 0;
      gconf_client_remove_dir(client, GCONF_TIME_FORMAT_PATH,
			   NULL);

    }
  }

  if(desktop_plugin->dbus_clockd_proxy)
    g_object_unref(desktop_plugin->dbus_clockd_proxy);

  if(desktop_plugin->dbus_calendar_proxy)
    g_object_unref(desktop_plugin->dbus_calendar_proxy);

  dbus_connection_remove_filter(dbus_g_connection_get_connection(desktop_plugin->dbus_session_conn),   
				handle_db_change,   
				object);
  dbus_g_connection_unref(desktop_plugin->dbus_system_conn);   
  dbus_g_connection_unref(desktop_plugin->dbus_session_conn);
  g_free(desktop_plugin->iD);
  g_object_unref(desktop_plugin->applet_bg);
  G_OBJECT_CLASS(cal_home_plugin_parent_class)->finalize (object);
}

static void
cal_home_plugin_class_init(CalHomePluginClass *klass) 
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
  widget_class->realize = cal_realize;
  widget_class->expose_event = cal_expose;
  G_OBJECT_CLASS(klass)->finalize = cal_plugin_finalize;
}

static void
cal_home_plugin_class_finalize(CalHomePluginClass *klass) 
{
}

