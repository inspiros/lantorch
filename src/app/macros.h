#pragma once

/**
 * Get default instance's parameter of option structs,
 * given that they implement default_instance().
 */
#define DEFAULT_OPTION(PARAM) default_instance().PARAM()
