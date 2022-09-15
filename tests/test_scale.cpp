#include <gtest/gtest.h>
#include <plugin_api.h>
#include <config_category.h>
#include <filter_plugin.h>
#include <filter.h>
#include <string.h>
#include <string>
#include <rapidjson/document.h>
#include <reading.h>
#include <reading_set.h>

using namespace std;
using namespace rapidjson;

extern "C" {
	PLUGIN_INFORMATION *plugin_info();
	void plugin_ingest(void *handle,
                   READINGSET *readingSet);
	PLUGIN_HANDLE plugin_init(ConfigCategory* config,
			  OUTPUT_HANDLE *outHandle,
			  OUTPUT_STREAM output);
	int called = 0;

	void Handler(void *handle, READINGSET *readings)
	{
		called++;
		*(READINGSET **)handle = readings;
	}
};

TEST(SCALE, ScaleInteger)
{
	PLUGIN_INFORMATION *info = plugin_info();
	ConfigCategory *config = new ConfigCategory("scale", info->config);
	ASSERT_NE(config, (ConfigCategory *)NULL);
	config->setItemsValueFromDefault();
	ASSERT_EQ(config->itemExists("factor"), true);
	config->setValue("factor", "2");
	config->setValue("enable", "true");
	ReadingSet *outReadings;
	void *handle = plugin_init(config, &outReadings, Handler);
	vector<Reading *> *readings = new vector<Reading *>;

	long testValue = 2;
	DatapointValue dpv(testValue);
	Datapoint *value = new Datapoint("test", dpv);
	Reading *in = new Reading("test", value);
	readings->push_back(in);

	ReadingSet readingSet(readings);
	plugin_ingest(handle, (READINGSET *)&readingSet);


	vector<Reading *>results = outReadings->getAllReadings();
	ASSERT_EQ(results.size(), 1);
	Reading *out = results[0];
	ASSERT_STREQ(out->getAssetName().c_str(), "test");
	ASSERT_EQ(out->getDatapointCount(), 1);
	vector<Datapoint *> points = out->getReadingData();
	ASSERT_EQ(points.size(), 1);
	Datapoint *outdp = points[0];
	ASSERT_STREQ(outdp->getName().c_str(), "test");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
	ASSERT_EQ(outdp->getData().toDouble(), 4.0);
}


TEST(SCALE, ScaleDouble)
{
	PLUGIN_INFORMATION *info = plugin_info();
	ConfigCategory *config = new ConfigCategory("scale", info->config);
	ASSERT_NE(config, (ConfigCategory *)NULL);
	config->setItemsValueFromDefault();
	ASSERT_EQ(config->itemExists("factor"), true);
	config->setValue("factor", "2");
	config->setValue("enable", "true");
	ReadingSet *outReadings;
	void *handle = plugin_init(config, &outReadings, Handler);
	vector<Reading *> *readings = new vector<Reading *>;

	double testValue = 1.5;
	DatapointValue dpv(testValue);
	Datapoint *value = new Datapoint("test", dpv);
	Reading *in = new Reading("test", value);
	readings->push_back(in);

	ReadingSet readingSet(readings);
	plugin_ingest(handle, (READINGSET *)&readingSet);


	vector<Reading *>results = outReadings->getAllReadings();
	ASSERT_EQ(results.size(), 1);
	Reading *out = results[0];
	ASSERT_STREQ(out->getAssetName().c_str(), "test");
	ASSERT_EQ(out->getDatapointCount(), 1);
	vector<Datapoint *> points = out->getReadingData();
	ASSERT_EQ(points.size(), 1);
	Datapoint *outdp = points[0];
	ASSERT_STREQ(outdp->getName().c_str(), "test");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
	ASSERT_EQ(outdp->getData().toDouble(), 3.0);
}

