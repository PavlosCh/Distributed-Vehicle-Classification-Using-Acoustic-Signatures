#include "BluetoothMaster.h"
#include <vector>
#include <algorithm>
#include <list>
#include <string>
 
//sensorNodeSound METHODS
sensorNodeSound::sensorNodeSound(long timeStampIn, long durationIn){
    timeStamp = timeStampIn;
    duration = durationIn;  
}
 
long sensorNodeSound::getTimeStamp(){
    return timeStamp;
}
 
long sensorNodeSound::getDuration(){
    return duration;
}
 
//NETWORK CLASS METHODS
NetworkNode::NetworkNode(std::uint8_t addrIn){
    addr = addrIn;
}  
 
void NetworkNode::addNodeSound(long timeStampIn, long duration){
    sensorNodeSound newSound(timeStampIn,duration);
    sensorNodeSound_list.push_back(newSound);
}
 
std::uint8_t NetworkNode::getAddr(){
    return addr;
}
 
long NetworkNode::getTimeStamp(int ind){
    return sensorNodeSound_list[ind].getTimeStamp();
}
 
long NetworkNode::getDuration(int ind){
    return sensorNodeSound_list[ind].getDuration();
}
 
int NetworkNode::getListLength(){
    return sensorNodeSound_list.size();
}
 
void NetworkNode::clearList(){
    sensorNodeSound_list.clear();
}
 
/**************************************************************************************************/
 
//XBEE BLUETOOTH CLASS METHODS
BluetoothMaster::BluetoothMaster(PinName _txIn, PinName _rxIn, PinName _gndEn, PinName _AT, PinName btn1): dataLink(_txIn,_rxIn), gndEn(_gndEn),AT_pin(_AT), button(btn1){
    dataLink.baud(38400);
    dataLink.attach(this,&BluetoothMaster::Rx_interrupt, Serial::RxIrq); //set interrupt function on receive pin
    main_timer.start(); //start main timer
    gndEn = 1; //controls HC-05 power
    AT_pin =0; //controls input to pin34 on HC-05
    wakeUp = 0; //reset wakeUp flag
    OK_flag = 0; //reset OK flag
    EoT_flag = 0; //reset end-of-transmission flag
    singleSensorMode = false; //initialise SSM state value
    printf("Nucleo with HC-05 is ready\n");
   
    //Pair and send clocks
    if(button==0)
        pairDevices(); //pair with both devices
    int dev = 1; //choose device 1
    setAT(dev); //connect to device 1
    while(clockAck == false){ //repeatedly transmit clock signal every 1s until acknowledgment received
        sendClock();
        wait(1);
    }
    clockAck = false; //reset clock acknowledge flag
   
    if(singleSensorMode == false){
        //repeat for device 2...
        dev = 2;
        setAT(dev);
        while(clockAck == false){    
            sendClock();
            wait(1);
        }
        clockAck = false; //reset clock acknowledge flag
    }
    printf("Clocks sent\n");
}
 
void BluetoothMaster::Rx_interrupt()
{
    std::vector<uint8_t> Rx_buffer;
    Timer t;
    t.start(); //start timer for bit receiving bits
    int start = t.read_ms(); //save start time
    while(t.read_ms () < (start+200)){
        while(dataLink.readable()){
           Rx_buffer.push_back(dataLink.getc());//add each incoming byte to buffer
            start = t.read_ms (); //update start time for each received bit
        }
    }
   
    //Check for 'OK' command for AT responses and set flag if received
    if (Rx_buffer[0] ==79 && Rx_buffer[1] == 75)
        OK_flag = 1;
    else{
    //Check valid destination address
        if (Rx_buffer[2] == node_addr)
            RxPacketControl(Rx_buffer); //call packet control function
        else
            printf("Packet destination address incorrect\n");
    }
    Rx_buffer.clear(); //clear the buffer ready for the next message
}
 
