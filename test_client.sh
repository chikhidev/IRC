#!/bin/bash

# Test script for IRC client connection
exec 3<>/dev/tcp/localhost/6667

echo "NICK testuser" >&3
echo "USER testuser 0 * :Test User" >&3
echo "PING server" >&3

# Read server responses
timeout 2 cat <&3

# Close connection
exec 3<&-
exec 3>&-