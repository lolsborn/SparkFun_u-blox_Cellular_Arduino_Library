/*
  Arduino library for the u-blox SARA-R5 LTE-M / NB-IoT modules with secure cloud, as used on the SparkFun MicroMod
  Asset Tracker By: Paul Clark October 19th 2020

  Based extensively on the:
  Arduino Library for the SparkFun LTE CAT M1/NB-IoT Shield - SARA-R4
  Written by Jim Lindblom @ SparkFun Electronics, September 5, 2018

  This Arduino library provides mechanisms to initialize and use
  the SARA-R5 module over either a SoftwareSerial or hardware serial port.

  Please see LICENSE.md for the license information

*/

#include "sfe_ublox_cellular.h"

SparkFun_ublox_Cellular::SparkFun_ublox_Cellular(int powerPin, int resetPin, uint8_t maxInitTries)
{
#ifdef UBX_CELL_SOFTWARE_SERIAL_ENABLED
    _softSerial = nullptr;
#endif
    _hardSerial = nullptr;
    _baud = 0;
    _resetPin = resetPin;
    _powerPin = powerPin;
    _invertPowerPin = false;
    _maxInitTries = maxInitTries;
    _socketListenCallback = nullptr;
    _socketReadCallback = nullptr;
    _socketReadCallbackPlus = nullptr;
    _socketCloseCallback = nullptr;
    _gpsRequestCallback = nullptr;
    _simStateReportCallback = nullptr;
    _psdActionRequestCallback = nullptr;
    _pingRequestCallback = nullptr;
    _httpCommandRequestCallback = nullptr;
    _mqttCommandRequestCallback = nullptr;
    _registrationCallback = nullptr;
    _epsRegistrationCallback = nullptr;
    _debugAtPort = nullptr;
    _debugPort = nullptr;
    _printDebug = false;
    _lastRemoteIP = {0, 0, 0, 0};
    _lastLocalIP = {0, 0, 0, 0};
    for (int i = 0; i < UBX_CELL_NUM_SOCKETS; i++)
        _lastSocketProtocol[i] = 0; // Set to zero initially. Will be set to TCP/UDP by socketOpen etc.
    _autoTimeZoneForBegin = true;
    _bufferedPollReentrant = false;
    _pollReentrant = false;
    _saraResponseBacklogLength = 0;
    _saraRXBuffer = nullptr;
    _pruneBuffer = nullptr;
    _saraResponseBacklog = nullptr;

    // Add base URC handlers
    addURCHandler(UBX_CELL_READ_SOCKET_URC, [this](const char *event) { return this->urcHandlerReadSocket(event); });
    addURCHandler(UBX_CELL_READ_UDP_SOCKET_URC,
                  [this](const char *event) { return this->urcHandlerReadUDPSocket(event); });
    addURCHandler(UBX_CELL_LISTEN_SOCKET_URC,
                  [this](const char *event) { return this->urcHandlerListeningSocket(event); });
    addURCHandler(UBX_CELL_CLOSE_SOCKET_URC, [this](const char *event) { return this->urcHandlerCloseSocket(event); });
    addURCHandler(UBX_CELL_GNSS_REQUEST_LOCATION_URC,
                  [this](const char *event) { return this->urcHandlerGNSSRequestLocation(event); });
    addURCHandler(UBX_CELL_SIM_STATE_URC, [this](const char *event) { return this->urcHandlerSIMState(event); });
    addURCHandler(UBX_CELL_HTTP_COMMAND_URC, [this](const char *event) { return this->urcHandlerHTTPCommand(event); });
    addURCHandler(UBX_CELL_MQTT_COMMAND_URC, [this](const char *event) { return this->urcHandlerMQTTCommand(event); });
    addURCHandler(UBX_CELL_PING_COMMAND_URC, [this](const char *event) { return this->urcHandlerPingCommand(event); });
    addURCHandler(UBX_CELL_FTP_COMMAND_URC, [this](const char *event) { return this->urcHandlerFTPCommand(event); });
    addURCHandler(UBX_CELL_REGISTRATION_STATUS_URC,
                  [this](const char *event) { return this->urcHandlerRegistrationStatus(event); });
    addURCHandler(UBX_CELL_EPSREGISTRATION_STATUS_URC,
                  [this](const char *event) { return this->urcHandlerEPSRegistrationStatus(event); });
}

SparkFun_ublox_Cellular::~SparkFun_ublox_Cellular(void)
{
    if (nullptr != _saraRXBuffer)
    {
        delete[] _saraRXBuffer;
        _saraRXBuffer = nullptr;
    }
    if (nullptr != _pruneBuffer)
    {
        delete[] _pruneBuffer;
        _pruneBuffer = nullptr;
    }
    if (nullptr != _saraResponseBacklog)
    {
        delete[] _saraResponseBacklog;
        _saraResponseBacklog = nullptr;
    }
}

#ifdef UBX_CELL_SOFTWARE_SERIAL_ENABLED
bool SparkFun_ublox_Cellular::begin(SoftwareSerial &softSerial, unsigned long baud)
{
    if (nullptr == _saraRXBuffer)
    {
        _saraRXBuffer = new char[_RXBuffSize];
        if (nullptr == _saraRXBuffer)
        {
            if (_printDebug == true)
                _debugPort->println(F("begin: not enough memory for _saraRXBuffer!"));
            return false;
        }
    }
    memset(_saraRXBuffer, 0, _RXBuffSize);

    if (nullptr == _pruneBuffer)
    {
        _pruneBuffer = new char[_RXBuffSize];
        if (nullptr == _pruneBuffer)
        {
            if (_printDebug == true)
                _debugPort->println(F("begin: not enough memory for _pruneBuffer!"));
            return false;
        }
    }
    memset(_pruneBuffer, 0, _RXBuffSize);

    if (nullptr == _saraResponseBacklog)
    {
        _saraResponseBacklog = new char[_RXBuffSize];
        if (nullptr == _saraResponseBacklog)
        {
            if (_printDebug == true)
                _debugPort->println(F("begin: not enough memory for _saraResponseBacklog!"));
            return false;
        }
    }
    memset(_saraResponseBacklog, 0, _RXBuffSize);

    UBX_CELL_error_t err;

    _softSerial = &softSerial;

    //err = init(baud);
    //if (err == UBX_CELL_ERROR_SUCCESS)
    //{
        return true;
    //}
    //return false;
}
#endif

bool SparkFun_ublox_Cellular::begin(HardwareSerial &hardSerial, unsigned long baud)
{
    if (nullptr == _saraRXBuffer)
    {
        _saraRXBuffer = new char[_RXBuffSize];
        if (nullptr == _saraRXBuffer)
        {
            if (_printDebug == true)
                _debugPort->println(F("begin: not enough memory for _saraRXBuffer!"));
            return false;
        }
    }
    memset(_saraRXBuffer, 0, _RXBuffSize);

    if (nullptr == _pruneBuffer)
    {
        _pruneBuffer = new char[_RXBuffSize];
        if (nullptr == _pruneBuffer)
        {
            if (_printDebug == true)
                _debugPort->println(F("begin: not enough memory for _pruneBuffer!"));
            return false;
        }
    }
    memset(_pruneBuffer, 0, _RXBuffSize);

    if (nullptr == _saraResponseBacklog)
    {
        _saraResponseBacklog = new char[_RXBuffSize];
        if (nullptr == _saraResponseBacklog)
        {
            if (_printDebug == true)
                _debugPort->println(F("begin: not enough memory for _saraResponseBacklog!"));
            return false;
        }
    }
    memset(_saraResponseBacklog, 0, _RXBuffSize);

    UBX_CELL_error_t err;

    _hardSerial = &hardSerial;

//    err = init(baud);
    //if (err == UBX_CELL_ERROR_SUCCESS)
    //{
        return true;
    //}
    //return false;
}

// Calling this function with nothing sets the debug port to Serial
// You can also call it with other streams like Serial1, SerialUSB, etc.
void SparkFun_ublox_Cellular::enableDebugging(Print &debugPort)
{
    _debugPort = &debugPort;
    _printDebug = true;
}

// Calling this function with nothing sets the debug port to Serial
// You can also call it with other streams like Serial1, SerialUSB, etc.
void SparkFun_ublox_Cellular::enableAtDebugging(Print &debugPort)
{
    _debugAtPort = &debugPort;
    _printAtDebug = true;
}

// This function was originally written by Matthew Menze for the LTE Shield (SARA-R4) library
// See: https://github.com/sparkfun/SparkFun_LTE_Shield_Arduino_Library/pull/8
// It does the same job as ::poll but also processed any 'old' data stored in the backlog first
// It also has a built-in timeout - which ::poll does not
bool SparkFun_ublox_Cellular::bufferedPoll(void)
{
    if (_bufferedPollReentrant == true) // Check for reentry (i.e. bufferedPoll has been called from inside a callback)
        return false;

    _bufferedPollReentrant = true;

    int avail = 0;
    char c = 0;
    bool handled = false;
    unsigned long timeIn = millis();
    char *event;
    int backlogLen = _saraResponseBacklogLength;

    memset(_saraRXBuffer, 0, _RXBuffSize); // Clear _saraRXBuffer

    // Does the backlog contain any data? If it does, copy it into _saraRXBuffer and then clear the backlog
    if (_saraResponseBacklogLength > 0)
    {
        // The backlog also logs reads from other tasks like transmitting.
        if (_printDebug == true)
        {
            _debugPort->print(F("bufferedPoll: backlog found! backlogLen is "));
            _debugPort->println(_saraResponseBacklogLength);
        }
        memcpy(_saraRXBuffer + avail, _saraResponseBacklog, _saraResponseBacklogLength);
        avail += _saraResponseBacklogLength;
        memset(_saraResponseBacklog, 0, _RXBuffSize); // Clear the backlog making sure it is NULL-terminated
        _saraResponseBacklogLength = 0;
    }

    if ((hwAvailable() > 0) || (backlogLen > 0)) // If either new data is available, or backlog had data.
    {
        // Check for incoming serial data. Copy it into the backlog

        // Important note:
        // On ESP32, Serial.available only provides an update every ~120 bytes during the reception of long messages:
        // https://gitter.im/espressif/arduino-esp32?at=5e25d6370a1cf54144909c85
        // Be aware that if a long message is being received, the code below will timeout after _rxWindowMillis = 2
        // millis. At 115200 baud, hwAvailable takes ~120 * 10 / 115200 = 10.4 millis before it indicates that data is
        // being received.

        while (((millis() - timeIn) < _rxWindowMillis) && (avail < _RXBuffSize))
        {
            if (hwAvailable() > 0) // hwAvailable can return -1 if the serial port is NULL
            {
                c = readChar();
                // bufferedPoll is only interested in the URCs.
                // The URCs are all readable.
                // strtok does not like NULL characters.
                // So we need to make sure no NULL characters are added to _saraRXBuffer
                if (c == '\0')
                    c = '0'; // Convert any NULLs to ASCII Zeros
                _saraRXBuffer[avail++] = c;
                timeIn = millis();
            }
            else
            {
                yield();
            }
        }

        // _saraRXBuffer now contains the backlog (if any) and the new serial data (if any)

        // A health warning about strtok:
        // strtok will convert any delimiters it finds ("\r\n" in our case) into NULL characters.
        // Also, be very careful that you do not use strtok within an strtok while loop.
        // The next call of strtok(NULL, ...) in the outer loop will use the pointer saved from the inner loop!
        // In our case, strtok is also used in pruneBacklog, which is called by waitForRespone or
        // sendCommandWithResponse, which is called by the parse functions called by processURCEvent... The solution is
        // to use strtok_r - the reentrant version of strtok

        char *preservedEvent;
        event = strtok_r(_saraRXBuffer, "\r\n",
                         &preservedEvent); // Look for an 'event' (_saraRXBuffer contains something ending in \r\n)

        if (event != nullptr)
            if (_printDebug == true)
                _debugPort->println(F("bufferedPoll: event(s) found! ===>"));

        while (event != nullptr) // Keep going until all events have been processed
        {
            if (_printDebug == true)
            {
                _debugPort->print(F("bufferedPoll: start of event: "));
                _debugPort->println(event);
            }

            // Process the event
            bool latestHandled = processURCEvent((const char *)event);
            if (latestHandled)
            {
                if ((true == _printAtDebug) && (nullptr != event))
                {
                    _debugAtPort->print(event);
                }
                handled = true; // handled will be true if latestHandled has ever been true
            }
            if ((_saraResponseBacklogLength > 0) &&
                ((avail + _saraResponseBacklogLength) < _RXBuffSize)) // Has any new data been added to the backlog?
            {
                if (_printDebug == true)
                {
                    _debugPort->println(F("bufferedPoll: new backlog added!"));
                }
                memcpy(_saraRXBuffer + avail, _saraResponseBacklog, _saraResponseBacklogLength);
                avail += _saraResponseBacklogLength;
                memset(_saraResponseBacklog, 0, _RXBuffSize); // Clear out the backlog buffer again.
                _saraResponseBacklogLength = 0;
            }

            // Walk through any remaining events
            event = strtok_r(nullptr, "\r\n", &preservedEvent);

            if (_printDebug == true)
                _debugPort->println(F("bufferedPoll: end of event")); // Just to denote end of processing event.

            if (event == nullptr)
                if (_printDebug == true)
                    _debugPort->println(F("bufferedPoll: <=== end of event(s)!"));
        }
    }

    _bufferedPollReentrant = false;

    return handled;
} // /bufferedPoll

bool SparkFun_ublox_Cellular::urcHandlerReadSocket(const char *event)
{
    // URC: +UUSORD (Read Socket Data)
    int socket, length;
    char *searchPtr = strnstr(event, UBX_CELL_READ_SOCKET_URC, _RXBuffSize);
    if (searchPtr != nullptr)
    {
        searchPtr += strlen(UBX_CELL_READ_SOCKET_URC); // Move searchPtr to first character - probably a space
        while (*searchPtr == ' ')
            searchPtr++; // Skip spaces
        int ret = sscanf(searchPtr, "%d,%d", &socket, &length);
        if (ret == 2)
        {
            if (_printDebug == true)
                _debugPort->println(F("processReadEvent: read socket data"));
            // From the UBX_CELL AT Commands Manual:
            // "For the UDP socket type the URC +UUSORD: <socket>,<length> notifies that a UDP packet has been received,
            //  either when buffer is empty or after a UDP packet has been read and one or more packets are stored in
            //  the buffer."
            // So we need to check if this is a TCP socket or a UDP socket:
            //  If UDP, we call parseSocketReadIndicationUDP.
            //  Otherwise, we call parseSocketReadIndication.
            if (_lastSocketProtocol[socket] == UBX_CELL_UDP)
            {
                if (_printDebug == true)
                    _debugPort->println(F(
                        "processReadEvent: received +UUSORD but socket is UDP. Calling parseSocketReadIndicationUDP"));
                parseSocketReadIndicationUDP(socket, length);
            }
            else
                parseSocketReadIndication(socket, length);
            return true;
        }
    }

    return false;
}

bool SparkFun_ublox_Cellular::urcHandlerReadUDPSocket(const char *event)
{
    // URC: +UUSORF (Receive From command (UDP only))
    int socket, length;
    char *searchPtr = strnstr(event, UBX_CELL_READ_UDP_SOCKET_URC, _RXBuffSize);
    if (searchPtr != nullptr)
    {
        searchPtr += strlen(UBX_CELL_READ_UDP_SOCKET_URC); // Move searchPtr to first character - probably a space
        while (*searchPtr == ' ')
            searchPtr++; // skip spaces
        int ret = sscanf(searchPtr, "%d,%d", &socket, &length);
        if (ret == 2)
        {
            if (_printDebug == true)
                _debugPort->println(F("processReadEvent: UDP receive"));
            parseSocketReadIndicationUDP(socket, length);
            return true;
        }
    }

    return false;
}

bool SparkFun_ublox_Cellular::urcHandlerListeningSocket(const char *event)
{
    // URC: +UUSOLI (Set Listening Socket)
    int socket = 0;
    int listenSocket = 0;
    unsigned int port = 0;
    unsigned int listenPort = 0;
    IPAddress remoteIP = {0, 0, 0, 0};
    IPAddress localIP = {0, 0, 0, 0};
    int remoteIPstore[4] = {0, 0, 0, 0};
    int localIPstore[4] = {0, 0, 0, 0};

    char *searchPtr = strnstr(event, UBX_CELL_LISTEN_SOCKET_URC, _RXBuffSize);
    if (searchPtr != nullptr)
    {
        searchPtr += strlen(UBX_CELL_LISTEN_SOCKET_URC); // Move searchPtr to first character - probably a space
        while (*searchPtr == ' ')
            searchPtr++; // skip spaces
        int ret = sscanf(searchPtr, "%d,\"%d.%d.%d.%d\",%u,%d,\"%d.%d.%d.%d\",%u", &socket, &remoteIPstore[0],
                         &remoteIPstore[1], &remoteIPstore[2], &remoteIPstore[3], &port, &listenSocket,
                         &localIPstore[0], &localIPstore[1], &localIPstore[2], &localIPstore[3], &listenPort);
        for (int i = 0; i <= 3; i++)
        {
            if (ret >= 5)
                remoteIP[i] = (uint8_t)remoteIPstore[i];
            if (ret >= 11)
                localIP[i] = (uint8_t)localIPstore[i];
        }
        if (ret >= 5)
        {
            if (_printDebug == true)
                _debugPort->println(F("processReadEvent: socket listen"));
            parseSocketListenIndication(listenSocket, localIP, listenPort, socket, remoteIP, port);
            return true;
        }
    }

    return false;
}

bool SparkFun_ublox_Cellular::urcHandlerCloseSocket(const char *event)
{
    // URC: +UUSOCL (Close Socket)
    int socket;
    char *searchPtr = strnstr(event, UBX_CELL_CLOSE_SOCKET_URC, _RXBuffSize);
    if (searchPtr != nullptr)
    {
        searchPtr += strlen(UBX_CELL_CLOSE_SOCKET_URC); // Move searchPtr to first character - probably a space
        while (*searchPtr == ' ')
            searchPtr++; // skip spaces
        int ret = sscanf(searchPtr, "%d", &socket);
        if (ret == 1)
        {
            if (_printDebug == true)
                _debugPort->println(F("processReadEvent: socket close"));
            if ((socket >= 0) && (socket <= 6))
            {
                if (_socketCloseCallback != nullptr)
                {
                    _socketCloseCallback(socket);
                }
            }
            return true;
        }
    }

    return false;
}

bool SparkFun_ublox_Cellular::urcHandlerGNSSRequestLocation(const char *event)
{
    // URC: +UULOC (Localization information - CellLocate and hybrid positioning)
    ClockData clck;
    PositionData gps;
    SpeedData spd;
    unsigned long uncertainty;
    int scanNum;
    int latH, lonH, alt;
    unsigned int speedU, cogU;
    char latL[10], lonL[10];
    int dateStore[5];

    // Maybe we should also scan for +UUGIND and extract the activated gnss system?

    // This assumes the ULOC response type is "0" or "1" - as selected by gpsRequest detailed
    char *searchPtr = strnstr(event, UBX_CELL_GNSS_REQUEST_LOCATION_URC, _RXBuffSize);
    if (searchPtr != nullptr)
    {
        searchPtr += strlen(UBX_CELL_GNSS_REQUEST_LOCATION_URC); // Move searchPtr to first character - probably a space
        while (*searchPtr == ' ')
            searchPtr++; // skip spaces
        scanNum = sscanf(searchPtr, "%d/%d/%d,%d:%d:%d.%d,%d.%[^,],%d.%[^,],%d,%lu,%u,%u,%*s", &dateStore[0],
                         &dateStore[1], &clck.date.year, &dateStore[2], &dateStore[3], &dateStore[4], &clck.time.ms,
                         &latH, latL, &lonH, lonL, &alt, &uncertainty, &speedU, &cogU);
        clck.date.day = dateStore[0];
        clck.date.month = dateStore[1];
        clck.time.hour = dateStore[2];
        clck.time.minute = dateStore[3];
        clck.time.second = dateStore[4];

        if (scanNum >= 13)
        {
            // Found a Location string!
            if (_printDebug == true)
            {
                _debugPort->println(F("processReadEvent: location"));
            }

            if (latH >= 0)
                gps.lat = (float)latH + ((float)atol(latL) / pow(10, strlen(latL)));
            else
                gps.lat = (float)latH - ((float)atol(latL) / pow(10, strlen(latL)));
            if (lonH >= 0)
                gps.lon = (float)lonH + ((float)atol(lonL) / pow(10, strlen(lonL)));
            else
                gps.lon = (float)lonH - ((float)atol(lonL) / pow(10, strlen(lonL)));
            gps.alt = (float)alt;
            if (scanNum >= 15) // If detailed response, get speed data
            {
                spd.speed = (float)speedU;
                spd.cog = (float)cogU;
            }

            // if (_printDebug == true)
            // {
            //   _debugPort->print(F("processReadEvent: location:  lat: "));
            //   _debugPort->print(gps.lat, 7);
            //   _debugPort->print(F(" lon: "));
            //   _debugPort->print(gps.lon, 7);
            //   _debugPort->print(F(" alt: "));
            //   _debugPort->print(gps.alt, 2);
            //   _debugPort->print(F(" speed: "));
            //   _debugPort->print(spd.speed, 2);
            //   _debugPort->print(F(" cog: "));
            //   _debugPort->println(spd.cog, 2);
            // }

            if (_gpsRequestCallback != nullptr)
            {
                _gpsRequestCallback(clck, gps, spd, uncertainty);
            }

            return true;
        }
    }

    return false;
}

bool SparkFun_ublox_Cellular::urcHandlerSIMState(const char *event)
{
    // URC: +UUSIMSTAT (SIM Status)
    UBX_CELL_sim_states_t state;
    int scanNum;
    int stateStore;

    char *searchPtr = strnstr(event, UBX_CELL_SIM_STATE_URC, _RXBuffSize);
    if (searchPtr != nullptr)
    {
        searchPtr += strlen(UBX_CELL_SIM_STATE_URC); // Move searchPtr to first character - probably a space
        while (*searchPtr == ' ')
            searchPtr++; // skip spaces
        scanNum = sscanf(searchPtr, "%d", &stateStore);

        if (scanNum == 1)
        {
            if (_printDebug == true)
                _debugPort->println(F("processReadEvent: SIM status"));

            state = (UBX_CELL_sim_states_t)stateStore;

            if (_simStateReportCallback != nullptr)
            {
                _simStateReportCallback(state);
            }

            return true;
        }
    }

    return false;
}

bool SparkFun_ublox_Cellular::urcHandlerHTTPCommand(const char *event)
{
    // URC: +UUHTTPCR (HTTP Command Result)
    int profile, command, result;
    int scanNum;

    char *searchPtr = strnstr(event, UBX_CELL_HTTP_COMMAND_URC, _RXBuffSize);
    if (searchPtr != nullptr)
    {
        searchPtr += strlen(UBX_CELL_HTTP_COMMAND_URC); // Move searchPtr to first character - probably a space
        while (*searchPtr == ' ')
            searchPtr++; // skip spaces
        scanNum = sscanf(searchPtr, "%d,%d,%d", &profile, &command, &result);

        if (scanNum == 3)
        {
            if (_printDebug == true)
                _debugPort->println(F("processReadEvent: HTTP command result"));

            if ((profile >= 0) && (profile < UBX_CELL_NUM_HTTP_PROFILES))
            {
                if (_httpCommandRequestCallback != nullptr)
                {
                    _httpCommandRequestCallback(profile, command, result);
                }
            }

            return true;
        }
    }

    return false;
}

