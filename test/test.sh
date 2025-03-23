# Start the server
EXE_DIR=../cmake-build-debug-wsl

echo "setting up test_cases"
rm logs/*_log.txt
python3 -c "import test_gen; test_gen.kv_test(1000)"

gnome-terminal -- "$SHELL" -c "$EXE_DIR/server/MRD_server > logs/server_output_log.txt 2>&1"

# Wait for the server to initialize
sleep 1

# Start the client
# Note: doesn't handle CRLF
echo $1
echo KV start
start=$SECONDS
for i in $(seq 1 "$1")
do
  echo "run $i"
  ./$EXE_DIR/client/MRD_client 127.0.0.1> logs/kv_"$i"_client_output_log.txt 2>&1 < test_cases.txt &
done
wait $!
duration=$(( SECONDS - start ))
echo "duration: $duration"
echo KV finish
python3 -c "import test_gen; test_gen.set_test(10, 100)"
for i in $(seq 1 "$1")
do
  echo "run $i"
  ./$EXE_DIR/client/MRD_client 127.0.0.1> logs/set_"$i"_client_output_log.txt 2>&1 < test_cases.txt &
done
wait $!
# Clean up: kill the server
pkill gnome-terminal
echo Test run ended

