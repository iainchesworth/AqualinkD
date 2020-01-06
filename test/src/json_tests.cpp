#include <cmath>
#include <gtest/gtest.h>
#include <json-c/json_tokener.h>

#include "json/json_messages.h"
#include "json/json_string_utils.h"
#include "hardware/buttons/buttons.h"
#include "version/version.h"

#include "aqualink.h"
#include "aq_mqtt.h"

static void verify_key_and_value_exist_in_json(struct json_object* obj, const char* EXPECTED_KEY, const int EXPECTED_VALUE)
{
    struct json_object* actual_obj;

    ASSERT_TRUE(json_object_object_get_ex(obj, EXPECTED_KEY, &actual_obj));
    ASSERT_TRUE(json_object_is_type(actual_obj, json_type_int));
    ASSERT_TRUE(EXPECTED_VALUE == json_object_get_int(actual_obj));
}

static void verify_key_and_value_exist_in_json(struct json_object* obj, const char* EXPECTED_KEY, const double EXPECTED_VALUE)
{
    struct json_object* actual_obj;

    ASSERT_TRUE(json_object_object_get_ex(obj, EXPECTED_KEY, &actual_obj));
    ASSERT_TRUE(json_object_is_type(actual_obj, json_type_double));
    ASSERT_TRUE(fabs(EXPECTED_VALUE - json_object_get_double(actual_obj)) < std::numeric_limits<double>::epsilon());
}

static void verify_key_and_value_exist_in_json(struct json_object* obj, const char* EXPECTED_KEY, const char* EXPECTED_VALUE)
{
    struct json_object* actual_obj;

    ASSERT_TRUE(json_object_object_get_ex(obj, EXPECTED_KEY, &actual_obj));
    ASSERT_TRUE(json_object_is_type(actual_obj, json_type_string));
    ASSERT_TRUE(0 == strcmp(EXPECTED_VALUE, json_object_get_string(actual_obj)));
}

static void verify_key_and_value_doesnt_exist_in_json(struct json_object* obj, const char* EXPECTED_KEY)
{
    struct json_object* actual_obj;

    ASSERT_FALSE(json_object_object_get_ex(obj, EXPECTED_KEY, &actual_obj));
}

TEST(JSON_Messages, MQTT_Status)
{
    const unsigned int MAX_JSON_LENGTH = 1024;
    char json[MAX_JSON_LENGTH];

    const int idx = 1, nvalue = 1;
    double tvalue = 26.3;

    int length = build_mqtt_status_JSON(json, MAX_JSON_LENGTH, idx, nvalue, tvalue);

    struct json_tokener* tok;
    struct json_object* obj;

    tok = json_tokener_new();
    obj = json_tokener_parse_ex(tok, json, MAX_JSON_LENGTH);

    ASSERT_TRUE(0 != obj);
    ASSERT_TRUE(json_tokener_success == json_tokener_get_error(tok));

    verify_key_and_value_exist_in_json(obj, "idx", idx);
    verify_key_and_value_exist_in_json(obj, "nvalue", nvalue);
    verify_key_and_value_exist_in_json(obj, "stype", "SetPoint");
    verify_key_and_value_exist_in_json(obj, "svalue", tvalue);

    json_object_put(obj);
    json_tokener_free(tok);
}

TEST(JSON_Messages, MQTT_Status_TEMP_UNKNOWN)
{
    const unsigned int MAX_JSON_LENGTH = 1024;
    char json[MAX_JSON_LENGTH];

    const int idx = 2, nvalue = 3;
    float tvalue = TEMP_UNKNOWN;

    int length = build_mqtt_status_JSON(json, MAX_JSON_LENGTH, idx, nvalue, tvalue);

    struct json_tokener* tok;
    struct json_object* obj;

    tok = json_tokener_new();
    obj = json_tokener_parse_ex(tok, json, MAX_JSON_LENGTH);

    ASSERT_TRUE(0 != obj);
    ASSERT_TRUE(json_tokener_success == json_tokener_get_error(tok));

    verify_key_and_value_exist_in_json(obj, "idx", idx);
    verify_key_and_value_exist_in_json(obj, "nvalue", nvalue);
    verify_key_and_value_doesnt_exist_in_json(obj, "stype");
    verify_key_and_value_doesnt_exist_in_json(obj, "svalue");

    json_object_put(obj);
    json_tokener_free(tok);
}

