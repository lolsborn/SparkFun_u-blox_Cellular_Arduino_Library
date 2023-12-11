// This file is purely for backwards compatibility with the original SARA-R5 library
// RegEx Replace: \#define\sSARA\_R5\_([A-Z0-9_]+).* \#define SARA\_R5\_$+ UBLOX\_AT\_$+

/*
  Arduino library for the u-blox SARA-R5 LTE-M / NB-IoT modules with secure cloud, as used on the SparkFun MicroMod Asset Tracker
  By: Paul Clark
  October 19th 2020

  Based extensively on the:
  Arduino Library for the SparkFun LTE CAT M1/NB-IoT Shield - SARA-R4
  Written by Jim Lindblom @ SparkFun Electronics, September 5, 2018

  This Arduino library provides mechanisms to initialize and use
  the SARA-R5 module over either a SoftwareSerial or hardware serial port.

  Please see LICENSE.md for the license information

*/

#ifndef SPARKFUN_SARA_R5_ARDUINO_LIBRARY_H
#define SPARKFUN_SARA_R5_ARDUINO_LIBRARY_H

#include "sfe_sara_r5.h"
#include "sfe_ublox_cellular.h"

#define SARA_R5_POWER_PIN -1 // Default to no pin
#define SARA_R5_RESET_PIN -1

// Timing
#define SARA_R5_STANDARD_RESPONSE_TIMEOUT 1000
#define SARA_R5_10_SEC_TIMEOUT 10000
#define SARA_R5_55_SECS_TIMEOUT 55000
#define SARA_R5_2_MIN_TIMEOUT 120000
#define SARA_R5_3_MIN_TIMEOUT 180000
#define SARA_R5_SET_BAUD_TIMEOUT 500
#define SARA_R5_POWER_OFF_PULSE_PERIOD 3200 // Hold PWR_ON low for this long to power the module off
#define SARA_R5_POWER_ON_PULSE_PERIOD 100 // Hold PWR_ON low for this long to power the module on (SARA-R510M8S)
#define SARA_R5_RESET_PULSE_PERIOD 23000 // Used to perform an abrupt emergency hardware shutdown. 23 seconds... (Yes, really!)
#define SARA_R5_POWER_OFF_TIMEOUT 40000 // Datasheet says 40 seconds...
#define SARA_R5_IP_CONNECT_TIMEOUT 130000
#define SARA_R5_POLL_DELAY 1
#define SARA_R5_SOCKET_WRITE_TIMEOUT 10000

