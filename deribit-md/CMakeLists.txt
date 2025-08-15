# CMakeList.txt : CMake project for deribit-md, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (deribit-md "main.cpp" "AppRunner.h" "Application.h" "Application.cpp" "AuthHandler.h" "AuthHandler.cpp" "MarketDataLogger.cpp" "MarketDataLogger.h" "ApplicationPersister.h" "ApplicationPersister.cpp"   "CmdLineOptions.h" "AppRunner.cpp" "main.h" "SimpleConfig.h" "SimpleConfig.cpp" "MessageProcessor.h" "MessageProcessor.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET deribit-md PROPERTY CXX_STANDARD 20)
endif()

# Dependencies
find_package(OpenSSL REQUIRED)
find_package(quickfix CONFIG REQUIRED)

target_link_libraries(deribit-md 
    ws2_32 wsock32 winmm crypt32 secur32
    ${OPENSSL_LIBRARIES} quickfix
)