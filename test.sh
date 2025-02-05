# Start the server
EXE_DIR=cmake-build-debug-wsl
./$EXE_DIR/server > server_output_log.txt 2>&1 &
SERVER_PID=$!

# Wait for the server to initialize
sleep 1

# Start the client
echo 3 Clients:
./$EXE_DIR/client
./$EXE_DIR/client
./$EXE_DIR/client


# Clean up: kill the server
kill $SERVER_PID
sleep 1
echo
echo Sever:
cat server_output_log.txt

