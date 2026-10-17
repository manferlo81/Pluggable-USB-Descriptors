#pragma once

#include <PluggableUSB.h>

class Pluggable_USB_Descriptors : protected PluggableUSBModule {  //

    private:
        uint16_t p__vendorID = 0x0000;
        uint16_t p__productID = 0x0000;
        bool p__IDs_set = false;

#if (IMANUFACTURER)
        // It is possible to set a manufacturer name
        const char* p__manufacturer = nullptr;
#endif

#if (IPRODUCT)
        // It is possible to set a product name
        const char* p__productName = nullptr;
#endif

#if (ISERIAL)
        // It is possible to set a serial number
        const char* p__serialNumber = nullptr;
#endif

    private:
        int p__USB_sendDeviceDescriptor();

#if (IMANUFACTURER || IPRODUCT || ISERIAL)
        // String descriptors supported

    private:
        int p__USB_sendStringDescriptorByIndex(const uint8_t descriptorIndex);

#endif

    protected:
        bool setup(USBSetup&) override final;
        int getInterface(uint8_t*) override final;
        int getDescriptor(USBSetup& setup) override final;

    public:
        Pluggable_USB_Descriptors();
        Pluggable_USB_Descriptors(const char* productName);

        void setIDs(const uint16_t vid, const uint16_t pid);
        void setProductName(const char* productName);
        void setManufacturer(const char* manufacturer);
        void setSerialNumber(const char* serialNumber);

};  //
