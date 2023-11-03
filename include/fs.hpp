#pragma once

#include "FS.h"
#include "LittleFS.h"

void init_littlefs();

fs::LittleFSFS &get_fs();