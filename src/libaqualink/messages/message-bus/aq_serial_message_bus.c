#include "aq_serial_message_bus.h"

#include <assert.h>
#include <string.h>

#include "cross-platform/threads.h"
#include "hardware/devices/hardware_device_types.h"

static MessageBus_Topic* topic_by_name(MessageBus* bus, const char* name)
{
    assert(0 != bus);
    assert(0 != name);

    MessageBus_Topic* t;

    for (t = bus->Topics.head; t != NULL; t = t->next) 
    {
        if (0 == strlen(t->name))
        {
            // This topic was registered using a DEVICE_ID so don't match (to try to read the name).
        }
        else if (0 == strcmp(name, t->name)) 
        {
            return t;
        }
        else
        {
            // No match...keep going.
        }
    }

    return 0;
}

static MessageBus_Topic* topic_by_type(MessageBus* bus, HardwareDeviceTypes device_type)
{
    assert(0 != bus);
    
    MessageBus_Topic* t;

    for (t = bus->Topics.head; t != NULL; t = t->next)
    {
        if (0 != strlen(t->name))
        {
            // If there is a name point, this topic was NOT registered using a TOPIC NAME so don't match it.
        }
        else if (device_type == t->hardware_device_type)
        {
            return t;
        }
        else
        {
            // No match...keep going.
        }
    }

    return 0;
}

void messagebus_init(MessageBus* bus)
{
    assert(0 != bus);

    memset(bus, 0, sizeof(MessageBus));
    
    mtx_init(&(bus->bus_mtx), mtx_plain | mtx_recursive);
    cnd_init(&(bus->bus_cv));
}

void messagebus_topic_init(MessageBus_Topic* topic, void* buffer, size_t buffer_len)
{
    assert(0 != topic);
    assert(0 != buffer);
    assert(0 < buffer_len);

    memset(topic, 0, sizeof(MessageBus_Topic));

    topic->buffer = buffer;
    topic->buffer_len = buffer_len;

    mtx_init(&(topic->topic_mtx), mtx_plain | mtx_recursive);
    cnd_init(&(topic->topic_cv));
}

static void messagebus_advertise_topic_add_to_bus(MessageBus* bus, MessageBus_Topic* topic)
{
    mtx_lock(&(bus->bus_mtx));

    if (0 != bus->Topics.head)
    {
        topic->next = bus->Topics.head;
    }
    bus->Topics.head = topic;

    for (MessageBus_NewTopicCB* cb = bus->new_topic_callback_list; cb != NULL; cb = cb->next)
    {
        cb->callback(bus, topic, cb->callback_arg);
    }

    cnd_broadcast(&(bus->bus_cv));
    mtx_unlock(&(bus->bus_mtx));
}

void messagebus_advertise_topic_by_name(MessageBus* bus, MessageBus_Topic* topic, const char* name)
{
    assert(0 != bus);
    assert(0 != topic);
    assert(0 != name);

    memset(topic->name, 0, sizeof(topic->name));
    strncpy(topic->name, name, TOPIC_NAME_MAX_LENGTH);

    messagebus_advertise_topic_add_to_bus(bus, topic);
}

void messagebus_advertise_topic_by_type(MessageBus* bus, MessageBus_Topic* topic, HardwareDeviceTypes device_type)
{
    assert(0 != bus);
    assert(0 != topic);

    memset(topic->name, 0, TOPIC_NAME_MAX_LENGTH);
    topic->hardware_device_type = device_type;

    messagebus_advertise_topic_add_to_bus(bus, topic);
}

MessageBus_Topic* messagebus_find_topic_by_name(MessageBus* bus, const char* name)
{
    assert(0 != bus);
    assert(0 != name);

    MessageBus_Topic* res;

    mtx_lock(&(bus->bus_mtx));

    res = topic_by_name(bus, name);

    mtx_unlock(&(bus->bus_mtx));

    return res;
}

MessageBus_Topic* messagebus_find_topic_by_type(MessageBus* bus, HardwareDeviceTypes device_type)
{
    assert(0 != bus);

    MessageBus_Topic* res;

    mtx_lock(&(bus->bus_mtx));

    res = topic_by_type(bus, device_type);

    mtx_unlock(&(bus->bus_mtx));

    return res;
}