// ## Suported AT Commands
// ### General
#define SARA_R5_COMMAND_AT UBX_CELL_COMMAND_AT
#define SARA_R5_COMMAND_ECHO UBX_CELL_COMMAND_ECHO
#define SARA_R5_COMMAND_MANU_ID UBX_CELL_COMMAND_MANU_ID
#define SARA_R5_COMMAND_MODEL_ID UBX_CELL_COMMAND_MODEL_ID
#define SARA_R5_COMMAND_FW_VER_ID UBX_CELL_COMMAND_FW_VER_ID
#define SARA_R5_COMMAND_SERIAL_NO UBX_CELL_COMMAND_SERIAL_NO
#define SARA_R5_COMMAND_IMEI UBX_CELL_COMMAND_IMEI
#define SARA_R5_COMMAND_IMSI UBX_CELL_COMMAND_IMSI
#define SARA_R5_COMMAND_CCID UBX_CELL_COMMAND_CCID
#define SARA_R5_COMMAND_REQ_CAP UBX_CELL_COMMAND_REQ_CAP
// ### Control and status
#define SARA_R5_COMMAND_POWER_OFF UBX_CELL_COMMAND_POWER_OFF
#define SARA_R5_COMMAND_FUNC UBX_CELL_COMMAND_FUNC
#define SARA_R5_COMMAND_CLOCK UBX_CELL_COMMAND_CLOCK
#define SARA_R5_COMMAND_AUTO_TZ UBX_CELL_COMMAND_AUTO_TZ
#define SARA_R5_COMMAND_TZ_REPORT UBX_CELL_COMMAND_TZ_REPORT
// ### Network service
#define SARA_R5_COMMAND_CNUM UBX_CELL_COMMAND_CNUM
#define SARA_R5_SIGNAL_QUALITY UBX_CELL_SIGNAL_QUALITY
#define SARA_R5_EXT_SIGNAL_QUALITY UBX_CELL_EXT_SIGNAL_QUALITY
#define SARA_R5_OPERATOR_SELECTION UBX_CELL_OPERATOR_SELECTION
#define SARA_R5_REGISTRATION_STATUS UBX_CELL_REGISTRATION_STATUS
#define SARA_R5_EPSREGISTRATION_STATUS UBX_CELL_EPSREGISTRATION_STATUS
#define SARA_R5_READ_OPERATOR_NAMES UBX_CELL_READ_OPERATOR_NAMES
#define SARA_R5_COMMAND_MNO UBX_CELL_COMMAND_MNO
// ### SIM
#define SARA_R5_SIM_STATE UBX_CELL_SIM_STATE
#define SARA_R5_COMMAND_SIMPIN UBX_CELL_COMMAND_SIMPIN
// ### SMS
#define SARA_R5_MESSAGE_FORMAT UBX_CELL_MESSAGE_FORMAT
#define SARA_R5_SEND_TEXT UBX_CELL_SEND_TEXT
#define SARA_R5_NEW_MESSAGE_IND UBX_CELL_NEW_MESSAGE_IND
#define SARA_R5_PREF_MESSAGE_STORE UBX_CELL_PREF_MESSAGE_STORE
#define SARA_R5_READ_TEXT_MESSAGE UBX_CELL_READ_TEXT_MESSAGE
#define SARA_R5_DELETE_MESSAGE UBX_CELL_DELETE_MESSAGE
// V24 control and V25ter (UART interface)
#define SARA_R5_FLOW_CONTROL UBX_CELL_FLOW_CONTROL
#define SARA_R5_COMMAND_BAUD UBX_CELL_COMMAND_BAUD
// ### Packet switched data services
#define SARA_R5_MESSAGE_PDP_DEF UBX_CELL_MESSAGE_PDP_DEF
#define SARA_R5_MESSAGE_PDP_CONFIG UBX_CELL_MESSAGE_PDP_CONFIG
#define SARA_R5_MESSAGE_PDP_ACTION UBX_CELL_MESSAGE_PDP_ACTION
#define SARA_R5_MESSAGE_PDP_CONTEXT_ACTIVATE UBX_CELL_MESSAGE_PDP_CONTEXT_ACTIVATE
#define SARA_R5_MESSAGE_ENTER_PPP UBX_CELL_MESSAGE_ENTER_PPP
#define SARA_R5_NETWORK_ASSIGNED_DATA UBX_CELL_NETWORK_ASSIGNED_DATA
// ### GPIO
#define SARA_R5_COMMAND_GPIO UBX_CELL_COMMAND_GPIO
// ### IP
#define SARA_R5_CREATE_SOCKET UBX_CELL_CREATE_SOCKET
#define SARA_R5_CLOSE_SOCKET UBX_CELL_CLOSE_SOCKET
#define SARA_R5_CONNECT_SOCKET UBX_CELL_CONNECT_SOCKET
#define SARA_R5_WRITE_SOCKET UBX_CELL_WRITE_SOCKET
#define SARA_R5_WRITE_UDP_SOCKET UBX_CELL_WRITE_UDP_SOCKET
#define SARA_R5_READ_SOCKET UBX_CELL_READ_SOCKET
#define SARA_R5_READ_UDP_SOCKET UBX_CELL_READ_UDP_SOCKET
#define SARA_R5_LISTEN_SOCKET UBX_CELL_LISTEN_SOCKET
#define SARA_R5_GET_ERROR UBX_CELL_GET_ERROR
#define SARA_R5_SOCKET_DIRECT_LINK UBX_CELL_SOCKET_DIRECT_LINK
#define SARA_R5_SOCKET_CONTROL UBX_CELL_SOCKET_CONTROL
#define SARA_R5_UD_CONFIGURATION UBX_CELL_UD_CONFIGURATION
// ### Ping
#define SARA_R5_PING_COMMAND UBX_CELL_PING_COMMAND
// ### HTTP
#define SARA_R5_HTTP_PROFILE UBX_CELL_HTTP_PROFILE
#define SARA_R5_HTTP_COMMAND UBX_CELL_HTTP_COMMAND
#define SARA_R5_HTTP_PROTOCOL_ERROR UBX_CELL_HTTP_PROTOCOL_ERROR

