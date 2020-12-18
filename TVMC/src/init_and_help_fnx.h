#ifndef INIT_AND_HELP_FNX_H
#define INIT_AND_HELP_FNX_H

#include "config.h"

#define GET_TVSTATE (digitalRead(TVSTATE))
#define GET_ET1 (digitalRead(ET1))
#define GET_ET2 (digitalRead(ET2))

void init_gpios( void );


#endif /* MYHEADER_H */