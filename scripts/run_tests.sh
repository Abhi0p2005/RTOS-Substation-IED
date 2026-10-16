#!/bin/bash

echo "=========================================="
echo "  Testing IED Protection System"
echo "=========================================="
echo ""

echo "Building native simulator..."
pio run -e native

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo ""
echo "Test 1: Normal Operation (5A rated)"
echo "Expected: No trip"
echo "------------------------------------------"
timeout 5s .pio/build/native/program <<EOF
status
quit
EOF

echo ""
echo ""
echo "Test 2: Overload Condition (1.5x rated = 7.5A)"
echo "Expected: ANSI 51 trip after calculated time"
echo "------------------------------------------"
timeout 10s .pio/build/native/program <<EOF
fault overload
status
EOF

echo ""
echo ""
echo "Test 3: Heavy Fault (3x rated = 15A)"
echo "Expected: ANSI 51 trip (faster than overload)"
echo "------------------------------------------"
timeout 5s .pio/build/native/program <<EOF
fault heavy
status
EOF

echo ""
echo ""
echo "Test 4: Instantaneous Fault (8x rated = 40A)"
echo "Expected: ANSI 50 trip within 50ms"
echo "------------------------------------------"
timeout 3s .pio/build/native/program <<EOF
fault instant
status
EOF

echo ""
echo ""
echo "=========================================="
echo "  All tests completed"
echo "=========================================="
