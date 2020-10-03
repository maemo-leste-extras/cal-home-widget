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
#include <ctime>
#include <clockd/libtime.h>
#include <hildon/hildon.h>
#include <CMulticalendar.h>
#include <Common.h>
#include <CalendarErrors.h>
#include "cal-home-calendar-util.h"

static time_t 
cal_home_advance_date_by_time_period(struct tm* start,
				     const CalTimePeriod& ctp)
{
  switch(ctp)
  {
  case CAL_TIME_PERIOD_WEEK:
    start->tm_mday+=7;
    break;
  case CAL_TIME_PERIOD_2WEEK:
    start->tm_mday+=2*7;
    break;
  case CAL_TIME_PERIOD_3WEEK:
    start->tm_mday+=3*7;
    break;
  case CAL_TIME_PERIOD_MONTH:
    start->tm_mon+=1;
    break;
  case CAL_TIME_PERIOD_2MONTH:
    start->tm_mon+=2;
    break;
  case CAL_TIME_PERIOD_3MONTH:
    start->tm_mon+=3;
    break;
  case CAL_TIME_PERIOD_6MONTH:
    start->tm_mon+=6;
    break;
  case CAL_TIME_PERIOD_12MONTH:
    start->tm_year+=1;
    break;
  default:
   start->tm_year+=1; 
  }
  return mktime(start);
}

void
cal_home_time_today_midnight(struct tm* today)
{
  time_get_local(today);
  today->tm_hour = today->tm_min = today->tm_sec = 0;
}

static void
cal_home_time_today_now(struct tm* today)
{
  time_get_local(today);
  today->tm_sec = 0;
}

static int
cal_row_count_for_cal_size(const gint& cal_size)
{
  if(cal_size == -1)
    return 9;
  else
    return cal_size;
}

static GdkPixbuf* 
get_calendar_pixbuf(CComponent* component)
{
  int error = 0;
  int eventType = component->getType();
  CCalendar* cal = CMulticalendar::MCInstance()->getCalendarById(component->getCalendarId(), error);

  GdkPixbuf* pixbuf = NULL;
  switch(cal->getCalendarColor())
  {
  case COLOUR_DARKBLUE:
    pixbuf = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), 
				      "calendar_colors_darkblue", 26,GTK_ICON_LOOKUP_NO_SVG, NULL);
    break;
  case COLOUR_DARKGREEN:
    pixbuf = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), 
				      "calendar_colors_darkgreen", 26,GTK_ICON_LOOKUP_NO_SVG, NULL);
    break;
  case COLOUR_DARKRED:
    pixbuf = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), 
				      "calendar_colors_darkred", 26,GTK_ICON_LOOKUP_NO_SVG, NULL);
    break;
  case COLOUR_ORANGE:
    pixbuf = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), 
				      "calendar_colors_orange", 26,GTK_ICON_LOOKUP_NO_SVG, NULL);
    break;
  case COLOUR_VIOLET:
    pixbuf = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), 
				      "calendar_colors_violet", 26,GTK_ICON_LOOKUP_NO_SVG, NULL);
    break;
  case COLOUR_YELLOW:
    pixbuf = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), 
				      "calendar_colors_yellow", 26,GTK_ICON_LOOKUP_NO_SVG, NULL);
    break;
  case COLOUR_BLUE:
    pixbuf = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), 
				      "calendar_colors_blue", 26,GTK_ICON_LOOKUP_NO_SVG, NULL);
    break;
  case COLOUR_RED:
    pixbuf = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), 
				      "calendar_colors_red", 26,GTK_ICON_LOOKUP_NO_SVG, NULL);
    break;
  case COLOUR_GREEN:
    pixbuf = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), 
				      "calendar_colors_green", 26,GTK_ICON_LOOKUP_NO_SVG, NULL);
    break;
  default:
    pixbuf = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), 
				      "calendar_colors_white", 26,GTK_ICON_LOOKUP_NO_SVG, NULL);
  }
  delete cal;
  if(eventType == E_TODO || eventType == E_BDAY || component->getAlarm())
  {
    cairo_t* cr;
    cairo_surface_t* surface = cairo_image_surface_create_for_data(gdk_pixbuf_get_pixels(pixbuf),
								   CAIRO_FORMAT_RGB24,
								   26, 26,
								   gdk_pixbuf_get_rowstride(pixbuf));
    GdkPixbuf* event_icon;
    if(eventType == E_BDAY)
      event_icon = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), 
					  "calendar_birthday", 16, GTK_ICON_LOOKUP_NO_SVG, NULL);
    else if(eventType == E_TODO)
      event_icon = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), 
					  "calendar_todo", 16, GTK_ICON_LOOKUP_NO_SVG, NULL);
    else
      event_icon = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), 
					  "calendar_alarm", 16, GTK_ICON_LOOKUP_NO_SVG, NULL);
    cr = cairo_create(surface);
    gdk_cairo_set_source_pixbuf(cr, event_icon, 5, 5);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    cairo_paint(cr);
    cairo_surface_destroy(surface);
    cairo_destroy(cr);
    g_object_unref(event_icon);
  }
  return pixbuf;
}

