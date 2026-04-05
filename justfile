# Use PowerShell strictly on Windows. Linux/macOS will safely default to 'sh'.
set windows-shell := ["pwsh", "-NoProfile", "-c"]

bdir := "build/debug"
rdir := "build/release"

default: b

# ==========================================
# DEBUG ENVIRONMENT (Development & Testing)
# ==========================================

# Configure CMake for Debug mode and generate compile_commands.json
set:
	cmake -G Ninja -S . -B {{bdir}} -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	-cmake -E copy "{{bdir}}/compile_commands.json" .

# Build the NanoMatch engine in Debug mode
[windows]
b:
	if (-Not (Test-Path "{{bdir}}")) { just set }
	cmake --build {{bdir}}

# Build the NanoMatch engine in Debug mode
[linux]
[macos]
b:
	if [ ! -d "{{bdir}}" ]; then just set; fi
	cmake --build {{bdir}}

# Run the NanoMatch engine (Debug)
r: b
	./{{bdir}}/NanoMatch

# ==========================================
# RELEASE ENVIRONMENT (Maximum Performance)
# ==========================================

# Configure CMake for Release mode (O3 Optimizations enabled)
setrel:
	cmake -G Ninja -S . -B {{rdir}} -DCMAKE_BUILD_TYPE=Release 

# Build the NanoMatch engine in Release mode
[windows]
brel:
	if (-Not (Test-Path "{{rdir}}")) { just setrel }
	cmake --build {{rdir}}

# Build the NanoMatch engine in Release mode
[linux]
[macos]
brel:
	if [ ! -d "{{rdir}}" ]; then just setrel; fi
	cmake --build {{rdir}}

# Run the NanoMatch engine with HFT performance metrics
rel: brel
	./{{rdir}}/NanoMatch

# ==========================================
# UTILITIES
# ==========================================

# Clean all build artifacts, cache folders, and generated files
c:
	-cmake -E rm -rf build compile_commands.json .cache


# Format all C++ source and header files using clang-format
fmt:
	clang-format -i -style=file src/*.cpp include/*.hpp