#define SARA_R5_MQTT_NVM UBX_CELL_MQTT_NVM
#define SARA_R5_MQTT_PROFILE UBX_CELL_MQTT_PROFILE
#define SARA_R5_MQTT_COMMAND UBX_CELL_MQTT_COMMAND
#define SARA_R5_MQTT_PROTOCOL_ERROR UBX_CELL_MQTT_PROTOCOL_ERROR
// ### FTP
#define SARA_R5_FTP_PROFILE UBX_CELL_FTP_PROFILE
#define SARA_R5_FTP_COMMAND UBX_CELL_FTP_COMMAND
#define SARA_R5_FTP_PROTOCOL_ERROR UBX_CELL_FTP_PROTOCOL_ERROR
// ### GNSS
#define SARA_R5_GNSS_POWER UBX_CELL_GNSS_POWER
#define SARA_R5_GNSS_ASSISTED_IND UBX_CELL_GNSS_ASSISTED_IND
#define SARA_R5_GNSS_REQUEST_LOCATION UBX_CELL_GNSS_REQUEST_LOCATION
#define SARA_R5_GNSS_GPRMC UBX_CELL_GNSS_GPRMC
#define SARA_R5_GNSS_CONFIGURE_SENSOR UBX_CELL_GNSS_CONFIGURE_SENSOR
#define SARA_R5_GNSS_CONFIGURE_LOCATION UBX_CELL_GNSS_CONFIGURE_LOCATION
#define SARA_R5_AIDING_SERVER_CONFIGURATION UBX_CELL_AIDING_SERVER_CONFIGURATION
// ### File System
// TO DO: Add support for file tags. Default tag to USER
#define SARA_R5_FILE_SYSTEM_READ_FILE UBX_CELL_FILE_SYSTEM_READ_FILE
#define SARA_R5_FILE_SYSTEM_READ_BLOCK UBX_CELL_FILE_SYSTEM_READ_BLOCK
#define SARA_R5_FILE_SYSTEM_DOWNLOAD_FILE UBX_CELL_FILE_SYSTEM_DOWNLOAD_FILE
#define SARA_R5_FILE_SYSTEM_LIST_FILES UBX_CELL_FILE_SYSTEM_LIST_FILES
#define SARA_R5_FILE_SYSTEM_DELETE_FILE UBX_CELL_FILE_SYSTEM_DELETE_FILE
// ### File System
// TO DO: Add support for file tags. Default tag to USER
#define SARA_R5_SEC_PROFILE UBX_CELL_SEC_PROFILE
#define SARA_R5_SEC_MANAGER UBX_CELL_SEC_MANAGER


// ### URC strings
#define SARA_R5_READ_SOCKET_URC UBX_CELL_READ_SOCKET_URC
#define SARA_R5_READ_UDP_SOCKET_URC UBX_CELL_READ_UDP_SOCKET_URC
#define SARA_R5_LISTEN_SOCKET_URC UBX_CELL_LISTEN_SOCKET_URC
#define SARA_R5_CLOSE_SOCKET_URC UBX_CELL_CLOSE_SOCKET_URC
#define SARA_R5_GNSS_REQUEST_LOCATION_URC UBX_CELL_GNSS_REQUEST_LOCATION_URC
#define SARA_R5_SIM_STATE_URC UBX_CELL_SIM_STATE_URC
#define SARA_R5_MESSAGE_PDP_ACTION_URC UBX_CELL_MESSAGE_PDP_ACTION_URC
#define SARA_R5_HTTP_COMMAND_URC UBX_CELL_HTTP_COMMAND_URC
#define SARA_R5_MQTT_COMMAND_URC UBX_CELL_MQTT_COMMAND_URC
#define SARA_R5_PING_COMMAND_URC UBX_CELL_PING_COMMAND_URC
#define SARA_R5_REGISTRATION_STATUS_URC UBX_CELL_REGISTRATION_STATUS_URC
#define SARA_R5_EPSREGISTRATION_STATUS_URC UBX_CELL_EPSREGISTRATION_STATUS_URC
#define SARA_R5_FTP_COMMAND_URC UBX_CELL_FTP_COMMAND_URC

// ### Response
#define SARA_R5_RESPONSE_MORE UBX_CELL_RESPONSE_MORE
#define SARA_R5_RESPONSE_OK UBX_CELL_RESPONSE_OK
#define SARA_R5_RESPONSE_ERROR UBX_CELL_RESPONSE_ERROR
#define SARA_R5_RESPONSE_CONNECT UBX_CELL_RESPONSE_CONNECT
#define SARA_R5_RESPONSE_OK_OR_ERROR nullptr

#define SARA_R5_NUM_SOCKETS 6

#define SARA_R5_NUM_SUPPORTED_BAUD 6
const unsigned long SARA_R5_SUPPORTED_BAUD[SARA_R5_NUM_SUPPORTED_BAUD] =
    {
        115200,
        9600,
        19200,
        38400,
        57600,
        230400};
#define SARA_R5_DEFAULT_BAUD_RATE 115200

// Flow control definitions for AT&K
// Note: SW (XON/XOFF) flow control is not supported on the SARA_R5
#define SARA_R5_DISABLE_FLOW_CONTROL UBX_CELL_DISABLE_FLOW_CONTROL
#define SARA_R5_ENABLE_FLOW_CONTROL UBX_CELL_ENABLE_FLOW_CONTROL

