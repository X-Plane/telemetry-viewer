# v0.6.1
Mostly a quality of life release.

 - Files can be renamed in the trace list on the left
 - Added functions to close and save individual files
 - Deselecting a file in the documents list removes it from the chart hover UI
 - Multiple CLI commands weren't properly supported
 - Multiple tests can be run back to back for a "best of X" configuration
 - Safe mode can be enabled from the test runner UI

# v0.6.0
This release brings with it the ability to open multiple traces and overlay them on top of each other to better compare performance tests

 - Dragging multiple files into the window will open all traces at once
 - Holding ctrl while dragging one or more files into the window will open the files on top of the additional files

# v0.5.0
This release is the first step towards getting this project put into proper shape to be used in the field

 - Qt 6 based
 - Uses Qt charts for charts instead of charts.js jammed into a browser window
 - FPS tests are now run with the event trace option enabled
 - FPS test CLI arguments can now be easily copied to
 - Linux and macOS fixes (aka it actually compiles and runs on these platforms now)
 - Misc fixes for smaller issues

# v0.4.0
Initial public release based off of Qt5 and charts.js for chart rendering