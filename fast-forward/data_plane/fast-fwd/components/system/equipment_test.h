#ifndef _EQUIPMENT_TEST_H_
#define _EQUIPMENT_TEST_H_

#include "cvmx-config.h"
#include "cvmx.h"

#define AC4X_PORT_VEF1 16
#define AC4X_PORT_VES1 2112
#define AC4X_PORT_VES2 2624
#define AC4X_PORT_VES3 3136

int32_t fwd_equipment_test_AC4X(cvmx_wqe_t* work);

#endif