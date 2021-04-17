#include "PTU_Controller.h"

PTU_Controller::PTU_Controller() {
    connected = false;

    if(PTU_Connect() > -1) {
        toOrigin();
    }
}

PTU_Controller::PTU_Controller(int _pos_x, int _pos_y) {
    connected = false;

    if(PTU_Connect() > -1) {
        move_abs(pos_x, pos_y);
    }
}

PTU_Controller::~PTU_Controller() {
    PTU_Disconnect();
}

int PTU_Controller::PTU_Connect() {
    int serial_port = open("/dev/ttyUSB0", O_RDWR);

    if(flock(serial_port, LOCK_EX | LOCK_NB) == -1) {
        std::cout << "Serial port with file descriptor " << serial_port << " is already locked by another process." << std::endl;
    }

    struct termios tty;

    if(tcgetattr(serial_port, &tty) != 0) {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        return -1;
    }

    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size 
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
    // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

    tty.c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 0;

    cfsetispeed(&tty, B38400);
    cfsetospeed(&tty, B38400);

    // Save tty settings, also checking for error
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return -1;
    }

    ptu_conn = serial_port;
    connected = true;

    return 0;
}

void PTU_Controller::PTU_Disconnect() {
    if(connected) {
        close(ptu_conn);
        connected = false;
    }
}

int PTU_Controller::toOrigin() {
    return move_abs(ORIGIN_X, ORIGIN_Y);
}


int PTU_Controller::tilt_abs(int pos) {
    return move_abs(pos_x, pos);
}

int PTU_Controller::pan_abs(int pos) {
    return move_abs(pos, pos_y);
}

int PTU_Controller::move_abs(int _pos_x, int _pos_y) {
    if(_pos_x < 0 || _pos_x > MAX_POSITION) {
        std::cout << "Invalid Pan Request!" << std::endl;
        return -1;
    }

    if(_pos_y < 0 || _pos_y > MAX_POSITION) {
        std::cout << "Invalid Tilt Request!" << std::endl;
        return -1;
    }

    if(connected) {
        int tilt_low, tilt_high, pan_low, pan_high;
        QueryPacket query;

        ToTwoBytes(pan, pan_low, pan_high);
        ToTwoBytes(tilt, tilt_low, tilt_high);

        query.Tilt_High = tilt_high;
        query.Tilt_Low = tilt_low;
        query.Pan_High = pan_high;
        query.Pan_Low = pan_low;
        query.OpCode = 0x08;


        return PTU_SendCommand(ptu_conn, query);    
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

int PTU_Controller::move(int pan, int tilt) {
    return move_abs(pos_x + pan, pos_y + tilt);
}

void PTU_Controller::ToTwoBytes(unsigned long number, int &low, int &high) {
    low = number & 0xFF;
    high = 0;
  
    if(number > 0xFF) {
      high = ((number & (0xFF << 8)) >> 8);
    }
}

int PTU_Controller::PTU_SendCommand(int ptu_conn, QueryPacket cmd) {
    char serialized_cmd[PTU_PACKET_SIZE_BYTES];

    serializeQueryPacket(cmd, serialized_cmd);
    int bytes_written = write(ptu_conn, serialized_cmd, sizeof(serialized_cmd));
    fsync(ptu_conn);
    usleep(250000);

    if(bytes_written < PTU_PACKET_SIZE_BYTES) return -1;

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