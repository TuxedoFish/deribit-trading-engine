## Getting started

- Dependencies: boost-iostreeams, openssl, quickfix using vcpkg

## Apps

### Marketdata service
- Streams marketdata from Deribit to an SBE queue.
- Settings store under settings/settings.md-<live|testnet>.txt
  `.\out\build\x64-debug\deribit-md\deribit-md.exe --app md-<live|testnet>`

### Gateway service
- Polls marketdata and orders queues and send out over FIX to Deribit
  - Pushes order updates on an outbound queue 
- Settings store under settings/settings.gw-<live|testnet>.txt
  `.\out\build\x64-debug\deribit-md\deribit-md.exe --app gw-<live|testnet>`

### Marketdata historical service
- Persist FIX messages to an output folder.
- Settings store under settings/settings.md-hist.txt
`.\out\build\x64-debug\deribit-md\deribit-md.exe --app md-hist`

### Marketdata process service
- Process raw FIX messages up to the end of the previous day into a binary format.
- Settings store under settings/settings.md-process.txt
`.\out\build\x64-debug\deribit-md\deribit-md.exe --app md-process`

## Notes

### SBE schema
- Note: this runs as part of the build process.
- If needed use this command to generate SBE manually using sbe-tool
  - `java -Dsbe.target.language=CPP -Dsbe.output.dir=generated -jar env/sbe-all-1.30.0.jar schema/messages.xml`
  
### Package patches
- Issue with vcpkg auto generated patch to remove throws 
- .vcpkg-clion/vcpkg/ports/quickfix/00001-fix-build.patch
- #define QUICKFIX_THROW(...) throw(__VA_ARGS__) 