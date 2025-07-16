#!/usr/bin/env python3
"""
Quick Demo Script for Alpaca Live Trading
========================================

This script demonstrates the complete setup and execution of live paper trading
with Alpaca API integration.
"""

import os
import sys
import subprocess
import time
from datetime import datetime

def print_header(title):
    """Print a formatted header"""
    print(f"\n{'='*60}")
    print(f"🚀 {title}")
    print(f"{'='*60}")

def print_section(title):
    """Print a formatted section"""
    print(f"\n{'-'*40}")
    print(f"📋 {title}")
    print(f"{'-'*40}")

def run_command(command, description):
    """Run a command and handle errors"""
    print(f"\n▶️  {description}")
    print(f"Command: {command}")
    
    try:
        result = subprocess.run(command, shell=True, capture_output=True, text=True)
        if result.returncode == 0:
            print(f"✅ Success!")
            if result.stdout:
                print(f"Output: {result.stdout[:200]}...")
        else:
            print(f"❌ Error: {result.stderr}")
            return False
    except Exception as e:
        print(f"❌ Exception: {e}")
        return False
    
    return True

def check_credentials():
    """Check if Alpaca credentials are set"""
    print_section("Checking Alpaca API Credentials")
    
    api_key = os.getenv('ALPACA_API_KEY')
    secret_key = os.getenv('ALPACA_SECRET_KEY')
    
    if api_key and secret_key:
        print("✅ Alpaca API credentials found in environment variables")
        print(f"API Key: {api_key[:8]}...{api_key[-4:]}")
        print(f"Secret Key: {secret_key[:8]}...{secret_key[-4:]}")
        return True
    else:
        print("❌ Alpaca API credentials not found")
        print("\nTo set up credentials:")
        print("1. Go to https://alpaca.markets/")
        print("2. Create a free account")
        print("3. Generate Paper Trading API keys")
        print("4. Set environment variables:")
        print("   Windows: set ALPACA_API_KEY=your_key")
        print("   Linux/Mac: export ALPACA_API_KEY=your_key")
        return False

def demo_investment_recommendations():
    """Display investment recommendations"""
    print_section("2024 Investment Recommendations")
    
    recommendations = {
        "🤖 AI/Technology Leaders": [
            ("NVDA", "$875", "AI chip leader, massive growth potential"),
            ("MSFT", "$420", "Cloud computing + AI integration"),
            ("GOOGL", "$140", "Undervalued AI leader with search dominance"),
            ("PLTR", "$18.50", "Government AI contracts, strong fundamentals")
        ],
        "🚀 Growth Opportunities": [
            ("TSLA", "$240", "Electric vehicle + energy storage leader"),
            ("SNOW", "$140", "Cloud data platform with enterprise adoption"),
            ("CRWD", "$285", "Cybersecurity growth in AI era"),
            ("ARKK", "$48", "Disruptive innovation ETF")
        ],
        "🛡️ Diversified & Defensive": [
            ("AAPL", "$185", "Stable ecosystem with AI hardware potential"),
            ("SPY", "$485", "Broad market exposure for stability"),
            ("JNJ", "$155", "Healthcare defensive play"),
            ("VTI", "$245", "Total US market diversification")
        ]
    }
    
    for category, stocks in recommendations.items():
        print(f"\n{category}:")
        for symbol, price, description in stocks:
            print(f"  • {symbol:6} ({price:>6}) - {description}")

def demo_strategy_overview():
    """Display strategy overview"""
    print_section("Trading Strategy Overview")
    
    print("🎯 Multi-Strategy Approach:")
    print("  1. Momentum Strategy (Primary)")
    print("     - Buy when fast SMA > slow SMA + threshold")
    print("     - Sell when fast SMA < slow SMA - threshold")
    print("     - 5-day and 20-day moving averages")
    print("     - 2% momentum threshold")
    
    print("\n  2. Position Sizing & Risk Management")
    print("     - 5% of portfolio per trade")
    print("     - 5% stop loss protection")
    print("     - Maximum 10 positions")
    print("     - Cash reserves maintained")
    
    print("\n  3. Enhanced Signals for AI/Growth Stocks")
    print("     - Higher confidence for NVDA, PLTR, TSLA")
    print("     - Sector-specific adjustments")
    print("     - Volatility-based positioning")

def run_demo():
    """Run the complete demo"""
    print_header("Alpaca Live Paper Trading Demo")
    
    print("🎯 This demo will:")
    print("1. Check your Alpaca API credentials")
    print("2. Show 2024 investment recommendations")
    print("3. Explain the trading strategy")
    print("4. Build and run the live trading application")
    print("5. Monitor real-time trading activity")
    
    # Check credentials
    if not check_credentials():
        print("\n⚠️  Demo will run in simulation mode without real API")
        time.sleep(2)
    
    # Show investment recommendations
    demo_investment_recommendations()
    
    # Show strategy overview
    demo_strategy_overview()
    
    # Ask user to continue
    print_section("Ready to Start Live Trading Demo")
    response = input("Continue with live trading demo? (y/n): ").strip().lower()
    
    if response != 'y':
        print("Demo cancelled by user")
        return
    
    # Change to build directory
    build_dir = "build/Debug"
    if not os.path.exists(build_dir):
        print(f"❌ Build directory not found: {build_dir}")
        print("Please build the project first:")
        print("  cmake --build . --config Debug")
        return
    
    # Run the live trading application
    print_section("Starting Live Trading Application")
    
    exe_path = os.path.join(build_dir, "alpaca_live_trading.exe")
    if not os.path.exists(exe_path):
        print(f"❌ Executable not found: {exe_path}")
        print("Please build the alpaca_live_trading target first")
        return
    
    print(f"🚀 Launching: {exe_path}")
    print("📊 Expected behavior:")
    print("  - API connection test")
    print("  - Market status check")
    print("  - Real-time price updates")
    print("  - Signal generation and execution")
    print("  - Portfolio tracking")
    
    print(f"\n⏰ Demo will run for 30 minutes")
    print("💡 Press Ctrl+C to stop early")
    
    try:
        # Run the executable
        subprocess.run([exe_path], cwd=build_dir, check=True)
        
    except KeyboardInterrupt:
        print("\n\n⚠️  Demo interrupted by user")
    except subprocess.CalledProcessError as e:
        print(f"\n❌ Application error: {e}")
    except Exception as e:
        print(f"\n❌ Unexpected error: {e}")
    
    print_section("Demo Complete")
    print("📊 Check the following files for results:")
    print(f"  - {build_dir}/live_trading_results.txt")
    print("  - Console output above")
    
    print("\n🎉 Next Steps:")
    print("1. Review the trading results")
    print("2. Adjust strategy parameters if needed")
    print("3. Test with real Alpaca API credentials")
    print("4. Scale up position sizes for live trading")
    print("5. Implement additional risk management")

def main():
    """Main demo function"""
    try:
        run_demo()
    except KeyboardInterrupt:
        print("\n\n⚠️  Demo interrupted by user")
    except Exception as e:
        print(f"\n❌ Demo failed: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()