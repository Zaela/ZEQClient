
#ifndef _ZEQ_ZONE_VIEWER_H
#define _ZEQ_ZONE_VIEWER_H

#include "types.h"

struct ZoneViewerData
{
	ZoneViewerData()
	{
		applyGravity = false;
		applyCollision = false;
	}

	bool applyGravity;
	bool applyCollision;
};

#endif