TEST(JSON_Messages, MQTT_StatusMessage)
{
    const unsigned int MAX_JSON_LENGTH = 1024;
    char json[MAX_JSON_LENGTH];

    const int idx = 3, nvalue = 2;
    char svalue[10] = "12345";

    int length = build_mqtt_status_message_JSON(json, MAX_JSON_LENGTH, idx, nvalue, svalue);

    struct json_tokener* tok;
    struct json_object* obj;

    tok = json_tokener_new();
    obj = json_tokener_parse_ex(tok, json, MAX_JSON_LENGTH);

    verify_key_and_value_exist_in_json(obj, "idx", idx);
    verify_key_and_value_exist_in_json(obj, "nvalue", nvalue);
    verify_key_and_value_exist_in_json(obj, "svalue", svalue);

    json_object_put(obj);
    json_tokener_free(tok);
}

TEST(JSON_Messages, MQTT_AqualinkStatus)
{
    const unsigned int MAX_JSON_LENGTH = 10240;
    char json[MAX_JSON_LENGTH];

    struct aqualinkdata testdata;
    unsigned char raw_status[AQ_PSTLEN] = { 0x01, 0x02, 0x03, 0x04, 0x05 };
    aqled aqualinkleds[TOTAL_LEDS] = { ON, OFF, ON, OFF, FLASH, ON, OFF, ON, OFF, ENABLE, ON, OFF, ON, OFF, LED_S_UNKNOWN, ON, OFF, ON, OFF, FLASH };
    aqkey aqbuttons[AqualinkButtonCount] = 
    { 
        { &aqualinkleds[0],  (char*) "LABEL_00", (char*) "NAME_00", (char*) "PDA_LABEL_00", 0x00, 0 },
        { &aqualinkleds[1],  (char*) "LABEL_01", (char*) "NAME_01", (char*) "PDA_LABEL_01", 0x01, 1 },
        { &aqualinkleds[2],  (char*) "LABEL_02", (char*) "NAME_02", (char*) "PDA_LABEL_02", 0x02, 2 },
        { &aqualinkleds[3],  (char*) "LABEL_03", (char*) "NAME_03", (char*) "PDA_LABEL_03", 0x03, 3 },
        { &aqualinkleds[4],  (char*) "LABEL_04", (char*) "NAME_04", (char*) "PDA_LABEL_04", 0x04, 4 },
        { &aqualinkleds[5],  (char*) "LABEL_05", (char*) "NAME_05", (char*) "PDA_LABEL_05", 0x05, 5 },
        { &aqualinkleds[6],  (char*) "LABEL_06", (char*) "NAME_06", (char*) "PDA_LABEL_06", 0x06, 6 },
        { &aqualinkleds[7],  (char*) "LABEL_07", (char*) "NAME_07", (char*) "PDA_LABEL_07", 0x07, 7 },
        { &aqualinkleds[8],  (char*) "LABEL_08", (char*) "NAME_08", (char*) "PDA_LABEL_08", 0x08, 8 },
        { &aqualinkleds[9],  (char*) "LABEL_09", (char*) "NAME_09", (char*) "PDA_LABEL_09", 0x09, 9 },
        { &aqualinkleds[10], (char*) "LABEL_10", (char*) "NAME_10", (char*) "PDA_LABEL_10", 0x0a, 10 },
        { &aqualinkleds[11], (char*) "LABEL_11", (char*) "NAME_11", (char*) "PDA_LABEL_11", 0x0b, 11 }
    };
    pump_detail pumps[MAX_PUMPS] = {
        { 0, 0, 0, 0x00, JANDY, &aqbuttons[0] },
        { 1, 1, 1, 0x01, JANDY, &aqbuttons[1] },
        { 2, 2, 2, 0x02, JANDY, &aqbuttons[2] },
        { 3, 3, 3, 0x03, JANDY, &aqbuttons[3] }
    };

    strcpy(testdata.version, "TEST");
    strcpy(testdata.date, "DATE");
    strcpy(testdata.time, "TIME");
    strcpy(testdata.last_message, "LASTMESSAGE");
    strcpy(testdata.last_display_message, "LASTDISPLAYMESSAGE");
    memcpy(testdata.raw_status, raw_status, AQ_PSTLEN * sizeof(unsigned char));
    memcpy(testdata.aqualinkleds, aqualinkleds, TOTAL_LEDS * sizeof(aqled));
    memcpy(testdata.aqbuttons, aqbuttons, AqualinkButtonCount * sizeof(aqkey));
    testdata.air_temp = 10;
    testdata.pool_temp = 20;
    testdata.spa_temp = 30;
    testdata.temp_units = CELSIUS;
    testdata.single_device = false;
    testdata.battery = LOW;
    testdata.frz_protect_set_point = 0;
    testdata.pool_htr_set_point = 25;
    testdata.spa_htr_set_point = 39;
    memset(&(testdata.active_thread), 0, sizeof(struct programmingthread));
    memset(&(testdata.unactioned), 0, sizeof(struct action));
    testdata.swg_percent = 50;
    testdata.swg_ppm = 3500;
    testdata.ar_swg_status = 0x10;
    testdata.swg_delayed_percent = 75;
    testdata.simulate_panel = false;
    testdata.service_mode_state = OFF;
    testdata.frz_protect_state = OFF;
    testdata.last_packet_type = 0x20;
    memcpy(&(testdata.pumps), pumps, MAX_PUMPS * sizeof(pump_detail));
    testdata.open_websockets = 3;
    testdata.boost = true;
    strcpy(testdata.boost_msg, "BOOSTMSG");

    int length = build_aqualink_status_JSON(&testdata, json, MAX_JSON_LENGTH);

    struct json_tokener* tok;
    struct json_object* obj, *leds_obj, *pump_obj;

    tok = json_tokener_new();
    obj = json_tokener_parse_ex(tok, json, MAX_JSON_LENGTH);

    verify_key_and_value_exist_in_json(obj, "version", testdata.version);
    verify_key_and_value_exist_in_json(obj, "aqualinkd_version", AQUALINKD_VERSION);
    verify_key_and_value_exist_in_json(obj, "date", testdata.date);
    verify_key_and_value_exist_in_json(obj, "time", testdata.time);
    verify_key_and_value_exist_in_json(obj, "status", testdata.last_display_message);
    verify_key_and_value_exist_in_json(obj, "air_temp", testdata.air_temp);
    verify_key_and_value_exist_in_json(obj, "pool_temp", testdata.pool_temp);
    verify_key_and_value_exist_in_json(obj, "spa_temp", testdata.spa_temp);
    verify_key_and_value_exist_in_json(obj, "swg_percent", testdata.swg_percent);
    verify_key_and_value_exist_in_json(obj, "swg_ppm", testdata.swg_ppm);
    verify_key_and_value_exist_in_json(obj, "temp_units", JSON_CELSIUS);
    verify_key_and_value_exist_in_json(obj, "battery", JSON_LOW);

    ASSERT_TRUE(json_object_object_get_ex(obj, "leds", &leds_obj));
    ASSERT_TRUE(json_object_is_type(leds_obj, json_type_object));
    verify_key_and_value_exist_in_json(leds_obj, SWG_TOPIC, JSON_ON);
    verify_key_and_value_exist_in_json(leds_obj, SWG_BOOST_TOPIC, JSON_ON);
    verify_key_and_value_exist_in_json(leds_obj, FREEZE_PROTECT, JSON_ENABLED);

    ASSERT_TRUE(json_object_object_get_ex(obj, "Pump_1", &pump_obj));
    ASSERT_TRUE(json_object_is_type(pump_obj, json_type_object));

    ASSERT_TRUE(json_object_object_get_ex(obj, "Pump_2", &pump_obj));
    ASSERT_TRUE(json_object_is_type(pump_obj, json_type_object));

    ASSERT_TRUE(json_object_object_get_ex(obj, "Pump_3", &pump_obj));
    ASSERT_TRUE(json_object_is_type(pump_obj, json_type_object));

    ASSERT_TRUE(json_object_object_get_ex(obj, "Pump_4", &pump_obj));
    ASSERT_TRUE(json_object_is_type(pump_obj, json_type_object));

    json_object_put(obj);
    json_tokener_free(tok);
}

