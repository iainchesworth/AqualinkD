#ifndef AQ_HARDWARE_DEVICE_TYPES_H_
#define AQ_HARDWARE_DEVICE_TYPES_H_

typedef unsigned char DeviceId;

extern const DeviceId INVALID_DEVICE_ID;

const char* const device_id_to_string(DeviceId device_id);

#endif // AQ_HARDWARE_DEVICE_TYPES_H_