static void
cal_fill_event_list_ui(CalHomePlugin* desktop_plugin)
{
  GtkTreeIter iter;
  gtk_container_foreach(GTK_CONTAINER(desktop_plugin->event_list_container), (GtkCallback)gtk_widget_destroy, NULL);
  gint row_count = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(desktop_plugin->event_list), NULL);

  gtk_table_resize(GTK_TABLE(desktop_plugin->event_list_container),row_count,3);
  if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(desktop_plugin->event_list),
				   &iter))
  {
    int i = 0;
    do
    {
      gchar* time;
      gchar* summary;
      GdkPixbuf *pixbuf;
      gtk_tree_model_get(GTK_TREE_MODEL(desktop_plugin->event_list),
			 &iter,
			 0, &time,
			 1, &summary,
			 2, &pixbuf,
			 -1);
      GtkWidget* line = gtk_hbox_new(FALSE, 0);

      GtkWidget* time_label = gtk_label_new(NULL);

      gtk_label_set_markup(GTK_LABEL(time_label), time);
      g_free(time);
      GtkWidget* cal_image = gtk_image_new_from_pixbuf(pixbuf);

      hildon_helper_set_logical_font(time_label,
				     "HomeSystemFont");
      hildon_helper_set_logical_color(time_label,
				      GTK_RC_FG,
				      GTK_STATE_NORMAL,
				      "ActiveTextColor");
      GtkWidget* summary_label = gtk_label_new(NULL);

      gtk_label_set_markup(GTK_LABEL(summary_label), summary);

      g_free(summary);
      
      hildon_helper_set_logical_font(summary_label,
				     "HomeSystemFont");
      hildon_helper_set_logical_color(summary_label,
				      GTK_RC_FG,
				      GTK_STATE_NORMAL,
				      "DefaultTextColor");

      gtk_misc_set_alignment(GTK_MISC(time_label), 0.0f, 0.5f);
      gtk_misc_set_alignment(GTK_MISC(summary_label), 0.0f, 0.5f);
      gtk_misc_set_alignment(GTK_MISC(cal_image), 1.0f, 0.5f);
      gtk_label_set_ellipsize(GTK_LABEL(summary_label), PANGO_ELLIPSIZE_END);
      gtk_table_attach(GTK_TABLE(desktop_plugin->event_list_container),
		       time_label, 0, 1, i, i+1,
		       GtkAttachOptions(GTK_FILL | GTK_SHRINK),
		       GTK_FILL,
		       2, 5);
      gtk_table_attach(GTK_TABLE(desktop_plugin->event_list_container),
		       summary_label, 1, 2, i, i+1,
		       GtkAttachOptions(GTK_FILL | GTK_EXPAND),
		       GTK_FILL,
		       0, 5);
      gtk_table_attach(GTK_TABLE(desktop_plugin->event_list_container),
		       cal_image, 2, 3, i, i+1,
		       GTK_FILL,
		       GTK_FILL,
		       8, 5);
      ++i;
    }while(gtk_tree_model_iter_next(GTK_TREE_MODEL(desktop_plugin->event_list),
				    &iter));
  }
}

