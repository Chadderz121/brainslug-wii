/* <bslug/version.h>
 *  by Alex Chadwick
 */

#ifndef BSLUG_VERSION_H_
#define BSLUG_VERSION_H_

#include <stdint.h>

#define BSLUG_VERSION(maj, min, rev) (((maj) << 24) | ((min) << 16) | (rev))

#define BSLUG_VERSION_MAJOR(ver)    ((uint8_t)(((ver) >> 24) & 0xff))
#define BSLUG_VERSION_MINOR(ver)    ((uint8_t)(((ver) >> 16) & 0xff))
#define BSLUG_VERSION_REVISION(ver) ((uint16_t)(((ver) >> 0) & 0xffff))

#define BSLUG_LIB_VERSION BSLUG_VERSION(0, 1, 2)

#endif /* BSLUG_VERSION_H_*/