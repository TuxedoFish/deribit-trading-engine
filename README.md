## Getting started

- Build the project in VSCODE

## Apps

### Marketdata historical service
- Persist FIX messages to an output folder.
- Settings store under settings/settings.md-hist.txt
`.\out\build\x64-debug\deribit-md\deribit-md.exe -app md-hist`

### Marketdata process service
- Process raw FIX messages up to the end of the previous day into a binary format.
- Settings store under settings/settings.md-process.txt
`.\out\build\x64-debug\deribit-md\deribit-md.exe -app md-hist`