void BluetoothMaster::pairDevices(){
    printf("AT pairing routine...\n");
    //sets HC-05 power low, then sets HC-05 pin 34 then turns on device
    gndEn = 0;
    AT_pin = 1;
    wait(1);
    gndEn =1;
    wait(1);
   
    string addr1 = "98D3,31,FB2551"; //address of first HC-06
    string addr2 = "98D3,31,FB22D6"; //address of second HC-06
   
    while(check_OK(1) == false){ //repeat until 'OK' received from HC-05
        dataLink.printf("AT+RMAAD\r\n"); //clear all paired devices
        printf("AT+RMAAD\n");
    }
    while(check_OK(1) == false){
        dataLink.printf("AT+CMODE=0\r\n"); //connect only to selected devices
        printf("AT+CMODE=0\n");
    }  
 
    while(check_OK(1) == false){
        dataLink.printf("AT+PSWD=1234\r\n"); //set default password
        printf("AT+PSWD=1234\n");
    }
    while(check_OK(1) == false){
        dataLink.printf("AT+INIT\r\n"); //initialise SPP profile
        printf("AT+INIT\n");
    }
    while(check_OK(5) == false){
        dataLink.printf("AT+PAIR=%s,5\r\n",addr1); //pair with first device
        printf("AT+PAIR=%s,5\n",addr1);
    }
    while(check_OK(5) == false){
        dataLink.printf("AT+PAIR=%s,5\r\n",addr2); //pair with second device
        printf("AT+PAIR=%s,5\n",addr2);
    }
    AT_pin = 0; //set HC-05 pin 34 low
    printf("Exiting setup routine. Press button now for single sensor mode\n");
    wait(3);
    if(button==0) //if user button pressed after pairing then put device in SSM
        singleSensorMode = true;
}
 
void BluetoothMaster::setAT(int device){
    bool repeat_routine = true; //routine automatically repeats in case of failure, sets to false if routine executes properly
    while(repeat_routine == true){
        int link_fail_count = 0; //counts how many times link fails so HC-05 can be pulsed off an on again after 3 fails
        printf("AT link routine...\n");
        //sets HC-05 power low, then sets HC-05 pin 34 then turns on device
        gndEn = 0;
        AT_pin = 1;
        wait(3);
        gndEn =1;
        wait(1);
   
        //connect to selected HC-06
        string addr;
        switch(device){
            case 1:
                addr = "98D3,31,FB2551"; //address of first HC-06
                break;
            case 2:
               addr = "98D3,31,FB22D6"; //address of second HC-06
               break;
            default:  
              addr = "98D3,31,FB22D6"; //default is device 2
        }
        check_OK(1); //runs to clear any previously set OK flags
        while(check_OK(1) == false){ //repeat until 'OK' received from HC-05
            dataLink.printf("AT+INIT\r\n"); //initialise the SPP profile
            printf("AT+INIT\n");
        }
        check_OK(1); //runs to clear any previously set OK flags
        while(check_OK(5) == false){
            dataLink.printf("AT+BIND=%s\r\n",addr); //bind to selected device
            printf("AT+BIND=%s\n",addr);
        }
        check_OK(1); //runs to clear any previously set OK flags
        while(check_OK(5) == false){
            dataLink.printf("AT+LINK=%s\r\n",addr); //connect to selected device
            printf("AT+LINK=%s\n",addr);
            link_fail_count++; //increment link fail count in case of failure
            if (link_fail_count >= 3)
                break; //exit 'while' loop if link has failed 3 times so routine can repeat
        }
        if (link_fail_count < 3)
            repeat_routine = false; //if link did not fail then break the repeat cycle
    }
    printf("Exiting AT routine\n");
    //pulse HC-05 power again to connect
    gndEn = 0;
    AT_pin = 0;
    wait(0.1);
    gndEn =1;
}
 