static void
cal_delete_ccomponentset_items(SortedCComponentSet& ccomponent_set)
{
  
  for(SortedCComponentSet::const_iterator iter = ccomponent_set.begin();
      iter!=ccomponent_set.end();
      ++iter)
  {
    delete (*iter);
  }
}

static gboolean 
cal_date_on_today(CComponent* component,
		  CalHomePlugin* desktop_plugin)
{
  struct tm time;
  time_t t = component->getDateStart();
  if(component->getType() == E_TODO ||
     component->getAllDay())
  {
    time_get_remote(t, component->getTzid().c_str(), &time);
  }
  else
  {

    time_get_local_ex(t, &time);

  }
  return time.tm_year == desktop_plugin->date_today.tm_year &&
    time.tm_mon == desktop_plugin->date_today.tm_mon &&
    time.tm_mday== desktop_plugin->date_today.tm_mday;
}

static void
_format_time(gchar* buffer, struct tm* time, gboolean has_24_time_format)
{
  if(has_24_time_format)
    strftime(buffer, 255,dgettext("hildon-libs","wdgt_va_24h_time"), time);
  else
  {
    if(time->tm_hour>11)
    {
      strftime(buffer, 255,dgettext("hildon-libs","wdgt_va_12h_time_pm"), time);
    }
    else
    {
      strftime(buffer, 255,dgettext("hildon-libs","wdgt_va_12h_time_am"), time);
    }
  }
}

static gboolean
_todo_is_old(CComponent* component, struct tm* midnight)
{
  time_t event_time = component->getDateStart();
  struct tm* time = localtime(&event_time);
  return
    (time->tm_year < midnight->tm_year) ||
    ((time->tm_year == midnight->tm_year)  
     && (time->tm_mon < midnight->tm_mon)) ||
    ((time->tm_year == midnight->tm_year)  && (time->tm_mon == midnight->tm_mon) &&
     time->tm_mday < midnight->tm_mday);
}

static void
_format_todo_time(CComponent* component, gchar* buf, CalHomePlugin* desktop_plugin)
{
  time_t start_time = component->getDateStart();
  if(cal_date_on_today(component, desktop_plugin))
  {
    desktop_plugin->today_events++;
    sprintf(buf,"%s", dgettext("calendar", "cal_va_allday_home"));
  }
  else
  {
    struct tm* time = localtime(&start_time);
    if(desktop_plugin->show_week_day_name)
    {
      strftime(buf, 255, dgettext("hildon-libs","wdgt_va_date_day_name_short"), time);
    }
    else
    {
      strftime(buf, 255, dgettext("hildon-libs","wdgt_va_date_short"), time);
    }
  }
}

