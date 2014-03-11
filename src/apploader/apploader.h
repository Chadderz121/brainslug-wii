/* apploader.h
 *   by Alex Chadwick
 */

#ifndef APPLOADER_H_
#define APPLOADER_H_

#include "library/event.h"

extern event_t apploader_event_disk_id;

int apploader_run_background(void);

#endif /* APPLOADER_H_ */