#define SARA_R5_ERROR_INVALID UBX_CELL_ERROR_INVALID
#define SARA_R5_ERROR_SUCCESS UBX_CELL_ERROR_SUCCESS
#define SARA_R5_ERROR_OUT_OF_MEMORY UBX_CELL_ERROR_OUT_OF_MEMORY
#define SARA_R5_ERROR_TIMEOUT UBX_CELL_ERROR_TIMEOUT
#define SARA_R5_ERROR_UNEXPECTED_PARAM UBX_CELL_ERROR_UNEXPECTED_PARAM
#define SARA_R5_ERROR_UNEXPECTED_RESPONSE UBX_CELL_ERROR_UNEXPECTED_RESPONSE
#define SARA_R5_ERROR_NO_RESPONSE UBX_CELL_ERROR_NO_RESPONSE
#define SARA_R5_ERROR_DEREGISTERED UBX_CELL_ERROR_DEREGISTERED
#define SARA_R5_ERROR_ZERO_READ_LENGTH UBX_CELL_ERROR_ZERO_READ_LENGTH
#define SARA_R5_ERROR_ERROR UBX_CELL_ERROR_ERROR
#define SARA_R5_SUCCESS SARA_R5_ERROR_SUCCESS

#define SARA_R5_REGISTRATION_INVALID UBX_CELL_REGISTRATION_INVALID
#define SARA_R5_REGISTRATION_NOT_REGISTERED UBX_CELL_REGISTRATION_NOT_REGISTERED
#define SARA_R5_REGISTRATION_HOME UBX_CELL_REGISTRATION_HOME
#define SARA_R5_REGISTRATION_SEARCHING UBX_CELL_REGISTRATION_SEARCHING
#define SARA_R5_REGISTRATION_DENIED UBX_CELL_REGISTRATION_DENIED
#define SARA_R5_REGISTRATION_UNKNOWN UBX_CELL_REGISTRATION_UNKNOWN
#define SARA_R5_REGISTRATION_ROAMING UBX_CELL_REGISTRATION_ROAMING
#define SARA_R5_REGISTRATION_HOME_SMS_ONLY UBX_CELL_REGISTRATION_HOME_SMS_ONLY
#define SARA_R5_REGISTRATION_ROAMING_SMS_ONLY UBX_CELL_REGISTRATION_ROAMING_SMS_ONLY
#define SARA_R5_REGISTRATION_EMERGENCY_SERV_ONLY UBX_CELL_REGISTRATION_EMERGENCY_SERV_ONLY
#define SARA_R5_REGISTRATION_HOME_CSFB_NOT_PREFERRED UBX_CELL_REGISTRATION_HOME_CSFB_NOT_PREFERRED
#define SARA_R5_REGISTRATION_ROAMING_CSFB_NOT_PREFERRED UBX_CELL_REGISTRATION_ROAMING_CSFB_NOT_PREFERRED

#define SARA_R5_TCP UBX_CELL_TCP
#define SARA_R5_UDP UBX_CELL_UDP

#define SARA_R5_TCP_SOCKET_STATUS_INACTIVE UBX_CELL_TCP_SOCKET_STATUS_INACTIVE
#define SARA_R5_TCP_SOCKET_STATUS_LISTEN UBX_CELL_TCP_SOCKET_STATUS_LISTEN
#define SARA_R5_TCP_SOCKET_STATUS_SYN_SENT UBX_CELL_TCP_SOCKET_STATUS_SYN_SENT
#define SARA_R5_TCP_SOCKET_STATUS_SYN_RCVD UBX_CELL_TCP_SOCKET_STATUS_SYN_RCVD
#define SARA_R5_TCP_SOCKET_STATUS_ESTABLISHED UBX_CELL_TCP_SOCKET_STATUS_ESTABLISHED
#define SARA_R5_TCP_SOCKET_STATUS_FIN_WAIT_1 UBX_CELL_TCP_SOCKET_STATUS_FIN_WAIT_1
#define SARA_R5_TCP_SOCKET_STATUS_FIN_WAIT_2 UBX_CELL_TCP_SOCKET_STATUS_FIN_WAIT_2
#define SARA_R5_TCP_SOCKET_STATUS_CLOSE_WAIT UBX_CELL_TCP_SOCKET_STATUS_CLOSE_WAIT
#define SARA_R5_TCP_SOCKET_STATUS_CLOSING UBX_CELL_TCP_SOCKET_STATUS_CLOSING
#define SARA_R5_TCP_SOCKET_STATUS_LAST_ACK UBX_CELL_TCP_SOCKET_STATUS_LAST_ACK
#define SARA_R5_TCP_SOCKET_STATUS_TIME_WAIT UBX_CELL_TCP_SOCKET_STATUS_TIME_WAIT

#define SARA_R5_MESSAGE_FORMAT_PDU UBX_CELL_MESSAGE_FORMAT_PDU
#define SARA_R5_MESSAGE_FORMAT_TEXT UBX_CELL_MESSAGE_FORMAT_TEXT