bool SparkFun_ublox_Cellular::urcHandlerMQTTCommand(const char *event)
{
    // URC: +UUMQTTC (MQTT Command Result)
    int command, result;
    int scanNum;
    int qos = -1;
    String topic;

    char *searchPtr = strnstr(event, UBX_CELL_MQTT_COMMAND_URC, _RXBuffSize);
    if (searchPtr != nullptr)
    {
        searchPtr += strlen(UBX_CELL_MQTT_COMMAND_URC); // Move searchPtr to first character - probably a space
        while (*searchPtr == ' ')
        {
            searchPtr++; // skip spaces
        }

        scanNum = sscanf(searchPtr, "%d,%d", &command, &result);
        if ((scanNum == 2) && (command == UBX_CELL_MQTT_COMMAND_SUBSCRIBE))
        {
            char topicC[100] = "";
            scanNum = sscanf(searchPtr, "%*d,%*d,%d,\"%[^\"]\"", &qos, topicC);
            topic = topicC;
        }
        if ((scanNum == 2) || (scanNum == 4))
        {
            if (_printDebug == true)
            {
                _debugPort->println(F("processReadEvent: MQTT command result"));
            }

            if (_mqttCommandRequestCallback != nullptr)
            {
                _mqttCommandRequestCallback(command, result);
            }

            return true;
        }
    }

    return false;
}

bool SparkFun_ublox_Cellular::urcHandlerPingCommand(const char *event)
{
    // URC: +UUPING (Ping Result)
    int retry = 0;
    int p_size = 0;
    int ttl = 0;
    String remote_host = "";
    IPAddress remoteIP = {0, 0, 0, 0};
    long rtt = 0;
    int scanNum;

    // Try to extract the UUPING retries and payload size
    char *searchPtr = strnstr(event, UBX_CELL_PING_COMMAND_URC, _RXBuffSize);
    if (searchPtr != nullptr)
    {
        searchPtr += strlen(UBX_CELL_PING_COMMAND_URC); // Move searchPtr to first character - probably a space
        while (*searchPtr == ' ')
            searchPtr++; // skip spaces
        scanNum = sscanf(searchPtr, "%d,%d,", &retry, &p_size);

        if (scanNum == 2)
        {
            if (_printDebug == true)
            {
                _debugPort->println(F("processReadEvent: ping"));
            }

            searchPtr = strchr(++searchPtr, '\"'); // Search to the first quote

            // Extract the remote host name, stop at the next quote
            while ((*(++searchPtr) != '\"') && (*searchPtr != '\0'))
            {
                remote_host.concat(*(searchPtr));
            }

            if (*searchPtr != '\0') // Make sure we found a quote
            {
                // Extract IP address
                int remoteIPstore[4];
                scanNum = sscanf(searchPtr, "\",\"%d.%d.%d.%d", &remoteIPstore[0], &remoteIPstore[1], &remoteIPstore[2],
                                 &remoteIPstore[3]);
                for (int i = 0; i <= 3; i++)
                {
                    remoteIP[i] = (uint8_t)remoteIPstore[i];
                }

                if (scanNum == 4) // Make sure we extracted enough data
                {
                    // Extract TTL, should be immediately after IP address
                    searchPtr = strchr(searchPtr + 2, ','); // +2 to skip the quote and comma
                    if (searchPtr != nullptr)
                    {
                        // It's possible the TTL is not present (eg. on LARA-R6), so we
                        // can ignore scanNum since ttl defaults to 0 anyways
                        scanNum = sscanf(searchPtr, ",%d", &ttl);

                        // Extract RTT, should be immediately after TTL
                        searchPtr = strchr(searchPtr + 1, ','); // +1 to skip the comma
                        if (searchPtr != nullptr)
                        {
                            scanNum = sscanf(searchPtr, ",%ld", &rtt);

                            // Callback, if it exists
                            if (_pingRequestCallback != nullptr)
                            {
                                _pingRequestCallback(retry, p_size, remote_host, remoteIP, ttl, rtt);
                            }
                        }
                    }
                }
            }
            return true;
        }
    }

    return false;
}

bool SparkFun_ublox_Cellular::urcHandlerFTPCommand(const char *event)
{
    // URC: +UUFTPCR (FTP Command Result)
    int ftpCmd;
    int ftpResult;
    int scanNum;
    char *searchPtr = strnstr(event, UBX_CELL_FTP_COMMAND_URC, _RXBuffSize);
    if (searchPtr != nullptr)
    {
        searchPtr += strlen(UBX_CELL_FTP_COMMAND_URC); // Move searchPtr to first character - probably a space
        while (*searchPtr == ' ')
        {
            searchPtr++; // skip spaces
        }

        scanNum = sscanf(searchPtr, "%d,%d", &ftpCmd, &ftpResult);
        if (scanNum == 2 && _ftpCommandRequestCallback != nullptr)
        {
            _ftpCommandRequestCallback(ftpCmd, ftpResult);
            return true;
        }
    }

    return false;
}

bool SparkFun_ublox_Cellular::urcHandlerRegistrationStatus(const char *event)
{
    // URC: +CREG
    int status = 0;
    unsigned int lac = 0, ci = 0, Act = 0;
    char *searchPtr = strnstr(event, UBX_CELL_REGISTRATION_STATUS_URC, _RXBuffSize);
    if (searchPtr != nullptr)
    {
        searchPtr += strlen(UBX_CELL_REGISTRATION_STATUS_URC); // Move searchPtr to first character - probably a space
        while (*searchPtr == ' ')
            searchPtr++; // skip spaces
        int scanNum = sscanf(searchPtr, "%d,\"%4x\",\"%4x\",%d", &status, &lac, &ci, &Act);
        if (scanNum == 4)
        {
            if (_printDebug == true)
                _debugPort->println(F("processReadEvent: CREG"));

            if (_registrationCallback != nullptr)
            {
                _registrationCallback((UBX_CELL_registration_status_t)status, lac, ci, Act);
            }

            return true;
        }
    }

    return false;
}

bool SparkFun_ublox_Cellular::urcHandlerEPSRegistrationStatus(const char *event)
{
    // URC: +CEREG
    int status = 0;
    unsigned int tac = 0, ci = 0, Act = 0;
    char *searchPtr = strnstr(event, UBX_CELL_EPSREGISTRATION_STATUS_URC, _RXBuffSize);
    if (searchPtr != nullptr)
    {
        searchPtr +=
            strlen(UBX_CELL_EPSREGISTRATION_STATUS_URC); // Move searchPtr to first character - probably a space
        while (*searchPtr == ' ')
            searchPtr++; // skip spaces
        int scanNum = sscanf(searchPtr, "%d,\"%4x\",\"%4x\",%d", &status, &tac, &ci, &Act);
        if (scanNum == 4)
        {
            if (_printDebug == true)
                _debugPort->println(F("processReadEvent: CEREG"));

            if (_epsRegistrationCallback != nullptr)
            {
                _epsRegistrationCallback((UBX_CELL_registration_status_t)status, tac, ci, Act);
            }

            return true;
        }
    }

    return false;
}

void SparkFun_ublox_Cellular::addURCHandler(const char *urcString, UBX_CELL_urc_handler_t urcHandler)
{
    _urcStrings.push_back(urcString);
    _urcHandlers.push_back(urcHandler);
}

// Parse incoming URC's - the associated parse functions pass the data to the user via the callbacks (if defined)
bool SparkFun_ublox_Cellular::processURCEvent(const char *event)
{
    // Iterate through each URC handler to see if it can handle this message
    for (auto urcHandler : _urcHandlers)
    {
        if (urcHandler(event))
        {
            // This handler took care of it, so we're done!
            return true;
        }
    }

    // None of the handlers took care of it
    return false;
}

// This is the original poll function.
// It is 'blocking' - it does not return when serial data is available until it receives a `\n`.
// ::bufferedPoll is the new improved version. It processes any data in the backlog and includes a timeout.
bool SparkFun_ublox_Cellular::poll(void)
{
    if (_pollReentrant == true) // Check for reentry (i.e. poll has been called from inside a callback)
        return false;

    _pollReentrant = true;

    int avail = 0;
    char c = 0;
    bool handled = false;

    memset(_saraRXBuffer, 0, _RXBuffSize); // Clear _saraRXBuffer

    if (hwAvailable() > 0) // hwAvailable can return -1 if the serial port is NULL
    {
        while (c != '\n') // Copy characters into _saraRXBuffer. Stop at the first new line
        {
            if (hwAvailable() > 0) // hwAvailable can return -1 if the serial port is NULL
            {
                c = readChar();
                _saraRXBuffer[avail++] = c;
            }
            else
            {
                yield();
            }
        }

        // Now search for all supported URC's
        handled = processURCEvent(_saraRXBuffer);
        if (handled && (true == _printAtDebug))
        {
            _debugAtPort->write(_saraRXBuffer, avail);
        }
        if ((handled == false) && (strlen(_saraRXBuffer) > 2))
        {
            if (_printDebug == true)
            {
                _debugPort->print(F("poll: "));
                _debugPort->println(_saraRXBuffer);
            }
        }
    }

    _pollReentrant = false;

    return handled;
}

void SparkFun_ublox_Cellular::setSocketListenCallback(void (*socketListenCallback)(int, IPAddress, unsigned int, int, IPAddress,
                                                                    unsigned int))
{
    _socketListenCallback = socketListenCallback;
}

void SparkFun_ublox_Cellular::setSocketReadCallback(void (*socketReadCallback)(int, String))
{
    _socketReadCallback = socketReadCallback;
}

void SparkFun_ublox_Cellular::setSocketReadCallbackPlus(void (*socketReadCallbackPlus)(
    int, const char *, int, IPAddress, int)) // socket, data, length, remoteAddress, remotePort
{
    _socketReadCallbackPlus = socketReadCallbackPlus;
}

void SparkFun_ublox_Cellular::setSocketCloseCallback(void (*socketCloseCallback)(int))
{
    _socketCloseCallback = socketCloseCallback;
}

void SparkFun_ublox_Cellular::setGpsReadCallback(void (*gpsRequestCallback)(ClockData time, PositionData gps, SpeedData spd,
                                                             unsigned long uncertainty))
{
    _gpsRequestCallback = gpsRequestCallback;
}

void SparkFun_ublox_Cellular::setSIMstateReportCallback(void (*simStateReportCallback)(UBX_CELL_sim_states_t state))
{
    _simStateReportCallback = simStateReportCallback;
}

void SparkFun_ublox_Cellular::setPSDActionCallback(void (*psdActionRequestCallback)(int result, IPAddress ip))
{
    _psdActionRequestCallback = psdActionRequestCallback;
}

void SparkFun_ublox_Cellular::setPingCallback(void (*pingRequestCallback)(int retry, int p_size, String remote_hostname, IPAddress ip,
                                                           int ttl, long rtt))
{
    _pingRequestCallback = pingRequestCallback;
}

void SparkFun_ublox_Cellular::setHTTPCommandCallback(void (*httpCommandRequestCallback)(int profile, int command, int result))
{
    _httpCommandRequestCallback = httpCommandRequestCallback;
}

void SparkFun_ublox_Cellular::setMQTTCommandCallback(void (*mqttCommandRequestCallback)(int command, int result))
{
    _mqttCommandRequestCallback = mqttCommandRequestCallback;
}

void SparkFun_ublox_Cellular::setFTPCommandCallback(void (*ftpCommandRequestCallback)(int command, int result))
{
    _ftpCommandRequestCallback = ftpCommandRequestCallback;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setRegistrationCallback(void (*registrationCallback)(UBX_CELL_registration_status_t status,
                                                                                unsigned int lac, unsigned int ci,
                                                                                int Act))
{
    _registrationCallback = registrationCallback;

    size_t cmdLen = strlen(UBX_CELL_REGISTRATION_STATUS) + 3;
    char command[cmdLen];
    snprintf(command, cmdLen, "%s=%d", UBX_CELL_REGISTRATION_STATUS, 2 /*enable URC with location*/);
    UBX_CELL_error_t err =
        sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setEpsRegistrationCallback(
    void (*registrationCallback)(UBX_CELL_registration_status_t status, unsigned int tac, unsigned int ci, int Act))
{
    _epsRegistrationCallback = registrationCallback;

    size_t cmdLen = strlen(UBX_CELL_EPSREGISTRATION_STATUS) + 3;
    char command[cmdLen];
    snprintf(command, cmdLen, "%s=%d", UBX_CELL_EPSREGISTRATION_STATUS, 2 /*enable URC with location*/);
    UBX_CELL_error_t err =
        sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    return err;
}

size_t SparkFun_ublox_Cellular::write(uint8_t c)
{
    return hwWrite(c);
}

size_t SparkFun_ublox_Cellular::write(const char *str)
{
    return hwPrint(str);
}

size_t SparkFun_ublox_Cellular::write(const char *buffer, size_t size)
{
    return hwWriteData(buffer, size);
}

UBX_CELL_error_t SparkFun_ublox_Cellular::at(void)
{
    UBX_CELL_error_t err;

    err = sendCommandWithResponse(nullptr, UBX_CELL_RESPONSE_OK, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::enableEcho(bool enable)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_COMMAND_ECHO) + 2;
    char command[cmdLen];
    snprintf(command, cmdLen, "%s%d", UBX_CELL_COMMAND_ECHO, enable ? 1 : 0);
    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    return err;
}

String SparkFun_ublox_Cellular::getManufacturerID(void)
{
    char response[minimumResponseAllocation];
    char idResponse[16] = {0x00}; // E.g. u-blox
    UBX_CELL_error_t err;

    err = sendCommandWithResponse(UBX_CELL_COMMAND_MANU_ID, UBX_CELL_RESPONSE_OK_OR_ERROR, response,
                                  UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        if (sscanf(response, "\r\n%15s\r\n", idResponse) != 1)
        {
            memset(idResponse, 0, 16);
        }
    }
    return String(idResponse);
}

String SparkFun_ublox_Cellular::getModelID(void)
{
    char response[minimumResponseAllocation];
    char idResponse[32] = {0x00}; // E.g. SARA-R510M8Q
    UBX_CELL_error_t err;

    err = sendCommandWithResponse(UBX_CELL_COMMAND_MODEL_ID, UBX_CELL_RESPONSE_OK_OR_ERROR, response,
                                  UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        if (sscanf(response, "\r\n%31s\r\n", idResponse) != 1)
        {
            memset(idResponse, 0, 16);
        }
    }
    return String(idResponse);
}

String SparkFun_ublox_Cellular::getFirmwareVersion(void)
{
    char response[minimumResponseAllocation];
    char idResponse[16] = {0x00}; // E.g. 11.40
    UBX_CELL_error_t err;

    err = sendCommandWithResponse(UBX_CELL_COMMAND_FW_VER_ID, UBX_CELL_RESPONSE_OK_OR_ERROR, response,
                                  UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        if (sscanf(response, "\r\n%15s\r\n", idResponse) != 1)
        {
            memset(idResponse, 0, 16);
        }
    }
    return String(idResponse);
}

String SparkFun_ublox_Cellular::getSerialNo(void)
{
    char response[minimumResponseAllocation];
    char idResponse[32] = {0x00}; // E.g. 357520070120767
    UBX_CELL_error_t err;

    err = sendCommandWithResponse(UBX_CELL_COMMAND_SERIAL_NO, UBX_CELL_RESPONSE_OK_OR_ERROR, response,
                                  UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        if (sscanf(response, "\r\n%31s\r\n", idResponse) != 1)
        {
            memset(idResponse, 0, 16);
        }
    }
    return String(idResponse);
}

String SparkFun_ublox_Cellular::getIMEI(void)
{
    char response[minimumResponseAllocation];
    char imeiResponse[32] = {0x00}; // E.g. 004999010640000
    UBX_CELL_error_t err;

    err = sendCommandWithResponse(UBX_CELL_COMMAND_IMEI, UBX_CELL_RESPONSE_OK_OR_ERROR, response,
                                  UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        if (sscanf(response, "\r\n%31s\r\n", imeiResponse) != 1)
        {
            memset(imeiResponse, 0, 16);
        }
    }
    return String(imeiResponse);
}

String SparkFun_ublox_Cellular::getIMSI(void)
{
    char response[minimumResponseAllocation];
    char imsiResponse[32] = {0x00}; // E.g. 222107701772423
    UBX_CELL_error_t err;

    err = sendCommandWithResponse(UBX_CELL_COMMAND_IMSI, UBX_CELL_RESPONSE_OK_OR_ERROR, response,
                                  UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        if (sscanf(response, "\r\n%31s\r\n", imsiResponse) != 1)
        {
            memset(imsiResponse, 0, 16);
        }
    }
    return String(imsiResponse);
}

String SparkFun_ublox_Cellular::getCCID(void)
{
    char response[minimumResponseAllocation];
    char ccidResponse[32] = {0x00}; // E.g. +CCID: 8939107900010087330
    UBX_CELL_error_t err;

    err = sendCommandWithResponse(UBX_CELL_COMMAND_CCID, UBX_CELL_RESPONSE_OK_OR_ERROR, response,
                                  UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        char *searchPtr = strnstr(response, "\r\n+CCID:", minimumResponseAllocation);
        if (searchPtr != nullptr)
        {
            searchPtr += strlen("\r\n+CCID:"); // Move searchPtr to first character - probably a space
            while (*searchPtr == ' ')
                searchPtr++; // skip spaces
            if (sscanf(searchPtr, "%31s", ccidResponse) != 1)
            {
                ccidResponse[0] = 0;
            }
        }
    }
    return String(ccidResponse);
}

String SparkFun_ublox_Cellular::getSubscriberNo(void)
{
    char response[minimumResponseAllocation];
    char idResponse[128] = {0x00}; // E.g. +CNUM: "ABCD . AAA","123456789012",129
    UBX_CELL_error_t err;

    err = sendCommandWithResponse(UBX_CELL_COMMAND_CNUM, UBX_CELL_RESPONSE_OK_OR_ERROR, response,
                                  UBX_CELL_10_SEC_TIMEOUT);
    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        char *searchPtr = strnstr(response, "\r\n+CNUM:", minimumResponseAllocation);
        if (searchPtr != nullptr)
        {
            searchPtr += strlen("\r\n+CNUM:"); // Move searchPtr to first character - probably a space
            while (*searchPtr == ' ')
                searchPtr++; // skip spaces
            if (sscanf(searchPtr, "%127s", idResponse) != 1)
            {
                idResponse[0] = 0;
            }
        }
    }
    return String(idResponse);
}

String SparkFun_ublox_Cellular::getCapabilities(void)
{
    char response[minimumResponseAllocation];
    char idResponse[128] = {0x00}; // E.g. +GCAP: +FCLASS, +CGSM
    UBX_CELL_error_t err;

    err = sendCommandWithResponse(UBX_CELL_COMMAND_REQ_CAP, UBX_CELL_RESPONSE_OK_OR_ERROR, response,
                                  UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        char *searchPtr = strnstr(response, "\r\n+GCAP:", minimumResponseAllocation);
        if (searchPtr != nullptr)
        {
            searchPtr += strlen("\r\n+GCAP:"); // Move searchPtr to first character - probably a space
            while (*searchPtr == ' ')
                searchPtr++; // skip spaces
            if (sscanf(searchPtr, "%127s", idResponse) != 1)
            {
                idResponse[0] = 0;
            }
        }
    }
    return String(idResponse);
}

UBX_CELL_error_t SparkFun_ublox_Cellular::reset(void)
{
    UBX_CELL_error_t err;

    err = functionality(SILENT_RESET_WITH_SIM);
    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        // Reset will set the baud rate back to 115200
        // beginSerial(9600);
        err = UBX_CELL_ERROR_INVALID;
        while (err != UBX_CELL_ERROR_SUCCESS)
        {
            beginSerial(UBX_CELL_DEFAULT_BAUD_RATE);
            setBaud(_baud);
            beginSerial(_baud);
            err = at();
        }
        return init(_baud);
    }
    return err;
}

String SparkFun_ublox_Cellular::clock(void)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_COMMAND_CLOCK) + 2;
    char command[cmdLen];
    char response[minimumResponseAllocation];
    char *clockBegin;
    char *clockEnd;

    snprintf(command, cmdLen, "%s?", UBX_CELL_COMMAND_CLOCK);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    if (err != UBX_CELL_ERROR_SUCCESS)
    {
        return "";
    }

    // Response format: \r\n+CCLK: "YY/MM/DD,HH:MM:SS-TZ"\r\n\r\nOK\r\n
    clockBegin = strchr(response, '\"'); // Find first quote
    if (clockBegin == nullptr)
    {
        return "";
    }
    clockBegin += 1;                     // Increment pointer to begin at first number
    clockEnd = strchr(clockBegin, '\"'); // Find last quote
    if (clockEnd == nullptr)
    {
        return "";
    }
    *(clockEnd) = '\0'; // Set last quote to null char -- end string

    String clock = String(clockBegin); // Extract the clock as a String _before_ freeing response

    return (clock);
}

UBX_CELL_error_t SparkFun_ublox_Cellular::clock(uint8_t *y, uint8_t *mo, uint8_t *d, uint8_t *h, uint8_t *min, uint8_t *s, int8_t *tz)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_COMMAND_CLOCK) + 2;
    char command[cmdLen];
    char response[minimumResponseAllocation];
    char tzPlusMinus;
    int scanNum = 0;

    int iy, imo, id, ih, imin, is, itz;

    snprintf(command, cmdLen, "%s?", UBX_CELL_COMMAND_CLOCK);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    // Response format (if TZ is negative): \r\n+CCLK: "YY/MM/DD,HH:MM:SS-TZ"\r\n\r\nOK\r\n
    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        char *searchPtr = strnstr(response, "+CCLK:", minimumResponseAllocation);
        if (searchPtr != nullptr)
        {
            searchPtr += strlen("+CCLK:"); //  Move searchPtr to first char
            while (*searchPtr == ' ')
                searchPtr++; // skip spaces
            scanNum =
                sscanf(searchPtr, "\"%d/%d/%d,%d:%d:%d%c%d\"\r\n", &iy, &imo, &id, &ih, &imin, &is, &tzPlusMinus, &itz);
        }
        if (scanNum == 8)
        {
            *y = iy;
            *mo = imo;
            *d = id;
            *h = ih;
            *min = imin;
            *s = is;
            if (tzPlusMinus == '-')
                *tz = 0 - itz;
            else
                *tz = itz;
        }
        else
            err = UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
    }

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setClock(uint8_t y, uint8_t mo, uint8_t d, uint8_t h, uint8_t min, uint8_t s, int8_t tz)
{
    // Convert y,mo,d,h,min,s,tz into a String
    // Some platforms don't support snprintf correctly (for %02d or %+02d) so we need to build the String manually
    // Format is "yy/MM/dd,hh:mm:ss+TZ"
    // TZ can be +/- and is in increments of 15 minutes (not hours)

    String theTime = "";

    theTime.concat(y / 10);
    theTime.concat(y % 10);
    theTime.concat('/');
    theTime.concat(mo / 10);
    theTime.concat(mo % 10);
    theTime.concat('/');
    theTime.concat(d / 10);
    theTime.concat(d % 10);
    theTime.concat(',');
    theTime.concat(h / 10);
    theTime.concat(h % 10);
    theTime.concat(':');
    theTime.concat(min / 10);
    theTime.concat(min % 10);
    theTime.concat(':');
    theTime.concat(s / 10);
    theTime.concat(s % 10);
    if (tz < 0)
    {
        theTime.concat('-');
        tz = 0 - tz;
    }
    else
        theTime.concat('+');
    theTime.concat(tz / 10);
    theTime.concat(tz % 10);

    return (setClock(theTime));
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setClock(String theTime)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_COMMAND_CLOCK) + theTime.length() + 8;
    char *command;

    command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    snprintf(command, cmdLen, "%s=\"%s\"", UBX_CELL_COMMAND_CLOCK, theTime.c_str());

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    free(command);
    return err;
}

