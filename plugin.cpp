/*
 * Fledge "scale" filter plugin.
 *
 * Copyright (c) 2018 Dianomic Systems
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Massimiliano Pinto
 */

#include <plugin_api.h>
#include <config_category.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string>
#include <iostream>
#include <filter_plugin.h>
#include <filter.h>
#include <reading_set.h>
#include <regex>
#include <version.h>

#define FILTER_NAME "scale"
#define SCALE_FACTOR "100.0"
#define DEFAULT_CONFIG "{\"plugin\" : { \"description\" : \"Scale filter plugin\", " \
                       		"\"type\" : \"string\", " \
				"\"default\" : \"" FILTER_NAME "\", \"readonly\": \"true\" }, " \
			 "\"enable\": {\"description\": \"A switch that can be used to enable or disable execution of " \
					 "the scale filter.\", " \
				"\"type\": \"boolean\", " \
				"\"displayName\": \"Enabled\", " \
				"\"default\": \"false\" }, " \
			"\"factor\" : {\"description\" : \"Scale factor for a reading value.\", " \
				"\"type\": \"float\", " \
				"\"default\": \"" SCALE_FACTOR "\", " \
				"\"order\" : \"1\", \"displayName\": \"Scale Factor\"}, " \
			"\"offset\" : {\"description\" : \"A constant offset to add to every value.\", " \
				"\"type\": \"float\", " \
				"\"default\": \"0.0\", " \
				"\"order\": \"2\", \"displayName\": \"Constant Offset\"}, " \
			"\"match\" : {\"description\" : \"An optional regular expression to match in the asset name.\", " \
				"\"type\": \"string\", " \
				"\"default\": \"\", " \
				"\"order\": \"3\", \"displayName\": \"Asset filter\"} }"
using namespace std;

/**
 * The Filter plugin interface
 */
extern "C" {

/**
 * The plugin information structure
 */
static PLUGIN_INFORMATION info = {
        FILTER_NAME,              // Name
        VERSION,                  // Version
        0,                        // Flags
        PLUGIN_TYPE_FILTER,       // Type
        "1.0.0",                  // Interface version
	DEFAULT_CONFIG	          // Default plugin configuration
};

typedef struct
{
	FledgeFilter	*handle;
	std::string	configCatName;
} FILTER_INFO;

/**
 * Return the information about this plugin
 */
PLUGIN_INFORMATION *plugin_info()
{
	return &info;
}

/**
 * Initialise the plugin, called to get the plugin handle and setup the
 * output handle that will be passed to the output stream. The output stream
 * is merely a function pointer that is called with the output handle and
 * the new set of readings generated by the plugin.
 *     (*output)(outHandle, readings);
 * Note that the plugin may not call the output stream if the result of
 * the filtering is that no readings are to be sent onwards in the chain.
 * This allows the plugin to discard data or to buffer it for aggregation
 * with data that follows in subsequent calls
 *
 * @param config	The configuration category for the filter
 * @param outHandle	A handle that will be passed to the output stream
 * @param output	The output stream (function pointer) to which data is passed
 * @return		An opaque handle that is used in all subsequent calls to the plugin
 */
PLUGIN_HANDLE plugin_init(ConfigCategory* config,
			  OUTPUT_HANDLE *outHandle,
			  OUTPUT_STREAM output)
{
	FILTER_INFO *info = new FILTER_INFO;
	info->handle = new FledgeFilter(FILTER_NAME,
					*config,
					outHandle,
					output);
	info->configCatName = config->getName();

	return (PLUGIN_HANDLE)info;
}

/**
 * Ingest a set of readings into the plugin for processing
 *
 * @param handle	The plugin handle returned from plugin_init
 * @param readingSet	The readings to process
 */
void plugin_ingest(PLUGIN_HANDLE *handle,
		   READINGSET *readingSet)
{
	FILTER_INFO *info = (FILTER_INFO *) handle;
	FledgeFilter* filter = info->handle;
	
	if (!filter->isEnabled())
	{
		// Current filter is not active: just pass the readings set
		filter->m_func(filter->m_data, readingSet);
		return;
	}

	// Get filter configuration
	double scaleFactor;
	if (filter->getConfig().itemExists("factor"))
	{
		scaleFactor = strtod(filter->getConfig().getValue("factor").c_str(), NULL);
	}
	else
	{
		scaleFactor = strtod(SCALE_FACTOR, NULL);
	}
	double offset = 0.0;
	if (filter->getConfig().itemExists("offset"))
	{
		offset = strtod(filter->getConfig().getValue("offset").c_str(), NULL);
	}
	string match;
	regex  *re = 0;
	if (filter->getConfig().itemExists("match"))
	{
		match = filter->getConfig().getValue("match");
		re = new regex(match);
	}

	// 1- We might need to transform the inout readings set: example
	// ReadingSet* newReadings = scale_readings(scaleFactor, readingSet);

	// Just get all the readings in the readingset
	const vector<Reading *>& readings = ((ReadingSet *)readingSet)->getAllReadings();

	AssetTracker *tracker = AssetTracker::getAssetTracker();
	// Iterate over the readings
	for (vector<Reading *>::const_iterator elem = readings.begin();
						      elem != readings.end();
						      ++elem)
	{
		if (tracker)
		{
			tracker->addAssetTrackingTuple(info->configCatName, (*elem)->getAssetName(), string("Filter"));
		}
		if (!match.empty())
		{
			string asset = (*elem)->getAssetName();
			if (! regex_match(asset, *re))
			{
				continue;
			}
		}
		// Get a reading DataPoint
		const vector<Datapoint *>& dataPoints = (*elem)->getReadingData();
		// Iterate over the datapoints
		for (vector<Datapoint *>::const_iterator it = dataPoints.begin(); it != dataPoints.end(); ++it)
		{
			// Get the reference to a DataPointValue
			DatapointValue& value = (*it)->getData();

			/*
			 * Deal with the T_INTEGER and T_FLOAT types.
			 * Try to preserve the type if possible but
			 * if a flaoting point scale or offset is applied
			 * then T_INTEGER values will turn into T_FLOAT.
			 */
			if (value.getType() == DatapointValue::T_INTEGER)
			{
				double newValue = value.toInt() * scaleFactor + offset;
				if (newValue == floor(newValue))
				{
					value.setValue(newValue);
				}
				else
				{
					value.setValue((long)newValue);
				}
			}
			else if (value.getType() == DatapointValue::T_FLOAT)
			{
				value.setValue(value.toDouble() * scaleFactor + offset);
			}
			else
			{
				// do nothing
			}
		}
	}

	// 2- optionally free reading set
	// delete (ReadingSet *)readingSet;
	// With the above DataPointValue change we don't need to free input data

	// 3- pass newReadings to filter->m_func instead of readings if needed.
	// With the value change we can pass same input readingset just modified
	filter->m_func(filter->m_data, readingSet);

	if (re)
		delete re;
}

/**
 * Plugin reconfiguration method
 *
 * @param handle	The plugin handle
 * @param newConfig	The updated configuration
 */
void plugin_reconfigure(PLUGIN_HANDLE *handle, const std::string& newConfig)
{
	FILTER_INFO *info = (FILTER_INFO *)handle;
	FledgeFilter* data = info->handle;
	data->setConfig(newConfig);
}

/**
 * Call the shutdown method in the plugin
 */
void plugin_shutdown(PLUGIN_HANDLE *handle)
{
	FILTER_INFO *info = (FILTER_INFO *) handle;
	delete info->handle;
	delete info;
}

// End of extern "C"
};