int BluetoothMaster::gatherData(){
    //SSM
    if (singleSensorMode == true)
        return gatherDataSSM();
    //Normal operation    
    long timeStamp_in = main_timer.read_ms(); //use current timer value as time stamp
    int fail_count = 0;
    int dev = 1; //choose first sensor node
    setAT(dev); //connect to device
    while(node_list[0].getListLength() <=0){ //send send requests until data has been received
        wait(4);
        sendRequest();
        wait(4);
        fail_count++; //increment fail counter in case of failure
        if ((fail_count >= 3) & (node_list[1].getListLength() <=0))
            setAT(dev); //if no data has been received after three iterations, try to connect again
    }
    while(EoT_flag == 0)
        wait(0.1); //pause until the EoT flag has been received so all data entries have been received
    EoT_flag = 0; //reset the EoT flag ready for the next transmission
    fail_count = 0; //reset fail count
    //repeat for device 2...
    dev = 2;
    setAT(dev);
    while(node_list[1].getListLength() <=0){
        wait(4);
        sendRequest();
        wait(4);
        fail_count++;
        if ((fail_count >= 3) & (node_list[1].getListLength() <=0))
            setAT(dev);
    }
    while(EoT_flag == 0)
        wait(0.1); //pause until the EoT flag has been received so all data samples have been received
    EoT_flag = 0; //reset the EoT flag ready for the next transmission
    printf("processing results\n");
    long durationDiffMin = 10000; //records minimum difference in duration between two sets of samples, initialised with arbitrary value
    const long toleranceTS = 5000; //tolerance for time stamp in ms
    long duration_record[2]; //stores matching durations from device 1 & 2
    long timeStamp_record[2]; //stores matching time stamps from device 1 & 2
 
    //print all saved sample first
    for (int i = 0;i<node_list.size();i++) //for each device
        for (int j = 0; j<node_list[i].getListLength();j++){ //for each saved sample
            long duration = node_list[i].getDuration(j);
            long timeStamp = node_list[i].getTimeStamp(j);
            printf("Node %d: Time stamp = %d, duration = %d\n", node_list[i].getAddr(),timeStamp,duration);
    }
    //find matching entries from each device
    for (int i = 0; i < node_list[0].getListLength();i++){ //for each entry from Sensor node 1
        long duration = node_list[0].getDuration(i); //extract duration
        long timeStamp = node_list[0].getTimeStamp(i); //extract time stamp
        //first match time stamp of Sensor node 1 to Nucleo time stamp within accepted tolerance
        if (((timeStamp >= timeStamp_in-toleranceTS) & (timeStamp <= timeStamp_in+toleranceTS))|((timeStamp_in >= timeStamp-toleranceTS) & (timeStamp_in <= timeStamp+toleranceTS))){    
            duration_record[0] = duration;
            timeStamp_record[0] = timeStamp;
            //then look for closest matching entry from Sensor node 2
            for (int j = 0;j < node_list[1].getListLength();j++){ //for each entry for Sensor node 2
                long duration = node_list[1].getDuration(j);
                long timeStamp = node_list[1].getTimeStamp(j);
                //check that time stamp is within accepted tolerance of Nucleo time stamp
                if (((timeStamp >= timeStamp_in-toleranceTS) & (timeStamp <= timeStamp_in+toleranceTS))|((timeStamp_in >= timeStamp-toleranceTS) & (timeStamp_in <= timeStamp+toleranceTS))){
                    long durationDiff = abs(duration - duration_record[0]); //calculate the difference between durations for both nodes' entries
                    if (durationDiff < durationDiffMin){ //if the signal durations are a closer match than previously, overwrite previous matching entry
                        durationDiffMin = durationDiff;
                        duration_record[1] = duration;
                        timeStamp_record[1] = timeStamp;
                    }
                }
            }
        }
    }
    node_list[0].clearList();//clear all enties from node 1
    node_list[1].clearList();//clear all entries from node 2
    //print out two matching samples
    for (int i = 0; i<2;i++){
        printf("ts %d: %d\n",i,timeStamp_record[i]);
        printf("d %d: %d\n",i,duration_record[i]);
    }
    printf("Nucleo TS:%d\n",timeStamp_in);
    int time_diff_var = timeStamp_record[1] - timeStamp_record[0]; //calculate time difference between two samples
    return time_diff_var; //return time difference
}
 