TEST(SCALE, ScaleDisabled)
{
	PLUGIN_INFORMATION *info = plugin_info();
	ConfigCategory *config = new ConfigCategory("scale", info->config);
	ASSERT_NE(config, (ConfigCategory *)NULL);
	config->setItemsValueFromDefault();
	ASSERT_EQ(config->itemExists("factor"), true);
	config->setValue("factor", "2");
	ReadingSet *outReadings;
	void *handle = plugin_init(config, &outReadings, Handler);
	vector<Reading *> *readings = new vector<Reading *>;

	long testValue = 2;
	DatapointValue dpv(testValue);
	Datapoint *value = new Datapoint("test", dpv);
	Reading *in = new Reading("test", value);
	readings->push_back(in);

	ReadingSet readingSet(readings);
	plugin_ingest(handle, (READINGSET *)&readingSet);


	vector<Reading *>results = outReadings->getAllReadings();
	ASSERT_EQ(results.size(), 1);
	Reading *out = results[0];
	ASSERT_STREQ(out->getAssetName().c_str(), "test");
	ASSERT_EQ(out->getDatapointCount(), 1);
	vector<Datapoint *> points = out->getReadingData();
	ASSERT_EQ(points.size(), 1);
	Datapoint *outdp = points[0];
	ASSERT_STREQ(outdp->getName().c_str(), "test");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_INTEGER);
	ASSERT_EQ(outdp->getData().toInt(), 2);
}

TEST(SCALE, ScaleString)
{
	PLUGIN_INFORMATION *info = plugin_info();
	ConfigCategory *config = new ConfigCategory("scale", info->config);
	ASSERT_NE(config, (ConfigCategory *)NULL);
	config->setItemsValueFromDefault();
	ASSERT_EQ(config->itemExists("factor"), true);
	config->setValue("factor", "2");
	config->setValue("enable", "true");
	ReadingSet *outReadings;
	void *handle = plugin_init(config, &outReadings, Handler);
	vector<Reading *> *readings = new vector<Reading *>;

	string testValue = "Untouched";
	DatapointValue dpv(testValue);
	Datapoint *value = new Datapoint("test", dpv);
	Reading *in = new Reading("test", value);
	readings->push_back(in);

	ReadingSet readingSet(readings);
	plugin_ingest(handle, (READINGSET *)&readingSet);


	vector<Reading *>results = outReadings->getAllReadings();
	ASSERT_EQ(results.size(), 1);
	Reading *out = results[0];
	ASSERT_STREQ(out->getAssetName().c_str(), "test");
	ASSERT_EQ(out->getDatapointCount(), 1);
	vector<Datapoint *> points = out->getReadingData();
	ASSERT_EQ(points.size(), 1);
	Datapoint *outdp = points[0];
	ASSERT_STREQ(outdp->getName().c_str(), "test");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_STRING);
	ASSERT_STREQ(outdp->getData().toStringValue().c_str(), "Untouched");
}

TEST(SCALE, ScaleMultiDP)
{
	PLUGIN_INFORMATION *info = plugin_info();
	ConfigCategory *config = new ConfigCategory("scale", info->config);
	ASSERT_NE(config, (ConfigCategory *)NULL);
	config->setItemsValueFromDefault();
	ASSERT_EQ(config->itemExists("factor"), true);
	config->setValue("factor", "2");
	config->setValue("enable", "true");
	ReadingSet *outReadings;
	void *handle = plugin_init(config, &outReadings, Handler);
	vector<Reading *> *readings = new vector<Reading *>;

	vector<Datapoint *> datapoints;
	string testValue = "Untouched";
	DatapointValue dpv(testValue);
	datapoints.push_back(new Datapoint("str", dpv));
	double doubleValue = 5.5;
	DatapointValue dpv1(doubleValue);
	datapoints.push_back(new Datapoint("double", dpv1));
	long longValue = 10;
	DatapointValue dpv2(longValue);
	datapoints.push_back(new Datapoint("integer", dpv2));
	Reading *in = new Reading("test", datapoints);
	readings->push_back(in);

	ReadingSet readingSet(readings);
	plugin_ingest(handle, (READINGSET *)&readingSet);


	vector<Reading *>results = outReadings->getAllReadings();
	ASSERT_EQ(results.size(), 1);
	Reading *out = results[0];
	ASSERT_STREQ(out->getAssetName().c_str(), "test");
	ASSERT_EQ(out->getDatapointCount(), 3);
	vector<Datapoint *> points = out->getReadingData();
	ASSERT_EQ(points.size(), 3);
	for (int i = 0; i < points.size(); i++)
	{
		Datapoint *outdp = points[i];
		if (outdp->getName().compare("str") == 0)
		{
			ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_STRING);
			ASSERT_STREQ(outdp->getData().toStringValue().c_str(), "Untouched");
		}
		else if (outdp->getName().compare("double") == 0)
		{
			ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
			ASSERT_EQ(outdp->getData().toDouble(), 11.0);
		}
		else if (outdp->getName().compare("integer") == 0)
		{
			ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
			ASSERT_EQ(outdp->getData().toDouble(), 20.0);
		}
	}
}


