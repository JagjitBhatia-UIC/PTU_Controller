#include "PTU_Controller.h"
#include <linux/joystick.h>
#include <cmath>


int main() {
    PTU_Controller ptu = PTU_Controller();
    ptu.toOrigin();

    std::cout << "READY" << std::endl;

    ptu.SetDelayMS(0);

    int js_fd = open("/dev/input/js0", O_RDONLY);


    struct js_event e;

    int js_x = 0;
    int js_y = 0;

    int xsensitivity = 1;
    int ysensitivity = 1;
    int sensitivity = 0;

    bool event_happened = false;

    // ptu.tilt(90);
    // ptu.pan(150);
    // ptu.tilt(-90);
    // ptu.pan(-150);
    // ptu.tilt(-90);
    // ptu.tilt(90);
    ptu.SetDelayMS(20);
    for(int j = 18; j<30; j += 1) {
        std::cout << "INCREMENT: " << j << std::endl;
         for(int i = 0; i<1000; i++) {
        ptu.pan(15);
    }
    for(int i = 0; i<1000; i++) {
        ptu.pan(-1 * 15);
    }

    }
   

    
    while(true) {
        event_happened = false;
        e = {};
        read(js_fd, &e, sizeof(e));
        if(e.type >= 0x80) continue;  // Ignore synthetic events

        if(e.type == 1 && e.value == 1 && e.number == 2) break;  // "X" Quit

        if(e.type == 1 && e.value == 1 && e.number == 0) ptu.toOrigin();  // "A" Origin/Init

        if(e.type == 1 && e.value == 1 && e.number == 4) {
            if(sensitivity - 5 >= 5) sensitivity -= 5;
        }

         if(e.type == 1 && e.value == 1 && e.number == 5) {
            if(sensitivity + 5 <= 1023) sensitivity += 5;
        }
    

        
         //std::cout << "TIMESTAMP: " << e.time << std::endl;
         //std::cout << "EVENT TYPE: " << (int) e.type << std::endl;
        //  std::cout << "EVENT VALUE: " << (int) e.value << std::endl;
        //  std::cout << "EVENT AXIS/BUTTON NUMBER: " << (int) e.number << std::endl;

        

         if(e.type == 2) {
            event_happened = true;

            if(e.number == 3) {
                js_x = round((xsensitivity/32767.0) * e.value);
                ptu.pan(js_x);

                std::cout << "PAN: " << js_x << std::endl;
            }

            if(e.number == 1) {
                js_y = round((ysensitivity/32767.0) * e.value);
                ptu.tilt(js_y);

                std::cout << "TILT: " << js_y << std::endl;
            }

            ptu.PrintState();
        }
        /*
        if(!event_happened && std::abs(js_x) == 5){
            std::cout << "OVERRIDE PAN" << std::endl;
            ptu.pan(js_x);
        } 
        if(!event_happened && std::abs(js_y) == 5) {
            std::cout << "OVERRIDE TILT" <<std::endl;
            ptu.tilt(js_y);

        }

        */

       

    }

    ptu.toOrigin();


    /*
    int delay = 1000;
    while(delay > 200) {
        std::cout << "DELAY: " << delay << std::endl;
        ptu.SetDelayMS(delay);
        ptu.toOrigin();
        ptu.pan(200);
        ptu.pan(-200);
        ptu.tilt(200);
        ptu.tilt(-200);
        ptu.pan(-500);
        ptu.pan(500);
        ptu.tilt(-200);
        ptu.tilt(200);
        delay = delay/2;
    }
    

    ptu.SetDelayMS(1000);
    //ptu.move_abs(0,511);
    ptu.pan_abs(511);
    ptu.tilt_abs(0);
    ptu.pan_abs(0);
    ptu.tilt_abs(1023);
    //ptu.move_abs(1023, 0);
    */
    
    /*
    
    std::string cmd;
    int val;

    while(true) {
        std::cout << "Enter Command: ";
        std::cin >> cmd;
        std::cout << std::endl;

        if(cmd == "quit") break;

        if(cmd == "pan" || cmd == "tilt") {
            std::cout << "Enter value: ";
            std::cin >> val;
            std::cout << std::endl;
            if(cmd == "pan") {
                 ptu.pan(val);
            }

            if(cmd == "tilt") {
                ptu.tilt(val);
            }
        }

        if(cmd == "origin") {
            ptu.toOrigin();
        }


    }

    */

    
}