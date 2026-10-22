#include "Pluggable_USB_Descriptors.h"

//  +--------------+  //
// (  Constructor  ) //
//  +--------------+  //

Pluggable_USB_Descriptors::Pluggable_USB_Descriptors(const char* productName) : PluggableUSBModule(0, 0, nullptr) {  //

    // Set product name
    setProductName(productName);

    // Plug USB device into PluggableUSB API
    PluggableUSB().plug(this);

}  //

//  +---------------------------+  //
// (  Descriptor Setter Methods  ) //
//  +---------------------------+  //

void Pluggable_USB_Descriptors::setIDs(const uint16_t vendorID, const uint16_t productID) {
    p__VendorID = vendorID;
    p__ProductID = productID;
    p__IDs_Set = true;
}

void Pluggable_USB_Descriptors::setProductName(const char* productName) {
#if (IPRODUCT)  // It is possible to set a product name

    // Set product name
    p__ProductName = productName;

#else  // It is NOT possible to set a product name

// Show warning if product name is not set
#warning "IPRODUCT is not defined, therefore setProductName has no effect"

#endif
}

void Pluggable_USB_Descriptors::setManufacturer(const char* manufacturer) {
#if (IMANUFACTURER)  // It is possible to set a manufacturer name

    // Set manufacturer name
    p__Manufacturer = manufacturer;

#else  // It is NOT possible to set a manufacturer name

// Show warning if manufacturer name is not set
#warning "IMANUFACTURER is not defined, therefore setManufacturer has no effect"

#endif
}

void Pluggable_USB_Descriptors::setSerialNumber(const char* serialNumber) {
#if (ISERIAL)  // It is possible to set a serial number

    // Set serial number
    p__SerialNumber = serialNumber;

#else  // It is NOT possible to set a serial number

// Show warning if serial number is not set
#warning "ISERIAL is not defined, therefore setSerialNumber has no effect"

#endif
}

//  +---------------------------------------+  //
// (  PluggableUSBModule Overridden Methods  ) //
//  +---------------------------------------+  //

bool Pluggable_USB_Descriptors::setup(USBSetup&) {
    return false;
}

int Pluggable_USB_Descriptors::getInterface(uint8_t*) {
    return 0;
}

int Pluggable_USB_Descriptors::getDescriptor(USBSetup& setup) {  //

    // Get descriptor type requested
    const uint8_t descriptorType = setup.wValueH;

    // Send descriptor according to requested type
    switch (descriptorType) {  //

        // Intercept Device Descriptor Request (Type 0x01)
        case USB_DEVICE_DESCRIPTOR_TYPE:
            // Send device descriptor
            return p__USB_sendDeviceDescriptor();

#if (IMANUFACTURER || IPRODUCT || ISERIAL)  // It is possible to set string descriptors

        // Intercept String Descriptor Requests (Type 0x03)
        case USB_STRING_DESCRIPTOR_TYPE:
            // Send string descriptor according to requested string index
            const uint8_t descriptorIndex = setup.wValueL;
            return p__USB_sendStringDescriptorByIndex(descriptorIndex);

#endif

    }  //

    // Hand off to default core handler
    return 0;

}  //

//  +----------------------------+  //
// (  Descriptor Sending Helpers  ) //
//  +----------------------------+  //

int Pluggable_USB_Descriptors::p__USB_send(const void* data, const int dataSize) {
    return USB_SendControl(0, data, dataSize);
}

#if (IMANUFACTURER || IPRODUCT || ISERIAL)  // It is possible to set string descriptors

void Pluggable_USB_Descriptors::p__USB_sendAnsiCharAsUnicode(const char ch) {
    const char16_t unicodeChar = (char16_t)ch;
    p__USB_send(&unicodeChar, sizeof(char16_t));
}