TEST(SCALE, ScaleOffsetMultiDP)
{
	PLUGIN_INFORMATION *info = plugin_info();
	ConfigCategory *config = new ConfigCategory("scale", info->config);
	ASSERT_NE(config, (ConfigCategory *)NULL);
	config->setItemsValueFromDefault();
	ASSERT_EQ(config->itemExists("factor"), true);
	config->setValue("factor", "2");
	config->setValue("offset", "100");
	config->setValue("enable", "true");
	ReadingSet *outReadings;
	void *handle = plugin_init(config, &outReadings, Handler);
	vector<Reading *> *readings = new vector<Reading *>;

	vector<Datapoint *> datapoints;
	string testValue = "Untouched";
	DatapointValue dpv(testValue);
	datapoints.push_back(new Datapoint("str", dpv));
	double doubleValue = 5.5;
	DatapointValue dpv1(doubleValue);
	datapoints.push_back(new Datapoint("double", dpv1));
	long longValue = 10;
	DatapointValue dpv2(longValue);
	datapoints.push_back(new Datapoint("integer", dpv2));
	Reading *in = new Reading("test", datapoints);
	readings->push_back(in);

	ReadingSet readingSet(readings);
	plugin_ingest(handle, (READINGSET *)&readingSet);


	vector<Reading *>results = outReadings->getAllReadings();
	ASSERT_EQ(results.size(), 1);
	Reading *out = results[0];
	ASSERT_STREQ(out->getAssetName().c_str(), "test");
	ASSERT_EQ(out->getDatapointCount(), 3);
	vector<Datapoint *> points = out->getReadingData();
	ASSERT_EQ(points.size(), 3);
	for (int i = 0; i < points.size(); i++)
	{
		Datapoint *outdp = points[i];
		if (outdp->getName().compare("str") == 0)
		{
			ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_STRING);
			ASSERT_STREQ(outdp->getData().toStringValue().c_str(), "Untouched");
		}
		else if (outdp->getName().compare("double") == 0)
		{
			ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
			ASSERT_EQ(outdp->getData().toDouble(), 111.0);
		}
		else if (outdp->getName().compare("integer") == 0)
		{
			ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
			ASSERT_EQ(outdp->getData().toDouble(), 120.0);
		}
	}
}

TEST(SCALE, ScaleNegativeOffsetMultiDP)
{
	PLUGIN_INFORMATION *info = plugin_info();
	ConfigCategory *config = new ConfigCategory("scale", info->config);
	ASSERT_NE(config, (ConfigCategory *)NULL);
	config->setItemsValueFromDefault();
	ASSERT_EQ(config->itemExists("factor"), true);
	config->setValue("factor", "2");
	config->setValue("offset", "-6");
	config->setValue("enable", "true");
	ReadingSet *outReadings;
	void *handle = plugin_init(config, &outReadings, Handler);
	vector<Reading *> *readings = new vector<Reading *>;

	vector<Datapoint *> datapoints;
	string testValue = "Untouched";
	DatapointValue dpv(testValue);
	datapoints.push_back(new Datapoint("str", dpv));
	double doubleValue = 5.5;
	DatapointValue dpv1(doubleValue);
	datapoints.push_back(new Datapoint("double", dpv1));
	long longValue = 10;
	DatapointValue dpv2(longValue);
	datapoints.push_back(new Datapoint("integer", dpv2));
	Reading *in = new Reading("test", datapoints);
	readings->push_back(in);

	ReadingSet readingSet(readings);
	plugin_ingest(handle, (READINGSET *)&readingSet);


	vector<Reading *>results = outReadings->getAllReadings();
	ASSERT_EQ(results.size(), 1);
	Reading *out = results[0];
	ASSERT_STREQ(out->getAssetName().c_str(), "test");
	ASSERT_EQ(out->getDatapointCount(), 3);
	vector<Datapoint *> points = out->getReadingData();
	ASSERT_EQ(points.size(), 3);
	for (int i = 0; i < points.size(); i++)
	{
		Datapoint *outdp = points[i];
		if (outdp->getName().compare("str") == 0)
		{
			ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_STRING);
			ASSERT_STREQ(outdp->getData().toStringValue().c_str(), "Untouched");
		}
		else if (outdp->getName().compare("double") == 0)
		{
			ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
			ASSERT_EQ(outdp->getData().toDouble(), 5.0);
		}
		else if (outdp->getName().compare("integer") == 0)
		{
			ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
			ASSERT_EQ(outdp->getData().toDouble(), 14.0);
		}
	}
}

