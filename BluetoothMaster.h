#include "mbed.h"
#include <vector>
#include <list>
#include <algorithm>
 
class sensorNodeSound{
private:
    long timeStamp;
    long duration;
public:
    sensorNodeSound(long timeStampIn, long durationIn);
    //constructor
   
    long getTimeStamp();
    //returns time stamp
   
    long getDuration();
    //returns duration
   
};
 
class NetworkNode{
private:
    std::uint8_t addr; //8 bit address vector
    std::vector< sensorNodeSound > sensorNodeSound_list; //vector of class sensorNodeSound used for all samples belonging to a node
public:
    NetworkNode(uint8_t addrIn);
    //constructor
   
    std::uint8_t getAddr();
    //returns address
   
    void addNodeSound(long timeStampIn, long duration);
    //creates object of class sensorNodeSound
   
    long getTimeStamp(int ind);
    //calls getTimeStamp of sensorNodeSound object in sensorNodeSound_list at given position
   
    long getDuration(int ind);
    //calls getDuration of sensorNodeSound object in sensorNodeSound_list at given position
   
    int getListLength();
    //returns size of sensorNodeSound_list vector
   
    void clearList();
    //clears sensorNodeSound_list
   
};
 
class BluetoothMaster {
private:
   
    Timer main_timer; //used for synchronisation with slaves
 
    std::vector< NetworkNode > node_list; //list of network nodes
   
    DigitalOut gndEn; //used to control gate of FET for HC-05 power supply
   
    DigitalOut AT_pin; //used to control pin 32 to put HC-05 into AT mode
   
    Serial dataLink; //declare global serial variable
   
    DigitalIn button; //user button used to send device into pairing routine
   
    bool singleSensorMode; //used to switch device into single sensor mode operation
   
    //Serial pc(SERIAL_TX, SERIAL_RX);
 
    const uint8_t node_addr = 0x01; //address of node
   
    std::vector<uint8_t> data; //vector to store received data
   
    bool OK_flag; //set by IRQ when incoming message is 'OK' and used by setAT and pairDevices to check when AT command has processed successfully
 
    bool clockAck; //clock acknowledge flag
   
    int EoT_flag; //flag to indicate the end of transmission, used by arduinos when sending log
 
public:
   
    bool wakeUp; //flag which is set when wakeUp command received
   
    BluetoothMaster(PinName _txIn, PinName _rxIn, PinName _gndEn, PinName _AT, PinName btn1);
    //constructor; enables interrupt and pairs devices
   
    void Rx_interrupt();
    //ISR for serial connection to HC-05
   
    void pairDevices();
    //puts HC-05 in AT mode and pairs with both HC-06 devices
   
    void setAT(int device);
    //puts HC-05 in AT mode and connects to a given HC-06 device
   
    int gatherData();
    //sends data requests to both sensor nodes in turn then finds matching samples and returns time difference
   
    int gatherDataSSM();
    //gathers data from one sensor only - faster but less accurate operation
   
    void sendClock();
    //transmits value of main_timer to HC-06 for synchronisation
   
    void sendRequest();
    //sends data requests to given sensor node
   
    void RxPacketControl(std::vector<uint8_t> & packet);
    //processes received data based on its function
   
    int checkSourceAddr(uint8_t addr);
    //returns position of given networkNode address. Adds new networkNode to list vector if no such address exists
   
    bool check_OK(int timeout);
    //check if OK_flag has been set within given timeout
   
};