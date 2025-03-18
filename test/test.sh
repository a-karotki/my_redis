# Start the server
EXE_DIR=../cmake-build-debug-wsl
gnome-terminal -- "$SHELL" -c "$EXE_DIR/server/MRD_server > logs/server_output_log.txt 2>&1"

# Wait for the server to initialize
sleep 1

# Start the client
# Note: doesn't handle CRLF
./$EXE_DIR/client/MRD_client > logs/client_output_log.txt 2>&1 < test_cases.txt


# Clean up: kill the server
pkill gnome-terminal
echo Test run ended

