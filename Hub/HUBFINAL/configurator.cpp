#include "configurator.h"

//      Configuration
//===============================================================================

    unsigned long long Configuration::currentTime() {
      if (timeSet)
        return receivedTime+millis()-timeOffset;
      else
        return millis();
    }
    
    char* Configuration::currentTime_str() {
      str_ll(currentTime(),time_str);
      return time_str;
    }
  
    void Configuration::send() {
      for (int i = 0; i < CONFIG_PARAMETERS; i++) {
        switch(i) {
          case CONFIG_TYPE_INDEX:
             ble.print(CONFIG_SET_CHAR);     //signal to begin
             break;
          case CONFIG_TIME_INDEX:
            ble.print(currentTime_str());
            break;
          case CONFIG_ID_INDEX:
            ble.print(initialID);
            break;
          case CONFIG_SCHEMA_INDEX:
            for(int j=0; j < format->fieldCount; j++) {
              ble.print(format->spec[j]);
              ble.print('.');
            }
            break;
        }
        ble.print(DELIMITER_CHAR);
      }
      ble.println();
    }  

