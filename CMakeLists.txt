cmake_minimum_required(VERSION 3.10)
project("SimpleFTP")

# Create server and client executables
add_executable("simpleftpserver" src/simpleftp_server.cpp)
add_executable("simpleftp" src/simpleftp.cpp)
