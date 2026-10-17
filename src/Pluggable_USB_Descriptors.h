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
        int p__USB_send(const void* data, const int dataSize) {
            return USB_SendControl(0, data, dataSize);
        }

        int p__USB_sendDeviceDescriptor() {  //

            // Hand off to default core handler if IDs are not set
            if (!p__IDs_set) return 0;

            // Initialize device descriptor
            DeviceDescriptor deviceDescriptor = {
                .len = sizeof(DeviceDescriptor),
                .dtype = USB_DEVICE_DESCRIPTOR_TYPE,
                .usbVersion = 0x0200,
                .deviceClass = 0x00,
                .deviceSubClass = 0x00,
                .deviceProtocol = 0x00,
                .packetSize0 = 64,
                .idVendor = p__vendorID,
                .idProduct = p__productID,
                .deviceVersion = 0x0100,
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

#if (IMANUFACTURER || IPRODUCT || ISERIAL)
        // String descriptors supported

        void p__USB_sendAnsiCharAsUnicode(const char ch) {
            const char16_t unicodeChar = (char16_t)ch;
            p__USB_send(&unicodeChar, sizeof(char16_t));
        }

        int p__USB_sendStringDescriptor(const char* descriptor) {  //

            // Declare constants
            constexpr uint8_t MAX_STRING_DESCRIPTOR_LENGTH = 31;
            constexpr uint8_t USB_STRING_DESCRIPTOR_HEADER_SIZE = 2 * sizeof(uint8_t);
            constexpr uint8_t DESCRIPTOR_TYPE_STRING =
                (uint8_t)USB_STRING_DESCRIPTOR_TYPE;

            // Exit if descriptor is NULL
            if (!descriptor) return 0;

            // Get descriptor length (capped at 31 chars to fit standard USB limits)
            const uint8_t descriptorLength =
                strnlen(descriptor, MAX_STRING_DESCRIPTOR_LENGTH);

            // Calculate total length of string descriptor
            const uint8_t descriptorUnicodeLength =
                (descriptorLength + 1) * sizeof(char16_t);
            const uint8_t totalLength =
                USB_STRING_DESCRIPTOR_HEADER_SIZE + descriptorUnicodeLength;

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

        int p__USB_sendStringDescriptorByIndex(const uint8_t descriptorIndex) {  //

            switch (descriptorIndex) {  //

#if (IMANUFACTURER)
                // Send manufacturer string descriptor
                case IMANUFACTURER: return p__USB_sendStringDescriptor(p__manufacturer);
#endif

#if (IPRODUCT)
                // Send product string descriptor
                case IPRODUCT: return p__USB_sendStringDescriptor(p__productName);
#endif

#if (ISERIAL)
                // Send serial number string descriptor
                case ISERIAL: return p__USB_sendStringDescriptor(p__serialNumber);
#endif

            }  //

            // Hand off to default core handler
            return 0;

        }  //

#endif

    protected:
        bool setup(USBSetup&) override final {
            return false;
        }

        int getInterface(uint8_t*) override final {
            return 0;
        }

        int getDescriptor(USBSetup& setup) override final {  //

            // Get descriptor type requested
            const uint8_t descriptorType = setup.wValueH;

            // Send descriptor according to requested type
            switch (descriptorType) {  //

                // Intercept Device Descriptor Request (Type 0x01)
                case USB_DEVICE_DESCRIPTOR_TYPE:
                    // Send device descriptor
                    return p__USB_sendDeviceDescriptor();

#if (IMANUFACTURER || IPRODUCT || ISERIAL)
                // String descriptors supported

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

    public:
        Pluggable_USB_Descriptors() : PluggableUSBModule(0, 0, nullptr) {
            // Plug USB device into PluggableUSB
            PluggableUSB().plug(this);
        }

        void setIDs(const uint16_t vid, const uint16_t pid) {
            p__vendorID = vid;
            p__productID = pid;
            p__IDs_set = true;
        }

        void setProductName(const char* productName) {
#if (IPRODUCT)  // It is possible to set a product name
            p__productName = productName;
#else  // It is NOT possible to set a product name

// Show warning if product name is not set
#warning "IPRODUCT is not defined, therefore setProductName has no effect"

#endif
        }

        void setManufacturer(const char* manufacturer) {
#if (IMANUFACTURER)  // It is possible to set a manufacturer name
            p__manufacturer = manufacturer;
#else  // It is NOT possible to set a manufacturer name

// Show warning if manufacturer name is not set
#warning "IMANUFACTURER is not defined, therefore setManufacturer has no effect"

#endif
        }

        void setSerialNumber(const char* serialNumber) {
#if (ISERIAL)  // It is possible to set a serial number
            p__serialNumber = serialNumber;
#else  // It is NOT possible to set a serial number

// Show warning if serial number is not set
#warning "ISERIAL is not defined, therefore setSerialNumber has no effect"

#endif
        }

};  //
