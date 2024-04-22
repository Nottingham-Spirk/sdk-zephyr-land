/*
 * Copyright (c) 2019 Foundries.io
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Source material for ebike object (42769):
 */

#define LOG_MODULE_NAME net_ebike_object
#define LOG_LEVEL CONFIG_LWM2M_LOG_LEVEL

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#include <stdint.h>
#include <zephyr/init.h>

#include "lwm2m_object.h"
#include "lwm2m_engine.h"
#include "lwm2m_resource_ids.h"
#include "custom_ebike.h"

#define EBIKE_VERSION_MAJOR 1
#define EBIKE_VERSION_MINOR 0
#define EBIKE_MAX_ID 6


#define MAX_INSTANCE_COUNT	1

/*
 * Calculate resource instances as follows:
 * start with EBIKE_MAX_ID
 */
#define RESOURCE_INSTANCE_COUNT        (EBIKE_MAX_ID)

/* resource state */
struct net_ebike_data {
	int16_t vehicalSpeedKmph;
	uint32_t totalDistanceKmDivTimes10;
	uint32_t tripDistanceKmTimes100;
	int16_t batteryVoltage;
	int16_t batteryCurrent;
	uint8_t  stateOfCharge;
};

static struct net_ebike_data ebike_data[MAX_INSTANCE_COUNT];

static struct lwm2m_engine_obj ebike;
static struct lwm2m_engine_obj_field fields[] = {
	OBJ_FIELD_DATA(SPEED_RID, R, U16),
	OBJ_FIELD_DATA(TOTAL_ODO_RID, R, U32),
	OBJ_FIELD_DATA(TRIP_ODO_RID, R, U32),
	OBJ_FIELD_DATA(BATT_V_RID, R, U16),
	OBJ_FIELD_DATA(BATT_I_RID, R, U16),
	OBJ_FIELD_DATA(BATT_SOC_RID, R, U16),
};

static struct lwm2m_engine_obj_inst inst[MAX_INSTANCE_COUNT];
static struct lwm2m_engine_res res[MAX_INSTANCE_COUNT][RESOURCE_INSTANCE_COUNT];
static struct lwm2m_engine_res_inst
		res_inst[MAX_INSTANCE_COUNT][RESOURCE_INSTANCE_COUNT];

static struct lwm2m_engine_obj_inst *ebike_create(uint16_t obj_inst_id)
{
	int index, avail = -1, i = 0, j = 0;

	/* Check that there is no other instance with this ID */
	for (index = 0; index < ARRAY_SIZE(inst); index++) {
		if (inst[index].obj && inst[index].obj_inst_id == obj_inst_id) {
			LOG_ERR("Can not create instance - "
				"already existing: %u", obj_inst_id);
			return NULL;
		}

		/* Save first available slot index */
		if (avail < 0 && !inst[index].obj) {
			avail = index;
		}
	}

	if (avail < 0) {
		LOG_ERR("Can not create instance - no more room: %u",
			obj_inst_id);
		return NULL;
	}

	/* Set default values */
	(void)memset(&ebike_data[avail], 0, sizeof(ebike_data[avail]));

	(void)memset(res[avail], 0,
		     sizeof(res[avail][0]) * ARRAY_SIZE(res[avail]));
	init_res_instance(res_inst[avail], ARRAY_SIZE(res_inst[avail]));

	/* initialize instance resource data */
	INIT_OBJ_RES_DATA(SPEED_RID, res[avail], i, res_inst[avail], j,
			  &ebike_data[avail].vehicalSpeedKmph,
			  sizeof(ebike_data[avail].vehicalSpeedKmph));
	INIT_OBJ_RES_DATA(TOTAL_ODO_RID, res[avail], i, res_inst[avail], j,
			  &ebike_data[avail].totalDistanceKmDivTimes10,
			  sizeof(ebike_data[avail].totalDistanceKmDivTimes10));
	INIT_OBJ_RES_DATA(TRIP_ODO_RID, res[avail], i, res_inst[avail], j,
			  &ebike_data[avail].tripDistanceKmTimes100,
			  sizeof(ebike_data[avail].tripDistanceKmTimes100));
	INIT_OBJ_RES_DATA(BATT_V_RID, res[avail], i, res_inst[avail], j,
			  &ebike_data[avail].batteryVoltage,
			  sizeof(ebike_data[avail].batteryVoltage));
	INIT_OBJ_RES_DATA(BATT_I_RID, res[avail], i, res_inst[avail],
			  j, &ebike_data[avail].batteryCurrent,
			  sizeof(ebike_data[avail].batteryCurrent));
	INIT_OBJ_RES_DATA(BATT_SOC_RID, res[avail], i, res_inst[avail],
			  j, &ebike_data[avail].stateOfCharge,
			  sizeof(ebike_data[avail].stateOfCharge));

	inst[avail].resources = res[avail];
	inst[avail].resource_count = i;

	LOG_DBG("Create ebike instance: %d", obj_inst_id);

	return &inst[avail];
}

static int net_ebike_init(void)
{
	ebike.obj_id = CUSTOM_OBJECT_EBIKE_ID;
	ebike.version_major = EBIKE_VERSION_MAJOR;
	ebike.version_minor = EBIKE_VERSION_MINOR;
	ebike.is_core = false;
	ebike.fields = fields;
	ebike.field_count = ARRAY_SIZE(fields);
	ebike.max_instance_count = ARRAY_SIZE(inst);
	ebike.create_cb = ebike_create;
	lwm2m_register_obj(&ebike);

	return 0;
}

SYS_INIT(net_ebike_init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