void SparkFun_ublox_Cellular::autoTimeZoneForBegin(bool tz)
{
    _autoTimeZoneForBegin = tz;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::autoTimeZone(bool enable)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_COMMAND_AUTO_TZ) + 3;
    char command[cmdLen];

    snprintf(command, cmdLen, "%s=%d", UBX_CELL_COMMAND_AUTO_TZ, enable ? 1 : 0);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    return err;
}

int8_t SparkFun_ublox_Cellular::rssi(void)
{
    size_t cmdLen = strlen(UBX_CELL_SIGNAL_QUALITY) + 1;
    char command[cmdLen];
    char response[minimumResponseAllocation];
    UBX_CELL_error_t err;
    int rssi;

    snprintf(command, cmdLen, "%s", UBX_CELL_SIGNAL_QUALITY);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, 10000, minimumResponseAllocation,
                                  AT_COMMAND);
    if (err != UBX_CELL_ERROR_SUCCESS)
    {
        return -1;
    }

    int scanned = 0;
    char *searchPtr = strnstr(response, "+CSQ:", minimumResponseAllocation);
    if (searchPtr != nullptr)
    {
        searchPtr += strlen("+CSQ:"); //  Move searchPtr to first char
        while (*searchPtr == ' ')
            searchPtr++; // skip spaces
        scanned = sscanf(searchPtr, "%d,%*d", &rssi);
    }
    if (scanned != 1)
    {
        rssi = -1;
    }

    return rssi;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::getExtSignalQuality(signal_quality &signal_quality)
{
    size_t cmdLen = strlen(UBX_CELL_EXT_SIGNAL_QUALITY) + 1;
    char command[cmdLen];
    char response[minimumResponseAllocation];
    UBX_CELL_error_t err;

    snprintf(command, cmdLen, "%s", UBX_CELL_EXT_SIGNAL_QUALITY);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, 10000, minimumResponseAllocation,
                                  AT_COMMAND);
    if (err != UBX_CELL_ERROR_SUCCESS)
    {
        return UBX_CELL_ERROR_ERROR;
    }

    int scanned = 0;
    const char *responseStr = "+CESQ:";
    char *searchPtr = strnstr(response, responseStr, minimumResponseAllocation);
    if (searchPtr != nullptr)
    {
        searchPtr += strlen(responseStr); //  Move searchPtr to first char
        while (*searchPtr == ' ')
            searchPtr++; // skip spaces
        scanned = sscanf(searchPtr, "%u,%u,%u,%u,%u,%u", &signal_quality.rxlev, &signal_quality.ber,
                         &signal_quality.rscp, &signal_quality.enc0, &signal_quality.rsrq, &signal_quality.rsrp);
    }

    err = UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
    if (scanned == 6)
    {
        err = UBX_CELL_ERROR_SUCCESS;
    }

    return err;
}

UBX_CELL_registration_status_t SparkFun_ublox_Cellular::registration(bool eps)
{
    const char *tag = eps ? UBX_CELL_EPSREGISTRATION_STATUS : UBX_CELL_REGISTRATION_STATUS;
    size_t cmdLen = strlen(UBX_CELL_EPSREGISTRATION_STATUS) + 3;
    char command[cmdLen];
    char response[minimumResponseAllocation];
    UBX_CELL_error_t err;
    int status;

    snprintf(command, cmdLen, "%s?", tag);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, UBX_CELL_STANDARD_RESPONSE_TIMEOUT,
                                  minimumResponseAllocation, AT_COMMAND);
    if (err != UBX_CELL_ERROR_SUCCESS)
    {
        return UBX_CELL_REGISTRATION_INVALID;
    }

    int scanned = 0;
    const char *startTag = eps ? UBX_CELL_EPSREGISTRATION_STATUS_URC : UBX_CELL_REGISTRATION_STATUS_URC;
    char *searchPtr = strnstr(response, startTag, minimumResponseAllocation);
    if (searchPtr != nullptr)
    {
        searchPtr += eps ? strlen(UBX_CELL_EPSREGISTRATION_STATUS_URC)
                         : strlen(UBX_CELL_REGISTRATION_STATUS_URC); //  Move searchPtr to first char
        while (*searchPtr == ' ')
            searchPtr++; // skip spaces
        scanned = sscanf(searchPtr, "%*d,%d", &status);
    }
    if (scanned != 1)
        status = UBX_CELL_REGISTRATION_INVALID;

    return (UBX_CELL_registration_status_t)status;
}

bool SparkFun_ublox_Cellular::setNetworkProfile(mobile_network_operator_t mno, bool autoReset, bool urcNotification)
{
    mobile_network_operator_t currentMno;

    // Check currently set MNO profile
    if (getMNOprofile(&currentMno) != UBX_CELL_ERROR_SUCCESS)
    {
        return false;
    }

    if (currentMno == mno)
    {
        return true;
    }

    // Disable transmit and receive so we can change operator
    if (functionality(MINIMUM_FUNCTIONALITY) != UBX_CELL_ERROR_SUCCESS)
    {
        return false;
    }

    if (setMNOprofile(mno, autoReset, urcNotification) != UBX_CELL_ERROR_SUCCESS)
    {
        return false;
    }

    if (reset() != UBX_CELL_ERROR_SUCCESS)
    {
        return false;
    }

    return true;
}

mobile_network_operator_t SparkFun_ublox_Cellular::getNetworkProfile(void)
{
    mobile_network_operator_t mno;
    UBX_CELL_error_t err;

    err = getMNOprofile(&mno);
    if (err != UBX_CELL_ERROR_SUCCESS)
    {
        return MNO_INVALID;
    }
    return mno;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setAPN(String apn, uint8_t cid, UBX_CELL_pdp_type pdpType)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_MESSAGE_PDP_DEF) + strlen(apn.c_str()) + 16;
    char *command;
    char pdpStr[8];

    memset(pdpStr, 0, 8);

    if (cid >= 8)
        return UBX_CELL_ERROR_UNEXPECTED_PARAM;

    command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    switch (pdpType)
    {
    case PDP_TYPE_INVALID:
        free(command);
        return UBX_CELL_ERROR_UNEXPECTED_PARAM;
        break;
    case PDP_TYPE_IP:
        memcpy(pdpStr, "IP", 2);
        break;
    case PDP_TYPE_NONIP:
        memcpy(pdpStr, "NONIP", 5);
        break;
    case PDP_TYPE_IPV4V6:
        memcpy(pdpStr, "IPV4V6", 6);
        break;
    case PDP_TYPE_IPV6:
        memcpy(pdpStr, "IPV6", 4);
        break;
    default:
        free(command);
        return UBX_CELL_ERROR_UNEXPECTED_PARAM;
        break;
    }
    if (apn == nullptr)
    {
        if (_printDebug == true)
            _debugPort->println(F("setAPN: nullptr"));
        snprintf(command, cmdLen, "%s=%d,\"%s\",\"\"", UBX_CELL_MESSAGE_PDP_DEF, cid, pdpStr);
    }
    else
    {
        if (_printDebug == true)
        {
            _debugPort->print(F("setAPN: "));
            _debugPort->println(apn);
        }
        snprintf(command, cmdLen, "%s=%d,\"%s\",\"%s\"", UBX_CELL_MESSAGE_PDP_DEF, cid, pdpStr, apn.c_str());
    }

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    free(command);

    return err;
}

// Return the Access Point Name and IP address for the chosen context identifier
UBX_CELL_error_t SparkFun_ublox_Cellular::getAPN(int cid, String *apn, IPAddress *ip, UBX_CELL_pdp_type *pdpType)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_MESSAGE_PDP_DEF) + 3;
    char command[cmdLen];
    char response[1024];

    if (cid > UBX_CELL_NUM_PDP_CONTEXT_IDENTIFIERS)
        return UBX_CELL_ERROR_ERROR;

    snprintf(command, cmdLen, "%s?", UBX_CELL_MESSAGE_PDP_DEF);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, UBX_CELL_STANDARD_RESPONSE_TIMEOUT,
                                  1024);

    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        // Example:
        // +CGDCONT: 0,"IP","payandgo.o2.co.uk","0.0.0.0",0,0,0,0,0,0,0,0,0,0
        // +CGDCONT: 1,"IP","payandgo.o2.co.uk.mnc010.mcc234.gprs","10.160.182.234",0,0,0,2,0,0,0,0,0,0
        int rcid = -1;
        char *searchPtr = response;

        bool keepGoing = true;
        while (keepGoing == true)
        {
            int scanned = 0;
            // Find the first/next occurrence of +CGDCONT:
            searchPtr = strnstr(searchPtr, "+CGDCONT:", 1024 - (searchPtr - response));
            if (searchPtr != nullptr)
            {
                char strPdpType[10];
                char strApn[128];
                int ipOct[4];

                searchPtr += strlen("+CGDCONT:"); // Point to the cid
                while (*searchPtr == ' ')
                    searchPtr++; // skip spaces
                scanned = sscanf(searchPtr, "%d,\"%[^\"]\",\"%[^\"]\",\"%d.%d.%d.%d", &rcid, strPdpType, strApn,
                                 &ipOct[0], &ipOct[1], &ipOct[2], &ipOct[3]);
                if ((scanned == 7) && (rcid == cid))
                {
                    if (apn)
                        *apn = strApn;
                    for (int o = 0; ip && (o < 4); o++)
                    {
                        (*ip)[o] = (uint8_t)ipOct[o];
                    }
                    if (pdpType)
                    {
                        *pdpType = (0 == strcmp(strPdpType, "IPV4V6")) ? PDP_TYPE_IPV4V6
                                   : (0 == strcmp(strPdpType, "IPV6")) ? PDP_TYPE_IPV6
                                   : (0 == strcmp(strPdpType, "IP"))   ? PDP_TYPE_IP
                                                                       : PDP_TYPE_INVALID;
                    }
                    keepGoing = false;
                }
            }
            else // We don't have a match so let's clear the APN and IP address
            {
                if (apn)
                    *apn = "";
                if (pdpType)
                    *pdpType = PDP_TYPE_INVALID;
                if (ip)
                    *ip = {0, 0, 0, 0};
                keepGoing = false;
            }
        }
    }
    else
    {
        err = UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
    }

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::getSimStatus(String *code)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_COMMAND_SIMPIN) + 2;
    char command[cmdLen];
    char response[minimumResponseAllocation];

    snprintf(command, cmdLen, "%s?", UBX_CELL_COMMAND_SIMPIN);
    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        int scanned = 0;
        char c[16];
        char *searchPtr = strnstr(response, "+CPIN:", minimumResponseAllocation);
        if (searchPtr != nullptr)
        {
            searchPtr += strlen("+CPIN:"); //  Move searchPtr to first char
            while (*searchPtr == ' ')
                searchPtr++; // skip spaces
            scanned = sscanf(searchPtr, "%15s\r\n", c);
        }
        if (scanned == 1)
        {
            if (code)
                *code = c;
        }
        else
            err = UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
    }

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setSimPin(String pin)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_COMMAND_SIMPIN) + 4 + pin.length();
    char *command;

    command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    snprintf(command, cmdLen, "%s=\"%s\"", UBX_CELL_COMMAND_SIMPIN, pin.c_str());
    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    free(command);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setSIMstateReportingMode(int mode)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_SIM_STATE) + 4;
    char command[cmdLen];

    snprintf(command, cmdLen, "%s=%d", UBX_CELL_SIM_STATE, mode);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::getSIMstateReportingMode(int *mode)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_SIM_STATE) + 3;
    char command[cmdLen];
    char response[minimumResponseAllocation];

    int m;

    snprintf(command, cmdLen, "%s?", UBX_CELL_SIM_STATE);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        int scanned = 0;
        char *searchPtr = strnstr(response, "+USIMSTAT:", minimumResponseAllocation);
        if (searchPtr != nullptr)
        {
            searchPtr += strlen("+USIMSTAT:"); //  Move searchPtr to first char
            while (*searchPtr == ' ')
                searchPtr++; // skip spaces
            scanned = sscanf(searchPtr, "%d\r\n", &m);
        }
        if (scanned == 1)
        {
            *mode = m;
        }
        else
            err = UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
    }

    return err;
}

const char *PPP_L2P[5] = {
    "", "PPP", "M-HEX", "M-RAW_IP", "M-OPT-PPP",
};

UBX_CELL_error_t SparkFun_ublox_Cellular::enterPPP(uint8_t cid, char dialing_type_char, unsigned long dialNumber,
                                    SparkFun_ublox_Cellular::UBX_CELL_l2p_t l2p)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_MESSAGE_ENTER_PPP) + 32;
    char command[cmdLen];

    if ((dialing_type_char != 0) && (dialing_type_char != 'T') && (dialing_type_char != 'P'))
    {
        return UBX_CELL_ERROR_UNEXPECTED_PARAM;
    }

    if (dialing_type_char != 0)
    {
        snprintf(command, cmdLen, "%s%c*%lu**%s*%u#", UBX_CELL_MESSAGE_ENTER_PPP, dialing_type_char, dialNumber,
                 PPP_L2P[l2p], (unsigned int)cid);
    }
    else
    {
        snprintf(command, cmdLen, "%s*%lu**%s*%u#", UBX_CELL_MESSAGE_ENTER_PPP, dialNumber, PPP_L2P[l2p],
                 (unsigned int)cid);
    }

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_CONNECT, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    return err;
}

uint8_t SparkFun_ublox_Cellular::getOperators(struct operator_stats *opRet, int maxOps)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_OPERATOR_SELECTION) + 3;
    char command[cmdLen];
    char *response;
    uint8_t opsSeen = 0;

    snprintf(command, cmdLen, "%s=?", UBX_CELL_OPERATOR_SELECTION);

    int responseSize = (maxOps + 1) * 48;
    response = ubx_cell_calloc_char(responseSize);
    if (response == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;

    // AT+COPS maximum response time is 3 minutes (180000 ms)
    err =
        sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, UBX_CELL_3_MIN_TIMEOUT, responseSize);

    // Sample responses:
    // +COPS: (3,"Verizon Wireless","VzW","311480",8),,(0,1,2,3,4),(0,1,2)
    // +COPS: (1,"313 100","313 100","313100",8),(2,"AT&T","AT&T","310410",8),(3,"311 480","311
    // 480","311480",8),,(0,1,2,3,4),(0,1,2)

    if (_printDebug == true)
    {
        _debugPort->print(F("getOperators: Response: {"));
        _debugPort->print(response);
        _debugPort->println(F("}"));
    }

    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        char *opBegin;
        char *opEnd;
        int op = 0;
        int stat;
        char longOp[26];
        char shortOp[11];
        int act;
        unsigned long numOp;

        opBegin = response;

        for (; op < maxOps; op++)
        {
            opBegin = strchr(opBegin, '(');
            if (opBegin == nullptr)
                break;
            opEnd = strchr(opBegin, ')');
            if (opEnd == nullptr)
                break;

            int sscanRead =
                sscanf(opBegin, "(%d,\"%[^\"]\",\"%[^\"]\",\"%lu\",%d)%*s", &stat, longOp, shortOp, &numOp, &act);
            if (sscanRead == 5)
            {
                opRet[op].stat = stat;
                opRet[op].longOp = (String)(longOp);
                opRet[op].shortOp = (String)(shortOp);
                opRet[op].numOp = numOp;
                opRet[op].act = act;
                opsSeen += 1;
            }
            // TODO: Search for other possible patterns here
            else
            {
                break; // Break out if pattern doesn't match.
            }
            opBegin = opEnd + 1; // Move opBegin to beginning of next value
        }
    }

    free(response);

    return opsSeen;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::registerOperator(struct operator_stats oper)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_OPERATOR_SELECTION) + 24;
    char command[cmdLen];

    snprintf(command, cmdLen, "%s=1,2,\"%lu\"", UBX_CELL_OPERATOR_SELECTION, oper.numOp);

    // AT+COPS maximum response time is 3 minutes (180000 ms)
    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_3_MIN_TIMEOUT);

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::automaticOperatorSelection()
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_OPERATOR_SELECTION) + 6;
    char command[cmdLen];

    snprintf(command, cmdLen, "%s=0,0", UBX_CELL_OPERATOR_SELECTION);

    // AT+COPS maximum response time is 3 minutes (180000 ms)
    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_3_MIN_TIMEOUT);

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::getOperator(String *oper)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_OPERATOR_SELECTION) + 3;
    char command[cmdLen];
    char response[minimumResponseAllocation];
    char *searchPtr;
    char mode;

    snprintf(command, cmdLen, "%s?", UBX_CELL_OPERATOR_SELECTION);

    // AT+COPS maximum response time is 3 minutes (180000 ms)
    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, UBX_CELL_3_MIN_TIMEOUT);

    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        searchPtr = strnstr(response, "+COPS:", minimumResponseAllocation);
        if (searchPtr != nullptr)
        {
            searchPtr += strlen("+COPS:"); //  Move searchPtr to first char
            while (*searchPtr == ' ')
                searchPtr++;   // skip spaces
            mode = *searchPtr; // Read first char -- should be mode
            if (mode == '2')   // Check for de-register
            {
                err = UBX_CELL_ERROR_DEREGISTERED;
            }
            // Otherwise if it's default, manual, set-only, or automatic
            else if ((mode == '0') || (mode == '1') || (mode == '3') || (mode == '4'))
            {
                *oper = "";
                searchPtr = strchr(searchPtr, '\"'); // Move to first quote
                if (searchPtr == nullptr)
                {
                    err = UBX_CELL_ERROR_DEREGISTERED;
                }
                else
                {
                    while ((*(++searchPtr) != '\"') && (*searchPtr != '\0'))
                    {
                        oper->concat(*(searchPtr));
                    }
                }
                if (_printDebug == true)
                {
                    _debugPort->print(F("getOperator: "));
                    _debugPort->println(*oper);
                }
            }
        }
    }

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::deregisterOperator(void)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_OPERATOR_SELECTION) + 6;
    char command[cmdLen];

    snprintf(command, cmdLen, "%s=2", UBX_CELL_OPERATOR_SELECTION);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_3_MIN_TIMEOUT);

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setSMSMessageFormat(UBX_CELL_message_format_t textMode)
{
    size_t cmdLen = strlen(UBX_CELL_MESSAGE_FORMAT) + 4;
    char command[cmdLen];
    UBX_CELL_error_t err;

    snprintf(command, cmdLen, "%s=%d", UBX_CELL_MESSAGE_FORMAT, (textMode == UBX_CELL_MESSAGE_FORMAT_TEXT) ? 1 : 0);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::sendSMS(String number, String message)
{
    char *command;
    char *messageCStr;
    char *numberCStr;
    UBX_CELL_error_t err;

    numberCStr = ubx_cell_calloc_char(number.length() + 2);
    if (numberCStr == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    number.toCharArray(numberCStr, number.length() + 1);

    size_t cmdLen = strlen(UBX_CELL_SEND_TEXT) + strlen(numberCStr) + 8;
    command = ubx_cell_calloc_char(cmdLen);
    if (command != nullptr)
    {
        snprintf(command, cmdLen, "%s=\"%s\"", UBX_CELL_SEND_TEXT, numberCStr);

        err = sendCommandWithResponse(command, ">", nullptr, UBX_CELL_10_SEC_TIMEOUT);
        free(command);
        free(numberCStr);
        if (err != UBX_CELL_ERROR_SUCCESS)
            return err;

        messageCStr = ubx_cell_calloc_char(message.length() + 1);
        if (messageCStr == nullptr)
        {
            hwWrite(ASCII_CTRL_Z);
            return UBX_CELL_ERROR_OUT_OF_MEMORY;
        }
        message.toCharArray(messageCStr, message.length() + 1);
        messageCStr[message.length()] = ASCII_CTRL_Z;

        err = sendCommandWithResponse(messageCStr, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_10_SEC_TIMEOUT,
                                      minimumResponseAllocation, NOT_AT_COMMAND);

        free(messageCStr);
    }
    else
    {
        free(numberCStr);
        err = UBX_CELL_ERROR_OUT_OF_MEMORY;
    }

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::getPreferredMessageStorage(int *used, int *total, String memory)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_PREF_MESSAGE_STORE) + 32;
    char command[cmdLen];
    char response[minimumResponseAllocation];
    int u;
    int t;

    snprintf(command, cmdLen, "%s=\"%s\"", UBX_CELL_PREF_MESSAGE_STORE, memory.c_str());

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, UBX_CELL_3_MIN_TIMEOUT);

    if (err != UBX_CELL_ERROR_SUCCESS)
    {
        return err;
    }

    int scanned = 0;
    char *searchPtr = strnstr(response, "+CPMS:", minimumResponseAllocation);
    if (searchPtr != nullptr)
    {
        searchPtr += strlen("+CPMS:"); //  Move searchPtr to first char
        while (*searchPtr == ' ')
            searchPtr++; // skip spaces
        scanned = sscanf(searchPtr, "%d,%d", &u, &t);
    }
    if (scanned == 2)
    {
        if (_printDebug == true)
        {
            _debugPort->print(F("getPreferredMessageStorage: memory1 (read and delete): "));
            _debugPort->print(memory);
            _debugPort->print(F(" used: "));
            _debugPort->print(u);
            _debugPort->print(F(" total: "));
            _debugPort->println(t);
        }
        *used = u;
        *total = t;
    }
    else
    {
        err = UBX_CELL_ERROR_INVALID;
    }

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::readSMSmessage(int location, String *unread, String *from, String *dateTime, String *message)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_READ_TEXT_MESSAGE) + 5;
    char command[cmdLen];
    char response[1024];

    snprintf(command, cmdLen, "%s=%d", UBX_CELL_READ_TEXT_MESSAGE, location);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, UBX_CELL_10_SEC_TIMEOUT, 1024);

    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        char *searchPtr = response;

        // Find the first occurrence of +CMGR:
        searchPtr = strnstr(searchPtr, "+CMGR:", 1024 - (searchPtr - response));
        if (searchPtr != nullptr)
        {
            searchPtr += strlen("+CMGR:"); //  Move searchPtr to first char
            while (*searchPtr == ' ')
                searchPtr++; // skip spaces
            int pointer = 0;
            while ((*(++searchPtr) != '\"') && (*searchPtr != '\0') && (pointer < 12))
            {
                unread->concat(*(searchPtr));
                pointer++;
            }
            if ((*searchPtr == '\0') || (pointer == 12))
            {
                return UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
            }
            // Search to the next quote
            searchPtr = strchr(++searchPtr, '\"');
            pointer = 0;
            while ((*(++searchPtr) != '\"') && (*searchPtr != '\0') && (pointer < 24))
            {
                from->concat(*(searchPtr));
                pointer++;
            }
            if ((*searchPtr == '\0') || (pointer == 24))
            {
                return UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
            }
            // Skip two commas
            searchPtr = strchr(++searchPtr, ',');
            searchPtr = strchr(++searchPtr, ',');
            // Search to the next quote
            searchPtr = strchr(++searchPtr, '\"');
            pointer = 0;
            while ((*(++searchPtr) != '\"') && (*searchPtr != '\0') && (pointer < 24))
            {
                dateTime->concat(*(searchPtr));
                pointer++;
            }
            if ((*searchPtr == '\0') || (pointer == 24))
            {
                return UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
            }
            // Search to the next new line
            searchPtr = strchr(++searchPtr, '\n');
            pointer = 0;
            while ((*(++searchPtr) != '\r') && (*searchPtr != '\n') && (*searchPtr != '\0') && (pointer < 512))
            {
                message->concat(*(searchPtr));
                pointer++;
            }
            if ((*searchPtr == '\0') || (pointer == 512))
            {
                return UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
            }
        }
        else
        {
            err = UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
        }
    }
    else
    {
        err = UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
    }

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::deleteSMSmessage(int location, int deleteFlag)
{
    size_t cmdLen = strlen(UBX_CELL_DELETE_MESSAGE) + 12;
    char command[cmdLen];
    UBX_CELL_error_t err;

    if (deleteFlag == 0)
        snprintf(command, cmdLen, "%s=%d", UBX_CELL_DELETE_MESSAGE, location);
    else
        snprintf(command, cmdLen, "%s=%d,%d", UBX_CELL_DELETE_MESSAGE, location, deleteFlag);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_55_SECS_TIMEOUT);

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setBaud(unsigned long baud)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_COMMAND_BAUD) + 7 + 12;
    char command[cmdLen];
    int b = 0;

    // Error check -- ensure supported baud
    for (; b < NUM_SUPPORTED_BAUD; b++)
    {
        if (UBX_CELL_SUPPORTED_BAUD[b] == baud)
        {
            break;
        }
    }
    if (b >= NUM_SUPPORTED_BAUD)
    {
        return UBX_CELL_ERROR_UNEXPECTED_PARAM;
    }

    // Construct command
    snprintf(command, cmdLen, "%s=%lu", UBX_CELL_COMMAND_BAUD, baud);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_SET_BAUD_TIMEOUT);

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setFlowControl(UBX_CELL_flow_control_t value)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_FLOW_CONTROL) + 16;
    char command[cmdLen];

    snprintf(command, cmdLen, "%s%d", UBX_CELL_FLOW_CONTROL, value);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setGpioMode(UBX_CELL_gpio_t gpio, UBX_CELL_gpio_mode_t mode, int value)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_COMMAND_GPIO) + 16;
    char command[cmdLen];

    // Example command: AT+UGPIOC=16,2
    // Example command: AT+UGPIOC=23,0,1
    if (mode == GPIO_OUTPUT)
        snprintf(command, cmdLen, "%s=%d,%d,%d", UBX_CELL_COMMAND_GPIO, gpio, mode, value);
    else
        snprintf(command, cmdLen, "%s=%d,%d", UBX_CELL_COMMAND_GPIO, gpio, mode);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_10_SEC_TIMEOUT);

    return err;
}

