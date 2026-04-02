#include <iostream>
#include <windows.h>
#include <hidapi.h>

#define VENDOR_ID  0x2508
#define BOOT_PID   0x0032 // Bootloader mode
#define NORMAL_PID 0x8032 // Normal mode

int main() {
    if (hid_init() != 0) {
        std::cerr << "HIDAPI is bad" << std::endl;
        return -1;
    }

    hid_device* handle = nullptr;

    handle = hid_open(VENDOR_ID, BOOT_PID, NULL);
    if (handle) {
        std::cout << "Detected Bootloader, escaping noww >w<" << std::endl;

        unsigned char bootBuffer[65] = { 0 };

        // Handshake from sub_1400B9230 
        bootBuffer[0] = 0x10;
        bootBuffer[1] = 0xFA;
        hid_write(handle, bootBuffer, 65);
        Sleep(50);

        // Send RUN command (0x01) to exit bootloader
        memset(bootBuffer, 0, 65);
        bootBuffer[0] = 0x10;
        bootBuffer[1] = 0x01;
        hid_write(handle, bootBuffer, 65);

        std::cout << "Run command sent. Waiting for reconnect..." << std::endl;
        hid_close(handle);
        handle = nullptr;
        Sleep(3000);
    }

    handle = hid_open(VENDOR_ID, NORMAL_PID, NULL);
    if (!handle) {
        std::cerr << "Not there -.-" << std::endl;
        hid_exit();
        return -1;
    }
    std::cout << "Connected in Normal Mode (8032) >w<" << std::endl;

    hid_set_nonblocking(handle, 1);

    // Init Handshake (sub_1400B9230) 
    unsigned char initBuffer[65] = { 0 };
    initBuffer[0] = 0x10;
    initBuffer[1] = 0xFA;
    if (hid_write(handle, initBuffer, 65) < 0) {
        std::cerr << "Failed to send init: " << hid_error(handle) << std::endl;
        hid_close(handle);
        return -1;
    }
    std::cout << "Init Handshake Sent" << std::endl;
    Sleep(100);

    // Movement
    unsigned char moveBuffer[65];
    unsigned char testIDs[] = { 0x01, 0x03, 0x04 };

    for (unsigned char currentID : testIDs) {
        std::cout << "\nTesting ID: 0x" << std::hex << (int)currentID << std::dec << std::endl;

        for (int i = 0; i < 88; i++) {
            memset(moveBuffer, 0, sizeof(moveBuffer));
            moveBuffer[0] = currentID;

            // Buttons: 1 - 21
            // Sticks:  22 - 25

            moveBuffer[16] = 100; // Press Cross/A (BUTTON_16)

            // Move left stick forward (Y-Axis)
            int8_t* stick_ly = (int8_t*)&moveBuffer[23]; // STICK_1_Y
            *stick_ly = -100; // -100 is forward

            // Move right stick right (X-Axis)
            int8_t* stick_rx = (int8_t*)&moveBuffer[24]; // STICK_2_X
            *stick_rx = 100; // 100 is right

            int res = hid_write(handle, moveBuffer, 65);
            if (res < 0) {
                std::cerr << "Gay: " << hid_error(handle) << std::endl;
                break;
            }

            std::cout << "Sent (" << i << ") with ID 0x" << (int)currentID << std::endl;
            Sleep(10);
        }
        Sleep(1000);
    }

    hid_close(handle);
    hid_exit();
    return 0;
}