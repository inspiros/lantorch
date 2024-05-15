#pragma once

/**
 * Get default instance's parameter of option structs,
 * given that they implement default constructor.
 */
#define DEFAULT_PARAM(CLS, PARAM) CLS().PARAM()