SparkFun_ublox_Cellular::UBX_CELL_gpio_mode_t SparkFun_ublox_Cellular::getGpioMode(UBX_CELL_gpio_t gpio)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_COMMAND_GPIO) + 2;
    char command[cmdLen];
    char response[minimumResponseAllocation];
    size_t charLen = 4;
    char gpioChar[charLen];
    char *gpioStart;
    int gpioMode;

    snprintf(command, cmdLen, "%s?", UBX_CELL_COMMAND_GPIO);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    if (err != UBX_CELL_ERROR_SUCCESS)
    {
        return GPIO_MODE_INVALID;
    }

    snprintf(gpioChar, charLen, "%d", gpio);                            // Convert GPIO to char array
    gpioStart = strnstr(response, gpioChar, minimumResponseAllocation); // Find first occurence of GPIO in response

    if (gpioStart == nullptr)
        return GPIO_MODE_INVALID; // If not found return invalid
    sscanf(gpioStart, "%*d,%d\r\n", &gpioMode);

    return (UBX_CELL_gpio_mode_t)gpioMode;
}

int SparkFun_ublox_Cellular::socketOpen(UBX_CELL_socket_protocol_t protocol, unsigned int localPort)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_CREATE_SOCKET) + 10;
    char command[cmdLen];
    char response[minimumResponseAllocation];
    int sockId = -1;
    char *responseStart;

    if (localPort == 0)
        snprintf(command, cmdLen, "%s=%d", UBX_CELL_CREATE_SOCKET, (int)protocol);
    else
        snprintf(command, cmdLen, "%s=%d,%d", UBX_CELL_CREATE_SOCKET, (int)protocol, localPort);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    if (err != UBX_CELL_ERROR_SUCCESS)
    {
        if (_printDebug == true)
        {
            _debugPort->print(F("socketOpen: Fail: Error: "));
            _debugPort->print(err);
            _debugPort->print(F("  Response: {"));
            _debugPort->print(response);
            _debugPort->println(F("}"));
        }
        return -1;
    }

    responseStart = strnstr(response, "+USOCR:", minimumResponseAllocation);
    if (responseStart == nullptr)
    {
        if (_printDebug == true)
        {
            _debugPort->print(F("socketOpen: Failure: {"));
            _debugPort->print(response);
            _debugPort->println(F("}"));
        }
        return -1;
    }

    responseStart += strlen("+USOCR:"); //  Move searchPtr to first char
    while (*responseStart == ' ')
        responseStart++; // skip spaces
    sscanf(responseStart, "%d", &sockId);
    _lastSocketProtocol[sockId] = (int)protocol;

    return sockId;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::socketClose(int socket, unsigned long timeout)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_CLOSE_SOCKET) + 10;
    char command[cmdLen];
    char response[minimumResponseAllocation];

    // if timeout is short, close asynchronously and don't wait for socket closure (we will get the URC later)
    // this will make sure the AT command parser is not confused during init()
    const char *format = (UBX_CELL_STANDARD_RESPONSE_TIMEOUT == timeout) ? "%s=%d,1" : "%s=%d";
    snprintf(command, cmdLen, format, UBX_CELL_CLOSE_SOCKET, socket);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, timeout);

    if ((err != UBX_CELL_ERROR_SUCCESS) && (_printDebug == true))
    {
        _debugPort->print(F("socketClose: Error: "));
        _debugPort->println(socketGetLastError());
    }

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::socketConnect(int socket, const char *address, unsigned int port)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_CONNECT_SOCKET) + strlen(address) + 11;
    char *command;

    command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    snprintf(command, cmdLen, "%s=%d,\"%s\",%d", UBX_CELL_CONNECT_SOCKET, socket, address, port);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_IP_CONNECT_TIMEOUT);

    free(command);

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::socketConnect(int socket, IPAddress address, unsigned int port)
{
    size_t charLen = 16;
    char charAddress[charLen];
    memset(charAddress, 0, 16);
    snprintf(charAddress, charLen, "%d.%d.%d.%d", address[0], address[1], address[2], address[3]);

    return (socketConnect(socket, (const char *)charAddress, port));
}

UBX_CELL_error_t SparkFun_ublox_Cellular::socketWrite(int socket, const char *str, int len)
{
    size_t cmdLen = strlen(UBX_CELL_WRITE_SOCKET) + 16;
    char command[cmdLen];
    char response[minimumResponseAllocation];
    UBX_CELL_error_t err;

    int dataLen = len == -1 ? strlen(str) : len;
    snprintf(command, cmdLen, "%s=%d,%d", UBX_CELL_WRITE_SOCKET, socket, dataLen);

    err = sendCommandWithResponse(command, "@", response, UBX_CELL_STANDARD_RESPONSE_TIMEOUT * 5);

    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        unsigned long writeDelay = millis();
        while (millis() < (writeDelay + 50))
            delay(1); // u-blox specification says to wait 50ms after receiving "@" to write data.

        if (len == -1)
        {
            if (_printDebug == true)
            {
                _debugPort->print(F("socketWrite: writing: "));
                _debugPort->println(str);
            }
            hwPrint(str);
        }
        else
        {
            if (_printDebug == true)
            {
                _debugPort->print(F("socketWrite: writing "));
                _debugPort->print(len);
                _debugPort->println(F(" bytes"));
            }
            hwWriteData(str, len);
        }

        err = waitForResponse(UBX_CELL_RESPONSE_OK, UBX_CELL_RESPONSE_ERROR, UBX_CELL_SOCKET_WRITE_TIMEOUT);
    }

    if (err != UBX_CELL_ERROR_SUCCESS)
    {
        if (_printDebug == true)
        {
            _debugPort->print(F("socketWrite: Error: "));
            _debugPort->print(err);
            _debugPort->print(F(" => {"));
            _debugPort->print(response);
            _debugPort->println(F("}"));
        }
    }

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::socketWrite(int socket, String str)
{
    return socketWrite(socket, str.c_str(), str.length());
}

UBX_CELL_error_t SparkFun_ublox_Cellular::socketWriteUDP(int socket, const char *address, int port, const char *str, int len)
{
    size_t cmdLen = 64;
    char command[cmdLen];
    char response[minimumResponseAllocation];
    UBX_CELL_error_t err;
    int dataLen = len == -1 ? strlen(str) : len;

    snprintf(command, cmdLen, "%s=%d,\"%s\",%d,%d", UBX_CELL_WRITE_UDP_SOCKET, socket, address, port, dataLen);
    err = sendCommandWithResponse(command, "@", response, UBX_CELL_STANDARD_RESPONSE_TIMEOUT * 5);

    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        if (len == -1) // If binary data we need to send a length.
        {
            hwPrint(str);
        }
        else
        {
            hwWriteData(str, len);
        }
        err = waitForResponse(UBX_CELL_RESPONSE_OK, UBX_CELL_RESPONSE_ERROR, UBX_CELL_SOCKET_WRITE_TIMEOUT);
    }
    else
    {
        if (_printDebug == true)
            _debugPort->print(F("socketWriteUDP: Error: "));
        if (_printDebug == true)
            _debugPort->println(socketGetLastError());
    }

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::socketWriteUDP(int socket, IPAddress address, int port, const char *str, int len)
{
    size_t charLen = 16;
    char charAddress[16];
    memset(charAddress, 0, 16);
    snprintf(charAddress, charLen, "%d.%d.%d.%d", address[0], address[1], address[2], address[3]);

    return (socketWriteUDP(socket, (const char *)charAddress, port, str, len));
}

UBX_CELL_error_t SparkFun_ublox_Cellular::socketWriteUDP(int socket, String address, int port, String str)
{
    return socketWriteUDP(socket, address.c_str(), port, str.c_str(), str.length());
}

UBX_CELL_error_t SparkFun_ublox_Cellular::socketRead(int socket, int length, char *readDest, int *bytesRead)
{
    size_t cmdLen = strlen(UBX_CELL_READ_SOCKET) + 32;
    char command[cmdLen];
    // We only need enough to read _saraR5maxSocketRead bytes - not the whole thing
    int responseLength = _saraR5maxSocketRead + strlen(UBX_CELL_READ_SOCKET) + minimumResponseAllocation;
    char response[responseLength];
    char *strBegin;
    int readIndexTotal = 0;
    int readIndexThisRead = 0;
    UBX_CELL_error_t err;
    int scanNum = 0;
    int readLength = 0;
    int socketStore = 0;
    int bytesLeftToRead = length;
    int bytesToRead;

    // Set *bytesRead to zero
    if (bytesRead != nullptr)
        *bytesRead = 0;

    // Check if length is zero
    if (length == 0)
    {
        if (_printDebug == true)
            _debugPort->print(F("socketRead: length is 0! Call socketReadAvailable?"));
        return UBX_CELL_ERROR_UNEXPECTED_PARAM;
    }

    // If there are more than _saraR5maxSocketRead (1024) bytes to be read,
    // we need to do multiple reads to get all the data

    while (bytesLeftToRead > 0)
    {
        if (bytesLeftToRead > _saraR5maxSocketRead) // Limit a single read to _saraR5maxSocketRead
            bytesToRead = _saraR5maxSocketRead;
        else
            bytesToRead = bytesLeftToRead;

        snprintf(command, cmdLen, "%s=%d,%d", UBX_CELL_READ_SOCKET, socket, bytesToRead);

        err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response,
                                      UBX_CELL_STANDARD_RESPONSE_TIMEOUT, responseLength);

        if (err != UBX_CELL_ERROR_SUCCESS)
        {
            if (_printDebug == true)
            {
                _debugPort->print(F("socketRead: sendCommandWithResponse err "));
                _debugPort->println(err);
            }
            return err;
        }

        // Extract the data
        char *searchPtr = strnstr(response, "+USORD:", responseLength);
        if (searchPtr != nullptr)
        {
            searchPtr += strlen("+USORD:"); //  Move searchPtr to first char
            while (*searchPtr == ' ')
                searchPtr++; // skip spaces
            scanNum = sscanf(searchPtr, "%d,%d", &socketStore, &readLength);
        }
        if (scanNum != 2)
        {
            if (_printDebug == true)
            {
                _debugPort->print(F("socketRead: error: scanNum is "));
                _debugPort->println(scanNum);
            }
            return UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
        }

        // Check that readLength == bytesToRead
        if (readLength != bytesToRead)
        {
            if (_printDebug == true)
            {
                _debugPort->print(F("socketRead: length mismatch! bytesToRead="));
                _debugPort->print(bytesToRead);
                _debugPort->print(F(" readLength="));
                _debugPort->println(readLength);
            }
        }

        // Check that readLength > 0
        if (readLength == 0)
        {
            if (_printDebug == true)
            {
                _debugPort->println(F("socketRead: zero length!"));
            }
            return UBX_CELL_ERROR_ZERO_READ_LENGTH;
        }

        // Find the first double-quote:
        strBegin = strchr(searchPtr, '\"');

        if (strBegin == nullptr)
        {
            return UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
        }

        // Now copy the data into readDest
        readIndexThisRead = 1; // Start after the quote
        while (readIndexThisRead < (readLength + 1))
        {
            readDest[readIndexTotal] = strBegin[readIndexThisRead];
            readIndexTotal++;
            readIndexThisRead++;
        }

        if (_printDebug == true)
            _debugPort->println(F("socketRead: success"));

        // Update *bytesRead
        if (bytesRead != nullptr)
            *bytesRead = readIndexTotal;

        // How many bytes are left to read?
        // This would have been bytesLeftToRead -= bytesToRead
        // Except the SARA can potentially return less data than requested...
        // So we need to subtract readLength instead.
        bytesLeftToRead -= readLength;
        if (_printDebug == true)
        {
            if (bytesLeftToRead > 0)
            {
                _debugPort->print(F("socketRead: multiple read. bytesLeftToRead: "));
                _debugPort->println(bytesLeftToRead);
            }
        }
    } // /while (bytesLeftToRead > 0)

    return UBX_CELL_ERROR_SUCCESS;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::socketReadAvailable(int socket, int *length)
{
    size_t cmdLen = strlen(UBX_CELL_READ_SOCKET) + 32;
    char command[cmdLen];
    char response[minimumResponseAllocation];
    UBX_CELL_error_t err;
    int scanNum = 0;
    int readLength = 0;
    int socketStore = 0;

    snprintf(command, cmdLen, "%s=%d,0", UBX_CELL_READ_SOCKET, socket);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        char *searchPtr = strnstr(response, "+USORD:", minimumResponseAllocation);
        if (searchPtr != nullptr)
        {
            searchPtr += strlen("+USORD:"); //  Move searchPtr to first char
            while (*searchPtr == ' ')
                searchPtr++; // skip spaces
            scanNum = sscanf(searchPtr, "%d,%d", &socketStore, &readLength);
        }
        if (scanNum != 2)
        {
            if (_printDebug == true)
            {
                _debugPort->print(F("socketReadAvailable: error: scanNum is "));
                _debugPort->println(scanNum);
            }
            return UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
        }

        *length = readLength;
    }

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::socketReadUDP(int socket, int length, char *readDest, IPAddress *remoteIPAddress,
                                         int *remotePort, int *bytesRead)
{
    size_t cmdLen = strlen(UBX_CELL_READ_UDP_SOCKET) + 32;
    char command[cmdLen];
    // We only need enough to read _saraR5maxSocketRead bytes - not the whole thing
    int responseLength = _saraR5maxSocketRead + strlen(UBX_CELL_READ_UDP_SOCKET) + minimumResponseAllocation;
    char response[responseLength];
    char *strBegin;
    int readIndexTotal = 0;
    int readIndexThisRead = 0;
    UBX_CELL_error_t err;
    int scanNum = 0;
    int remoteIPstore[4] = {0, 0, 0, 0};
    int portStore = 0;
    int readLength = 0;
    int socketStore = 0;
    int bytesLeftToRead = length;
    int bytesToRead;

    // Set *bytesRead to zero
    if (bytesRead != nullptr)
        *bytesRead = 0;

    // Check if length is zero
    if (length == 0)
    {
        if (_printDebug == true)
            _debugPort->print(F("socketReadUDP: length is 0! Call socketReadAvailableUDP?"));
        return UBX_CELL_ERROR_UNEXPECTED_PARAM;
    }

    // If there are more than _saraR5maxSocketRead (1024) bytes to be read,
    // we need to do multiple reads to get all the data

    while (bytesLeftToRead > 0)
    {
        if (bytesLeftToRead > _saraR5maxSocketRead) // Limit a single read to _saraR5maxSocketRead
            bytesToRead = _saraR5maxSocketRead;
        else
            bytesToRead = bytesLeftToRead;

        snprintf(command, cmdLen, "%s=%d,%d", UBX_CELL_READ_UDP_SOCKET, socket, bytesToRead);

        err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response,
                                      UBX_CELL_STANDARD_RESPONSE_TIMEOUT, responseLength);

        if (err != UBX_CELL_ERROR_SUCCESS)
        {
            if (_printDebug == true)
            {
                _debugPort->print(F("socketReadUDP: sendCommandWithResponse err "));
                _debugPort->println(err);
            }
            return err;
        }

        // Extract the data
        char *searchPtr = strnstr(response, "+USORF:", responseLength);
        if (searchPtr != nullptr)
        {
            searchPtr += strlen("+USORF:"); //  Move searchPtr to first char
            while (*searchPtr == ' ')
                searchPtr++; // skip spaces
            scanNum = sscanf(searchPtr, "%d,\"%d.%d.%d.%d\",%d,%d", &socketStore, &remoteIPstore[0], &remoteIPstore[1],
                             &remoteIPstore[2], &remoteIPstore[3], &portStore, &readLength);
        }
        if (scanNum != 7)
        {
            if (_printDebug == true)
            {
                _debugPort->print(F("socketReadUDP: error: scanNum is "));
                _debugPort->println(scanNum);
            }
            return UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
        }

        // Check that readLength == bytesToRead
        if (readLength != bytesToRead)
        {
            if (_printDebug == true)
            {
                _debugPort->print(F("socketReadUDP: length mismatch! bytesToRead="));
                _debugPort->print(bytesToRead);
                _debugPort->print(F(" readLength="));
                _debugPort->println(readLength);
            }
        }

        // Check that readLength > 0
        if (readLength == 0)
        {
            if (_printDebug == true)
            {
                _debugPort->println(F("socketRead: zero length!"));
            }
            return UBX_CELL_ERROR_ZERO_READ_LENGTH;
        }

        // Find the third double-quote
        strBegin = strchr(searchPtr, '\"');
        strBegin = strchr(strBegin + 1, '\"');
        strBegin = strchr(strBegin + 1, '\"');

        if (strBegin == nullptr)
        {
            return UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
        }

        // Now copy the data into readDest
        readIndexThisRead = 1; // Start after the quote
        while (readIndexThisRead < (readLength + 1))
        {
            readDest[readIndexTotal] = strBegin[readIndexThisRead];
            readIndexTotal++;
            readIndexThisRead++;
        }

        // If remoteIPaddress is not nullptr, copy the remote IP address
        if (remoteIPAddress != nullptr)
        {
            IPAddress tempAddress;
            for (int i = 0; i <= 3; i++)
            {
                tempAddress[i] = (uint8_t)remoteIPstore[i];
            }
            *remoteIPAddress = tempAddress;
        }

        // If remotePort is not nullptr, copy the remote port
        if (remotePort != nullptr)
        {
            *remotePort = portStore;
        }

        if (_printDebug == true)
            _debugPort->println(F("socketReadUDP: success"));

        // Update *bytesRead
        if (bytesRead != nullptr)
            *bytesRead = readIndexTotal;

        // How many bytes are left to read?
        // This would have been bytesLeftToRead -= bytesToRead
        // Except the SARA can potentially return less data than requested...
        // So we need to subtract readLength instead.
        bytesLeftToRead -= readLength;
        if (_printDebug == true)
        {
            if (bytesLeftToRead > 0)
            {
                _debugPort->print(F("socketReadUDP: multiple read. bytesLeftToRead: "));
                _debugPort->println(bytesLeftToRead);
            }
        }
    } // /while (bytesLeftToRead > 0)

    return UBX_CELL_ERROR_SUCCESS;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::socketReadAvailableUDP(int socket, int *length)
{
    size_t cmdLen = strlen(UBX_CELL_READ_UDP_SOCKET) + 32;
    char command[cmdLen];
    char response[minimumResponseAllocation];
    UBX_CELL_error_t err;
    int scanNum = 0;
    int readLength = 0;
    int socketStore = 0;

    snprintf(command, cmdLen, "%s=%d,0", UBX_CELL_READ_UDP_SOCKET, socket);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        char *searchPtr = strnstr(response, "+USORF:", minimumResponseAllocation);
        if (searchPtr != nullptr)
        {
            searchPtr += strlen("+USORF:"); //  Move searchPtr to first char
            while (*searchPtr == ' ')
                searchPtr++; // skip spaces
            scanNum = sscanf(searchPtr, "%d,%d", &socketStore, &readLength);
        }
        if (scanNum != 2)
        {
            if (_printDebug == true)
            {
                _debugPort->print(F("socketReadAvailableUDP: error: scanNum is "));
                _debugPort->println(scanNum);
            }
            return UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
        }

        *length = readLength;
    }

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::socketListen(int socket, unsigned int port)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_LISTEN_SOCKET) + 9;
    char command[cmdLen];

    snprintf(command, cmdLen, "%s=%d,%d", UBX_CELL_LISTEN_SOCKET, socket, port);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::socketDirectLinkMode(int socket)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_SOCKET_DIRECT_LINK) + 8;
    char command[cmdLen];

    snprintf(command, cmdLen, "%s=%d", UBX_CELL_SOCKET_DIRECT_LINK, socket);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_CONNECT, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::socketDirectLinkTimeTrigger(int socket, unsigned long timerTrigger)
{
    // valid range is 0 (trigger disabled), 100-120000
    if (!((timerTrigger == 0) || ((timerTrigger >= 100) && (timerTrigger <= 120000))))
        return UBX_CELL_ERROR_ERROR;

    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_UD_CONFIGURATION) + 16;
    char command[cmdLen];

    snprintf(command, cmdLen, "%s=5,%d,%ld", UBX_CELL_UD_CONFIGURATION, socket, timerTrigger);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::socketDirectLinkDataLengthTrigger(int socket, int dataLengthTrigger)
{
    // valid range is 0, 3-1472 for UDP
    if (!((dataLengthTrigger == 0) || ((dataLengthTrigger >= 3) && (dataLengthTrigger <= 1472))))
        return UBX_CELL_ERROR_ERROR;

    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_UD_CONFIGURATION) + 16;
    char command[cmdLen];

    snprintf(command, cmdLen, "%s=6,%d,%d", UBX_CELL_UD_CONFIGURATION, socket, dataLengthTrigger);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::socketDirectLinkCharacterTrigger(int socket, int characterTrigger)
{
    // The allowed range is -1, 0-255, the factory-programmed value is -1; -1 means trigger disabled.
    if (!((characterTrigger >= -1) && (characterTrigger <= 255)))
        return UBX_CELL_ERROR_ERROR;

    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_UD_CONFIGURATION) + 16;
    char command[cmdLen];

    snprintf(command, cmdLen, "%s=7,%d,%d", UBX_CELL_UD_CONFIGURATION, socket, characterTrigger);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::socketDirectLinkCongestionTimer(int socket, unsigned long congestionTimer)
{
    // valid range is 0, 1000-72000
    if (!((congestionTimer == 0) || ((congestionTimer >= 1000) && (congestionTimer <= 72000))))
        return UBX_CELL_ERROR_ERROR;

    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_UD_CONFIGURATION) + 16;
    char command[cmdLen];

    snprintf(command, cmdLen, "%s=8,%d,%ld", UBX_CELL_UD_CONFIGURATION, socket, congestionTimer);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::querySocketType(int socket, UBX_CELL_socket_protocol_t *protocol)
{
    size_t cmdLen = strlen(UBX_CELL_SOCKET_CONTROL) + 16;
    char command[cmdLen];
    char response[minimumResponseAllocation];
    UBX_CELL_error_t err;
    int scanNum = 0;
    int socketStore = 0;
    int paramVal;

    snprintf(command, cmdLen, "%s=%d,0", UBX_CELL_SOCKET_CONTROL, socket);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        char *searchPtr = strnstr(response, "+USOCTL:", minimumResponseAllocation);
        if (searchPtr != nullptr)
        {
            searchPtr += strlen("+USOCTL:"); //  Move searchPtr to first char
            while (*searchPtr == ' ')
                searchPtr++; // skip spaces
            scanNum = sscanf(searchPtr, "%d,0,%d", &socketStore, &paramVal);
        }
        if (scanNum != 2)
        {
            if (_printDebug == true)
            {
                _debugPort->print(F("querySocketType: error: scanNum is "));
                _debugPort->println(scanNum);
            }
            return UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
        }

        *protocol = (UBX_CELL_socket_protocol_t)paramVal;
        _lastSocketProtocol[socketStore] = paramVal;
    }

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::querySocketLastError(int socket, int *error)
{
    size_t cmdLen = strlen(UBX_CELL_SOCKET_CONTROL) + 16;
    char command[cmdLen];
    char response[minimumResponseAllocation];
    UBX_CELL_error_t err;
    int scanNum = 0;
    int socketStore = 0;
    int paramVal;

    snprintf(command, cmdLen, "%s=%d,1", UBX_CELL_SOCKET_CONTROL, socket);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        char *searchPtr = strnstr(response, "+USOCTL:", minimumResponseAllocation);
        if (searchPtr != nullptr)
        {
            searchPtr += strlen("+USOCTL:"); //  Move searchPtr to first char
            while (*searchPtr == ' ')
                searchPtr++; // skip spaces
            scanNum = sscanf(searchPtr, "%d,1,%d", &socketStore, &paramVal);
        }
        if (scanNum != 2)
        {
            if (_printDebug == true)
            {
                _debugPort->print(F("querySocketLastError: error: scanNum is "));
                _debugPort->println(scanNum);
            }
            return UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
        }

        *error = paramVal;
    }

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::querySocketTotalBytesSent(int socket, uint32_t *total)
{
    size_t cmdLen = strlen(UBX_CELL_SOCKET_CONTROL) + 16;
    char command[cmdLen];
    char response[minimumResponseAllocation];
    UBX_CELL_error_t err;
    int scanNum = 0;
    int socketStore = 0;
    long unsigned int paramVal;

    snprintf(command, cmdLen, "%s=%d,2", UBX_CELL_SOCKET_CONTROL, socket);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        char *searchPtr = strnstr(response, "+USOCTL:", minimumResponseAllocation);
        if (searchPtr != nullptr)
        {
            searchPtr += strlen("+USOCTL:"); //  Move searchPtr to first char
            while (*searchPtr == ' ')
                searchPtr++; // skip spaces
            scanNum = sscanf(searchPtr, "%d,2,%lu", &socketStore, &paramVal);
        }
        if (scanNum != 2)
        {
            if (_printDebug == true)
            {
                _debugPort->print(F("querySocketTotalBytesSent: error: scanNum is "));
                _debugPort->println(scanNum);
            }
            return UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
        }

        *total = (uint32_t)paramVal;
    }

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::querySocketTotalBytesReceived(int socket, uint32_t *total)
{
    size_t cmdLen = strlen(UBX_CELL_SOCKET_CONTROL) + 16;
    char command[cmdLen];
    char response[minimumResponseAllocation];
    UBX_CELL_error_t err;
    int scanNum = 0;
    int socketStore = 0;
    long unsigned int paramVal;

    snprintf(command, cmdLen, "%s=%d,3", UBX_CELL_SOCKET_CONTROL, socket);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        char *searchPtr = strnstr(response, "+USOCTL:", minimumResponseAllocation);
        if (searchPtr != nullptr)
        {
            searchPtr += strlen("+USOCTL:"); //  Move searchPtr to first char
            while (*searchPtr == ' ')
                searchPtr++; // skip spaces
            scanNum = sscanf(searchPtr, "%d,3,%lu", &socketStore, &paramVal);
        }
        if (scanNum != 2)
        {
            if (_printDebug == true)
            {
                _debugPort->print(F("querySocketTotalBytesReceived: error: scanNum is "));
                _debugPort->println(scanNum);
            }
            return UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
        }

        *total = (uint32_t)paramVal;
    }

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::querySocketRemoteIPAddress(int socket, IPAddress *address, int *port)
{
    size_t cmdLen = strlen(UBX_CELL_SOCKET_CONTROL) + 16;
    char command[cmdLen];
    char response[minimumResponseAllocation];
    UBX_CELL_error_t err;
    int scanNum = 0;
    int socketStore = 0;
    int paramVals[5];

    snprintf(command, cmdLen, "%s=%d,4", UBX_CELL_SOCKET_CONTROL, socket);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        char *searchPtr = strnstr(response, "+USOCTL:", minimumResponseAllocation);
        if (searchPtr != nullptr)
        {
            searchPtr += strlen("+USOCTL:"); //  Move searchPtr to first char
            while (*searchPtr == ' ')
                searchPtr++; // skip spaces
            scanNum = sscanf(searchPtr, "%d,4,\"%d.%d.%d.%d\",%d", &socketStore, &paramVals[0], &paramVals[1],
                             &paramVals[2], &paramVals[3], &paramVals[4]);
        }
        if (scanNum != 6)
        {
            if (_printDebug == true)
            {
                _debugPort->print(F("querySocketRemoteIPAddress: error: scanNum is "));
                _debugPort->println(scanNum);
            }
            return UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
        }

        IPAddress tempAddress = {(uint8_t)paramVals[0], (uint8_t)paramVals[1], (uint8_t)paramVals[2],
                                 (uint8_t)paramVals[3]};
        *address = tempAddress;
        *port = paramVals[4];
    }

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::querySocketStatusTCP(int socket, UBX_CELL_tcp_socket_status_t *status)
{
    size_t cmdLen = strlen(UBX_CELL_SOCKET_CONTROL) + 16;
    char command[cmdLen];
    char response[minimumResponseAllocation];
    UBX_CELL_error_t err;
    int scanNum = 0;
    int socketStore = 0;
    int paramVal;

    snprintf(command, cmdLen, "%s=%d,10", UBX_CELL_SOCKET_CONTROL, socket);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        char *searchPtr = strnstr(response, "+USOCTL:", minimumResponseAllocation);
        if (searchPtr != nullptr)
        {
            searchPtr += strlen("+USOCTL:"); //  Move searchPtr to first char
            while (*searchPtr == ' ')
                searchPtr++; // skip spaces
            scanNum = sscanf(searchPtr, "%d,10,%d", &socketStore, &paramVal);
        }
        if (scanNum != 2)
        {
            if (_printDebug == true)
            {
                _debugPort->print(F("querySocketStatusTCP: error: scanNum is "));
                _debugPort->println(scanNum);
            }
            return UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
        }

        *status = (UBX_CELL_tcp_socket_status_t)paramVal;
    }

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::querySocketOutUnackData(int socket, uint32_t *total)
{
    size_t cmdLen = strlen(UBX_CELL_SOCKET_CONTROL) + 16;
    char command[cmdLen];
    char response[minimumResponseAllocation];
    UBX_CELL_error_t err;
    int scanNum = 0;
    int socketStore = 0;
    long unsigned int paramVal;

    snprintf(command, cmdLen, "%s=%d,11", UBX_CELL_SOCKET_CONTROL, socket);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        char *searchPtr = strnstr(response, "+USOCTL:", minimumResponseAllocation);
        if (searchPtr != nullptr)
        {
            searchPtr += strlen("+USOCTL:"); //  Move searchPtr to first char
            while (*searchPtr == ' ')
                searchPtr++; // skip spaces
            scanNum = sscanf(searchPtr, "%d,11,%lu", &socketStore, &paramVal);
        }
        if (scanNum != 2)
        {
            if (_printDebug == true)
            {
                _debugPort->print(F("querySocketOutUnackData: error: scanNum is "));
                _debugPort->println(scanNum);
            }
            return UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
        }

        *total = (uint32_t)paramVal;
    }

    return err;
}