TEST(JSON_Messages, MQTT_AqualinkStatus_TEMP_UNKNOWN)
{
    const unsigned int MAX_JSON_LENGTH = 10240;
    char json[MAX_JSON_LENGTH];

    struct aqualinkdata testdata;
    unsigned char raw_status[AQ_PSTLEN] = { 0x01, 0x02, 0x03, 0x04, 0x05 };
    aqled aqualinkleds[TOTAL_LEDS] = { ON, OFF, ON, OFF, FLASH, ON, OFF, ON, OFF, ENABLE, ON, OFF, ON, OFF, LED_S_UNKNOWN, ON, OFF, ON, OFF, FLASH };
    aqkey aqbuttons[AqualinkButtonCount] =
    {
        { &aqualinkleds[0],  (char*)"LABEL_00", (char*)"NAME_00", (char*)"PDA_LABEL_00", 0x00, 0 },
        { &aqualinkleds[1],  (char*)"LABEL_01", (char*)"NAME_01", (char*)"PDA_LABEL_01", 0x01, 1 },
        { &aqualinkleds[2],  (char*)"LABEL_02", (char*)"NAME_02", (char*)"PDA_LABEL_02", 0x02, 2 },
        { &aqualinkleds[3],  (char*)"LABEL_03", (char*)"NAME_03", (char*)"PDA_LABEL_03", 0x03, 3 },
        { &aqualinkleds[4],  (char*)"LABEL_04", (char*)"NAME_04", (char*)"PDA_LABEL_04", 0x04, 4 },
        { &aqualinkleds[5],  (char*)"LABEL_05", (char*)"NAME_05", (char*)"PDA_LABEL_05", 0x05, 5 },
        { &aqualinkleds[6],  (char*)"LABEL_06", (char*)"NAME_06", (char*)"PDA_LABEL_06", 0x06, 6 },
        { &aqualinkleds[7],  (char*)"LABEL_07", (char*)"NAME_07", (char*)"PDA_LABEL_07", 0x07, 7 },
        { &aqualinkleds[8],  (char*)"LABEL_08", (char*)"NAME_08", (char*)"PDA_LABEL_08", 0x08, 8 },
        { &aqualinkleds[9],  (char*)"LABEL_09", (char*)"NAME_09", (char*)"PDA_LABEL_09", 0x09, 9 },
        { &aqualinkleds[10], (char*)"LABEL_10", (char*)"NAME_10", (char*)"PDA_LABEL_10", 0x0a, 10 },
        { &aqualinkleds[11], (char*)"LABEL_11", (char*)"NAME_11", (char*)"PDA_LABEL_11", 0x0b, 11 }
    };
    pump_detail pumps[MAX_PUMPS] = {
        { TEMP_UNKNOWN, TEMP_UNKNOWN, TEMP_UNKNOWN, 0x00, JANDY, &aqbuttons[0] },
        { TEMP_UNKNOWN, TEMP_UNKNOWN, TEMP_UNKNOWN, 0x01, JANDY, &aqbuttons[1] },
        { TEMP_UNKNOWN, TEMP_UNKNOWN, TEMP_UNKNOWN, 0x02, JANDY, &aqbuttons[2] },
        { 3, 3, 3,                                  0x03, JANDY, &aqbuttons[3] }
    };

    strcpy(testdata.version, "TEST");
    strcpy(testdata.date, "DATE");
    strcpy(testdata.time, "TIME");
    strcpy(testdata.last_message, "LASTMESSAGE");
    strcpy(testdata.last_display_message, "LASTDISPLAYMESSAGE");
    memcpy(testdata.raw_status, raw_status, AQ_PSTLEN * sizeof(unsigned char));
    memcpy(testdata.aqualinkleds, aqualinkleds, TOTAL_LEDS * sizeof(aqled));
    memcpy(testdata.aqbuttons, aqbuttons, AqualinkButtonCount * sizeof(aqkey));
    testdata.air_temp = TEMP_UNKNOWN;
    testdata.pool_temp = TEMP_UNKNOWN;
    testdata.spa_temp = TEMP_UNKNOWN;
    testdata.temp_units = FAHRENHEIT;
    testdata.single_device = false;
    testdata.battery = OK;
    testdata.frz_protect_set_point = TEMP_UNKNOWN;
    testdata.pool_htr_set_point = 25;
    testdata.spa_htr_set_point = 39;
    memset(&(testdata.active_thread), 0, sizeof(struct programmingthread));
    memset(&(testdata.unactioned), 0, sizeof(struct action));
    testdata.swg_percent = TEMP_UNKNOWN;
    testdata.swg_ppm = TEMP_UNKNOWN;
    testdata.ar_swg_status = 0x10;
    testdata.swg_delayed_percent = 75;
    testdata.simulate_panel = false;
    testdata.service_mode_state = OFF;
    testdata.frz_protect_state = OFF;
    testdata.last_packet_type = 0x20;
    memcpy(&(testdata.pumps), pumps, MAX_PUMPS * sizeof(pump_detail));
    testdata.open_websockets = 3;
    testdata.boost = true;
    strcpy(testdata.boost_msg, "BOOSTMSG");

    int length = build_aqualink_status_JSON(&testdata, json, MAX_JSON_LENGTH);

    struct json_tokener* tok;
    struct json_object* obj, * leds_obj, * pump_obj;

    tok = json_tokener_new();
    obj = json_tokener_parse_ex(tok, json, MAX_JSON_LENGTH);

    verify_key_and_value_exist_in_json(obj, "version", testdata.version);
    verify_key_and_value_exist_in_json(obj, "aqualinkd_version", AQUALINKD_VERSION);
    verify_key_and_value_exist_in_json(obj, "date", testdata.date);
    verify_key_and_value_exist_in_json(obj, "time", testdata.time);
    verify_key_and_value_exist_in_json(obj, "status", testdata.last_display_message);
    verify_key_and_value_doesnt_exist_in_json(obj, "air_temp");
    verify_key_and_value_doesnt_exist_in_json(obj, "pool_temp");
    verify_key_and_value_doesnt_exist_in_json(obj, "spa_temp");
    verify_key_and_value_doesnt_exist_in_json(obj, "swg_percent");
    verify_key_and_value_doesnt_exist_in_json(obj, "swg_ppm");
    verify_key_and_value_exist_in_json(obj, "temp_units", JSON_FAHRENHEIT);
    verify_key_and_value_exist_in_json(obj, "battery", JSON_OK);

    ASSERT_TRUE(json_object_object_get_ex(obj, "leds", &leds_obj));
    ASSERT_TRUE(json_object_is_type(leds_obj, json_type_object));
    verify_key_and_value_doesnt_exist_in_json(leds_obj, SWG_TOPIC);
    verify_key_and_value_doesnt_exist_in_json(leds_obj, SWG_BOOST_TOPIC);
    verify_key_and_value_doesnt_exist_in_json(obj, FREEZE_PROTECT);

    ASSERT_FALSE(json_object_object_get_ex(obj, "Pump_1", &pump_obj));
    ASSERT_FALSE(json_object_object_get_ex(obj, "Pump_2", &pump_obj));
    ASSERT_FALSE(json_object_object_get_ex(obj, "Pump_3", &pump_obj));

    ASSERT_TRUE(json_object_object_get_ex(obj, "Pump_4", &pump_obj));
    ASSERT_TRUE(json_object_is_type(pump_obj, json_type_object));

    json_object_put(obj);
    json_tokener_free(tok);
}