static void
cal_condense_ccomponents_set_to_list_store(const SortedCComponentSet& ccomponent_set,
					   GtkListStore* list_store,
					   CalHomePlugin* desktop_plugin,
					   gsize up_to)
{
  struct tm end_date;
  struct tm end_date_today;
  cal_home_time_today_now(&end_date);
  cal_home_time_today_midnight(&end_date_today);
  end_date_today.tm_mday++;
  int earliest_update_end_time = mktime(&end_date_today);
  time_t end_time_today = mktime(&end_date_today);
  time_t end_time = cal_home_advance_date_by_time_period(&end_date,
							 desktop_plugin->cal_time_period);
  time_t date_today_time = mktime(&desktop_plugin->date_today);
  time_t system_time_offset = CMulticalendar::MCInstance()->getSystemTimeShift();

  desktop_plugin->today_events = 0;

  GtkTreeIter iter;
  for(SortedCComponentSet::const_iterator event_iter = ccomponent_set.begin();
      event_iter != ccomponent_set.end() && up_to>0;
      ++event_iter)
  {
    gtk_list_store_append(list_store,
			  &iter);
    string summary = (*event_iter)->getSummary();

    if((*event_iter)->getType() == E_TODO &&
       (*event_iter)->getStatus() == 1)
    {
      gchar* escaped_markedup = g_markup_printf_escaped("<s>%s</s>", summary.c_str());
      summary = string(escaped_markedup);
      g_free(escaped_markedup);
    }
    else
    {      
      struct tm today_date_today;
      cal_home_time_today_midnight(&today_date_today);

      gchar* escaped_markedup;
      if((*event_iter)->getType() == E_TODO &&
	 _todo_is_old(*event_iter, &today_date_today))
      {
	desktop_plugin->today_events++;
	escaped_markedup = g_markup_printf_escaped("<span foreground=\"#ff0000\"><b>%s</b></span>", summary.c_str());
      }
      else
      {
	escaped_markedup = g_markup_printf_escaped("%s", summary.c_str());
      }

      summary = string(escaped_markedup);
      g_free(escaped_markedup);
    }
    time_t event_start_time = (*event_iter)->getDateStart();
    time_t event_end_time = (*event_iter)->getDateEnd();
    time_t event_time_offset = (*event_iter)->getTzOffset();
    /*
    if(CMulticalendar::MCInstance()->getSystemTimeShift() != (*event_iter)->getTzOffset() &&
       ((*event_iter)->getAllDay() ||
	(*event_iter)->getType() == E_TODO))
    {
      event_end_time += event_time_offset - system_time_offset;
      event_start_time += event_time_offset - system_time_offset;
    }
    */


    if(event_end_time > date_today_time && earliest_update_end_time>event_end_time)
    {
      earliest_update_end_time = event_end_time+60;
    }
    if(date_today_time < event_start_time &&
       earliest_update_end_time > event_start_time)
    {
      earliest_update_end_time = event_start_time+60;
    }
    char buf[255];
    //    struct tm* time = localtime(&event_start_time);
    struct tm time;
    if((*event_iter)->getType() == E_TODO ||
       ((*event_iter)->getAllDay()))
    {
      time_get_remote(event_start_time, (*event_iter)->getTzid().c_str(), &time);
    }
    else
    {
      time_get_local_ex(event_start_time, &time);
    }

    if((*event_iter)->getType() == E_TODO)
    {
      _format_todo_time(*event_iter, buf, desktop_plugin);
      gtk_list_store_set(list_store, &iter,
			 0, buf,
			 -1);
    }
    else if(cal_date_on_today((*event_iter), desktop_plugin) || 
	    (event_start_time <= date_today_time && (event_end_time == -1 || event_end_time >= date_today_time)))
    {
      ++desktop_plugin->today_events;

      if(((*event_iter)->getAllDay() || (event_start_time <= date_today_time && 
						  end_time >= end_time_today)))
	{      
	  gtk_list_store_set(list_store, &iter,
			     0, dgettext("calendar", "cal_va_allday_home"),
			     -1);
      }
      else
      {
	_format_time(buf, &time, desktop_plugin->has_24_time_format);
	gtk_list_store_set(list_store, &iter,
			   0, buf,
			   -1);
      }
    }
    else
    {
      if(desktop_plugin->show_week_day_name)
      {
	gchar* tt = g_strdup_printf("%s", dgettext("hildon-libs","wdgt_va_date_day_name_short"));
	strftime(buf, 255, tt, &time);
	g_free(tt);
      }
      else
	strftime(buf, 255,dgettext("hildon-libs","wdgt_va_date_short"), &time);
      //strftime(buf, 255,dgettext("hildon-libs","wdgt_va_date"), &time);
      gtk_list_store_set(list_store, &iter,
			 0, buf,
			 -1);
    }
    GdkPixbuf *pixbuf = get_calendar_pixbuf(*event_iter);
    
    gtk_list_store_set(list_store, &iter,
		       1, summary.c_str(), 
		       2, pixbuf,
		       -1);
    
    g_object_unref(pixbuf);
    --up_to;
  }
  
  if(desktop_plugin->time_out_handler)
  {
    g_source_remove(desktop_plugin->time_out_handler);
  }
  desktop_plugin->time_to_update = earliest_update_end_time;
}