// Issues command to get last socket error, then prints to serial. Also updates rx/backlog buffers.
int SparkFun_ublox_Cellular::socketGetLastError()
{
    UBX_CELL_error_t err;
    size_t cmdLen = 64;
    char command[cmdLen];
    char response[minimumResponseAllocation];
    int errorCode = -1;

    snprintf(command, cmdLen, "%s", UBX_CELL_GET_ERROR);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        char *searchPtr = strnstr(response, "+USOER:", minimumResponseAllocation);
        if (searchPtr != nullptr)
        {
            searchPtr += strlen("+USOER:"); //  Move searchPtr to first char
            while (*searchPtr == ' ')
                searchPtr++; // skip spaces
            sscanf(searchPtr, "%d", &errorCode);
        }
    }

    return errorCode;
}

IPAddress SparkFun_ublox_Cellular::lastRemoteIP(void)
{
    return _lastRemoteIP;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::resetHTTPprofile(int profile)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_HTTP_PROFILE) + 16;
    char command[cmdLen];

    if (profile >= UBX_CELL_NUM_HTTP_PROFILES)
        return UBX_CELL_ERROR_ERROR;

    snprintf(command, cmdLen, "%s=%d", UBX_CELL_HTTP_PROFILE, profile);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setHTTPserverIPaddress(int profile, IPAddress address)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_HTTP_PROFILE) + 64;
    char command[cmdLen];

    if (profile >= UBX_CELL_NUM_HTTP_PROFILES)
        return UBX_CELL_ERROR_ERROR;

    snprintf(command, cmdLen, "%s=%d,%d,\"%d.%d.%d.%d\"", UBX_CELL_HTTP_PROFILE, profile,
             UBX_CELL_HTTP_OP_CODE_SERVER_IP, address[0], address[1], address[2], address[3]);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setHTTPserverName(int profile, String server)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_HTTP_PROFILE) + 12 + server.length();
    char *command;

    if (profile >= UBX_CELL_NUM_HTTP_PROFILES)
        return UBX_CELL_ERROR_ERROR;

    command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    snprintf(command, cmdLen, "%s=%d,%d,\"%s\"", UBX_CELL_HTTP_PROFILE, profile, UBX_CELL_HTTP_OP_CODE_SERVER_NAME,
             server.c_str());

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    free(command);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setHTTPusername(int profile, String username)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_HTTP_PROFILE) + 12 + username.length();
    char *command;

    if (profile >= UBX_CELL_NUM_HTTP_PROFILES)
        return UBX_CELL_ERROR_ERROR;

    command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    snprintf(command, cmdLen, "%s=%d,%d,\"%s\"", UBX_CELL_HTTP_PROFILE, profile, UBX_CELL_HTTP_OP_CODE_USERNAME,
             username.c_str());

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    free(command);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setHTTPpassword(int profile, String password)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_HTTP_PROFILE) + 12 + password.length();
    char *command;

    if (profile >= UBX_CELL_NUM_HTTP_PROFILES)
        return UBX_CELL_ERROR_ERROR;

    command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    snprintf(command, cmdLen, "%s=%d,%d,\"%s\"", UBX_CELL_HTTP_PROFILE, profile, UBX_CELL_HTTP_OP_CODE_PASSWORD,
             password.c_str());

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    free(command);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setHTTPauthentication(int profile, bool authenticate)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_HTTP_PROFILE) + 32;
    char command[cmdLen];

    if (profile >= UBX_CELL_NUM_HTTP_PROFILES)
        return UBX_CELL_ERROR_ERROR;

    snprintf(command, cmdLen, "%s=%d,%d,%d", UBX_CELL_HTTP_PROFILE, profile, UBX_CELL_HTTP_OP_CODE_AUTHENTICATION,
             authenticate);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setHTTPserverPort(int profile, int port)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_HTTP_PROFILE) + 32;
    char command[cmdLen];

    if (profile >= UBX_CELL_NUM_HTTP_PROFILES)
        return UBX_CELL_ERROR_ERROR;

    snprintf(command, cmdLen, "%s=%d,%d,%d", UBX_CELL_HTTP_PROFILE, profile, UBX_CELL_HTTP_OP_CODE_SERVER_PORT, port);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setHTTPcustomHeader(int profile, String header)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_HTTP_PROFILE) + 12 + header.length();
    char *command;

    if (profile >= UBX_CELL_NUM_HTTP_PROFILES)
        return UBX_CELL_ERROR_ERROR;

    command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    snprintf(command, cmdLen, "%s=%d,%d,\"%s\"", UBX_CELL_HTTP_PROFILE, profile,
             UBX_CELL_HTTP_OP_CODE_ADD_CUSTOM_HEADERS, header.c_str());

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    free(command);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setHTTPsecure(int profile, bool secure, int secprofile)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_HTTP_PROFILE) + 32;
    char command[cmdLen];

    if (profile >= UBX_CELL_NUM_HTTP_PROFILES)
        return UBX_CELL_ERROR_ERROR;

    if (secprofile == -1)
        snprintf(command, cmdLen, "%s=%d,%d,%d", UBX_CELL_HTTP_PROFILE, profile, UBX_CELL_HTTP_OP_CODE_SECURE, secure);
    else
        snprintf(command, cmdLen, "%s=%d,%d,%d,%d", UBX_CELL_HTTP_PROFILE, profile, UBX_CELL_HTTP_OP_CODE_SECURE,
                 secure, secprofile);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::ping(String remote_host, int retry, int p_size, unsigned long timeout, int ttl)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_PING_COMMAND) + 48 + remote_host.length();
    char *command;

    command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    snprintf(command, cmdLen, "%s=\"%s\",%d,%d,%ld,%d", UBX_CELL_PING_COMMAND, remote_host.c_str(), retry, p_size,
             timeout, ttl);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    free(command);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::sendHTTPGET(int profile, String path, String responseFilename)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_HTTP_COMMAND) + 24 + path.length() + responseFilename.length();
    char *command;

    if (profile >= UBX_CELL_NUM_HTTP_PROFILES)
        return UBX_CELL_ERROR_ERROR;

    command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    snprintf(command, cmdLen, "%s=%d,%d,\"%s\",\"%s\"", UBX_CELL_HTTP_COMMAND, profile, UBX_CELL_HTTP_COMMAND_GET,
             path.c_str(), responseFilename.c_str());

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    free(command);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::sendHTTPPOSTdata(int profile, String path, String responseFilename, String data,
                                            UBX_CELL_http_content_types_t httpContentType)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_HTTP_COMMAND) + 24 + path.length() + responseFilename.length() + data.length();
    char *command;

    if (profile >= UBX_CELL_NUM_HTTP_PROFILES)
        return UBX_CELL_ERROR_ERROR;

    command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    snprintf(command, cmdLen, "%s=%d,%d,\"%s\",\"%s\",\"%s\",%d", UBX_CELL_HTTP_COMMAND, profile,
             UBX_CELL_HTTP_COMMAND_POST_DATA, path.c_str(), responseFilename.c_str(), data.c_str(), httpContentType);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    free(command);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::sendHTTPPOSTfile(int profile, String path, String responseFilename, String requestFile,
                                            UBX_CELL_http_content_types_t httpContentType)
{
    UBX_CELL_error_t err;
    size_t cmdLen =
        strlen(UBX_CELL_HTTP_COMMAND) + 24 + path.length() + responseFilename.length() + requestFile.length();
    char *command;

    if (profile >= UBX_CELL_NUM_HTTP_PROFILES)
        return UBX_CELL_ERROR_ERROR;

    command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    snprintf(command, cmdLen, "%s=%d,%d,\"%s\",\"%s\",\"%s\",%d", UBX_CELL_HTTP_COMMAND, profile,
             UBX_CELL_HTTP_COMMAND_POST_FILE, path.c_str(), responseFilename.c_str(), requestFile.c_str(),
             httpContentType);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    free(command);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::getHTTPprotocolError(int profile, int *error_class, int *error_code)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_HTTP_PROTOCOL_ERROR) + 4;
    char command[cmdLen];
    char response[minimumResponseAllocation];

    int rprofile, eclass, ecode;

    snprintf(command, cmdLen, "%s=%d", UBX_CELL_HTTP_PROTOCOL_ERROR, profile);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        int scanned = 0;
        char *searchPtr = strnstr(response, "+UHTTPER:", minimumResponseAllocation);
        if (searchPtr != nullptr)
        {
            searchPtr += strlen("+UHTTPER:"); //  Move searchPtr to first char
            while (*searchPtr == ' ')
                searchPtr++; // skip spaces
            scanned = sscanf(searchPtr, "%d,%d,%d\r\n", &rprofile, &eclass, &ecode);
        }
        if (scanned == 3)
        {
            *error_class = eclass;
            *error_code = ecode;
        }
        else
            err = UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
    }

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::nvMQTT(UBX_CELL_mqtt_nv_parameter_t parameter)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_MQTT_NVM) + 10;
    char command[cmdLen];

    snprintf(command, cmdLen, "%s=%d", UBX_CELL_MQTT_NVM, parameter);
    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setMQTTclientId(const String &clientId)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_MQTT_PROFILE) + clientId.length() + 10;
    char *command;

    command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    snprintf(command, cmdLen, "%s=%d,\"%s\"", UBX_CELL_MQTT_PROFILE, UBX_CELL_MQTT_PROFILE_CLIENT_ID, clientId.c_str());
    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    free(command);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setMQTTserver(const String &serverName, int port)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_MQTT_PROFILE) + serverName.length() + 16;
    char *command;

    command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    snprintf(command, cmdLen, "%s=%d,\"%s\",%d", UBX_CELL_MQTT_PROFILE, UBX_CELL_MQTT_PROFILE_SERVERNAME,
             serverName.c_str(), port);
    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    free(command);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setMQTTcredentials(const String &userName, const String &pwd)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_MQTT_PROFILE) + userName.length() + pwd.length() + 16;
    char *command;

    command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    snprintf(command, cmdLen, "%s=%d,\"%s\",\"%s\"", UBX_CELL_MQTT_PROFILE, UBX_CELL_MQTT_PROFILE_USERNAMEPWD,
             userName.c_str(), pwd.c_str());
    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    free(command);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setMQTTsecure(bool secure, int secprofile)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_MQTT_PROFILE) + 16;
    char command[cmdLen];

    if (secprofile == -1)
        snprintf(command, cmdLen, "%s=%d,%d", UBX_CELL_MQTT_PROFILE, UBX_CELL_MQTT_PROFILE_SECURE, secure);
    else
        snprintf(command, cmdLen, "%s=%d,%d,%d", UBX_CELL_MQTT_PROFILE, UBX_CELL_MQTT_PROFILE_SECURE, secure,
                 secprofile);
    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::connectMQTT(void)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_MQTT_COMMAND) + 10;
    char command[cmdLen];

    snprintf(command, cmdLen, "%s=%d", UBX_CELL_MQTT_COMMAND, UBX_CELL_MQTT_COMMAND_LOGIN);
    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::disconnectMQTT(void)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_MQTT_COMMAND) + 10;
    char command[cmdLen];

    snprintf(command, cmdLen, "%s=%d", UBX_CELL_MQTT_COMMAND, UBX_CELL_MQTT_COMMAND_LOGOUT);
    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::subscribeMQTTtopic(int max_Qos, const String &topic)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_MQTT_COMMAND) + 16 + topic.length();
    char *command;

    command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    snprintf(command, cmdLen, "%s=%d,%d,\"%s\"", UBX_CELL_MQTT_COMMAND, UBX_CELL_MQTT_COMMAND_SUBSCRIBE, max_Qos,
             topic.c_str());
    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    free(command);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::unsubscribeMQTTtopic(const String &topic)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_MQTT_COMMAND) + 16 + topic.length();
    char *command;

    command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    snprintf(command, cmdLen, "%s=%d,\"%s\"", UBX_CELL_MQTT_COMMAND, UBX_CELL_MQTT_COMMAND_UNSUBSCRIBE, topic.c_str());
    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    free(command);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::readMQTT(int *pQos, String *pTopic, uint8_t *readDest, int readLength, int *bytesRead)
{
    size_t cmdLen = strlen(UBX_CELL_MQTT_COMMAND) + 10;
    char command[cmdLen];
    char *response;
    UBX_CELL_error_t err;
    int scanNum = 0;
    int total_length, topic_length, data_length;

    // Set *bytesRead to zero
    if (bytesRead != nullptr)
        *bytesRead = 0;

    // Allocate memory for the response
    int responseLength = readLength + minimumResponseAllocation;
    response = ubx_cell_calloc_char(responseLength);
    if (response == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;

    // Note to self: if the file contents contain "OK\r\n" sendCommandWithResponse will return true too early...
    // To try and avoid this, look for \"\r\n\r\nOK\r\n there is a extra \r\n beetween " and the the standard \r\nOK\r\n
    const char mqttReadTerm[] = "\"\r\n\r\nOK\r\n";
    snprintf(command, cmdLen, "%s=%d,%d", UBX_CELL_MQTT_COMMAND, UBX_CELL_MQTT_COMMAND_READ, 1);
    err = sendCommandWithResponse(command, mqttReadTerm, response, (5 * UBX_CELL_STANDARD_RESPONSE_TIMEOUT),
                                  responseLength);

    if (err != UBX_CELL_ERROR_SUCCESS)
    {
        if (_printDebug == true)
        {
            _debugPort->print(F("readMQTT: sendCommandWithResponse err "));
            _debugPort->println(err);
        }
        free(response);
        return err;
    }

    // Extract the data
    char *searchPtr = strnstr(response, "+UMQTTC:", responseLength);
    int cmd = 0;
    if (searchPtr != nullptr)
    {
        searchPtr += strlen("+UMQTTC:"); //  Move searchPtr to first char
        while (*searchPtr == ' ')
            searchPtr++; // skip spaces
        scanNum =
            sscanf(searchPtr, "%d,%d,%d,%d,\"%*[^\"]\",%d,\"", &cmd, pQos, &total_length, &topic_length, &data_length);
    }
    if ((scanNum != 5) || (cmd != UBX_CELL_MQTT_COMMAND_READ))
    {
        if (_printDebug == true)
        {
            _debugPort->print(F("readMQTT: error: scanNum is "));
            _debugPort->println(scanNum);
        }
        free(response);
        return UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
    }

    err = UBX_CELL_ERROR_SUCCESS;
    searchPtr = strnstr(searchPtr, "\"", responseLength - (searchPtr - response));
    if (searchPtr != nullptr)
    {
        if (pTopic)
        {
            searchPtr[topic_length + 1] = '\0'; // zero terminate
            *pTopic = searchPtr + 1;
            searchPtr[topic_length + 1] = '\"'; // restore
        }
        searchPtr =
            strnstr(searchPtr + topic_length + 2, "\"", responseLength - (searchPtr + topic_length + 2 - response));
        if (readDest && (searchPtr != nullptr) && (response + responseLength >= searchPtr + data_length + 1) &&
            (searchPtr[data_length + 1] == '"'))
        {
            if (data_length > readLength)
            {
                data_length = readLength;
                if (_printDebug == true)
                {
                    _debugPort->print(F("readMQTT: error: trucate message"));
                }
                err = UBX_CELL_ERROR_OUT_OF_MEMORY;
            }
            memcpy(readDest, searchPtr + 1, data_length);
            *bytesRead = data_length;
        }
        else
        {
            if (_printDebug == true)
            {
                _debugPort->print(F("readMQTT: error: message end "));
            }
            err = UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
        }
    }
    free(response);

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::mqttPublishTextMsg(const String &topic, const char *const msg, uint8_t qos, bool retain)
{
    if (topic.length() < 1 || msg == nullptr)
    {
        return UBX_CELL_ERROR_INVALID;
    }

    UBX_CELL_error_t err;

    char sanitized_msg[MAX_MQTT_DIRECT_MSG_LEN + 1];
    memset(sanitized_msg, 0, sizeof(sanitized_msg));

    // Check the message length and truncate if necessary.
    size_t msg_len = strnlen(msg, MAX_MQTT_DIRECT_MSG_LEN);
    if (msg_len > MAX_MQTT_DIRECT_MSG_LEN)
    {
        msg_len = MAX_MQTT_DIRECT_MSG_LEN;
    }

    strncpy(sanitized_msg, msg, msg_len);
    char *msg_ptr = sanitized_msg;
    while (*msg_ptr != 0)
    {
        if (*msg_ptr == '"')
        {
            *msg_ptr = ' ';
        }

        msg_ptr++;
    }

    size_t cmdLen = strlen(UBX_CELL_MQTT_COMMAND) + 20 + topic.length() + msg_len;
    char *command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;

    snprintf(command, cmdLen, "%s=%d,%u,%u,0,\"%s\",\"%s\"", UBX_CELL_MQTT_COMMAND, UBX_CELL_MQTT_COMMAND_PUBLISH, qos,
             (retain ? 1 : 0), topic.c_str(), sanitized_msg);

    sendCommand(command, true);
    err = waitForResponse(UBX_CELL_RESPONSE_MORE, UBX_CELL_RESPONSE_ERROR, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        sendCommand(msg, false);
        err = waitForResponse(UBX_CELL_RESPONSE_OK, UBX_CELL_RESPONSE_ERROR, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    }

    free(command);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::mqttPublishBinaryMsg(const String &topic, const char *const msg, size_t msg_len, uint8_t qos,
                                                bool retain)
{
    /*
     * The modem prints the '>' as the signal to send the binary message content.
     * at+umqttc=9,0,0,"topic",4
     *
     * >"xY"
     * OK
     *
     * +UUMQTTC: 9,1
     */
    if (topic.length() < 1 || msg == nullptr || msg_len > MAX_MQTT_DIRECT_MSG_LEN)
    {
        return UBX_CELL_ERROR_INVALID;
    }

    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_MQTT_COMMAND) + 20 + topic.length();
    char *command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;

    snprintf(command, cmdLen, "%s=%d,%u,%u,\"%s\",%u", UBX_CELL_MQTT_COMMAND, UBX_CELL_MQTT_COMMAND_PUBLISHBINARY, qos,
             (retain ? 1 : 0), topic.c_str(), msg_len);

    sendCommand(command, true);
    err = waitForResponse(UBX_CELL_RESPONSE_MORE, UBX_CELL_RESPONSE_ERROR, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        sendCommand(msg, false);
        err = waitForResponse(UBX_CELL_RESPONSE_OK, UBX_CELL_RESPONSE_ERROR, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    }

    free(command);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::mqttPublishFromFile(const String &topic, const String &filename, uint8_t qos, bool retain)
{
    if (topic.length() < 1 || filename.length() < 1)
    {
        return UBX_CELL_ERROR_INVALID;
    }

    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_MQTT_COMMAND) + 20 + topic.length() + filename.length();
    char *command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;

    snprintf(command, cmdLen, "%s=%d,%u,%u,\"%s\",\"%s\"", UBX_CELL_MQTT_COMMAND, UBX_CELL_MQTT_COMMAND_PUBLISHFILE,
             qos, (retain ? 1 : 0), topic.c_str(), filename.c_str());

    sendCommand(command, true);
    err = waitForResponse(UBX_CELL_RESPONSE_OK, UBX_CELL_RESPONSE_ERROR, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    free(command);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::getMQTTprotocolError(int *error_code, int *error_code2)
{
    UBX_CELL_error_t err;
    char response[minimumResponseAllocation];

    int code, code2;

    err = sendCommandWithResponse(UBX_CELL_MQTT_PROTOCOL_ERROR, UBX_CELL_RESPONSE_OK_OR_ERROR, response,
                                  UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        int scanned = 0;
        char *searchPtr = strnstr(response, "+UMQTTER:", minimumResponseAllocation);
        if (searchPtr != nullptr)
        {
            searchPtr += strlen("+UMQTTER:"); //  Move searchPtr to first char
            while (*searchPtr == ' ')
                searchPtr++; // skip spaces
            scanned = sscanf(searchPtr, "%d,%d\r\n", &code, &code2);
        }
        if (scanned == 2)
        {
            *error_code = code;
            *error_code2 = code2;
        }
        else
            err = UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
    }

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setFTPserver(const String &serverName)
{
    constexpr size_t cmdLen = 145;
    char command[cmdLen]; // long enough for AT+UFTP=1,<128 bytes>

    snprintf(command, cmdLen, "%s=%d,\"%s\"", UBX_CELL_FTP_PROFILE, UBX_CELL_FTP_PROFILE_SERVERNAME,
             serverName.c_str());
    return sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setFTPtimeouts(const unsigned int timeout, const unsigned int cmd_linger,
                                          const unsigned int data_linger)
{
    constexpr size_t cmdLen = 64;
    char command[cmdLen]; // long enough for AT+UFTP=1,<128 bytes>

    snprintf(command, cmdLen, "%s=%d,%u,%u,%u", UBX_CELL_FTP_PROFILE, UBX_CELL_FTP_PROFILE_TIMEOUT, timeout, cmd_linger,
             data_linger);
    return sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setFTPcredentials(const String &userName, const String &pwd)
{
    UBX_CELL_error_t err;
    constexpr size_t cmdLen = 48;
    char command[cmdLen]; // long enough for AT+UFTP=n,<30 bytes>

    snprintf(command, cmdLen, "%s=%d,\"%s\"", UBX_CELL_FTP_PROFILE, UBX_CELL_FTP_PROFILE_USERNAME, userName.c_str());
    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    if (err != UBX_CELL_ERROR_SUCCESS)
    {
        return err;
    }

    snprintf(command, cmdLen, "%s=%d,\"%s\"", UBX_CELL_FTP_PROFILE, UBX_CELL_FTP_PROFILE_PWD, pwd.c_str());
    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::connectFTP(void)
{
    constexpr size_t cmdLen = 16;
    char command[cmdLen]; // long enough for AT+UFTPC=n

    snprintf(command, cmdLen, "%s=%d", UBX_CELL_FTP_COMMAND, UBX_CELL_FTP_COMMAND_LOGIN);
    return sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
}

UBX_CELL_error_t SparkFun_ublox_Cellular::disconnectFTP(void)
{
    constexpr size_t cmdLen = 16;
    char command[cmdLen]; // long enough for AT+UFTPC=n

    snprintf(command, cmdLen, "%s=%d", UBX_CELL_FTP_COMMAND, UBX_CELL_FTP_COMMAND_LOGOUT);
    return sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
}

UBX_CELL_error_t SparkFun_ublox_Cellular::ftpGetFile(const String &filename)
{
    size_t cmdLen = strlen(UBX_CELL_FTP_COMMAND) + (2 * filename.length()) + 16;
    char *command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;

    snprintf(command, cmdLen, "%s=%d,\"%s\",\"%s\"", UBX_CELL_FTP_COMMAND, UBX_CELL_FTP_COMMAND_GET_FILE,
             filename.c_str(), filename.c_str());
    // memset(response, 0, sizeof(response));
    // sendCommandWithResponse(command, UBX_CELL_RESPONSE_CONNECT, response, 8000 /* ms */, response_len);
    UBX_CELL_error_t err =
        sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    free(command);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::getFTPprotocolError(int *error_code, int *error_code2)
{
    UBX_CELL_error_t err;
    char response[minimumResponseAllocation];

    int code, code2;

    err = sendCommandWithResponse(UBX_CELL_FTP_PROTOCOL_ERROR, UBX_CELL_RESPONSE_OK_OR_ERROR, response,
                                  UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        int scanned = 0;
        char *searchPtr = strnstr(response, "+UFTPER:", minimumResponseAllocation);
        if (searchPtr != nullptr)
        {
            searchPtr += strlen("+UFTPER:"); //  Move searchPtr to first char
            while (*searchPtr == ' ')
            {
                searchPtr++; // skip spaces
            }
            scanned = sscanf(searchPtr, "%d,%d\r\n", &code, &code2);
        }

        if (scanned == 2)
        {
            *error_code = code;
            *error_code2 = code2;
        }
        else
        {
            err = UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
        }
    }

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::resetSecurityProfile(int secprofile)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_SEC_PROFILE) + 6;
    char command[cmdLen];

    snprintf(command, cmdLen, "%s=%d", UBX_CELL_SEC_PROFILE, secprofile);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::configSecurityProfile(int secprofile, UBX_CELL_sec_profile_parameter_t parameter, int value)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_SEC_PROFILE) + 10;
    char command[cmdLen];

    snprintf(command, cmdLen, "%s=%d,%d,%d", UBX_CELL_SEC_PROFILE, secprofile, parameter, value);
    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::configSecurityProfileString(int secprofile, UBX_CELL_sec_profile_parameter_t parameter,
                                                       String value)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_SEC_PROFILE) + value.length() + 10;
    char *command;

    command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    snprintf(command, cmdLen, "%s=%d,%d,\"%s\"", UBX_CELL_SEC_PROFILE, secprofile, parameter, value.c_str());
    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    free(command);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setSecurityManager(UBX_CELL_sec_manager_opcode_t opcode,
                                              UBX_CELL_sec_manager_parameter_t parameter, String name, String data)
{
    size_t cmdLen = strlen(UBX_CELL_SEC_MANAGER) + name.length() + 20;
    char *command;
    char response[minimumResponseAllocation];
    UBX_CELL_error_t err;

    command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    int dataLen = data.length();
    snprintf(command, cmdLen, "%s=%d,%d,\"%s\",%d", UBX_CELL_SEC_MANAGER, opcode, parameter, name.c_str(), dataLen);

    err = sendCommandWithResponse(command, ">", response, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        if (_printDebug == true)
        {
            _debugPort->print(F("dataDownload: writing "));
            _debugPort->print(dataLen);
            _debugPort->println(F(" bytes"));
        }
        hwWriteData(data.c_str(), dataLen);
        err = waitForResponse(UBX_CELL_RESPONSE_OK, UBX_CELL_RESPONSE_ERROR, UBX_CELL_STANDARD_RESPONSE_TIMEOUT * 3);
    }

    if (err != UBX_CELL_ERROR_SUCCESS)
    {
        if (_printDebug == true)
        {
            _debugPort->print(F("dataDownload: Error: "));
            _debugPort->print(err);
            _debugPort->print(F(" => {"));
            _debugPort->print(response);
            _debugPort->println(F("}"));
        }
    }

    free(command);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::activatePDPcontext(bool status, int cid)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_MESSAGE_PDP_CONTEXT_ACTIVATE) + 32;
    char command[cmdLen];

    if (cid >= UBX_CELL_NUM_PDP_CONTEXT_IDENTIFIERS)
        return UBX_CELL_ERROR_ERROR;

    if (cid == -1)
        snprintf(command, cmdLen, "%s=%d", UBX_CELL_MESSAGE_PDP_CONTEXT_ACTIVATE, status);
    else
        snprintf(command, cmdLen, "%s=%d,%d", UBX_CELL_MESSAGE_PDP_CONTEXT_ACTIVATE, status, cid);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    return err;
}

bool SparkFun_ublox_Cellular::isGPSon(void)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_GNSS_POWER) + 2;
    char command[cmdLen];
    char response[minimumResponseAllocation];
    bool on = false;

    snprintf(command, cmdLen, "%s?", UBX_CELL_GNSS_POWER);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, UBX_CELL_10_SEC_TIMEOUT);

    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        // Example response: "+UGPS: 0" for off "+UGPS: 1,0,1" for on
        // Search for a ':' followed by a '1' or ' 1'
        char *pch1 = strchr(response, ':');
        if (pch1 != nullptr)
        {
            char *pch2 = strchr(response, '1');
            if ((pch2 != nullptr) && ((pch2 == pch1 + 1) || (pch2 == pch1 + 2)))
                on = true;
        }
    }

    return on;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::gpsPower(bool enable, gnss_system_t gnss_sys, gnss_aiding_mode_t gnss_aiding)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_GNSS_POWER) + 32; // gnss_sys could be up to three digits
    char command[cmdLen];
    bool gpsState;

    // Don't turn GPS on/off if it's already on/off
    gpsState = isGPSon();
    if ((enable && gpsState) || (!enable && !gpsState))
    {
        return UBX_CELL_ERROR_SUCCESS;
    }

    // GPS power management
    if (enable)
    {
        snprintf(command, cmdLen, "%s=1,%d,%d", UBX_CELL_GNSS_POWER, gnss_aiding, gnss_sys);
    }
    else
    {
        snprintf(command, cmdLen, "%s=0", UBX_CELL_GNSS_POWER);
    }

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, 10000);

    return err;
}

/*
UBX_CELL_error_t SparkFun_ublox_Cellular::gpsEnableClock(bool enable)
{
    // AT+UGZDA=<0,1>
    UBX_CELL_error_t err = UBX_CELL_ERROR_SUCCESS;
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::gpsGetClock(struct ClockData *clock)
{
    // AT+UGZDA?
    UBX_CELL_error_t err = UBX_CELL_ERROR_SUCCESS;
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::gpsEnableFix(bool enable)
{
    // AT+UGGGA=<0,1>
    UBX_CELL_error_t err = UBX_CELL_ERROR_SUCCESS;
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::gpsGetFix(struct PositionData *pos)
{
    // AT+UGGGA?
    UBX_CELL_error_t err = UBX_CELL_ERROR_SUCCESS;
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::gpsEnablePos(bool enable)
{
    // AT+UGGLL=<0,1>
    UBX_CELL_error_t err = UBX_CELL_ERROR_SUCCESS;
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::gpsGetPos(struct PositionData *pos)
{
    // AT+UGGLL?
    UBX_CELL_error_t err = UBX_CELL_ERROR_SUCCESS;
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::gpsEnableSat(bool enable)
{
    // AT+UGGSV=<0,1>
    UBX_CELL_error_t err = UBX_CELL_ERROR_SUCCESS;
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::gpsGetSat(uint8_t *sats)
{
    // AT+UGGSV?
    UBX_CELL_error_t err = UBX_CELL_ERROR_SUCCESS;
    return err;
}
*/

UBX_CELL_error_t SparkFun_ublox_Cellular::gpsEnableRmc(bool enable)
{
    // AT+UGRMC=<0,1>
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_GNSS_GPRMC) + 3;
    char command[cmdLen];

    // ** Don't call gpsPower here. It causes problems for +UTIME and the PPS signal **
    // ** Call isGPSon and gpsPower externally if required **
    // if (!isGPSon())
    // {
    //     err = gpsPower(true);
    //     if (err != UBX_CELL_ERROR_SUCCESS)
    //     {
    //         return err;
    //     }
    // }

    snprintf(command, cmdLen, "%s=%d", UBX_CELL_GNSS_GPRMC, enable ? 1 : 0);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_10_SEC_TIMEOUT);

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::gpsGetRmc(struct PositionData *pos, struct SpeedData *spd, struct ClockData *clk,
                                     bool *valid)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_GNSS_GPRMC) + 2;
    char command[cmdLen];
    char response[minimumResponseAllocation];
    char *rmcBegin;

    snprintf(command, cmdLen, "%s?", UBX_CELL_GNSS_GPRMC);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, UBX_CELL_10_SEC_TIMEOUT);
    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        // Fast-forward response string to $GPRMC starter
        rmcBegin = strnstr(response, "$GPRMC", minimumResponseAllocation);
        if (rmcBegin == nullptr)
        {
            err = UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
        }
        else
        {
            *valid = parseGPRMCString(rmcBegin, pos, clk, spd);
        }
    }

    return err;
}