#define SARA_R5_UTIME_MODE_STOP UBX_CELL_UTIME_MODE_STOP
#define SARA_R5_UTIME_MODE_PPS UBX_CELL_UTIME_MODE_PPS
#define SARA_R5_UTIME_MODE_ONE_SHOT UBX_CELL_UTIME_MODE_ONE_SHOT
#define SARA_R5_UTIME_MODE_EXT_INT UBX_CELL_UTIME_MODE_EXT_INT

#define SARA_R5_UTIME_SENSOR_NONE UBX_CELL_UTIME_SENSOR_NONE
#define SARA_R5_UTIME_SENSOR_GNSS_LTE UBX_CELL_UTIME_SENSOR_GNSS_LTE
#define SARA_R5_UTIME_SENSOR_LTE UBX_CELL_UTIME_SENSOR_LTE

#define SARA_R5_UTIME_URC_CONFIGURATION_DISABLED UBX_CELL_UTIME_URC_CONFIGURATION_DISABLED
#define SARA_R5_UTIME_URC_CONFIGURATION_ENABLED UBX_CELL_UTIME_URC_CONFIGURATION_ENABLED

#define SARA_R5_SIM_NOT_PRESENT UBX_CELL_SIM_NOT_PRESENT
#define SARA_R5_SIM_PIN_NEEDED UBX_CELL_SIM_PIN_NEEDED
#define SARA_R5_SIM_PIN_BLOCKED UBX_CELL_SIM_PIN_BLOCKED
#define SARA_R5_SIM_PUK_BLOCKED UBX_CELL_SIM_PUK_BLOCKED
#define SARA_R5_SIM_NOT_OPERATIONAL UBX_CELL_SIM_NOT_OPERATIONAL
#define SARA_R5_SIM_RESTRICTED UBX_CELL_SIM_RESTRICTED
#define SARA_R5_SIM_OPERATIONAL UBX_CELL_SIM_OPERATIONAL
//SARA_R5_SIM_PHONEBOOK_READY, // Not reported by SARA-R5
//SARA_R5_SIM_USIM_PHONEBOOK_READY, // Not reported by SARA-R5
//SARA_R5_SIM_TOOLKIT_REFRESH_SUCCESSFUL, // Not reported by SARA-R5
//SARA_R5_SIM_TOOLKIT_REFRESH_UNSUCCESSFUL, // Not reported by SARA-R5
//SARA_R5_SIM_PPP_CONNECTION_ACTIVE, // Not reported by SARA-R5
//SARA_R5_SIM_VOICE_CALL_ACTIVE, // Not reported by SARA-R5
//SARA_R5_SIM_CSD_CALL_ACTIVE // Not reported by SARA-R5

#define SARA_R5_NUM_PSD_PROFILES 6             // Number of supported PSD profiles
#define SARA_R5_NUM_PDP_CONTEXT_IDENTIFIERS 11 // Number of supported PDP context identifiers
#define SARA_R5_NUM_HTTP_PROFILES 4            // Number of supported HTTP profiles

#define SARA_R5_HTTP_OP_CODE_SERVER_IP UBX_CELL_HTTP_OP_CODE_SERVER_IP
#define SARA_R5_HTTP_OP_CODE_SERVER_NAME UBX_CELL_HTTP_OP_CODE_SERVER_NAME
#define SARA_R5_HTTP_OP_CODE_USERNAME UBX_CELL_HTTP_OP_CODE_USERNAME
#define SARA_R5_HTTP_OP_CODE_PASSWORD UBX_CELL_HTTP_OP_CODE_PASSWORD
#define SARA_R5_HTTP_OP_CODE_AUTHENTICATION UBX_CELL_HTTP_OP_CODE_AUTHENTICATION
#define SARA_R5_HTTP_OP_CODE_SERVER_PORT UBX_CELL_HTTP_OP_CODE_SERVER_PORT
#define SARA_R5_HTTP_OP_CODE_SECURE UBX_CELL_HTTP_OP_CODE_SECURE
#define SARA_R5_HTTP_OP_CODE_REQUEST_TIMEOUT UBX_CELL_HTTP_OP_CODE_REQUEST_TIMEOUT
#define SARA_R5_HTTP_OP_CODE_ADD_CUSTOM_HEADERS UBX_CELL_HTTP_OP_CODE_ADD_CUSTOM_HEADERS

#define SARA_R5_HTTP_COMMAND_HEAD UBX_CELL_HTTP_COMMAND_HEAD
#define SARA_R5_HTTP_COMMAND_GET UBX_CELL_HTTP_COMMAND_GET
#define SARA_R5_HTTP_COMMAND_DELETE UBX_CELL_HTTP_COMMAND_DELETE
#define SARA_R5_HTTP_COMMAND_PUT UBX_CELL_HTTP_COMMAND_PUT
#define SARA_R5_HTTP_COMMAND_POST_FILE UBX_CELL_HTTP_COMMAND_POST_FILE
#define SARA_R5_HTTP_COMMAND_POST_DATA UBX_CELL_HTTP_COMMAND_POST_DATA
#define SARA_R5_HTTP_COMMAND_GET_FOTA UBX_CELL_HTTP_COMMAND_GET_FOTA

