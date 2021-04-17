#include "PTU_Controller.h"

int main() {
    PTU_Controller ptu = PTU_Controller();
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
}