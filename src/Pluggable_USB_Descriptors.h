#pragma once

#include <PluggableUSB.h>

class Pluggable_USB_Descriptors : protected PluggableUSBModule {  //

private:
    uint16_t p__VendorID = 0x0000;
    uint16_t p__ProductID = 0x0000;
    bool p__IDs_Set = false;

#if (IPRODUCT)  // It is possible to set a product name
private:
    const char* p__ProductName;
#endif

#if (IMANUFACTURER)  // It is possible to set a manufacturer name
private:
    const char* p__Manufacturer = nullptr;
#endif

#if (ISERIAL)  // It is possible to set a serial number
private:
    const char* p__SerialNumber = nullptr;
#endif

public:
    Pluggable_USB_Descriptors(const char* productName = nullptr);

public:
    void setIDs(const uint16_t vendorID, const uint16_t productID);
    void setProductName(const char* productName);
    void setManufacturer(const char* manufacturer);
    void setSerialNumber(const char* serialNumber);

private:
    void plug();

protected:
    bool setup(USBSetup&) override final;
    int getInterface(uint8_t*) override final;
    int getDescriptor(USBSetup&) override final;

private:
    static int p__USB_send(const void* data, const int dataSize);

#if (IMANUFACTURER || IPRODUCT || ISERIAL)  // It is possible to set string descriptors
private:
    static void p__USB_sendAnsiCharAsUnicode(const char ch);
    static int p__USB_sendStringDescriptor(const char* descriptor);
#endif

private:
    int p__USB_sendDeviceDescriptor();

#if (IMANUFACTURER || IPRODUCT || ISERIAL)  // It is possible to set string descriptors
private:
    int p__USB_sendStringDescriptorByIndex(const uint8_t);
#endif

};  //
