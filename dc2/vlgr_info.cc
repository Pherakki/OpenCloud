#include <array>

#include "common/log.h"

#include "dc2/villager.h"
#include "dc2/vlgr_info.h"

set_log_channel("vlgr_info");

// 01F62BD0
static std::array<CVillagerPlace, 0x200> VlgrPlace{};