#ifndef AQ_SERIAL_MESSAGE_BUS_H_
#define AQ_SERIAL_MESSAGE_BUS_H_

#include <stdbool.h>

#include "cross-platform/threads.h"
#include "hardware/devices/hardware_device_types.h"

#define TOPIC_NAME_MAX_LENGTH 64
#define TOPIC_MAX_MSG_LENGTH 64

typedef struct topic_s 
{
    mtx_t topic_mtx;
    cnd_t topic_cv;

    char name[TOPIC_NAME_MAX_LENGTH + 1];
    HardwareDeviceTypes hardware_device_type;

    void* buffer;
    size_t buffer_len;

    bool published;

    struct messagebus_watcher_s* watchers;
    struct topic_s* next;
    void* metadata;
} 
MessageBus_Topic;

typedef struct 
{
    mtx_t bus_mtx;
    cnd_t bus_cv;

    struct 
    {
        MessageBus_Topic* head;
    } 
    Topics;

    struct messagebus_new_topic_cb_s* new_topic_callback_list;
} 
MessageBus;

typedef struct messagebus_watchgroup_s 
{
    mtx_t watchgroup_mtx;
    cnd_t watchgroup_cv;

    MessageBus_Topic* published_topic;
} 
MessageBus_WatchGroup;

typedef struct messagebus_watcher_s 
{
    MessageBus_WatchGroup* group;
    struct messagebus_watcher_s* next;
} 
MessageBox_Watcher;

typedef void (*MessageBus_NewTopicCB_Func)(MessageBus*, MessageBus_Topic*, void*);

typedef struct messagebus_new_topic_cb_s 
{
    MessageBus_NewTopicCB_Func callback;
    void* callback_arg;
    struct messagebus_new_topic_cb_s* next;
} 
MessageBus_NewTopicCB;

void messagebus_init(MessageBus* bus);
void messagebus_topic_init(MessageBus_Topic* topic, void* buffer, size_t buffer_len);
void messagebus_advertise_topic_by_name(MessageBus* bus, MessageBus_Topic* topic, const char* name);
void messagebus_advertise_topic_by_type(MessageBus* bus, MessageBus_Topic* topic, HardwareDeviceTypes device_type);
MessageBus_Topic* messagebus_find_topic_by_name(MessageBus* bus, const char* name);
MessageBus_Topic* messagebus_find_topic_by_type(MessageBus* bus, HardwareDeviceTypes device_type);
MessageBus_Topic* messagebus_find_topic_blocking(MessageBus* bus, const char* name);
bool messagebus_topic_publish(MessageBus_Topic* topic, const void* buf, size_t buf_len);
bool messagebus_topic_read(MessageBus_Topic* topic, void* buf, size_t buf_len);
void messagebus_topic_wait(MessageBus_Topic* topic, void* buf, size_t buf_len);
void messagebus_watchgroup_init(MessageBus_WatchGroup* group);
void messagebus_watchgroup_watch(MessageBox_Watcher* watcher, MessageBus_WatchGroup* group, MessageBus_Topic* topic);
MessageBus_Topic* messagebus_watchgroup_wait(MessageBus_WatchGroup* group);
void messagebus_new_topic_callback_register(MessageBus* bus, MessageBus_NewTopicCB* cb, MessageBus_NewTopicCB_Func, void* arg);

#endif // AQ_SERIAL_MESSAGE_BUS_H_
