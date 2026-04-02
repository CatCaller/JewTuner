#include <iostream>
#include <windows.h>
#include <hidapi.h>

#define VENDOR_ID  0x2508
#define PRODUCT_ID 0x8032 

int main() {

    if (hid_init() != 0) {
        std::cerr << "HIDAPI is bad" << std::endl;
        return -1;
    }

    hid_device* handle = hid_open(VENDOR_ID, PRODUCT_ID, NULL);
    if (!handle) {
        std::cerr << "Not there -.-" << std::endl;
        return -1;
    }
    std::cout << "Connected >w<" << std::endl;

    hid_set_nonblocking(handle, 1);

    // init handshake sub_1400B9230
    unsigned char initBuffer[576]; // allocates 0x240
    memset(initBuffer, 0, sizeof(initBuffer));

    initBuffer[0] = 0x10; // Report ID
    initBuffer[1] = 0xFA; // Command ID

    int res = hid_write(handle, initBuffer, sizeof(initBuffer));

    if (res < 0) {
        std::cerr << "Failed to send init: " << hid_error(handle) << std::endl;

        hid_close(handle);
        return -1;
    }

    std::cout << "Init Handshake Sent" << std::endl;

    Sleep(100);

    //movement

    unsigned char moveBuffer[65]; // payload + 1 report ID

    // one if these is it i assume
    unsigned char testIDs[] = { 0x01, 0x02, 0x03, 0x04, 0x05 };

    for (unsigned char currentID : testIDs) {
        std::cout << "\nTesting ID: 0x" << std::hex << (int)currentID << std::dec << std::endl;

        for (int i = 0; i < 88; i++) {
            memset(moveBuffer, 0, sizeof(moveBuffer));

            moveBuffer[0] = currentID;

            // Buttons: 1 - 21
            // Sticks:  22 - 25

            moveBuffer[16] = 100; // Press Cross/A

            // Move left stick forward (Y-Axis)

            int8_t* stick_ly = (int8_t*)&moveBuffer[23];
            *stick_ly = -100; // -100 is fully up/forward i assume

            // Move right stick right (X-Axis)
            int8_t* stick_rx = (int8_t*)&moveBuffer[24];
            *stick_rx = 100; // 100 is fully right probably

            res = hid_write(handle, moveBuffer, sizeof(moveBuffer));
            if (res < 0) {
                std::cerr << "Gay: " << hid_error(handle) << std::endl;
				break; // break but try next ID
            }

            std::cout << "Sent (" << i << ") with ID 0x" << std::hex << (int)currentID << std::dec << std::endl;

            Sleep(10); //they did 1ms sleep in disasm
        }

        Sleep(1000); // wait so we see which works
    }

    hid_close(handle);
    hid_exit();
    return 0;
}