int BluetoothMaster::gatherDataSSM(){
    long timeStamp_in = main_timer.read_ms(); //use current timer value as time stamp
    while(node_list[0].getListLength() <=0){ //send send requests until data has been received
        sendRequest();
        wait(4);
    }
    while(EoT_flag == 0)
        wait(0.1); //pause until the EoT flag has been received so all data entries have been received
    EoT_flag = 0; //reset the EoT flag ready for the next transmission
   
    printf("processing results\n");
    const long toleranceTS = 5000; //tolerance for time stamp in ms
    long duration_record; //stores matching durations from device 1 & 2
    long timeStamp_record; //stores matching time stamps from device 1 & 2
 
    //print all saved sample first
    for (int j = 0; j<node_list[0].getListLength();j++){ //for each saved sample
        long duration = node_list[0].getDuration(j);
        long timeStamp = node_list[0].getTimeStamp(j);
        printf("Node %d: Time stamp = %d, duration = %d\n", node_list[0].getAddr(),timeStamp,duration);
    }
    //find matching samples from each device
    for (int i = 0; i < node_list[0].getListLength();i++){
        long duration = node_list[0].getDuration(i);
        long timeStamp = node_list[0].getTimeStamp(i);
        //match time stamp of Sensor node to Nucleo time stamp
        if (((timeStamp >= timeStamp_in-toleranceTS) & (timeStamp <= timeStamp_in+toleranceTS))|((timeStamp_in >= timeStamp-toleranceTS) & (timeStamp_in <= timeStamp+toleranceTS))){    
            duration_record = duration;
            timeStamp_record = timeStamp;
        }
    }
    node_list[0].clearList();//clear entries for Sensor node
    //print out two matching sample
    printf("ts: %d\n",timeStamp_record);
    printf("Nucleo TS:%d\n",timeStamp_in);
    int time_diff_var = timeStamp_in - timeStamp_record; //calculate time difference between two samples
    return time_diff_var; //return time difference
}
 
 
void BluetoothMaster::sendClock()
{
   
    std::vector<uint8_t> transmissionPacket; //create new vector packet
    //populate packet
    transmissionPacket.push_back(0x01); //this node ID
    transmissionPacket.push_back(0x02); //packet ID
    transmissionPacket.push_back(0x00); //destination ID of any connected slave
    long set_time = main_timer.read_ms(); //read current value of timer and break into 4 bytes
    transmissionPacket.push_back((set_time & 0xff000000) >> 24);
    transmissionPacket.push_back((set_time & 0x00ff0000) >> 16);
    transmissionPacket.push_back((set_time & 0x0000ff00) >> 8);
    transmissionPacket.push_back(set_time & 0x000000ff);
         
    for (int i = 0; i < transmissionPacket.size(); i++){
        dataLink.printf("%c",transmissionPacket[i]); //send packet
        wait(0.1);
    }
}
 
void BluetoothMaster::sendRequest()
{
    std::vector<uint8_t> transmissionPacket; //create new vector packet
    //populate packet
    transmissionPacket.push_back(0x01); //this node ID
    transmissionPacket.push_back(0x03); //packet ID
    transmissionPacket.push_back(0x00); //destination ID of any connected slave
         
    for (int i = 0; i < transmissionPacket.size(); i++){
        dataLink.printf("%c",transmissionPacket[i]); //send packet
        wait(0.1);
    }
}
 
void BluetoothMaster::RxPacketControl(std::vector<uint8_t> & packet)
{
    uint8_t SourceAddr = packet[0]; //extract source address
    uint8_t command = packet[1]; //take packet code
    switch (command) { //index for different commands
        case 0x01:{ //Receive sample command
            int node_pos = checkSourceAddr(SourceAddr); //get position of node in node_list/add new node
            long timeStamp = (packet[3] << 24) + (packet[4] << 16) + (packet[5] << 8) + (packet[6]); //extract sample time stamp
            long duration = (packet[7] << 24) + (packet[8] << 16) + (packet[9] << 8) + (packet[10]); //extract sample duration
            node_list[node_pos].addNodeSound(timeStamp,duration); //add time stamp and duration to new sound entry
            break;
        }
        case 0x04:{ //Wake_up command
            printf("Received wake_up\n");
            wakeUp = 1; //set wake up flag
            break;
        }
        case 0x09:{ //Clock acknowledge
            int node_pos = checkSourceAddr(SourceAddr); //save new device
            printf("Received clock acknowledge\n");
            clockAck = true; //set clock acknowledge flag
            break;
        }
        case 0x0A:{ //EoT
            int node_pos = checkSourceAddr(SourceAddr);
            printf("Received end of transmission\n");
            EoT_flag = 1; //set EoT flag
            break;
        }
        default:
          printf("Received API address not recognised: %c\n",command);
    }
}
 
int BluetoothMaster::checkSourceAddr(uint8_t addr)
{
    bool exists = false;
    for (int i = 0; i<node_list.size();i++){ //search each entry in node_list for matching address
        if(node_list[i].getAddr() == addr){
            exists = true;
            return i; //return position of node
        }
    }
    if (exists == false){ //add new address to list if no match found
        NetworkNode newNode(addr);//create new node
        node_list.push_back(newNode); //add new node to list
        return (node_list.size()-1); //return position of new node
    }
}
 
bool BluetoothMaster::check_OK(int timeout){
    long start_time = main_timer.read_ms(); //save the current value of the timer
    long end_time = start_time + (timeout*1000); //set a value to the given timeout in ms
    while(main_timer.read_ms() < end_time){ //check if OK flag has been set within timeout
        if(OK_flag == 1){
            OK_flag = 0; //reset flag
            return 1;
        }
    }
    return 0;
}