/*
UBX_CELL_error_t SparkFun_ublox_Cellular::gpsEnableSpeed(bool enable)
{
    // AT+UGVTG=<0,1>
    UBX_CELL_error_t err = UBX_CELL_ERROR_SUCCESS;
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::gpsGetSpeed(struct SpeedData *speed)
{
    // AT+UGVTG?
    UBX_CELL_error_t err = UBX_CELL_ERROR_SUCCESS;
    return err;
}
*/

UBX_CELL_error_t SparkFun_ublox_Cellular::gpsRequest(unsigned int timeout, uint32_t accuracy, bool detailed, unsigned int sensor)
{
    // AT+ULOC=2,<useCellLocate>,<detailed>,<timeout>,<accuracy>
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_GNSS_REQUEST_LOCATION) + 24;
    char command[cmdLen];

    // This function will only work if the GPS module is initially turned off.
    if (isGPSon())
    {
        gpsPower(false);
    }

    if (timeout > 999)
        timeout = 999;
    if (accuracy > 999999)
        accuracy = 999999;

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
    snprintf(command, cmdLen, "%s=2,%d,%d,%d,%d", UBX_CELL_GNSS_REQUEST_LOCATION, sensor, detailed ? 1 : 0, timeout,
             accuracy);
#else
    snprintf(command, cmdLen, "%s=2,%d,%d,%d,%ld", UBX_CELL_GNSS_REQUEST_LOCATION, sensor, detailed ? 1 : 0, timeout,
             accuracy);
#endif

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_10_SEC_TIMEOUT);

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::gpsAidingServerConf(const char *primaryServer, const char *secondaryServer,
                                               const char *authToken, unsigned int days, unsigned int period,
                                               unsigned int resolution, unsigned int gnssTypes, unsigned int mode,
                                               unsigned int dataType)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_AIDING_SERVER_CONFIGURATION) + 256;
    char command[cmdLen];

    snprintf(command, cmdLen, "%s=\"%s\",\"%s\",\"%s\",%d,%d,%d,%d,%d,%d", UBX_CELL_AIDING_SERVER_CONFIGURATION,
             primaryServer, secondaryServer, authToken, days, period, resolution, gnssTypes, mode, dataType);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    return err;
}

// OK for text files. But will fail with binary files (containing \0) on some platforms.
UBX_CELL_error_t SparkFun_ublox_Cellular::appendFileContents(String filename, const char *str, int len)
{
    size_t cmdLen = strlen(UBX_CELL_FILE_SYSTEM_DOWNLOAD_FILE) + filename.length() + 10;
    char *command;
    char response[minimumResponseAllocation];
    UBX_CELL_error_t err;

    command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    int dataLen = len == -1 ? strlen(str) : len;
    snprintf(command, cmdLen, "%s=\"%s\",%d", UBX_CELL_FILE_SYSTEM_DOWNLOAD_FILE, filename.c_str(), dataLen);

    err = sendCommandWithResponse(command, ">", response, UBX_CELL_STANDARD_RESPONSE_TIMEOUT * 2);

    unsigned long writeDelay = millis();
    while (millis() < (writeDelay + 50))
        delay(1); // uBlox specification says to wait 50ms after receiving "@" to write data.

    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        if (_printDebug == true)
        {
            _debugPort->print(F("fileDownload: writing "));
            _debugPort->print(dataLen);
            _debugPort->println(F(" bytes"));
        }
        hwWriteData(str, dataLen);

        err = waitForResponse(UBX_CELL_RESPONSE_OK, UBX_CELL_RESPONSE_ERROR, UBX_CELL_STANDARD_RESPONSE_TIMEOUT * 5);
    }
    if (err != UBX_CELL_ERROR_SUCCESS)
    {
        if (_printDebug == true)
        {
            _debugPort->print(F("fileDownload: Error: "));
            _debugPort->print(err);
            _debugPort->print(F(" => {"));
            _debugPort->print(response);
            _debugPort->println(F("}"));
        }
    }

    free(command);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::appendFileContents(String filename, String str)
{
    return appendFileContents(filename, str.c_str(), str.length());
}

// OK for text files. But will fail with binary files (containing \0) on some platforms.
UBX_CELL_error_t SparkFun_ublox_Cellular::getFileContents(String filename, String *contents)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_FILE_SYSTEM_READ_FILE) + filename.length() + 8;
    char *command;
    char *response;

    // Start by getting the file size so we know in advance how much data to expect
    int fileSize = 0;
    err = getFileSize(filename, &fileSize);
    if (err != UBX_CELL_SUCCESS)
    {
        if (_printDebug == true)
        {
            _debugPort->print(F("getFileContents: getFileSize returned err "));
            _debugPort->println(err);
        }
        return err;
    }

    command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    snprintf(command, cmdLen, "%s=\"%s\"", UBX_CELL_FILE_SYSTEM_READ_FILE, filename.c_str());

    response = ubx_cell_calloc_char(fileSize + minimumResponseAllocation);
    if (response == nullptr)
    {
        if (_printDebug == true)
        {
            _debugPort->print(F("getFileContents: response alloc failed: "));
            _debugPort->println(fileSize + minimumResponseAllocation);
        }
        free(command);
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    }

    // A large file will completely fill the backlog buffer - but it will be pruned afterwards
    // Note to self: if the file contents contain "OK\r\n" sendCommandWithResponse will return true too early...
    // To try and avoid this, look for \"\r\nOK\r\n
    const char fileReadTerm[] = "\r\nOK\r\n"; // LARA-R6 returns "\"\r\n\r\nOK\r\n" while SARA-R5 return "\"\r\nOK\r\n";
    err = sendCommandWithResponse(command, fileReadTerm, response, (5 * UBX_CELL_STANDARD_RESPONSE_TIMEOUT),
                                  (fileSize + minimumResponseAllocation));

    if (err != UBX_CELL_ERROR_SUCCESS)
    {
        if (_printDebug == true)
        {
            _debugPort->print(F("getFileContents: sendCommandWithResponse returned err "));
            _debugPort->println(err);
        }
        free(command);
        free(response);
        return err;
    }

    // Response format: \r\n+URDFILE: "filename",36,"these bytes are the data of the file"\r\n\r\nOK\r\n
    int scanned = 0;
    int readFileSize = 0;
    char *searchPtr = strnstr(response, "+URDFILE:", fileSize + minimumResponseAllocation);
    if (searchPtr != nullptr)
    {
        searchPtr = strchr(searchPtr, '\"');   // Find the first quote
        searchPtr = strchr(++searchPtr, '\"'); // Find the second quote

        scanned = sscanf(searchPtr, "\",%d,", &readFileSize); // Get the file size (again)
        if (scanned == 1)
        {
            searchPtr = strchr(++searchPtr, '\"'); // Find the third quote

            if (searchPtr == nullptr)
            {
                if (_printDebug == true)
                {
                    _debugPort->println(F("getFileContents: third quote not found!"));
                }
                free(command);
                free(response);
                return UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
            }

            int bytesRead = 0;

            while (bytesRead < readFileSize)
            {
                searchPtr++; // Increment searchPtr then copy file char into contents
                // Important Note: some implementations of concat, like the one on ESP32, are binary-compatible.
                // But some, like SAMD, are not. They use strlen or strcpy internally - which don't like \0's.
                // The only true binary-compatible solution is to use getFileContents(String filename, char
                // *contents)...
                contents->concat(*(searchPtr)); // Append file char to contents
                bytesRead++;
            }
            if (_printDebug == true)
            {
                _debugPort->print(F("getFileContents: total bytes read: "));
                _debugPort->println(bytesRead);
            }
            err = UBX_CELL_ERROR_SUCCESS;
        }
        else
        {
            if (_printDebug == true)
            {
                _debugPort->print(F("getFileContents: sscanf failed! scanned is "));
                _debugPort->println(scanned);
            }
            err = UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
        }
    }
    else
    {
        if (_printDebug == true)
            _debugPort->println(F("getFileContents: strnstr failed!"));
        err = UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
    }

    free(command);
    free(response);
    return err;
}