static string
cal_put_age_on_summary(const int& born,
		       const time_t& birthday,
		       const string& summary)
{
  struct tm* birthday_time = localtime(&birthday); 
  int age = 1900 + birthday_time->tm_year - born;
  gchar* new_summary_c = g_strdup_printf("%s (%d)", summary.c_str(), age);
  string new_summary(new_summary_c);
  g_free(new_summary_c);
  return new_summary;
}

static vector<CComponent*> 
calendar_getComponents(int calendarId,
		       int type, int iStDate,
		       int iEndDate, int iLimit,
		       int iOffset,
		       int &error)
{
  vector <CComponent *>vListEntry;
  error = CALENDAR_OPERATION_SUCCESSFUL;
  
  CCalendarDB *pDb = CCalendarDB::Instance();

  int iNewStDate = 0;
  int iNewEndDate = 0;
  CMulticalendar *mc = CMulticalendar::MCInstance();
  int time_diff = time_get_time_diff(iStDate, mc->getSystemTimeZone().c_str(), "UTC");

  iNewStDate = iStDate - time_diff;
  iNewEndDate = iEndDate - time_diff;

  if (pDb == 0) 
  {
    error = CALENDAR_APP_ERROR;
    return vListEntry;
  }
  char *sqlQuery = 
    sqlite3_mprintf(
		    "select * from %s where "
		    "%s = %d  AND  %s = %d AND (%s - %s) < %d AND (%s - %s) > %d AND (%s != %d AND %s != %d) "
		    "union select * from %s where "
		    "%s = %d AND  %s = %d  AND (%s = 2  OR allday = 1) AND (%s - %s)>= %d AND (%s - %s)< %d AND (%s != %d AND %s != %d) " 
		    "union select * from %s where "
		    "%s = %d AND %s = %d  AND allday = 0 AND (%s = %d OR %s = %d) AND (%d < %s OR %d = %s) "
		    "union select * from %s where "
		    "%s = %d AND %s = %d  AND allday = 1 AND (%s = %d OR %s = %d) AND (%d < (%s + %s) OR %d = %s) "
		    "LIMIT %d OFFSET %d",
		    COMPONENTS_TABLE,ENTRY_FIELD_CALID, calendarId,
		    ENTRY_FIELD_TYPE, type,ENTRY_FIELD_DTSTART, ENTRY_FIELD_TZOFFSET, iNewEndDate,ENTRY_FIELD_DTEND, 
		    ENTRY_FIELD_TZOFFSET, iNewStDate,
		    ENTRY_FIELD_FLAGS, HAS_RECURRENCE,ENTRY_FIELD_FLAGS, HAS_RECURRENCE_ALARM,
		    
		    COMPONENTS_TABLE,ENTRY_FIELD_CALID, calendarId, ENTRY_FIELD_TYPE, type, ENTRY_FIELD_TYPE,
		    ENTRY_FIELD_DTSTART, ENTRY_FIELD_TZOFFSET, iNewStDate, 
		    ENTRY_FIELD_DTSTART, ENTRY_FIELD_TZOFFSET, iNewEndDate,
		    ENTRY_FIELD_FLAGS, HAS_RECURRENCE,ENTRY_FIELD_FLAGS, HAS_RECURRENCE_ALARM, 
		    
		    COMPONENTS_TABLE,ENTRY_FIELD_CALID, calendarId, ENTRY_FIELD_TYPE, type,
		    ENTRY_FIELD_FLAGS, HAS_RECURRENCE,ENTRY_FIELD_FLAGS, HAS_RECURRENCE_ALARM,iStDate, 
		    ENTRY_FIELD_UNTIL,NULLID, ENTRY_FIELD_UNTIL,
		    
		    COMPONENTS_TABLE,ENTRY_FIELD_CALID, calendarId,ENTRY_FIELD_TYPE, type, 
		    ENTRY_FIELD_FLAGS, HAS_RECURRENCE,ENTRY_FIELD_FLAGS, HAS_RECURRENCE_ALARM,
		    iNewStDate, ENTRY_FIELD_UNTIL, ENTRY_FIELD_TZOFFSET,NULLID, ENTRY_FIELD_UNTIL,
		    iLimit, iOffset);

  int sqliteError = 0;
  QueryResult *queryResult = pDb->getRecords(sqlQuery,sqliteError);
  pDb->sqliteErrorMapper(sqliteError, error);
  sqlite3_free(sqlQuery);

  if(queryResult == 0) 
  {
    if(error == CALENDAR_OPERATION_SUCCESSFUL)
    {
      error = CALENDAR_FETCH_NOITEMS;
    }
  }
  else
  {
    for(int i = 0; i < queryResult->iRow; i++) 
    {
      CComponent *componentEntry = new CComponent();
      ASSERTION(componentEntry);
      componentEntry->setCalendarId(calendarId);

      for(int j = 0; j < queryResult->iColumn; j++) 
      {
	int k = queryResult->iColumn + (i * queryResult->iColumn);

	if ((queryResult->pResult[k + j]) == 0)
	  continue;

	switch (j) 
	{
	case DB_COLUMN_ID2:
	  break;
	case DB_COLUMN_ID1:
	  componentEntry->setId(queryResult->pResult[k + j]);
	  break;
	case DB_COLUMN_ID3:
	  componentEntry->setType(atoi(queryResult->pResult[k + j]));
	  break;
	case DB_COLUMN_ID4:
	  componentEntry->setFlags(atoi(queryResult->pResult[k + j]));
	  break;
	case DB_COLUMN_ID5:
	  componentEntry->setDateStart(atoi(queryResult->pResult[k + j]));
	  break;
	case DB_COLUMN_ID6:
	  componentEntry-> setDateEnd(atoi(queryResult->pResult[k + j]));
	  break;
	case DB_COLUMN_ID7:
	  if(queryResult->pResult[k + j]) 
	  {
	    componentEntry->setSummary((string) queryResult-> pResult[k + j]);
	  }
	  break;
	case DB_COLUMN_ID8:
	  if(queryResult->pResult[k + j]) 
	  {
	    componentEntry->setLocation((string) queryResult->pResult[k + j]);
	  }
	  break;
	case DB_COLUMN_ID9:
	  if(queryResult->pResult[k + j]) 
	  {
	    componentEntry->setDescription((string) queryResult->pResult[k + j]);
	  }
	  break;
	case DB_COLUMN_ID10:
	  componentEntry->setStatus(atoi(queryResult->pResult[k + j]));
	  break;
	case DB_COLUMN_ID11:
	  if(queryResult->pResult[k + j]) 
	  {
	    componentEntry->setGUid(queryResult->pResult[k + j]);
	  }
	  break;
	case DB_COLUMN_ID12:
	  componentEntry-> setUntil(atoi (queryResult->pResult[k + j]));
	  break;
	case DB_COLUMN_ID13:
	  componentEntry-> setAllDay(atoi (queryResult->pResult[k + j]));
	  break;
	case DB_COLUMN_ID14:
	  componentEntry->setCreatedTime(atoi(queryResult->pResult[k + j]));
	  break;
	case DB_COLUMN_ID15:
	  componentEntry->setLastModified(atoi(queryResult->pResult[k + j]));
	  break;
	case DB_COLUMN_ID16:
	  componentEntry->setTzid(queryResult->pResult[k + j]);
	  break;
	default:
	  break;
	} 
      }

      if(componentEntry->getFlags() == HAS_ALARM || componentEntry->getFlags() == HAS_RECURRENCE_ALARM)
	componentEntry->getAlarmProperties();
    
      if(componentEntry->getFlags() == HAS_RECURRENCE || componentEntry->getFlags() == HAS_RECURRENCE_ALARM)
      {
	componentEntry->getRecurrenceProperties();
	if(componentEntry->getInstanceNumber(iStDate, iEndDate)) 
	{
	  vListEntry.push_back(componentEntry);
	}
	else 
	{
	  delete componentEntry;
	  componentEntry = 0;
	}
      }
      else
      {
	vListEntry.push_back(componentEntry);
      }
    }
    if(queryResult) 
    {
      sqlite3_free_table(queryResult->pResult);
      delete queryResult;
    }
  }
  return vListEntry;
}