int Pluggable_USB_Descriptors::p__USB_sendStringDescriptor(const char* descriptor) {  //

    // Declare constants
    constexpr uint8_t MAX_STRING_DESCRIPTOR_LENGTH = 31;
    constexpr uint8_t USB_STRING_DESCRIPTOR_HEADER_SIZE = 2 * sizeof(uint8_t);
    constexpr uint8_t DESCRIPTOR_TYPE_STRING = (uint8_t)USB_STRING_DESCRIPTOR_TYPE;

    // Exit if descriptor is NULL
    if (!descriptor) return 0;

    // Get descriptor length (capped at 31 chars to fit standard USB limits)
    const uint8_t descriptorLength = strnlen(descriptor, MAX_STRING_DESCRIPTOR_LENGTH);

    // Calculate total length of string descriptor
    const uint8_t descriptorUnicodeLength = (descriptorLength + 1) * sizeof(char16_t);
    const uint8_t totalLength = USB_STRING_DESCRIPTOR_HEADER_SIZE + descriptorUnicodeLength;

    // Send USB String Descriptor Header: [byte Length, byte Type (0x03)]
    p__USB_send(&totalLength, sizeof(uint8_t));
    p__USB_send(&DESCRIPTOR_TYPE_STRING, sizeof(uint8_t));

    // Stream UTF-16LE characters straight to USB endpoint
    for (uint8_t i = 0; i < descriptorLength; i++) {
        p__USB_sendAnsiCharAsUnicode(descriptor[i]);
    }

    // Send NULL character to terminate string descriptor
    p__USB_sendAnsiCharAsUnicode('\0');

    // Return total length of unicode string descriptor
    return totalLength;

}  //

#endif

//  +----------------------------+  //
// (  Descriptor Sending Methods  ) //
//  +----------------------------+  //

int Pluggable_USB_Descriptors::p__USB_sendDeviceDescriptor() {  //

    // Hand off to default core handler if IDs are not set
    if (!p__IDs_Set) return 0;

    constexpr uint8_t DEVICE_CLASS = 0x00;
    constexpr uint8_t DEVICE_SUB_CLASS = 0x00;
    constexpr uint8_t DEVICE_PROTOCOL = 0x00;
    constexpr uint16_t DEVICE_VERSION = 0x0100;

    // Initialize device descriptor
    DeviceDescriptor deviceDescriptor = {
        .len = sizeof(DeviceDescriptor),
        .dtype = USB_DEVICE_DESCRIPTOR_TYPE,
        .usbVersion = USB_VERSION,
        .deviceClass = DEVICE_CLASS,
        .deviceSubClass = DEVICE_SUB_CLASS,
        .deviceProtocol = DEVICE_PROTOCOL,
        .packetSize0 = 64,
        .idVendor = p__VendorID,
        .idProduct = p__ProductID,
        .deviceVersion = DEVICE_VERSION,
#if (IMANUFACTURER)
        .iManufacturer = IMANUFACTURER,
#else
        .iManufacturer = 0,
#endif
#if (IPRODUCT)
        .iProduct = IPRODUCT,
#else
        .iProduct = 0,
#endif
#if (ISERIAL)
        .iSerialNumber = ISERIAL,
#else
        .iSerialNumber = 0,
#endif
        .bNumConfigurations = 1,
    };

    // Send device descriptor
    return p__USB_send(&deviceDescriptor, sizeof(DeviceDescriptor));

}  //

#if (IMANUFACTURER || IPRODUCT || ISERIAL)  // It is possible to set string descriptors

int Pluggable_USB_Descriptors::p__USB_sendStringDescriptorByIndex(const uint8_t descriptorIndex) {  //

    switch (descriptorIndex) {  //

#if (IMANUFACTURER)  // It is possible to set a manufacturer name

        // Intersect manufacturer string descriptor request and send manufacturer string descriptor
        case IMANUFACTURER: return p__USB_sendStringDescriptor(p__Manufacturer);

#endif

#if (IPRODUCT)  // It is possible to set a product name

        // Intersect product string descriptor request and send product string descriptor
        case IPRODUCT: return p__USB_sendStringDescriptor(p__ProductName);

#endif

#if (ISERIAL)  // It is possible to set a serial number

        // Intersect serial number string descriptor request and send serial number string descriptor
        case ISERIAL: return p__USB_sendStringDescriptor(p__SerialNumber);

#endif

    }  //

    // Hand off to default core handler is descriptor request has not been handled
    return 0;

}  //

#endif