// OK for binary files. Make sure contents can hold the entire file. Get the size first with getFileSize.
UBX_CELL_error_t SparkFun_ublox_Cellular::getFileContents(String filename, char *contents)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_FILE_SYSTEM_READ_FILE) + filename.length() + 8;
    char *command;
    char *response;

    // Start by getting the file size so we know in advance how much data to expect
    int fileSize = 0;
    err = getFileSize(filename, &fileSize);
    if (err != UBX_CELL_SUCCESS)
    {
        if (_printDebug == true)
        {
            _debugPort->print(F("getFileContents: getFileSize returned err "));
            _debugPort->println(err);
        }
        return err;
    }

    command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    snprintf(command, cmdLen, "%s=\"%s\"", UBX_CELL_FILE_SYSTEM_READ_FILE, filename.c_str());

    response = ubx_cell_calloc_char(fileSize + minimumResponseAllocation);
    if (response == nullptr)
    {
        if (_printDebug == true)
        {
            _debugPort->print(F("getFileContents: response alloc failed: "));
            _debugPort->println(fileSize + minimumResponseAllocation);
        }
        free(command);
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    }

    // A large file will completely fill the backlog buffer - but it will be pruned afterwards
    // Note to self: if the file contents contain "OK\r\n" sendCommandWithResponse will return true too early...
    // To try and avoid this, look for \"\r\nOK\r\n
    const char fileReadTerm[] = "\"\r\nOK\r\n";
    err = sendCommandWithResponse(command, fileReadTerm, response, (5 * UBX_CELL_STANDARD_RESPONSE_TIMEOUT),
                                  (fileSize + minimumResponseAllocation));

    if (err != UBX_CELL_ERROR_SUCCESS)
    {
        if (_printDebug == true)
        {
            _debugPort->print(F("getFileContents: sendCommandWithResponse returned err "));
            _debugPort->println(err);
        }
        free(command);
        free(response);
        return err;
    }

    // Response format: \r\n+URDFILE: "filename",36,"these bytes are the data of the file"\r\n\r\nOK\r\n
    int scanned = 0;
    int readFileSize = 0;
    char *searchPtr = strnstr(response, "+URDFILE:", fileSize + minimumResponseAllocation);
    if (searchPtr != nullptr)
    {
        searchPtr = strchr(searchPtr, '\"');   // Find the first quote
        searchPtr = strchr(++searchPtr, '\"'); // Find the second quote

        scanned = sscanf(searchPtr, "\",%d,", &readFileSize); // Get the file size (again)
        if (scanned == 1)
        {
            searchPtr = strchr(++searchPtr, '\"'); // Find the third quote

            if (searchPtr == nullptr)
            {
                if (_printDebug == true)
                {
                    _debugPort->println(F("getFileContents: third quote not found!"));
                }
                free(command);
                free(response);
                return UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
            }

            int bytesRead = 0;

            while (bytesRead < readFileSize)
            {
                searchPtr++;                      // Increment searchPtr then copy file char into contents
                contents[bytesRead] = *searchPtr; // Append file char to contents
                bytesRead++;
            }
            if (_printDebug == true)
            {
                _debugPort->print(F("getFileContents: total bytes read: "));
                _debugPort->println(bytesRead);
            }
            err = UBX_CELL_ERROR_SUCCESS;
        }
        else
        {
            if (_printDebug == true)
            {
                _debugPort->print(F("getFileContents: sscanf failed! scanned is "));
                _debugPort->println(scanned);
            }
            err = UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
        }
    }
    else
    {
        if (_printDebug == true)
            _debugPort->println(F("getFileContents: strnstr failed!"));
        err = UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
    }

    free(command);
    free(response);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::getFileBlock(const String &filename, char *buffer, size_t offset, size_t requested_length,
                                        size_t &bytes_read)
{
    bytes_read = 0;
    if (filename.length() < 1 || buffer == nullptr || requested_length < 1)
    {
        return UBX_CELL_ERROR_UNEXPECTED_PARAM;
    }

    // trying to get a byte at a time does not seem to be reliable so this method must use
    // a real UART.
    if (_hardSerial == nullptr)
    {
        if (_printDebug == true)
        {
            _debugPort->println(F("getFileBlock: only works with a hardware UART"));
        }
        return UBX_CELL_ERROR_INVALID;
    }

    size_t cmdLen = filename.length() + 32;
    char *cmd = ubx_cell_calloc_char(cmdLen);
    if (cmd == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    snprintf(cmd, cmdLen, "at+urdblock=\"%s\",%zu,%zu\r\n", filename.c_str(), offset, requested_length);
    sendCommand(cmd, false);

    int ich;
    char ch;
    int quote_count = 0;
    size_t comma_idx = 0;

    while (quote_count < 3)
    {
        ich = _hardSerial->read();
        if (ich < 0)
        {
            continue;
        }
        ch = (char)(ich & 0xFF);
        cmd[bytes_read++] = ch;
        if (ch == '"')
        {
            quote_count++;
        }
        else if (ch == ',' && comma_idx == 0)
        {
            comma_idx = bytes_read;
        }
    }

    cmd[bytes_read] = 0;
    cmd[bytes_read - 2] = 0;

    // Example response:
    // +URDBLOCK: "wombat.bin",64000,"<data starts here>... "<cr><lf>
    size_t data_length = strtoul(&cmd[comma_idx], nullptr, 10);
    free(cmd);

    bytes_read = 0;
    size_t bytes_remaining = data_length;
    while (bytes_read < data_length)
    {
        // This method seems more reliable than reading a byte at a time.
        size_t rc = _hardSerial->readBytes(&buffer[bytes_read], bytes_remaining);
        bytes_read += rc;
        bytes_remaining -= rc;
    }

    return UBX_CELL_ERROR_SUCCESS;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::getFileSize(String filename, int *size)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_FILE_SYSTEM_LIST_FILES) + filename.length() + 8;
    char *command;
    char response[minimumResponseAllocation];

    command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    snprintf(command, cmdLen, "%s=2,\"%s\"", UBX_CELL_FILE_SYSTEM_LIST_FILES, filename.c_str());

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    if (err != UBX_CELL_ERROR_SUCCESS)
    {
        if (_printDebug == true)
        {
            _debugPort->print(F("getFileSize: Fail: Error: "));
            _debugPort->print(err);
            _debugPort->print(F("  Response: {"));
            _debugPort->print(response);
            _debugPort->println(F("}"));
        }
        free(command);
        return err;
    }

    char *responseStart = strnstr(response, "+ULSTFILE:", minimumResponseAllocation);
    if (responseStart == nullptr)
    {
        if (_printDebug == true)
        {
            _debugPort->print(F("getFileSize: Failure: {"));
            _debugPort->print(response);
            _debugPort->println(F("}"));
        }
        free(command);
        return UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
    }

    int fileSize;
    responseStart += strlen("+ULSTFILE:"); //  Move searchPtr to first char
    while (*responseStart == ' ')
        responseStart++; // skip spaces
    sscanf(responseStart, "%d", &fileSize);
    *size = fileSize;

    free(command);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::deleteFile(String filename)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_FILE_SYSTEM_DELETE_FILE) + filename.length() + 8;
    char *command;

    command = ubx_cell_calloc_char(cmdLen);
    if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
    snprintf(command, cmdLen, "%s=\"%s\"", UBX_CELL_FILE_SYSTEM_DELETE_FILE, filename.c_str());

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    if (err != UBX_CELL_ERROR_SUCCESS)
    {
        if (_printDebug == true)
        {
            _debugPort->print(F("deleteFile: Fail: Error: "));
            _debugPort->println(err);
        }
    }

    free(command);
    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::modulePowerOff(void)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_COMMAND_POWER_OFF) + 6;
    char command[cmdLen];

    snprintf(command, cmdLen, "%s", UBX_CELL_COMMAND_POWER_OFF);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_POWER_OFF_TIMEOUT);

    return err;
}

void SparkFun_ublox_Cellular::modulePowerOn(void)
{
    if (_powerPin >= 0)
    {
        powerOn();
    }
    else
    {
        if (_printDebug == true)
            _debugPort->println(F("modulePowerOn: not supported. _powerPin not defined."));
    }
}

/////////////
// Private //
/////////////

