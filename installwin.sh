if [[ "$MSYSTEM" != "MINGW64" ]]; then
    echo "Please run this in the MINGW64 shell (not MSYS or CMD)"
    exit 1
fi
echo "Updating MSYS64, please wait..."
pacman -Sy -y mingw-w64-x86_64-toolchain make mingw-w64-x86_64-nlohmann-json mingw-w64-x86_64-curl
echo "Dependencies installed. Now building and installing mcpk..."
make
echo "Running installation..."
make installwin
