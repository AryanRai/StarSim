#!/usr/bin/env python3
"""
StarSim - Comms Alpha v3.0 Integration Demo

This script demonstrates the integration between StarSim's ParsecCore and
Comms Alpha v3.0 by running all components together:
1. Stream Handler with physics support
2. C++ physics simulation using InputManager
3. AriesUI with physics widgets

Usage:
    python run_integration_demo.py [--no-ui] [--build-dir BUILD_DIR]

Options:
    --no-ui         Run without starting AriesUI
    --build-dir     Path to ParsecCore build directory (default: ./ParsecCore/build)
"""

import os
import sys
import subprocess
import time
import signal
import argparse
import threading
import webbrowser
import platform
import json
from pathlib import Path

# Add parent directory to path
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

def start_stream_handler():
    """Start the Stream Handler with physics support"""
    try:
        # Import the stream handler module
        sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), ".."))
        from sh.stream_handlerv3_0_physics import app
        
        print("\n[Stream Handler] Starting Stream Handler v3.0 with Physics Support...")
        app.listen(3000, lambda config: print(f"[Stream Handler] Listening on http://localhost:3000"))
        app.run()
    except ImportError as e:
        print(f"\n[ERROR] Failed to import Stream Handler: {e}")
        print("Make sure you're running this script from the StarSim directory")
        sys.exit(1)
    except Exception as e:
        print(f"\n[ERROR] Failed to start Stream Handler: {e}")
        sys.exit(1)

def build_cpp_example(build_dir):
    """Build the C++ InputManager example"""
    build_path = os.path.abspath(build_dir)
    
    # Create build directory if it doesn't exist
    os.makedirs(build_path, exist_ok=True)
    
    print(f"\n[Build] Building C++ example in {build_path}...")
    
    # Run CMake
    cmake_cmd = ["cmake", ".."]
    if platform.system() == "Windows":
        cmake_cmd = ["cmake", "..", "-G", "Visual Studio 16 2019"]
    
    try:
        subprocess.run(cmake_cmd, cwd=build_path, check=True)
        
        # Build the example
        build_cmd = ["cmake", "--build", ".", "--config", "Debug"]
        subprocess.run(build_cmd, cwd=build_path, check=True)
        
        print("[Build] C++ example built successfully")
        return True
    except subprocess.CalledProcessError as e:
        print(f"\n[ERROR] Build failed: {e}")
        return False

def run_cpp_example(build_dir):
    """Run the C++ InputManager example"""
    build_path = os.path.abspath(build_dir)
    
    # Find the example executable
    if platform.system() == "Windows":
        example_path = os.path.join(build_path, "Debug", "input_manager_example.exe")
    else:
        example_path = os.path.join(build_path, "input_manager_example")
    
    if not os.path.exists(example_path):
        print(f"\n[ERROR] Example executable not found at {example_path}")
        print("Please build the C++ example first")
        return False
    
    print(f"\n[Simulation] Running physics simulation: {example_path}")
    
    try:
        # Run the example
        subprocess.run([example_path], check=True)
        print("\n[Simulation] Physics simulation completed")
        return True
    except subprocess.CalledProcessError as e:
        print(f"\n[ERROR] Simulation failed: {e}")
        return False

def start_ariesui():
    """Start AriesUI with the StarSim demo layout"""
    # Find AriesUI directory
    ariesui_dir = os.path.abspath(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "..", "ui", "ariesUI"))
    
    if not os.path.exists(ariesui_dir):
        print(f"\n[ERROR] AriesUI directory not found at {ariesui_dir}")
        return False
    
    print(f"\n[AriesUI] Starting AriesUI from {ariesui_dir}...")
    
    # Check if we should use npm or yarn
    package_manager = "npm"
    if os.path.exists(os.path.join(ariesui_dir, "yarn.lock")):
        package_manager = "yarn"
    
    # Start AriesUI
    try:
        if platform.system() == "Windows":
            # On Windows, start in a new command window
            cmd = f"cd {ariesui_dir} && {package_manager} run dev"
            subprocess.Popen(["start", "cmd", "/k", cmd], shell=True)
        else:
            # On Unix, start in the background
            cmd = f"cd {ariesui_dir} && {package_manager} run dev"
            subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        
        # Wait for AriesUI to start
        print("[AriesUI] Waiting for AriesUI to start...")
        time.sleep(5)
        
        # Open browser
        webbrowser.open("http://localhost:3000")
        
        print("[AriesUI] AriesUI started")
        return True
    except Exception as e:
        print(f"\n[ERROR] Failed to start AriesUI: {e}")
        return False

def load_starsim_demo_layout():
    """Load the StarSim demo layout in AriesUI"""
    # Find the demo layout file
    layout_path = os.path.abspath(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "..", "Public", "AriesMods", "layouts", "starsim_demo.json"))
    
    if not os.path.exists(layout_path):
        print(f"\n[ERROR] StarSim demo layout not found at {layout_path}")
        return False
    
    print(f"\n[AriesUI] Loading StarSim demo layout from {layout_path}...")
    
    try:
        # Read the layout file
        with open(layout_path, "r") as f:
            layout = json.load(f)
        
        # TODO: Implement layout loading via API
        # For now, just print instructions
        print("\n[AriesUI] Please load the StarSim demo layout manually:")
        print("1. Open AriesUI in your browser")
        print("2. Click on 'Layouts' in the sidebar")
        print("3. Select 'StarSim Physics Demo'")
        print("4. Connect to the Stream Handler (ws://localhost:3000)")
        
        return True
    except Exception as e:
        print(f"\n[ERROR] Failed to load StarSim demo layout: {e}")
        return False

def main():
    parser = argparse.ArgumentParser(description="StarSim - Comms Alpha v3.0 Integration Demo")
    parser.add_argument("--no-ui", action="store_true", help="Run without starting AriesUI")
    parser.add_argument("--build-dir", default="./ParsecCore/build", help="Path to ParsecCore build directory")
    args = parser.parse_args()
    
    # Print banner
    print("=" * 80)
    print("StarSim - Comms Alpha v3.0 Integration Demo")
    print("=" * 80)
    
    # Start Stream Handler in a separate thread
    print("\n[Setup] Starting Stream Handler...")
    sh_thread = threading.Thread(target=start_stream_handler, daemon=True)
    sh_thread.start()
    
    # Wait for Stream Handler to start
    time.sleep(2)
    
    # Build C++ example if needed
    build_dir = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), args.build_dir))
    if not os.path.exists(os.path.join(build_dir, "input_manager_example")):
        if not build_cpp_example(build_dir):
            print("\n[ERROR] Failed to build C++ example")
            return
    
    # Start AriesUI if requested
    if not args.no_ui:
        if not start_ariesui():
            print("\n[WARNING] Failed to start AriesUI")
        else:
            load_starsim_demo_layout()
    
    # Run C++ example
    run_cpp_example(build_dir)
    
    # Keep running until user interrupts
    try:
        print("\n[Demo] Integration demo running. Press Ctrl+C to exit")
        while sh_thread.is_alive():
            time.sleep(1)
    except KeyboardInterrupt:
        print("\n[Demo] Exiting...")

if __name__ == "__main__":
    main()