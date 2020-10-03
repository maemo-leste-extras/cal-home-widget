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
#ifndef CAL_HOME_CALENDAR_UTIL_H
#define CAL_HOME_CALENDAR_UTIL_H


#include <set>
#include <CComponent.h>
#include "cal-home-widget.h"


struct CComponentCompare
{
  bool operator()(CComponent* c1, CComponent* c2)
  {
    return c1->getDateStart() < c2->getDateStart();
  }
};

typedef multiset<CComponent*, CComponentCompare> SortedCComponentSet;

void
cal_home_load_db_events(CalHomePlugin* desktop_plugin);
void
cal_home_time_today_midnight(struct tm* today);
#endif
