#include "PTU_Controller.h"

PTU_Controller::PTU_Controller() {
    connected = false;
    pos_x = -1;
    pos_y = -1;
    rate_ms = DEFAULT_MSG_RATE_MS;

    if(PTU_Connect() > -1) {
        toOrigin();
    }
}

PTU_Controller::PTU_Controller(int _pos_x, int _pos_y) {
    connected = false;
    pos_x = -1;
    pos_y = -1;
    rate_ms = DEFAULT_MSG_RATE_MS;


    if(PTU_Connect() > -1) {
        move_abs(_pos_x, _pos_y);
    }
}

PTU_Controller::~PTU_Controller() {
    PTU_Disconnect();
}

int PTU_Controller::PTU_Connect() {
    ptu = SerialAdapter((char*) "/dev/ttyUSB0", O_RDWR, 9600);
    if(ptu.isConnected()) connected = true;
    else return -1;
    sleep(PTU_SERIAL_MAGIC_NUMBER);  // Sleep for "magic number" seconds before enabling serial writes

    return 0;
}

void PTU_Controller::PTU_Disconnect() {
    if(connected) {
        ptu.disconnect();
        connected = false;
    }
}

int PTU_Controller::toOrigin() {
    int ret = move_abs(ORIGIN_X, ORIGIN_Y);
    pos_x = ORIGIN_X;
    pos_y = ORIGIN_Y;
    return ret;
}


int PTU_Controller::tilt_abs(int pos) {
    return move_abs(pos_x, pos);
}

int PTU_Controller::pan_abs(int pos) {
    return move_abs(pos, pos_y);
}

int PTU_Controller::move_abs(int _pos_x, int _pos_y) {
    if(connected) {
        if(_pos_x < 0 || _pos_x > MAX_POSITION) {
            std::cout << "Invalid Pan Request!" << std::endl;
            return -1;
        }

        if(_pos_y < 530 || _pos_y > 610) {
            std::cout << "Invalid Tilt Request!" << std::endl;
            return -1;
        }

        int tilt_low, tilt_high, pan_low, pan_high;
        QueryPacket query;

        ToTwoBytes(_pos_x, pan_low, pan_high);
        ToTwoBytes(_pos_y, tilt_low, tilt_high);

        query.Tilt_High = tilt_high;
        query.Tilt_Low = tilt_low;
        query.Pan_High = pan_high;
        query.Pan_Low = pan_low;
        query.OpCode = 0x08;

        if(PTU_SendCommand(query) > -1) {
            pos_x = _pos_x;
            pos_y = _pos_y;
            return 0;
        }    
    }

    std::cout << "PTU Not Connected!" << std::endl;
    return -1;
}

int PTU_Controller::tilt(int inc) {
    return tilt_abs(pos_y + inc);
}

int PTU_Controller::pan(int inc) {
    return pan_abs(pos_x + inc);
}

int PTU_Controller::move(int _pan, int _tilt) {
    return move_abs(pos_x + _pan, pos_y + _tilt);
}

void PTU_Controller::ToTwoBytes(unsigned long number, int &low, int &high) {
    low = number & 0xFF;
    high = 0;
  
    if(number > 0xFF) {
      high = ((number & (0xFF << 8)) >> 8);
    }
}

void PTU_Controller::SetDelayMS(int delay) {
    rate_ms = delay;
}

int PTU_Controller::PTU_SendCommand(QueryPacket cmd) {
    char serialized_cmd[PTU_PACKET_SIZE_BYTES];

    serializeQueryPacket(cmd, serialized_cmd);
    if(!ptu.SerialWrite(serialized_cmd, sizeof(serialized_cmd))) return -1;
    usleep(rate_ms * MS_TO_NS);

    return 0;
}

void PTU_Controller::serializeQueryPacket(QueryPacket query, char (&cmd_packet)[PTU_PACKET_SIZE_BYTES]) {
    char b1 = 0xFF;  // Header - always 0xFF (255)
    char b2 = query.Pan_High;
    char b3 = query.Pan_Low;
    char b4 = query.Tilt_High;
    char b5 = query.Tilt_Low;
    char b6 = 0x00;  // Button value - always 0x00 (0)
    char b7 = query.OpCode;
    std::cout << b7 << std::endl;
    char b8 = (255 - (b2 + b3 + b4 + b5 + b6 + b7) % 256);  // Compute checksum
  
    cmd_packet[0] = b1;
    cmd_packet[1] = b2;
    cmd_packet[2] = b3;
    cmd_packet[3] = b4;
    cmd_packet[4] = b5;
    cmd_packet[5] = b6;
    cmd_packet[6] = b7;
    cmd_packet[7] = b8;
    
    
}

void PTU_Controller::PrintState() {
    std::cout << std::endl;

    std::cout << "----PTU State----" << std::endl;
    std::cout << std::endl;

    std::cout << "Connected: " << connected << std::endl;
    std::cout << std::endl;

    std::cout << "Pan Position: " << pos_x << std::endl;
    std::cout << std::endl;

    std::cout << "Tilt Position: " << pos_y << std::endl;
    std::cout << std::endl;

    std::cout << "----------------" << std::endl;
    
    std::cout << std::endl;

}