static void
cal_read_todos_for_calendar(CCalendar* calendar,
			    time_t start_time,
			    time_t end_time,
			    SortedCComponentSet& all_components,
			    CalHomePlugin* desktop_plugin)
{
  int error = 0;
  int offset = 0;
  int size = 0;
  int limit = 200;
  struct tm today_midnight; 
  cal_home_time_today_midnight(&today_midnight);
  do
  {
    vector<CComponent*> calendars_components = calendar_getComponents(calendar->getCalendarId(),
								      E_TODO,
								      start_time, 
								      end_time,
								      limit,
								      offset,
								      error);

    size = calendars_components.size();
    offset+=200;
    limit+=200;
    time_t last_start_date = start_time;
    for(int k=0;k<calendars_components.size();++k)
    {
      CComponent* component = calendars_components[k];
      time_t event_time = component->getDateStart();
      if(_todo_is_old(component, &today_midnight))
      {
	if((component->getStatus() == 0) &&
	   (desktop_plugin->show_old_undone_tasks))
	{
	  all_components.insert(component);
	}
	else
	{
	  delete component;
	}
      }
      else if((component->getStatus() == 0) ||
	      desktop_plugin->show_done_tasks)
      {
	all_components.insert(component);
      }
      else
      {
	delete component;
      }
    }
  }while(size != 0);
}