#define SARA_R5_HTTP_CONTENT_APPLICATION_X_WWW UBX_CELL_HTTP_CONTENT_APPLICATION_X_WWW
#define SARA_R5_HTTP_CONTENT_TEXT_PLAIN UBX_CELL_HTTP_CONTENT_TEXT_PLAIN
#define SARA_R5_HTTP_CONTENT_APPLICATION_OCTET UBX_CELL_HTTP_CONTENT_APPLICATION_OCTET
#define SARA_R5_HTTP_CONTENT_MULTIPART_FORM UBX_CELL_HTTP_CONTENT_MULTIPART_FORM
#define SARA_R5_HTTP_CONTENT_APPLICATION_JSON UBX_CELL_HTTP_CONTENT_APPLICATION_JSON
#define SARA_R5_HTTP_CONTENT_APPLICATION_XML UBX_CELL_HTTP_CONTENT_APPLICATION_XML
#define SARA_R5_HTTP_CONTENT_USER_DEFINED UBX_CELL_HTTP_CONTENT_USER_DEFINED

#define SARA_R5_MQTT_NV_RESTORE UBX_CELL_MQTT_NV_RESTORE
#define SARA_R5_MQTT_NV_SET UBX_CELL_MQTT_NV_SET
#define SARA_R5_MQTT_NV_STORE UBX_CELL_MQTT_NV_STORE

#define SARA_R5_MQTT_PROFILE_CLIENT_ID UBX_CELL_MQTT_PROFILE_CLIENT_ID
#define SARA_R5_MQTT_PROFILE_SERVERNAME UBX_CELL_MQTT_PROFILE_SERVERNAME
#define SARA_R5_MQTT_PROFILE_IPADDRESS UBX_CELL_MQTT_PROFILE_IPADDRESS
#define SARA_R5_MQTT_PROFILE_USERNAMEPWD UBX_CELL_MQTT_PROFILE_USERNAMEPWD
#define SARA_R5_MQTT_PROFILE_QOS UBX_CELL_MQTT_PROFILE_QOS
#define SARA_R5_MQTT_PROFILE_RETAIN UBX_CELL_MQTT_PROFILE_RETAIN
#define SARA_R5_MQTT_PROFILE_TOPIC UBX_CELL_MQTT_PROFILE_TOPIC
#define SARA_R5_MQTT_PROFILE_MESSAGE UBX_CELL_MQTT_PROFILE_MESSAGE
#define SARA_R5_MQTT_PROFILE_INACTIVITYTIMEOUT UBX_CELL_MQTT_PROFILE_INACTIVITYTIMEOUT
#define SARA_R5_MQTT_PROFILE_SECURE UBX_CELL_MQTT_PROFILE_SECURE

#define SARA_R5_MQTT_COMMAND_INVALID UBX_CELL_MQTT_COMMAND_INVALID
#define SARA_R5_MQTT_COMMAND_LOGOUT UBX_CELL_MQTT_COMMAND_LOGOUT
#define SARA_R5_MQTT_COMMAND_LOGIN UBX_CELL_MQTT_COMMAND_LOGIN
#define SARA_R5_MQTT_COMMAND_PUBLISH UBX_CELL_MQTT_COMMAND_PUBLISH
#define SARA_R5_MQTT_COMMAND_PUBLISHFILE UBX_CELL_MQTT_COMMAND_PUBLISHFILE
#define SARA_R5_MQTT_COMMAND_SUBSCRIBE UBX_CELL_MQTT_COMMAND_SUBSCRIBE
#define SARA_R5_MQTT_COMMAND_UNSUBSCRIBE UBX_CELL_MQTT_COMMAND_UNSUBSCRIBE
#define SARA_R5_MQTT_COMMAND_READ UBX_CELL_MQTT_COMMAND_READ
#define SARA_R5_MQTT_COMMAND_RCVMSGFORMAT UBX_CELL_MQTT_COMMAND_RCVMSGFORMAT
#define SARA_R5_MQTT_COMMAND_PING UBX_CELL_MQTT_COMMAND_PING
#define SARA_R5_MQTT_COMMAND_PUBLISHBINARY UBX_CELL_MQTT_COMMAND_PUBLISHBINARY

