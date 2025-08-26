#!/usr/bin/env python3
"""
StarSim Build Script

This script builds the StarSim ParsecCore library and examples.

Usage:
    python build.py [--clean] [--build-dir BUILD_DIR]

Options:
    --clean         Clean the build directory before building
    --build-dir     Path to build directory (default: ./ParsecCore/build)
"""

import os
import sys
import subprocess
import argparse
import platform
import shutil
from pathlib import Path

def clean_build_dir(build_dir):
    """Clean the build directory"""
    build_path = Path(build_dir).absolute()
    
    if build_path.exists():
        print(f"Cleaning build directory: {build_path}")
        shutil.rmtree(build_path)
    
    os.makedirs(build_path, exist_ok=True)
    return True

def build_parsec_core(build_dir, clean=False):
    """Build ParsecCore library and examples"""
    # Get absolute path to ParsecCore directory
    parsec_dir = Path(__file__).parent / "ParsecCore"
    build_path = Path(build_dir).absolute()
    
    if not parsec_dir.exists():
        print(f"Error: ParsecCore directory not found at {parsec_dir}")
        return False
    
    # Clean build directory if requested
    if clean:
        clean_build_dir(build_path)
    else:
        os.makedirs(build_path, exist_ok=True)
    
    print(f"Building ParsecCore in {build_path}")
    
    # Configure CMake
    cmake_cmd = ["cmake", str(parsec_dir)]
    
    # Use Visual Studio generator on Windows
    if platform.system() == "Windows":
        cmake_cmd.extend(["-G", "Visual Studio 16 2019"])
    
    try:
        print("Running CMake configuration...")
        subprocess.run(cmake_cmd, cwd=build_path, check=True)
        
        # Build
        print("Building ParsecCore...")
        build_cmd = ["cmake", "--build", ".", "--config", "Debug"]
        subprocess.run(build_cmd, cwd=build_path, check=True)
        
        print("ParsecCore built successfully!")
        return True
    except subprocess.CalledProcessError as e:
        print(f"Build failed: {e}")
        return False

def main():
    parser = argparse.ArgumentParser(description="StarSim Build Script")
    parser.add_argument("--clean", action="store_true", help="Clean the build directory before building")
    parser.add_argument("--build-dir", default="./ParsecCore/build", help="Path to build directory")
    args = parser.parse_args()
    
    # Print banner
    print("=" * 80)
    print("StarSim Build Script")
    print("=" * 80)
    
    # Build ParsecCore
    build_dir = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), args.build_dir))
    success = build_parsec_core(build_dir, args.clean)
    
    if success:
        print("\nBuild completed successfully!")
        print("\nTo run the integration demo:")
        print(f"python run_integration_demo.py --build-dir {args.build_dir}")
    else:
        print("\nBuild failed!")
        sys.exit(1)

if __name__ == "__main__":
    main()