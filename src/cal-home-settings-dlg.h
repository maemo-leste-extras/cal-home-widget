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
#ifndef CAL_HOME_SETTINGS_DLG_H
#define CAL_HOME_SETTINGS_DLG_H
#include <gtk/gtk.h>
#include "cal-home-widget.h"

typedef struct _cal_settings_data_t cal_settings_data_t;

struct _cal_settings_data_t
{
  gsize number_visible_cals;
  gint* visible_cals;
};

void
cal_show_settings_dialog(GtkWidget* widget, gpointer data);


#endif