static void
cal_read_ccomponents_for_calendar(CCalendar* calendar,
				  time_t start_time,
				  time_t end_time,
				  SortedCComponentSet& all_components,
				  gboolean birthday_only,
				  CalHomePlugin* desktop_plugin)
{
  int error = 0;
  int offset = 0;
  int size = 0;
  int limit = 500;
  do
  {
    int event_type = E_EVENT;
    if(birthday_only)
      event_type = E_BDAY;
    gchar buf[255];

    vector<CComponent*> calendars_components = calendar_getComponents(calendar->getCalendarId(),
								      event_type,
								      start_time, end_time,
								      limit,
								      offset,
								      error);

    size = calendars_components.size();
    offset=limit+1;
    limit+=500;
    time_t last_start_date = start_time;
    struct tm today_midnight; 
    
    cal_home_time_today_midnight(&today_midnight);
    time_t today_midnight_time = mktime(&today_midnight);
    for(int k = 0;k<calendars_components.size();++k)
    {
      CComponent* component = calendars_components[k];
      vector<time_t> instance_times =
	component->getInstanceTimes(last_start_date, 
				    end_time);

      if((FALSE==birthday_only && (calendars_components[k]->getType() == E_BDAY)) 
	 ||
	 (birthday_only && (calendars_components[k]->getType() == E_EVENT))
	 )
      {
	delete calendars_components[k];
      }
      else
      {
	if(instance_times.empty())
	{
	  all_components.insert(component);
	}
	else
	{
	  time_t system_time_offset = CMulticalendar::MCInstance()->getSystemTimeShift();
	  vector<time_t>::const_iterator iter = instance_times.begin();
	  do
	  {
	    // clone entry with instance time as start time
	    // and set end time 
	    CComponent* c = new CComponent(*component);
	    c->setDateEnd((*iter) + c->getDateEnd() - c->getDateStart());
	    c->setDateStart((*iter));

	    if(c->getType() == E_BDAY &&
	       desktop_plugin->show_age)
	    {
	      string new_summary = cal_put_age_on_summary(component->getStatus(), 
							  c->getDateStart(),
							  c->getSummary());
	      c->setSummary(new_summary);
	    }
	    all_components.insert(c);
	    ++iter;
	  }while(iter!=instance_times.end() && desktop_plugin->show_all_recurrences);
	  delete component;
	}
      }
    }
  }while(error == CALENDAR_OPERATION_SUCCESSFUL);
}

