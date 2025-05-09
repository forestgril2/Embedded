#include "flash_controller.h"

// Define static members
bool FlashController::_initialized = false;
uint32_t FlashController::_version = 0;
FlashController::LogLevel FlashController::_logLevel = FlashController::LogLevel::INFO;
const char* FlashController::_logPrefix = "FLASH"; 