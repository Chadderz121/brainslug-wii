/* apploader.h
 *   by Alex Chadwick
 */

#ifndef APPLOADER_H_
#define APPLOADER_H_

#include "library/event.h"

typedef void (*apploader_game_entry_t)(void);

extern event_t apploader_event_disk_id;
extern event_t apploader_event_complete;
extern apploader_game_entry_t apploader_game_entry_fn;

int apploader_run_background(void);

#endif /* APPLOADER_H_ */
