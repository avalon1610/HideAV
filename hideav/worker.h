#pragma once
#include "stdafx.h"

typedef struct _HAV_CONTEXT {
	HANDLE Port;
	BOOLEAN CleaningUp;
	HANDLE ShutDown;
} HAV_CONTEXT,*PHAV_CONTEXT;