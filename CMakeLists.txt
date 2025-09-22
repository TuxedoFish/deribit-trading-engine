# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("deribit-trading-engine")

set(CMAKE_CXX_STANDARD 20)

# Define generated files
set(GENERATED_DIR ${CMAKE_SOURCE_DIR}/generated)
set(SBE_SCHEMA ${CMAKE_SOURCE_DIR}/schema/messages.xml)
set(SBE_JAR ${CMAKE_SOURCE_DIR}/env/sbe-all-1.30.0.jar)
set(SBE_STAMP ${GENERATED_DIR}/sbe_generated.stamp)

# Generated files
add_custom_command(
        OUTPUT ${SBE_STAMP}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_DIR}
        COMMAND java -Dsbe.target.language=CPP
          -Dsbe.output.dir=${GENERATED_DIR}
          -Dsbe.validation.warnings.fatal=true
          -Dsbe.validation.stop.on.error=true
          -Dsbe.xinclude.aware=true
          -jar ${SBE_JAR}
          ${SBE_SCHEMA}
        COMMAND ${CMAKE_COMMAND} -E touch ${SBE_STAMP}
        DEPENDS ${SBE_SCHEMA} ${SBE_JAR}
        COMMENT "Generating SBE C++ code from schema"
)
add_custom_target(generate_sbe_code DEPENDS ${SBE_STAMP})

# Dependencies
find_package(OpenSSL REQUIRED)
find_package(quickfix CONFIG REQUIRED)

# Add source to this project's executable.
add_executable (deribit-trading-engine "src/main.cpp" "include/AppRunner.h" "include/Application.h" "src/Application.cpp" "include/util/AuthHandler.h" "src/util/AuthHandler.cpp" "src/marketdata/MarketDataLogger.cpp" "include/marketdata/MarketDataLogger.h" "include/historical/ApplicationPersister.h" "src/historical/ApplicationPersister.cpp"   "include/util/CmdLineOptions.h" "src/AppRunner.cpp" "include/main.h" "include/util/SimpleConfig.h" "src/util/SimpleConfig.cpp" "include/marketdata/MessageProcessor.h" "src/marketdata/MessageProcessor.cpp" "src/historical/FileMessageProcessor.cpp" "include/historical/FileMessageProcessor.h" "include/marketdata/SBEBinaryWriter.h" "src/marketdata/SBEBinaryWriter.cpp"
        src/util/SBEUtils.cpp
        include/util/SBEUtils.h
        include/marketdata/FixCustomTags.h
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET deribit-trading-engine PROPERTY CXX_STANDARD 20)
endif()

# Dependencies
find_package(OpenSSL REQUIRED)
find_package(quickfix CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS iostreams)

if(WIN32)
  target_link_libraries(deribit-trading-engine
          ws2_32 wsock32 winmm crypt32 secur32
  )
elseif(UNIX)
  target_link_libraries(deribit-trading-engine
          pthread dl
  )
endif()

target_link_libraries(deribit-trading-engine
        ${OPENSSL_LIBRARIES} quickfix
        Boost::iostreams
)

# Generated code
target_include_directories(deribit-trading-engine PRIVATE
        ${CMAKE_SOURCE_DIR}/generated
)

add_dependencies(deribit-trading-engine generate_sbe_code)
