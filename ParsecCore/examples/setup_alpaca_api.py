#!/usr/bin/env python3
"""
Alpaca API Setup Script for Live Paper Trading
==============================================

This script helps you set up Alpaca API credentials and test the connection
for live paper trading with the StarSim trading system.
"""

import os
import sys
import json
import time
import requests
import pandas as pd
from datetime import datetime, timedelta
from typing import Dict, List, Optional
import alpaca_trade_api as tradeapi

class AlpacaSetup:
    def __init__(self):
        self.api_key = None
        self.secret_key = None
        self.base_url = "https://paper-api.alpaca.markets"
        self.api = None
        
    def get_credentials(self):
        """Get API credentials from user input or environment variables"""
        print("🔑 Setting up Alpaca API Credentials")
        print("=" * 50)
        
        # Check environment variables first
        env_api_key = os.getenv('ALPACA_API_KEY')
        env_secret_key = os.getenv('ALPACA_SECRET_KEY')
        
        if env_api_key and env_secret_key:
            print("✅ Found credentials in environment variables")
            self.api_key = env_api_key
            self.secret_key = env_secret_key
            return True
        
        print("📝 Please obtain your Alpaca API credentials:")
        print("1. Go to https://alpaca.markets/")
        print("2. Create a free account")
        print("3. Navigate to 'Paper Trading' section")
        print("4. Generate API keys")
        print()
        
        self.api_key = input("Enter your Alpaca API Key: ").strip()
        self.secret_key = input("Enter your Alpaca Secret Key: ").strip()
        
        if not self.api_key or not self.secret_key:
            print("❌ Invalid credentials provided")
            return False
        
        # Save to environment file
        env_content = f"""# Alpaca API Credentials
export ALPACA_API_KEY="{self.api_key}"
export ALPACA_SECRET_KEY="{self.secret_key}"

# To use these credentials, run:
# source alpaca_env.sh
"""
        
        with open('alpaca_env.sh', 'w') as f:
            f.write(env_content)
        
        print("✅ Credentials saved to alpaca_env.sh")
        print("   Run 'source alpaca_env.sh' to load them")
        
        return True
    
    def test_connection(self):
        """Test the API connection"""
        print("\n🔗 Testing API Connection")
        print("=" * 30)
        
        try:
            self.api = tradeapi.REST(
                self.api_key,
                self.secret_key,
                self.base_url,
                api_version='v2'
            )
            
            # Test account access
            account = self.api.get_account()
            print(f"✅ Connection successful!")
            print(f"Account ID: {account.id}")
            print(f"Account Status: {account.status}")
            print(f"Pattern Day Trader: {account.pattern_day_trader}")
            print(f"Buying Power: ${float(account.buying_power):,.2f}")
            print(f"Cash: ${float(account.cash):,.2f}")
            print(f"Portfolio Value: ${float(account.portfolio_value):,.2f}")
            
            return True
            
        except Exception as e:
            print(f"❌ Connection failed: {e}")
            return False
    
    def check_market_status(self):
        """Check current market status"""
        print("\n📊 Market Status")
        print("=" * 20)
        
        try:
            clock = self.api.get_clock()
            print(f"Market Open: {clock.is_open}")
            print(f"Current Time: {clock.timestamp}")
            print(f"Next Open: {clock.next_open}")
            print(f"Next Close: {clock.next_close}")
            
            # Get calendar for today
            today = datetime.now().date()
            calendar = self.api.get_calendar(start=today, end=today)
            
            if calendar:
                market_day = calendar[0]
                print(f"Market Opens: {market_day.open}")
                print(f"Market Closes: {market_day.close}")
            
            return clock.is_open
            
        except Exception as e:
            print(f"❌ Error getting market status: {e}")
            return False
    
    def get_sample_quotes(self, symbols: List[str] = None):
        """Get sample quotes for testing"""
        if symbols is None:
            symbols = ['AAPL', 'MSFT', 'GOOGL', 'NVDA', 'TSLA', 'SPY']
        
        print(f"\n📈 Sample Quotes for {len(symbols)} symbols")
        print("=" * 40)
        
        try:
            quotes = self.api.get_latest_quotes(symbols, feed='iex')
            
            for symbol in symbols:
                if symbol in quotes:
                    quote = quotes[symbol]
                    print(f"{symbol:6} | Bid: ${quote.bid_price:8.2f} | Ask: ${quote.ask_price:8.2f} | "
                          f"Spread: ${quote.ask_price - quote.bid_price:6.2f}")
                else:
                    print(f"{symbol:6} | No quote available")
            
            return quotes
            
        except Exception as e:
            print(f"❌ Error getting quotes: {e}")
            return None
    
    def test_order_placement(self):
        """Test order placement (will be rejected due to insufficient funds)"""
        print("\n🔄 Testing Order Placement")
        print("=" * 30)
        
        try:
            # Place a small test order (this will likely be rejected)
            test_order = self.api.submit_order(
                symbol='AAPL',
                qty=1,
                side='buy',
                type='market',
                time_in_force='day'
            )
            
            print(f"✅ Test order submitted: {test_order.id}")
            print(f"Symbol: {test_order.symbol}")
            print(f"Quantity: {test_order.qty}")
            print(f"Side: {test_order.side}")
            print(f"Status: {test_order.status}")
            
            # Cancel the order immediately
            self.api.cancel_order(test_order.id)
            print("✅ Test order canceled successfully")
            
            return True
            
        except Exception as e:
            print(f"⚠️  Order test result: {e}")
            print("   This is expected for new paper trading accounts")
            return False
    
    def generate_trading_universe(self):
        """Generate recommended trading universe with current prices"""
        print("\n🌟 Generating Trading Universe")
        print("=" * 35)
        
        # 2024 Investment Universe
        universe = {
            'AI_TECH': ['NVDA', 'GOOGL', 'MSFT', 'AAPL', 'AMZN', 'META'],
            'GROWTH': ['TSLA', 'PLTR', 'SNOW', 'CRWD', 'DDOG', 'MDB', 'NET'],
            'ETFS': ['SPY', 'QQQ', 'VTI', 'ARKK', 'XLK', 'SMH'],
            'DEFENSIVE': ['JNJ', 'PG', 'KO', 'WMT', 'BRK.B', 'VZ']
        }
        
        trading_data = {}
        
        for category, symbols in universe.items():
            print(f"\n{category}:")
            category_data = {}
            
            try:
                quotes = self.api.get_latest_quotes(symbols, feed='iex')
                
                for symbol in symbols:
                    if symbol in quotes:
                        quote = quotes[symbol]
                        mid_price = (quote.bid_price + quote.ask_price) / 2
                        category_data[symbol] = {
                            'price': mid_price,
                            'bid': quote.bid_price,
                            'ask': quote.ask_price,
                            'spread': quote.ask_price - quote.bid_price,
                            'category': category
                        }
                        
                        print(f"  {symbol:6} | ${mid_price:8.2f} | Spread: ${quote.ask_price - quote.bid_price:5.2f}")
                    else:
                        print(f"  {symbol:6} | No quote available")
                
                trading_data[category] = category_data
                
            except Exception as e:
                print(f"  ❌ Error getting quotes for {category}: {e}")
        
        # Save to JSON
        with open('trading_universe.json', 'w') as f:
            json.dump(trading_data, f, indent=2, default=str)
        
        print(f"\n✅ Trading universe saved to trading_universe.json")
        return trading_data
    
    def create_config_file(self):
        """Create configuration file for C++ application"""
        print("\n⚙️  Creating Configuration File")
        print("=" * 35)
        
        config = {
            "alpaca_api": {
                "api_key": self.api_key,
                "secret_key": self.secret_key,
                "base_url": self.base_url,
                "paper_trading": True
            },
            "trading_parameters": {
                "initial_capital": 100000.0,
                "max_position_size": 0.05,
                "stop_loss_pct": 0.05,
                "take_profit_pct": 0.15,
                "max_positions": 10,
                "risk_per_trade": 0.02
            },
            "strategy_config": {
                "momentum_lookback": 20,
                "momentum_threshold": 0.02,
                "sma_fast": 5,
                "sma_slow": 20,
                "rsi_oversold": 30,
                "rsi_overbought": 70
            },
            "universe": [
                "AAPL", "MSFT", "GOOGL", "AMZN", "NVDA", "META", "TSLA",
                "PLTR", "SNOW", "CRWD", "DDOG", "MDB", "NET", "OKTA",
                "SPY", "QQQ", "VTI", "ARKK", "XLK", "SMH",
                "JNJ", "PG", "KO", "WMT", "BRK.B", "VZ"
            ],
            "risk_management": {
                "max_portfolio_heat": 0.20,
                "max_correlation": 0.7,
                "var_confidence": 0.95,
                "max_drawdown": 0.10
            }
        }
        
        with open('live_trading_config.json', 'w') as f:
            json.dump(config, f, indent=2)
        
        print("✅ Configuration saved to live_trading_config.json")
        return config
    
    def download_historical_data(self, days_back: int = 30):
        """Download historical data for backtesting"""
        print(f"\n📊 Downloading {days_back} days of historical data")
        print("=" * 50)
        
        symbols = ['AAPL', 'MSFT', 'GOOGL', 'NVDA', 'TSLA', 'PLTR', 'SPY', 'QQQ']
        
        try:
            end_date = datetime.now()
            start_date = end_date - timedelta(days=days_back)
            
            for symbol in symbols:
                try:
                    bars = self.api.get_bars(
                        symbol,
                        tradeapi.TimeFrame.Day,
                        start_date.strftime('%Y-%m-%d'),
                        end_date.strftime('%Y-%m-%d'),
                        feed='iex'
                    )
                    
                    df = bars.df
                    df.to_csv(f'historical_data_{symbol}.csv')
                    
                    print(f"✅ {symbol}: {len(df)} bars downloaded")
                    
                except Exception as e:
                    print(f"❌ {symbol}: {e}")
                
                time.sleep(0.2)  # Rate limiting
            
            print("✅ Historical data download complete")
            
        except Exception as e:
            print(f"❌ Error downloading historical data: {e}")
    
    def run_setup(self):
        """Run the complete setup process"""
        print("🚀 Alpaca API Setup for Live Paper Trading")
        print("=" * 50)
        
        # Step 1: Get credentials
        if not self.get_credentials():
            return False
        
        # Step 2: Test connection
        if not self.test_connection():
            return False
        
        # Step 3: Check market status
        market_open = self.check_market_status()
        
        # Step 4: Get sample quotes
        self.get_sample_quotes()
        
        # Step 5: Test order placement
        self.test_order_placement()
        
        # Step 6: Generate trading universe
        self.generate_trading_universe()
        
        # Step 7: Create config file
        self.create_config_file()
        
        # Step 8: Download historical data
        self.download_historical_data()
        
        print("\n🎉 Setup Complete!")
        print("=" * 20)
        print("✅ API credentials configured")
        print("✅ Connection tested successfully")
        print("✅ Trading universe generated")
        print("✅ Configuration files created")
        print("✅ Historical data downloaded")
        
        print("\n📋 Next Steps:")
        print("1. Review live_trading_config.json for trading parameters")
        print("2. Check trading_universe.json for symbol prices")
        print("3. Load credentials: source alpaca_env.sh")
        print("4. Compile and run the C++ live trading application")
        
        if market_open:
            print("\n🟢 Market is currently OPEN - Ready for live trading!")
        else:
            print("\n🔴 Market is currently CLOSED - Wait for market hours")
        
        return True

def main():
    """Main setup function"""
    setup = AlpacaSetup()
    
    try:
        setup.run_setup()
        
    except KeyboardInterrupt:
        print("\n\n⚠️  Setup interrupted by user")
        sys.exit(1)
    except Exception as e:
        print(f"\n❌ Setup failed: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()