import os

print("Current CLI targets", COMMAND_LINE_TARGETS)
print("Current Build targets", BUILD_TARGETS)

dir_path = os.getcwd()
print(dir_path)

def generate_index_hpp():
    raw_html = ""
    with open("./include/index.html") as inputt:
        raw_html = inputt.read()

    with open("./include/index.hpp","w+") as outt:
        outt.write('''
#pragma once
#include <Arduino.h>

#include "index_processor.hpp"

const char index_html[] PROGMEM = R"rawliteral(
''')
        
        outt.write(raw_html)

        outt.write('''
)rawliteral";
'''
)

#

generate_index_hpp()