TEST(SCALE, ScaleMatch)
{
	PLUGIN_INFORMATION *info = plugin_info();
	ConfigCategory *config = new ConfigCategory("scale", info->config);
	ASSERT_NE(config, (ConfigCategory *)NULL);
	config->setItemsValueFromDefault();
	ASSERT_EQ(config->itemExists("factor"), true);
	config->setValue("factor", "4");
	config->setValue("offset", "10");
	config->setValue("match", "test.*");
	config->setValue("enable", "true");
	ReadingSet *outReadings;
	void *handle = plugin_init(config, &outReadings, Handler);
	vector<Reading *> *readings = new vector<Reading *>;

	vector<Datapoint *> datapoints;
	double testValue = 30;
	DatapointValue dpv(testValue);
	datapoints.push_back(new Datapoint("test1", dpv));
	double doubleValue = 5.5;
	DatapointValue dpv1(doubleValue);
	datapoints.push_back(new Datapoint("test2", dpv1));
	long longValue = 10;
	Reading *in = new Reading("test", datapoints);
	readings->push_back(in);

	vector<Datapoint *> datapoints1;
	DatapointValue dpv2(testValue);
	datapoints1.push_back(new Datapoint("test1", dpv2));
	DatapointValue dpv3(doubleValue);
	datapoints1.push_back(new Datapoint("test2", dpv3));
	DatapointValue dpv4(longValue);
	datapoints1.push_back(new Datapoint("untouched", dpv4));
	readings->push_back(new Reading("untouched", datapoints1));

	ReadingSet readingSet(readings);
	plugin_ingest(handle, (READINGSET *)&readingSet);


	vector<Reading *>results = outReadings->getAllReadings();
	ASSERT_EQ(results.size(), 2);
	for (int j = 0; j < results.size(); j++)
	{
		Reading *out = results[j];
		if (out->getAssetName().compare("test") == 0)
		{
			ASSERT_EQ(out->getDatapointCount(), 2);
			vector<Datapoint *> points = out->getReadingData();
			ASSERT_EQ(points.size(), 2);
			for (int i = 0; i < points.size(); i++)
			{
				Datapoint *outdp = points[i];
				if (outdp->getName().compare("test1") == 0)
				{
					ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
					ASSERT_EQ(outdp->getData().toDouble(), 130);
				}
				else if (outdp->getName().compare("test2") == 0)
				{
					ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
					ASSERT_EQ(outdp->getData().toDouble(), 32);
				}
			}
		}
		else if (out->getAssetName().compare("untouched") == 0)
		{
			ASSERT_EQ(out->getDatapointCount(), 3);
			vector<Datapoint *> points = out->getReadingData();
			ASSERT_EQ(points.size(), 3);
			for (int i = 0; i < points.size(); i++)
			{
				Datapoint *outdp = points[i];
				if (outdp->getName().compare("test1") == 0)
				{
					ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
					ASSERT_EQ(outdp->getData().toDouble(), 30);
				}
				else if (outdp->getName().compare("test2") == 0)
				{
					ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
					ASSERT_EQ(outdp->getData().toDouble(), 5.5);
				}
				else if (outdp->getName().compare("test3") == 0)
				{
					ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
					ASSERT_EQ(outdp->getData().toDouble(), 10.0);
				}
			}
		}
	}
}
