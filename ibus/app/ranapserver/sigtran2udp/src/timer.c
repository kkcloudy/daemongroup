/****************************************************************************
** Description:
** Code to start and process expiry of the timer. Sample application
** needs just one timer.
*****************************************************************************
** Copyright(C) 2005 Shabd Communications Pvt. Ltd. http://www.shabdcom.org
*****************************************************************************
** Contact:
** vkgupta@shabdcom.org
*****************************************************************************
** License :
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*****************************************************************************/

#include <timer.h>

static int	remaining_dur = 2;
static void	*timer_data;

int um3_timer_start(int dur, void *data)
{
    remaining_dur = dur;
    timer_data = data;
    return 0;
}

void * um3_timer_ckexpiry(void *p)
{		
    if (0 != remaining_dur) {
        remaining_dur--;
        if (0 == remaining_dur)
            um3_timer_expiry(timer_data);
    }
    return 0;
}