#define SARA_R5_FTP_PROFILE_IPADDRESS UBX_CELL_FTP_PROFILE_IPADDRESS
#define SARA_R5_FTP_PROFILE_SERVERNAME UBX_CELL_FTP_PROFILE_SERVERNAME
#define SARA_R5_FTP_PROFILE_USERNAME UBX_CELL_FTP_PROFILE_USERNAME
#define SARA_R5_FTP_PROFILE_PWD UBX_CELL_FTP_PROFILE_PWD
#define SARA_R5_FTP_PROFILE_ACCOUNT UBX_CELL_FTP_PROFILE_ACCOUNT
#define SARA_R5_FTP_PROFILE_TIMEOUT UBX_CELL_FTP_PROFILE_TIMEOUT
#define SARA_R5_FTP_PROFILE_MODE UBX_CELL_FTP_PROFILE_MODE

#define SARA_R5_FTP_COMMAND_INVALID UBX_CELL_FTP_COMMAND_INVALID
#define SARA_R5_FTP_COMMAND_LOGOUT UBX_CELL_FTP_COMMAND_LOGOUT
#define SARA_R5_FTP_COMMAND_LOGIN UBX_CELL_FTP_COMMAND_LOGIN
#define SARA_R5_FTP_COMMAND_DELETE_FILE UBX_CELL_FTP_COMMAND_DELETE_FILE
#define SARA_R5_FTP_COMMAND_RENAME_FILE UBX_CELL_FTP_COMMAND_RENAME_FILE
#define SARA_R5_FTP_COMMAND_GET_FILE UBX_CELL_FTP_COMMAND_GET_FILE
#define SARA_R5_FTP_COMMAND_PUT_FILE UBX_CELL_FTP_COMMAND_PUT_FILE
#define SARA_R5_FTP_COMMAND_GET_FILE_DIRECT UBX_CELL_FTP_COMMAND_GET_FILE_DIRECT
#define SARA_R5_FTP_COMMAND_PUT_FILE_DIRECT UBX_CELL_FTP_COMMAND_PUT_FILE_DIRECT
#define SARA_R5_FTP_COMMAND_CHANGE_DIR UBX_CELL_FTP_COMMAND_CHANGE_DIR
#define SARA_R5_FTP_COMMAND_MKDIR UBX_CELL_FTP_COMMAND_MKDIR
#define SARA_R5_FTP_COMMAND_RMDIR UBX_CELL_FTP_COMMAND_RMDIR
#define SARA_R5_FTP_COMMAND_DIR_INFO UBX_CELL_FTP_COMMAND_DIR_INFO
#define SARA_R5_FTP_COMMAND_LS UBX_CELL_FTP_COMMAND_LS
#define SARA_R5_FTP_COMMAND_GET_FOTA_FILE UBX_CELL_FTP_COMMAND_GET_FOTA_FILE

#define SARA_R5_PSD_CONFIG_PARAM_PROTOCOL UBX_CELL_PSD_CONFIG_PARAM_PROTOCOL
#define SARA_R5_PSD_CONFIG_PARAM_APN UBX_CELL_PSD_CONFIG_PARAM_APN
//SARA_R5_PSD_CONFIG_PARAM_USERNAME, // Not allowed on SARA-R5
//SARA_R5_PSD_CONFIG_PARAM_PASSWORD, // Not allowed on SARA-R5
#define SARA_R5_PSD_CONFIG_PARAM_DNS1 UBX_CELL_PSD_CONFIG_PARAM_DNS1
#define SARA_R5_PSD_CONFIG_PARAM_DNS2 UBX_CELL_PSD_CONFIG_PARAM_DNS2
//SARA_R5_PSD_CONFIG_PARAM_AUTHENTICATION, // Not allowed on SARA-R5
//SARA_R5_PSD_CONFIG_PARAM_IP_ADDRESS, // Not allowed on SARA-R5
//SARA_R5_PSD_CONFIG_PARAM_DATA_COMPRESSION, // Not allowed on SARA-R5
//SARA_R5_PSD_CONFIG_PARAM_HEADER_COMPRESSION, // Not allowed on SARA-R5
#define SARA_R5_PSD_CONFIG_PARAM_MAP_TO_CID UBX_CELL_PSD_CONFIG_PARAM_MAP_TO_CID

#define SARA_R5_PSD_PROTOCOL_IPV4 UBX_CELL_PSD_PROTOCOL_IPV4
#define SARA_R5_PSD_PROTOCOL_IPV6 UBX_CELL_PSD_PROTOCOL_IPV6
#define SARA_R5_PSD_PROTOCOL_IPV4V6_V4_PREF UBX_CELL_PSD_PROTOCOL_IPV4V6_V4_PREF
#define SARA_R5_PSD_PROTOCOL_IPV4V6_V6_PREF UBX_CELL_PSD_PROTOCOL_IPV4V6_V6_PREF