MessageBus_Topic* messagebus_find_topic_blocking(MessageBus* bus, const char* name)
{
    assert(0 != bus);
    assert(0 != name);

    MessageBus_Topic* res = 0;

    mtx_lock(&(bus->bus_mtx));

    while (0 == res) 
    {
        res = topic_by_name(bus, name);

        if (0 == res)
        {
            cnd_wait(&(bus->bus_cv), &(bus->bus_mtx));
        }
    }

    mtx_unlock(&(bus->bus_mtx));

    return res;
}

bool messagebus_topic_publish(MessageBus_Topic* topic, const void* buf, size_t buf_len)
{
    assert(0 != topic);
    assert(0 != buf);
    assert(0 < buf_len);

    if (topic->buffer_len < buf_len) 
    {
        return false;
    }

    mtx_lock(&(topic->topic_mtx));

    memcpy(topic->buffer, buf, buf_len);
    topic->published = true;
    cnd_broadcast(&(topic->topic_cv));

    MessageBox_Watcher* w;

    for (w = topic->watchers; w != NULL; w = w->next) 
    {
        mtx_lock(&(w->group->watchgroup_mtx));
        w->group->published_topic = topic;
        cnd_broadcast(&(w->group->watchgroup_cv));
        mtx_unlock(&(w->group->watchgroup_mtx));
    }

    mtx_unlock(&(topic->topic_mtx));

    return true;
}

bool messagebus_topic_read(MessageBus_Topic* topic, void* buf, size_t buf_len)
{
    assert(0 != topic);
    assert(0 != buf);
    assert(0 < buf_len);

    bool success = false;
    mtx_lock(&(topic->topic_mtx));

    if (topic->published) 
    {
        success = true;
        memcpy(buf, topic->buffer, buf_len);
    }

    mtx_unlock(&(topic->topic_mtx));

    return success;
}

void messagebus_topic_wait(MessageBus_Topic* topic, void* buf, size_t buf_len)
{
    assert(0 != topic);
    assert(0 != buf);
    assert(0 < buf_len);

    mtx_lock(&(topic->topic_mtx));
    cnd_wait(&(topic->topic_cv), &(topic->topic_mtx));

    memcpy(buf, topic->buffer, buf_len);

    mtx_unlock(&(topic->topic_mtx));
}

void messagebus_watchgroup_init(MessageBus_WatchGroup* group)
{
    assert(0 != group);

    mtx_init(&(group->watchgroup_mtx), mtx_plain | mtx_recursive);
    cnd_init(&(group->watchgroup_cv));
}

void messagebus_watchgroup_watch(MessageBox_Watcher* watcher, MessageBus_WatchGroup* group, MessageBus_Topic* topic)
{
    assert(0 != watcher);
    assert(0 != group);
    assert(0 != topic);

    mtx_lock(&(topic->topic_mtx));
    mtx_lock(&(group->watchgroup_mtx));

    watcher->group = group;

    watcher->next = topic->watchers;
    topic->watchers = watcher;

    mtx_unlock(&(group->watchgroup_mtx));
    mtx_unlock(&(topic->topic_mtx));
}

MessageBus_Topic* messagebus_watchgroup_wait(MessageBus_WatchGroup* group)
{
    assert(0 != group);

    MessageBus_Topic* res;

    mtx_lock(&(group->watchgroup_mtx));
    cnd_wait(&(group->watchgroup_cv), &(group->watchgroup_mtx));

    res = group->published_topic;

    mtx_unlock(&(group->watchgroup_mtx));

    return res;
}

void messagebus_new_topic_callback_register(MessageBus* bus, MessageBus_NewTopicCB* cb, MessageBus_NewTopicCB_Func cb_func, void* arg)
{
    assert(0 != bus);
    assert(0 != cb);
    assert(0 != cb_func);
    assert(0 != arg);

    mtx_lock(&(bus->bus_mtx));
    cb->callback = cb_func;
    cb->callback_arg = arg;

    cb->next = bus->new_topic_callback_list;
    bus->new_topic_callback_list = cb;

    mtx_unlock(&(bus->bus_mtx));
}