UBX_CELL_error_t SparkFun_ublox_Cellular::init(unsigned long baud, SparkFun_ublox_Cellular::UBX_CELL_init_type_t initType)
{
    int retries = _maxInitTries;
    UBX_CELL_error_t err = UBX_CELL_ERROR_SUCCESS;

    beginSerial(baud);

    do
    {
        if (_printDebug == true)
            _debugPort->println(F("init: Begin module init."));

        if (initType == UBX_CELL_INIT_AUTOBAUD)
        {
            if (_printDebug == true)
                _debugPort->println(F("init: Attempting autobaud connection to module."));

            err = autobaud(baud);

            if (err != UBX_CELL_ERROR_SUCCESS)
            {
                initType = UBX_CELL_INIT_RESET;
            }
        }
        else if (initType == UBX_CELL_INIT_RESET)
        {
            if (_printDebug == true)
                _debugPort->println(F("init: Power cycling module."));

            powerOff();
            delay(UBX_CELL_POWER_OFF_PULSE_PERIOD);
            powerOn();
            beginSerial(baud);
            delay(2000);

            err = at();
            if (err != UBX_CELL_ERROR_SUCCESS)
            {
                initType = UBX_CELL_INIT_AUTOBAUD;
            }
        }
        if (err == UBX_CELL_ERROR_SUCCESS)
        {
            err = enableEcho(false); // = disableEcho
            if (err != UBX_CELL_ERROR_SUCCESS)
            {
                if (_printDebug == true)
                    _debugPort->println(F("init: Module failed echo test."));
                initType = UBX_CELL_INIT_AUTOBAUD;
            }
        }
    } while ((retries--) && (err != UBX_CELL_ERROR_SUCCESS));

    // we tried but seems failed
    if (err != UBX_CELL_ERROR_SUCCESS)
    {
        if (_printDebug == true)
            _debugPort->println(F("init: Module failed to init. Exiting."));
        return (UBX_CELL_ERROR_NO_RESPONSE);
    }

    if (_printDebug == true)
        _debugPort->println(F("init: Module responded successfully."));

    _baud = baud;
    setGpioMode(GPIO1, NETWORK_STATUS);
    // setGpioMode(GPIO2, GNSS_SUPPLY_ENABLE);
    setGpioMode(GPIO6, TIME_PULSE_OUTPUT);
    setSMSMessageFormat(UBX_CELL_MESSAGE_FORMAT_TEXT);
    autoTimeZone(_autoTimeZoneForBegin);
    for (int i = 0; i < UBX_CELL_NUM_SOCKETS; i++)
    {
        socketClose(i, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    }

    return UBX_CELL_ERROR_SUCCESS;
}

void SparkFun_ublox_Cellular::invertPowerPin(bool invert)
{
    _invertPowerPin = invert;
}

// Do a graceful power off. Hold the PWR_ON pin low for UBX_CELL_POWER_OFF_PULSE_PERIOD
// Note: +CPWROFF () is preferred to this.
void SparkFun_ublox_Cellular::powerOff(void)
{
    if (_powerPin >= 0)
    {
        if (_invertPowerPin) // Set the pin state before making it an output
            digitalWrite(_powerPin, HIGH);
        else
            digitalWrite(_powerPin, LOW);
        pinMode(_powerPin, OUTPUT);
        if (_invertPowerPin) // Set the pin state
            digitalWrite(_powerPin, HIGH);
        else
            digitalWrite(_powerPin, LOW);
        delay(UBX_CELL_POWER_OFF_PULSE_PERIOD);
        pinMode(_powerPin, INPUT); // Return to high-impedance, rely on (e.g.) SARA module internal pull-up
        if (_printDebug == true)
            _debugPort->println(F("powerOff: complete"));
    }
}

void SparkFun_ublox_Cellular::powerOn(void)
{
    if (_powerPin >= 0)
    {
        if (_invertPowerPin) // Set the pin state before making it an output
            digitalWrite(_powerPin, HIGH);
        else
            digitalWrite(_powerPin, LOW);
        pinMode(_powerPin, OUTPUT);
        if (_invertPowerPin) // Set the pin state
            digitalWrite(_powerPin, HIGH);
        else
            digitalWrite(_powerPin, LOW);
        delay(UBX_CELL_POWER_ON_PULSE_PERIOD);
        pinMode(_powerPin, INPUT); // Return to high-impedance, rely on (e.g.) SARA module internal pull-up
        // delay(2000);               // Do this in init. Wait before sending AT commands to module. 100 is too short.
        if (_printDebug == true)
            _debugPort->println(F("powerOn: complete"));
    }
}

// This does an abrupt emergency hardware shutdown of the SARA-R5 series modules.
// It only works if you have access to both the RESET_N and PWR_ON pins.
// You cannot use this function on the SparkFun Asset Tracker and RESET_N is tied to the MicroMod processor !RESET!...
void SparkFun_ublox_Cellular::hwReset(void)
{
    if ((_resetPin >= 0) && (_powerPin >= 0))
    {
        digitalWrite(_resetPin, HIGH); // Start by making sure the RESET_N pin is high
        pinMode(_resetPin, OUTPUT);
        digitalWrite(_resetPin, HIGH);

        if (_invertPowerPin) // Now pull PWR_ON low - invert as necessary (on the Asset Tracker)
        {
            digitalWrite(_powerPin, HIGH); // Inverted - Asset Tracker
            pinMode(_powerPin, OUTPUT);
            digitalWrite(_powerPin, HIGH);
        }
        else
        {
            digitalWrite(_powerPin, LOW); // Not inverted
            pinMode(_powerPin, OUTPUT);
            digitalWrite(_powerPin, LOW);
        }

        delay(UBX_CELL_RESET_PULSE_PERIOD); // Wait 23 seconds... (Yes, really!)

        digitalWrite(_resetPin, LOW); // Now pull RESET_N low

        delay(100); // Wait a little... (The data sheet doesn't say how long for)

        if (_invertPowerPin) // Now pull PWR_ON high - invert as necessary (on the Asset Tracker)
        {
            digitalWrite(_powerPin, LOW); // Inverted - Asset Tracker
        }
        else
        {
            digitalWrite(_powerPin, HIGH); // Not inverted
        }

        delay(1500); // Wait 1.5 seconds

        digitalWrite(_resetPin, HIGH); // Now pull RESET_N high again

        pinMode(_resetPin, INPUT); // Return to high-impedance, rely on SARA module internal pull-up
        pinMode(_powerPin, INPUT); // Return to high-impedance, rely on SARA module internal pull-up
    }
}

UBX_CELL_error_t SparkFun_ublox_Cellular::functionality(UBX_CELL_functionality_t function)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_COMMAND_FUNC) + 16;
    char command[cmdLen];

    snprintf(command, cmdLen, "%s=%d", UBX_CELL_COMMAND_FUNC, function);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_3_MIN_TIMEOUT);

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::setMNOprofile(mobile_network_operator_t mno, bool autoReset, bool urcNotification)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_COMMAND_MNO) + 9;
    char command[cmdLen];

    if (mno == MNO_SIM_ICCID) // Only add autoReset and urcNotification if mno is MNO_SIM_ICCID
        snprintf(command, cmdLen, "%s=%d,%d,%d", UBX_CELL_COMMAND_MNO, (uint8_t)mno, (uint8_t)autoReset,
                 (uint8_t)urcNotification);
    else
        snprintf(command, cmdLen, "%s=%d", UBX_CELL_COMMAND_MNO, (uint8_t)mno);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::getMNOprofile(mobile_network_operator_t *mno)
{
    UBX_CELL_error_t err;
    size_t cmdLen = strlen(UBX_CELL_COMMAND_MNO) + 2;
    char command[cmdLen];
    char response[minimumResponseAllocation];
    mobile_network_operator_t o;
    int d;
    int r;
    int u;
    int oStore;

    snprintf(command, cmdLen, "%s?", UBX_CELL_COMMAND_MNO);

    err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK_OR_ERROR, response, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
    if (err != UBX_CELL_ERROR_SUCCESS)
        return err;

    int scanned = 0;
    char *searchPtr = strnstr(response, "+UMNOPROF:", minimumResponseAllocation);
    if (searchPtr != nullptr)
    {
        searchPtr += strlen("+UMNOPROF:"); //  Move searchPtr to first char
        while (*searchPtr == ' ')
            searchPtr++; // skip spaces
        scanned = sscanf(searchPtr, "%d,%d,%d,%d", &oStore, &d, &r, &u);
    }
    o = (mobile_network_operator_t)oStore;

    if (scanned >= 1)
    {
        if (_printDebug == true)
        {
            _debugPort->print(F("getMNOprofile: MNO is: "));
            _debugPort->println(o);
        }
        *mno = o;
    }
    else
    {
        err = UBX_CELL_ERROR_INVALID;
    }

    return err;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::waitForResponse(const char *expectedResponse, const char *expectedError, uint16_t timeout)
{
    unsigned long timeIn;
    bool found = false;
    bool error = false;
    int responseIndex = 0, errorIndex = 0;
    // bool printedSomething = false;

    timeIn = millis();

    int responseLen = (int)strlen(expectedResponse);
    int errorLen = (int)strlen(expectedError);

    while ((!found) && ((timeIn + timeout) > millis()))
    {
        if (hwAvailable() > 0) // hwAvailable can return -1 if the serial port is nullptr
        {
            char c = readChar();
            // if (_printDebug == true)
            // {
            //   if (printedSomething == false)
            //     _debugPort->print(F("waitForResponse: "));
            //   _debugPort->write(c);
            //   printedSomething = true;
            // }
            if ((responseIndex < responseLen) && (c == expectedResponse[responseIndex]))
            {
                if (++responseIndex == responseLen)
                {
                    found = true;
                }
            }
            else
            {
                responseIndex = ((responseIndex < responseLen) && (c == expectedResponse[0])) ? 1 : 0;
            }
            if ((errorIndex < errorLen) && (c == expectedError[errorIndex]))
            {
                if (++errorIndex == errorLen)
                {
                    error = true;
                    found = true;
                }
            }
            else
            {
                errorIndex = ((errorIndex < errorLen) && (c == expectedError[0])) ? 1 : 0;
            }
            //_saraResponseBacklog is a global array that holds the backlog of any events
            // that came in while waiting for response. To be processed later within bufferedPoll().
            // Note: the expectedResponse or expectedError will also be added to the backlog.
            // The backlog is only used by bufferedPoll to process the URCs - which are all readable.
            // bufferedPoll uses strtok - which does not like nullptr characters.
            // So let's make sure no NULLs end up in the backlog!
            if (_saraResponseBacklogLength < _RXBuffSize) // Don't overflow the buffer
            {
                if (c == '\0')
                    _saraResponseBacklog[_saraResponseBacklogLength++] = '0'; // Change NULLs to ASCII Zeros
                else
                    _saraResponseBacklog[_saraResponseBacklogLength++] = c;
            }
        }
        else
        {
            yield();
        }
    }

    // if (_printDebug == true)
    //   if (printedSomething)
    //     _debugPort->println();

    pruneBacklog(); // Prune any incoming non-actionable URC's and responses/errors from the backlog

    if (found == true)
    {
        if (true == _printAtDebug)
        {
            _debugAtPort->print((error == true) ? expectedError : expectedResponse);
        }

        return (error == true) ? UBX_CELL_ERROR_ERROR : UBX_CELL_ERROR_SUCCESS;
    }

    return UBX_CELL_ERROR_NO_RESPONSE;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::sendCommandWithResponse(const char *command, const char *expectedResponse,
                                                   char *responseDest, unsigned long commandTimeout, int destSize,
                                                   bool at)
{
    bool found = false;
    bool error = false;
    int responseIndex = 0;
    int errorIndex = 0;
    int destIndex = 0;
    unsigned int charsRead = 0;
    int responseLen = 0;
    int errorLen = 0;
    const char *expectedError = nullptr;
    bool printResponse = false; // Change to true to print the full response
    bool printedSomething = false;

    if (_printDebug == true)
    {
        _debugPort->print(F("sendCommandWithResponse: Command: "));
        _debugPort->println(String(command));
    }

    sendCommand(command, at); // Sending command needs to dump data to backlog buffer as well.
    unsigned long timeIn = millis();
    if (UBX_CELL_RESPONSE_OK_OR_ERROR == expectedResponse)
    {
        expectedResponse = UBX_CELL_RESPONSE_OK;
        expectedError = UBX_CELL_RESPONSE_ERROR;
        responseLen = sizeof(UBX_CELL_RESPONSE_OK) - 1;
        errorLen = sizeof(UBX_CELL_RESPONSE_ERROR) - 1;
    }
    else
    {
        responseLen = (int)strlen(expectedResponse);
    }

    while ((!found) && ((timeIn + commandTimeout) > millis()))
    {
        if (hwAvailable() > 0) // hwAvailable can return -1 if the serial port is nullptr
        {
            char c = readChar();
            if ((printResponse = true) && (_printDebug == true))
            {
                if (printedSomething == false)
                {
                    _debugPort->print(F("sendCommandWithResponse: Response: "));
                    printedSomething = true;
                }
                _debugPort->write(c);
            }
            if (responseDest != nullptr)
            {
                if (destIndex < destSize) // Only add this char to response if there is room for it
                    responseDest[destIndex] = c;
                destIndex++;
                if (destIndex == destSize)
                {
                    if (_printDebug == true)
                    {
                        if ((printResponse = true) && (printedSomething))
                            _debugPort->println();
                        _debugPort->print(F("sendCommandWithResponse: Panic! responseDest is full!"));
                        if ((printResponse = true) && (printedSomething))
                            _debugPort->print(F("sendCommandWithResponse: Ignored response: "));
                    }
                }
            }
            charsRead++;
            if ((errorIndex < errorLen) && (c == expectedError[errorIndex]))
            {
                if (++errorIndex == errorLen)
                {
                    error = true;
                    found = true;
                }
            }
            else
            {
                errorIndex = ((errorIndex < errorLen) && (c == expectedError[0])) ? 1 : 0;
            }
            if ((responseIndex < responseLen) && (c == expectedResponse[responseIndex]))
            {
                if (++responseIndex == responseLen)
                {
                    found = true;
                }
            }
            else
            {
                responseIndex = ((responseIndex < responseLen) && (c == expectedResponse[0])) ? 1 : 0;
            }
            //_saraResponseBacklog is a global array that holds the backlog of any events
            // that came in while waiting for response. To be processed later within bufferedPoll().
            // Note: the expectedResponse or expectedError will also be added to the backlog
            // The backlog is only used by bufferedPoll to process the URCs - which are all readable.
            // bufferedPoll uses strtok - which does not like NULL characters.
            // So let's make sure no NULLs end up in the backlog!
            if (_saraResponseBacklogLength < _RXBuffSize) // Don't overflow the buffer
            {
                if (c == '\0')
                    _saraResponseBacklog[_saraResponseBacklogLength++] = '0'; // Change NULLs to ASCII Zeros
                else
                    _saraResponseBacklog[_saraResponseBacklogLength++] = c;
            }
        }
        else
        {
            yield();
        }
    }

    if (_printDebug == true)
        if ((printResponse = true) && (printedSomething))
            _debugPort->println();

    pruneBacklog(); // Prune any incoming non-actionable URC's and responses/errors from the backlog

    if (found)
    {
        if ((true == _printAtDebug) && ((nullptr != responseDest) || (nullptr != expectedResponse)))
        {
            _debugAtPort->print((nullptr != responseDest) ? responseDest : expectedResponse);
        }
        return error ? UBX_CELL_ERROR_ERROR : UBX_CELL_ERROR_SUCCESS;
    }
    else if (charsRead == 0)
    {
        return UBX_CELL_ERROR_NO_RESPONSE;
    }
    else
    {
        if ((true == _printAtDebug) && (nullptr != responseDest))
        {
            _debugAtPort->print(responseDest);
        }
        return UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
    }
}

// Send a custom command with an expected (potentially partial) response, store entire response
UBX_CELL_error_t SparkFun_ublox_Cellular::sendCustomCommandWithResponse(const char *command, const char *expectedResponse,
                                                         char *responseDest, unsigned long commandTimeout, bool at)
{
    // Assume the user has allocated enough storage for any response. Set destSize to 32766.
    return sendCommandWithResponse(command, expectedResponse, responseDest, commandTimeout, 32766, at);
}

void SparkFun_ublox_Cellular::sendCommand(const char *command, bool at)
{
    // Check for incoming serial data. Copy it into the backlog

    // Important note:
    // On ESP32, Serial.available only provides an update every ~120 bytes during the reception of long messages:
    // https://gitter.im/espressif/arduino-esp32?at=5e25d6370a1cf54144909c85
    // Be aware that if a long message is being received, the code below will timeout after _rxWindowMillis = 2 millis.
    // At 115200 baud, hwAvailable takes ~120 * 10 / 115200 = 10.4 millis before it indicates that data is being
    // received.

    unsigned long timeIn = millis();
    if (hwAvailable() > 0) // hwAvailable can return -1 if the serial port is NULL
    {
        while (((millis() - timeIn) < _rxWindowMillis) &&
               (_saraResponseBacklogLength < _RXBuffSize)) // May need to escape on newline?
        {
            if (hwAvailable() > 0) // hwAvailable can return -1 if the serial port is NULL
            {
                //_saraResponseBacklog is a global array that holds the backlog of any events
                // that came in while waiting for response. To be processed later within bufferedPoll().
                // Note: the expectedResponse or expectedError will also be added to the backlog
                // The backlog is only used by bufferedPoll to process the URCs - which are all readable.
                // bufferedPoll uses strtok - which does not like NULL characters.
                // So let's make sure no NULLs end up in the backlog!
                char c = readChar();
                if (c == '\0') // Make sure no NULL characters end up in the backlog! Change them to ASCII Zeros
                    c = '0';
                _saraResponseBacklog[_saraResponseBacklogLength++] = c;
                timeIn = millis();
            }
            else
            {
                yield();
            }
        }
    }

    // Now send the command
    if (at)
    {
        hwPrint(UBX_CELL_COMMAND_AT);
        hwPrint(command);
        hwPrint("\r\n");
    }
    else
    {
        hwPrint(command);
    }
}

UBX_CELL_error_t SparkFun_ublox_Cellular::parseSocketReadIndication(int socket, int length)
{
    UBX_CELL_error_t err;
    char *readDest;

    if ((socket < 0) || (length < 0))
    {
        return UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
    }

    // Return now if both callbacks pointers are nullptr - otherwise the data will be read and lost!
    if ((_socketReadCallback == nullptr) && (_socketReadCallbackPlus == nullptr))
        return UBX_CELL_ERROR_INVALID;

    readDest = ubx_cell_calloc_char(length + 1);
    if (readDest == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;

    int bytesRead;
    err = socketRead(socket, length, readDest, &bytesRead);
    if (err != UBX_CELL_ERROR_SUCCESS)
    {
        free(readDest);
        return err;
    }

    if (_socketReadCallback != nullptr)
    {
        String dataAsString = ""; // Create an empty string
        // Copy the data from readDest into the String in a binary-compatible way
        // Important Note: some implementations of concat, like the one on ESP32, are binary-compatible.
        // But some, like SAMD, are not. They use strlen or strcpy internally - which don't like \0's.
        // The only true binary-compatible solution is to use socketReadCallbackPlus...
        for (int i = 0; i < bytesRead; i++)
            dataAsString.concat(readDest[i]);
        _socketReadCallback(socket, dataAsString);
    }

    if (_socketReadCallbackPlus != nullptr)
    {
        IPAddress dummyAddress = {0, 0, 0, 0};
        int dummyPort = 0;
        _socketReadCallbackPlus(socket, (const char *)readDest, bytesRead, dummyAddress, dummyPort);
    }

    free(readDest);
    return UBX_CELL_ERROR_SUCCESS;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::parseSocketReadIndicationUDP(int socket, int length)
{
    UBX_CELL_error_t err;
    char *readDest;
    IPAddress remoteAddress = {0, 0, 0, 0};
    int remotePort = 0;

    if ((socket < 0) || (length < 0))
    {
        return UBX_CELL_ERROR_UNEXPECTED_RESPONSE;
    }

    // Return now if both callbacks pointers are nullptr - otherwise the data will be read and lost!
    if ((_socketReadCallback == nullptr) && (_socketReadCallbackPlus == nullptr))
        return UBX_CELL_ERROR_INVALID;

    readDest = ubx_cell_calloc_char(length + 1);
    if (readDest == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;

    int bytesRead;
    err = socketReadUDP(socket, length, readDest, &remoteAddress, &remotePort, &bytesRead);
    if (err != UBX_CELL_ERROR_SUCCESS)
    {
        free(readDest);
        return err;
    }

    if (_socketReadCallback != nullptr)
    {
        String dataAsString = ""; // Create an empty string
        // Important Note: some implementations of concat, like the one on ESP32, are binary-compatible.
        // But some, like SAMD, are not. They use strlen or strcpy internally - which don't like \0's.
        // The only true binary-compatible solution is to use socketReadCallbackPlus...
        for (int i = 0; i < bytesRead; i++) // Copy the data from readDest into the String in a binary-compatible way
            dataAsString.concat(readDest[i]);
        _socketReadCallback(socket, dataAsString);
    }

    if (_socketReadCallbackPlus != nullptr)
    {
        _socketReadCallbackPlus(socket, (const char *)readDest, bytesRead, remoteAddress, remotePort);
    }

    free(readDest);
    return UBX_CELL_ERROR_SUCCESS;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::parseSocketListenIndication(int listeningSocket, IPAddress localIP,
                                                       unsigned int listeningPort, int socket, IPAddress remoteIP,
                                                       unsigned int port)
{
    _lastLocalIP = localIP;
    _lastRemoteIP = remoteIP;

    if (_socketListenCallback != nullptr)
    {
        _socketListenCallback(listeningSocket, localIP, listeningPort, socket, remoteIP, port);
    }

    return UBX_CELL_ERROR_SUCCESS;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::parseSocketCloseIndication(String *closeIndication)
{
    int search;
    int socket;

    search = closeIndication->indexOf(UBX_CELL_CLOSE_SOCKET_URC);
    search += strlen(UBX_CELL_CLOSE_SOCKET_URC);
    while (closeIndication->charAt(search) == ' ')
        search++; // skip spaces

    // Socket will be first integer, should be single-digit number between 0-6:
    socket = closeIndication->substring(search, search + 1).toInt();

    if (_socketCloseCallback != nullptr)
    {
        _socketCloseCallback(socket);
    }

    return UBX_CELL_ERROR_SUCCESS;
}

size_t SparkFun_ublox_Cellular::hwPrint(const char *s)
{
    if ((true == _printAtDebug) && (nullptr != s))
    {
        _debugAtPort->print(s);
    }
    if (_hardSerial != nullptr)
    {
        return _hardSerial->print(s);
    }
#ifdef UBX_CELL_SOFTWARE_SERIAL_ENABLED
    else if (_softSerial != nullptr)
    {
        return _softSerial->print(s);
    }
#endif

    return (size_t)0;
}

size_t SparkFun_ublox_Cellular::hwWriteData(const char *buff, int len)
{
    if ((true == _printAtDebug) && (nullptr != buff) && (0 < len))
    {
        _debugAtPort->write(buff, len);
    }
    if (_hardSerial != nullptr)
    {
        return _hardSerial->write((const uint8_t *)buff, len);
    }
#ifdef UBX_CELL_SOFTWARE_SERIAL_ENABLED
    else if (_softSerial != nullptr)
    {
        return _softSerial->write((const uint8_t *)buff, len);
    }
#endif
    return (size_t)0;
}

size_t SparkFun_ublox_Cellular::hwWrite(const char c)
{
    if (true == _printAtDebug)
    {
        _debugAtPort->write(c);
    }
    if (_hardSerial != nullptr)
    {
        return _hardSerial->write(c);
    }
#ifdef UBX_CELL_SOFTWARE_SERIAL_ENABLED
    else if (_softSerial != nullptr)
    {
        return _softSerial->write(c);
    }
#endif

    return (size_t)0;
}

int SparkFun_ublox_Cellular::readAvailable(char *inString)
{
    int len = 0;

    if (_hardSerial != nullptr)
    {
        while (_hardSerial->available())
        {
            char c = (char)_hardSerial->read();
            if (inString != nullptr)
            {
                inString[len++] = c;
            }
        }
        if (inString != nullptr)
        {
            inString[len] = 0;
        }
        // if (_printDebug == true)
        //   _debugPort->println(inString);
    }
#ifdef UBX_CELL_SOFTWARE_SERIAL_ENABLED
    else if (_softSerial != nullptr)
    {
        while (_softSerial->available())
        {
            char c = (char)_softSerial->read();
            if (inString != nullptr)
            {
                inString[len++] = c;
            }
        }
        if (inString != nullptr)
        {
            inString[len] = 0;
        }
    }
#endif

    return len;
}

char SparkFun_ublox_Cellular::readChar(void)
{
    char ret = 0;

    if (_hardSerial != nullptr)
    {
        ret = (char)_hardSerial->read();
    }
#ifdef UBX_CELL_SOFTWARE_SERIAL_ENABLED
    else if (_softSerial != nullptr)
    {
        ret = (char)_softSerial->read();
    }
#endif

    return ret;
}

int SparkFun_ublox_Cellular::hwAvailable(void)
{
    if (_hardSerial != nullptr)
    {
        return _hardSerial->available();
    }
#ifdef UBX_CELL_SOFTWARE_SERIAL_ENABLED
    else if (_softSerial != nullptr)
    {
        return _softSerial->available();
    }
#endif

    return -1;
}

void SparkFun_ublox_Cellular::beginSerial(unsigned long baud)
{
    delay(100);
    if (_hardSerial != nullptr)
    {
        _hardSerial->end();
        _hardSerial->begin(baud);
    }
#ifdef UBX_CELL_SOFTWARE_SERIAL_ENABLED
    else if (_softSerial != nullptr)
    {
        _softSerial->end();
        _softSerial->begin(baud);
    }
#endif
    delay(100);
}

void SparkFun_ublox_Cellular::setTimeout(unsigned long timeout)
{
    if (_hardSerial != nullptr)
    {
        _hardSerial->setTimeout(timeout);
    }
#ifdef UBX_CELL_SOFTWARE_SERIAL_ENABLED
    else if (_softSerial != nullptr)
    {
        _softSerial->setTimeout(timeout);
    }
#endif
}

bool SparkFun_ublox_Cellular::find(char *target)
{
    bool found = false;
    if (_hardSerial != nullptr)
    {
        found = _hardSerial->find(target);
    }
#ifdef UBX_CELL_SOFTWARE_SERIAL_ENABLED
    else if (_softSerial != nullptr)
    {
        found = _softSerial->find(target);
    }
#endif
    return found;
}

UBX_CELL_error_t SparkFun_ublox_Cellular::autobaud(unsigned long desiredBaud)
{
    UBX_CELL_error_t err = UBX_CELL_ERROR_INVALID;
    int b = 0;

    while ((err != UBX_CELL_ERROR_SUCCESS) && (b < NUM_SUPPORTED_BAUD))
    {
        beginSerial(UBX_CELL_SUPPORTED_BAUD[b++]);
        setBaud(desiredBaud);
        beginSerial(desiredBaud);
        err = at();
    }
    if (err == UBX_CELL_ERROR_SUCCESS)
    {
        beginSerial(desiredBaud);
    }
    return err;
}

char *SparkFun_ublox_Cellular::ubx_cell_calloc_char(size_t num)
{
    return (char *)calloc(num, sizeof(char));
}

// This prunes the backlog of non-actionable events. If new actionable events are added, you must modify the if
// statement.
void SparkFun_ublox_Cellular::pruneBacklog()
{
    char *event;

    // if (_printDebug == true)
    // {
    //   if (_saraResponseBacklogLength > 0) //Handy for debugging new parsing.
    //   {
    //     _debugPort->println(F("pruneBacklog: before pruning, backlog was:"));
    //     _debugPort->println(_saraResponseBacklog);
    //     _debugPort->println(F("pruneBacklog: end of backlog"));
    //   }
    //   else
    //   {
    //     _debugPort->println(F("pruneBacklog: backlog was empty"));
    //   }
    // }

    memset(_pruneBuffer, 0, _RXBuffSize); // Clear the _pruneBuffer

    _saraResponseBacklogLength = 0; // Zero the backlog length

    char *preservedEvent;
    event = strtok_r(_saraResponseBacklog, "\r\n", &preservedEvent); // Look for an 'event' - something ending in \r\n

    while (event != nullptr) // If event is actionable, add it to pruneBuffer.
    {
        // These are the events we want to keep so they can be processed by poll / bufferedPoll
        for (auto urcString : _urcStrings)
        {
            if (strnstr(event, urcString, _RXBuffSize - (event - _saraResponseBacklog)) != nullptr)
            {
                strcat(_pruneBuffer, event);  // The URCs are all readable text so using strcat is OK
                strcat(_pruneBuffer, "\r\n"); // strtok blows away delimiter, but we want that for later.
                _saraResponseBacklogLength +=
                    strlen(event) + 2; // Add the length of this event to _saraResponseBacklogLength
                break;                 // No need to check any other events
            }
        }

        event = strtok_r(nullptr, "\r\n", &preservedEvent); // Walk though any remaining events
    }

    memset(_saraResponseBacklog, 0, _RXBuffSize); // Clear out backlog buffer.
    memcpy(_saraResponseBacklog, _pruneBuffer,
           _saraResponseBacklogLength); // Copy the pruned buffer back into the backlog

    // if (_printDebug == true)
    // {
    //   if (_saraResponseBacklogLength > 0) //Handy for debugging new parsing.
    //   {
    //     _debugPort->println(F("pruneBacklog: after pruning, backlog is now:"));
    //     _debugPort->println(_saraResponseBacklog);
    //     _debugPort->println(F("pruneBacklog: end of backlog"));
    //   }
    //   else
    //   {
    //     _debugPort->println(F("pruneBacklog: backlog is now empty"));
    //   }
    // }
}

// GPS Helper Functions:

// Read a source string until a delimiter is hit, store the result in destination
char *SparkFun_ublox_Cellular::readDataUntil(char *destination, unsigned int destSize, char *source, char delimiter)
{

    char *strEnd;
    size_t len;

    strEnd = strchr(source, delimiter);

    if (strEnd != nullptr)
    {
        len = strEnd - source;
        memset(destination, 0, destSize);
        memcpy(destination, source, len);
    }

    return strEnd;
}

bool SparkFun_ublox_Cellular::parseGPRMCString(char *rmcString, PositionData *pos, ClockData *clk, SpeedData *spd)
{
    char *ptr, *search;
    unsigned long tTemp;
    char tempData[TEMP_NMEA_DATA_SIZE];

    // if (_printDebug == true)
    // {
    //   _debugPort->println(F("parseGPRMCString: rmcString: "));
    //   _debugPort->println(rmcString);
    // }

    // Fast-forward test to first value:
    ptr = strchr(rmcString, ',');
    ptr++; // Move ptr past first comma

    // If the next character is another comma, there's no time data
    // Find time:
    search = readDataUntil(tempData, TEMP_NMEA_DATA_SIZE, ptr, ',');
    // Next comma should be present and not the next position
    if ((search != nullptr) && (search != ptr))
    {
        pos->utc = atof(tempData); // Extract hhmmss.ss as float
        tTemp = pos->utc;          // Convert to unsigned long (discard the digits beyond the decimal point)
        clk->time.ms = ((unsigned int)(pos->utc * 100)) % 100; // Extract the milliseconds
        clk->time.hour = tTemp / 10000;
        tTemp -= ((unsigned long)clk->time.hour * 10000);
        clk->time.minute = tTemp / 100;
        tTemp -= ((unsigned long)clk->time.minute * 100);
        clk->time.second = tTemp;
    }
    else
    {
        pos->utc = 0.0;
        clk->time.hour = 0;
        clk->time.minute = 0;
        clk->time.second = 0;
    }
    ptr = search + 1; // Move pointer to next value

    // Find status character:
    search = readDataUntil(tempData, TEMP_NMEA_DATA_SIZE, ptr, ',');
    // Should be a single character: V = Data invalid, A = Data valid
    if ((search != nullptr) && (search == ptr + 1))
    {
        pos->status = *ptr; // Assign char at ptr to status
    }
    else
    {
        pos->status = 'X'; // Made up very bad status
    }
    ptr = search + 1;

    // Find latitude:
    search = readDataUntil(tempData, TEMP_NMEA_DATA_SIZE, ptr, ',');
    if ((search != nullptr) && (search != ptr))
    {
        pos->lat = atof(tempData);              // Extract ddmm.mmmmm as float
        unsigned long lat_deg = pos->lat / 100; // Extract the degrees
        pos->lat -= (float)lat_deg * 100.0;     // Subtract the degrees leaving only the minutes
        pos->lat /= 60.0;                       // Convert minutes into degrees
        pos->lat += (float)lat_deg;             // Finally add the degrees back on again
    }
    else
    {
        pos->lat = 0.0;
    }
    ptr = search + 1;

    // Find latitude hemishpere
    search = readDataUntil(tempData, TEMP_NMEA_DATA_SIZE, ptr, ',');
    if ((search != nullptr) && (search == ptr + 1))
    {
        if (*ptr == 'S')      // Is the latitude South
            pos->lat *= -1.0; // Make lat negative
    }
    ptr = search + 1;

    // Find longitude:
    search = readDataUntil(tempData, TEMP_NMEA_DATA_SIZE, ptr, ',');
    if ((search != nullptr) && (search != ptr))
    {
        pos->lon = atof(tempData);              // Extract dddmm.mmmmm as float
        unsigned long lon_deg = pos->lon / 100; // Extract the degrees
        pos->lon -= (float)lon_deg * 100.0;     // Subtract the degrees leaving only the minutes
        pos->lon /= 60.0;                       // Convert minutes into degrees
        pos->lon += (float)lon_deg;             // Finally add the degrees back on again
    }
    else
    {
        pos->lon = 0.0;
    }
    ptr = search + 1;

    // Find longitude hemishpere
    search = readDataUntil(tempData, TEMP_NMEA_DATA_SIZE, ptr, ',');
    if ((search != nullptr) && (search == ptr + 1))
    {
        if (*ptr == 'W')      // Is the longitude West
            pos->lon *= -1.0; // Make lon negative
    }
    ptr = search + 1;

    // Find speed
    search = readDataUntil(tempData, TEMP_NMEA_DATA_SIZE, ptr, ',');
    if ((search != nullptr) && (search != ptr))
    {
        spd->speed = atof(tempData); // Extract speed over ground in knots
        spd->speed *= 0.514444;      // Convert to m/s
    }
    else
    {
        spd->speed = 0.0;
    }
    ptr = search + 1;

    // Find course over ground
    search = readDataUntil(tempData, TEMP_NMEA_DATA_SIZE, ptr, ',');
    if ((search != nullptr) && (search != ptr))
    {
        spd->cog = atof(tempData);
    }
    else
    {
        spd->cog = 0.0;
    }
    ptr = search + 1;

    // Find date
    search = readDataUntil(tempData, TEMP_NMEA_DATA_SIZE, ptr, ',');
    if ((search != nullptr) && (search != ptr))
    {
        tTemp = atol(tempData);
        clk->date.day = tTemp / 10000;
        tTemp -= ((unsigned long)clk->date.day * 10000);
        clk->date.month = tTemp / 100;
        tTemp -= ((unsigned long)clk->date.month * 100);
        clk->date.year = tTemp;
    }
    else
    {
        clk->date.day = 0;
        clk->date.month = 0;
        clk->date.year = 0;
    }
    ptr = search + 1;

    // Find magnetic variation in degrees:
    search = readDataUntil(tempData, TEMP_NMEA_DATA_SIZE, ptr, ',');
    if ((search != nullptr) && (search != ptr))
    {
        spd->magVar = atof(tempData);
    }
    else
    {
        spd->magVar = 0.0;
    }
    ptr = search + 1;

    // Find magnetic variation direction
    search = readDataUntil(tempData, TEMP_NMEA_DATA_SIZE, ptr, ',');
    if ((search != nullptr) && (search == ptr + 1))
    {
        if (*ptr == 'W')         // Is the magnetic variation West
            spd->magVar *= -1.0; // Make magnetic variation negative
    }
    ptr = search + 1;

    // Find position system mode
    // Possible values for posMode: N = No fix, E = Estimated/Dead reckoning fix, A = Autonomous GNSS fix,
    //                              D = Differential GNSS fix, F = RTK float, R = RTK fixed
    search = readDataUntil(tempData, TEMP_NMEA_DATA_SIZE, ptr, '*');
    if ((search != nullptr) && (search = ptr + 1))
    {
        pos->mode = *ptr;
    }
    else
    {
        pos->mode = 'X';
    }
    ptr = search + 1;

    if (pos->status == 'A')
    {
        return true;
    }
    return false;
}
