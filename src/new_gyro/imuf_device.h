#pragma once
#include "includes.h"
#include "gyro.h"

extern void imuf_write_data(filteredData_t* data);
extern void imuf_write_quaternion_data(quaternion_record_t* data);