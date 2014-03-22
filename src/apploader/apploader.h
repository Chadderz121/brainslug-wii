/* apploader.h
 *   by Alex Chadwick
 */

#ifndef APPLOADER_H_
#define APPLOADER_H_

#include <stdbool.h>
#include <stdint.h>

#include "library/event.h"

typedef void (*apploader_game_entry_t)(void);

extern event_t apploader_event_disk_id;
extern event_t apploader_event_complete;
extern apploader_game_entry_t apploader_game_entry_fn;
extern uint8_t *apploader_app0_start;
extern uint8_t *apploader_app0_end;
extern uint8_t *apploader_app1_start;
extern uint8_t *apploader_app1_end;

bool Apploader_Init(void);
bool Apploader_RunBackground(void);

#endif /* APPLOADER_H_ */