#define SARA_R5_PSD_ACTION_RESET UBX_CELL_PSD_ACTION_RESET
#define SARA_R5_PSD_ACTION_STORE UBX_CELL_PSD_ACTION_STORE
#define SARA_R5_PSD_ACTION_LOAD UBX_CELL_PSD_ACTION_LOAD
#define SARA_R5_PSD_ACTION_ACTIVATE UBX_CELL_PSD_ACTION_ACTIVATE
#define SARA_R5_PSD_ACTION_DEACTIVATE UBX_CELL_PSD_ACTION_DEACTIVATE

#define SARA_R5_SEC_PROFILE_PARAM_CERT_VAL_LEVEL UBX_CELL_SEC_PROFILE_PARAM_CERT_VAL_LEVEL
#define SARA_R5_SEC_PROFILE_PARAM_TLS_VER UBX_CELL_SEC_PROFILE_PARAM_TLS_VER
#define SARA_R5_SEC_PROFILE_PARAM_CYPHER_SUITE UBX_CELL_SEC_PROFILE_PARAM_CYPHER_SUITE
#define SARA_R5_SEC_PROFILE_PARAM_ROOT_CA UBX_CELL_SEC_PROFILE_PARAM_ROOT_CA
#define SARA_R5_SEC_PROFILE_PARAM_HOSTNAME UBX_CELL_SEC_PROFILE_PARAM_HOSTNAME
#define SARA_R5_SEC_PROFILE_PARAM_CLIENT_CERT UBX_CELL_SEC_PROFILE_PARAM_CLIENT_CERT
#define SARA_R5_SEC_PROFILE_PARAM_CLIENT_KEY UBX_CELL_SEC_PROFILE_PARAM_CLIENT_KEY
#define SARA_R5_SEC_PROFILE_PARAM_CLIENT_KEY_PWD UBX_CELL_SEC_PROFILE_PARAM_CLIENT_KEY_PWD
#define SARA_R5_SEC_PROFILE_PARAM_PSK UBX_CELL_SEC_PROFILE_PARAM_PSK
#define SARA_R5_SEC_PROFILE_PARAM_PSK_IDENT UBX_CELL_SEC_PROFILE_PARAM_PSK_IDENT
#define SARA_R5_SEC_PROFILE_PARAM_SNI UBX_CELL_SEC_PROFILE_PARAM_SNI

#define SARA_R5_SEC_PROFILE_CERTVAL_OPCODE_NO UBX_CELL_SEC_PROFILE_CERTVAL_OPCODE_NO
#define SARA_R5_SEC_PROFILE_CERTVAL_OPCODE_YESNOURL UBX_CELL_SEC_PROFILE_CERTVAL_OPCODE_YESNOURL
#define SARA_R5_SEC_PROFILE_CERVTAL_OPCODE_YESURL UBX_CELL_SEC_PROFILE_CERVTAL_OPCODE_YESURL
#define SARA_R5_SEC_PROFILE_CERTVAL_OPCODE_YESURLDATE UBX_CELL_SEC_PROFILE_CERTVAL_OPCODE_YESURLDATE

#define SARA_R5_SEC_PROFILE_TLS_OPCODE_ANYVER UBX_CELL_SEC_PROFILE_TLS_OPCODE_ANYVER
#define SARA_R5_SEC_PROFILE_TLS_OPCODE_VER1_0 UBX_CELL_SEC_PROFILE_TLS_OPCODE_VER1_0
#define SARA_R5_SEC_PROFILE_TLS_OPCODE_VER1_1 UBX_CELL_SEC_PROFILE_TLS_OPCODE_VER1_1
#define SARA_R5_SEC_PROFILE_TLS_OPCODE_VER1_2 UBX_CELL_SEC_PROFILE_TLS_OPCODE_VER1_2
#define SARA_R5_SEC_PROFILE_TLS_OPCODE_VER1_3 UBX_CELL_SEC_PROFILE_TLS_OPCODE_VER1_3

#define SARA_R5_SEC_PROFILE_SUITE_OPCODE_PROPOSEDDEFAULT UBX_CELL_SEC_PROFILE_SUITE_OPCODE_PROPOSEDDEFAULT

#define SARA_R5_SEC_MANAGER_OPCODE_IMPORT UBX_CELL_SEC_MANAGER_OPCODE_IMPORT

#define SARA_R5_SEC_MANAGER_ROOTCA UBX_CELL_SEC_MANAGER_ROOTCA
#define SARA_R5_SEC_MANAGER_CLIENT_CERT UBX_CELL_SEC_MANAGER_CLIENT_CERT
#define SARA_R5_SEC_MANAGER_CLIENT_KEY UBX_CELL_SEC_MANAGER_CLIENT_KEY
#define SARA_R5_SEC_MANAGER_SERVER_CERT UBX_CELL_SEC_MANAGER_SERVER_CERT

#endif //SPARKFUN_SARA_R5_ARDUINO_LIBRARY_H