static void
cal_read_ccomponents_all_visible(SortedCComponentSet& all_components,
				 CalHomePlugin* desktop_plugin)
{
  time_t start_time = mktime(&desktop_plugin->date_today);
  struct tm end_date = desktop_plugin->date_today;
  time_t end_time = cal_home_advance_date_by_time_period(&end_date,
							 desktop_plugin->cal_time_period);
  struct tm today_midnight; 
  cal_home_time_today_midnight(&today_midnight);
  time_t today_midnight_time = mktime(&today_midnight);

  for(int i=0;i<desktop_plugin->number_visible_cals;++i)
  {
    int error = 0;
    CCalendar* cal = CMulticalendar::MCInstance()->getCalendarById(desktop_plugin->visible_cals[i], error);
    if(error == 500)
    {
      
      if(desktop_plugin->visible_entry_mode == VISIBLE_ENTRY_EVENT ||
	 desktop_plugin->visible_entry_mode == VISIBLE_ENTRY_EVENT_AND_TASK)
      {
	cal_read_ccomponents_for_calendar(cal,
					  start_time,
					  end_time,
					  all_components, FALSE,
					  desktop_plugin);
	
	// gnark, birthday events start and end
	// at 00:00:00
	
	cal_read_ccomponents_for_calendar(cal,
					  today_midnight_time,
					  end_time,
					  all_components, TRUE,
					  desktop_plugin);
      }
      if(desktop_plugin->visible_entry_mode == VISIBLE_ENTRY_TASK ||
	 desktop_plugin->visible_entry_mode == VISIBLE_ENTRY_EVENT_AND_TASK)
      {
	if(desktop_plugin->show_old_undone_tasks)
	{
	  cal_read_todos_for_calendar(cal,
				      -1,
				      end_time,
				      all_components,
				      desktop_plugin);
	}
	else
	{
	  cal_read_todos_for_calendar(cal, 
				      today_midnight_time, 
				      end_time, 
				      all_components, 
				      desktop_plugin); 
	}
      }
    }
    delete cal;
  }
}

void 
cal_home_load_db_events(CalHomePlugin* desktop_plugin)
{
  cal_home_time_today_now(&desktop_plugin->date_today);
  gtk_list_store_clear(desktop_plugin->event_list);
  SortedCComponentSet all_components;
  cal_read_ccomponents_all_visible(all_components,
				   desktop_plugin);
  cal_condense_ccomponents_set_to_list_store(all_components,
					     desktop_plugin->event_list,
					     desktop_plugin,
					     cal_row_count_for_cal_size(desktop_plugin->cal_row_count));
  cal_delete_ccomponentset_items(all_components);
  cal_fill_event_list_ui(desktop_plugin);
  desktop_plugin->update_view = FALSE;
  gtk_widget_show_all(desktop_plugin->event_list_container);
}
