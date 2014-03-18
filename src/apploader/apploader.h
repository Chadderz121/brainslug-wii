/* apploader.h
 *   by Alex Chadwick
 */

#ifndef APPLOADER_H_
#define APPLOADER_H_

#include <stdbool.h>

#include "library/event.h"

typedef void (*apploader_game_entry_t)(void);

extern event_t apploader_event_disk_id;
extern event_t apploader_event_complete;
extern apploader_game_entry_t apploader_game_entry_fn;

bool Apploader_Init(void);
bool Apploader_RunBackground(void);

#endif /* APPLOADER_H_ */
