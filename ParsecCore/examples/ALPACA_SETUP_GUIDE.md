# Alpaca API Live Paper Trading Setup Guide

## 🚀 Overview

This guide walks you through setting up live paper trading with the Alpaca API integration. You'll learn how to:
- Get Alpaca API credentials
- Set up the development environment
- Configure the trading bot
- Run live paper trading sessions
- Monitor and analyze performance

## 📋 Prerequisites

### Required Accounts
1. **Alpaca Markets Account** (Free)
   - Visit: https://alpaca.markets/
   - Create account and verify email
   - Access paper trading immediately (no funding required)

### Required Software
- **C++ Compiler**: Visual Studio 2022 or MinGW
- **CMake**: Version 3.14 or higher
- **Python**: Version 3.8 or higher (for setup scripts)

### Python Dependencies
```bash
pip install alpaca-trade-api pandas numpy requests
```

## 🔑 Step 1: Get Alpaca API Credentials

### Create Alpaca Account
1. Go to https://alpaca.markets/
2. Click "Sign Up" and create account
3. Verify your email address
4. Complete basic profile information

### Generate API Keys
1. Log into your Alpaca dashboard
2. Navigate to "Paper Trading" section
3. Click "Generate API Keys"
4. Copy your API Key and Secret Key
5. **Important**: Keep these credentials secure!

### Paper Trading vs Live Trading
- **Paper Trading**: Free, uses simulated money ($100k default)
- **Live Trading**: Requires account funding and additional verification
- **Recommendation**: Start with paper trading for testing

## 🛠️ Step 2: Setup Development Environment

### Clone and Build
```bash
# Navigate to ParsecCore directory
cd ParsecCore

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build the project
cmake --build . --config Debug
```

### Install Python Dependencies
```bash
# Install required Python packages
pip install alpaca-trade-api pandas numpy matplotlib seaborn requests

# Run setup script
python ../examples/setup_alpaca_api.py
```

## ⚙️ Step 3: Configure API Credentials

### Method 1: Environment Variables (Recommended)
```bash
# Windows
set ALPACA_API_KEY=your_api_key_here
set ALPACA_SECRET_KEY=your_secret_key_here

# Linux/Mac
export ALPACA_API_KEY=your_api_key_here
export ALPACA_SECRET_KEY=your_secret_key_here
```

### Method 2: Direct Input
The application will prompt for credentials if environment variables are not set.

### Method 3: Configuration File
Create `alpaca_credentials.json`:
```json
{
  "api_key": "your_api_key_here",
  "secret_key": "your_secret_key_here",
  "paper_trading": true
}
```

## 🔧 Step 4: Run Setup Script

```bash
# Navigate to examples directory
cd examples

# Run the setup script
python setup_alpaca_api.py
```

### What the Setup Script Does:
1. **Tests API Connection**: Verifies credentials work
2. **Checks Market Status**: Shows if market is open/closed
3. **Downloads Sample Data**: Gets recent price data
4. **Generates Trading Universe**: Creates symbol list with prices
5. **Creates Config Files**: Sets up trading parameters

### Expected Output:
```
🚀 Alpaca API Setup for Live Paper Trading
==================================================

🔑 Setting up Alpaca API Credentials
✅ Found credentials in environment variables

🔗 Testing API Connection
✅ Connection successful!
Account ID: 12345678-1234-1234-1234-123456789012
Account Status: ACTIVE
Buying Power: $100,000.00

📊 Market Status
Market Open: True
Current Time: 2024-01-15 14:30:00-05:00

🎉 Setup Complete!
```

## 🎯 Step 5: Build and Run Live Trading

### Build the Live Trading Application
```bash
# From ParsecCore/build directory
cmake --build . --config Debug --target alpaca_live_trading
```

### Run the Live Trading Bot
```bash
# Navigate to Debug directory
cd Debug

# Run with environment variables
alpaca_live_trading.exe

# Or run with direct credential input
alpaca_live_trading.exe
```

## 📊 Step 6: Monitor Live Trading

### Real-time Dashboard
The application displays a live dashboard every 5 cycles:

```
======================================================================
🎯 LIVE TRADING DASHBOARD
======================================================================
Runtime: 45 minutes
Market Status: OPEN 🟢

💰 Portfolio Summary:
Portfolio Value: $102,350.00
Cash Balance: $87,500.00
Total Return: 2.35%

📊 Active Positions:
  NVDA  : 17 @ $875.30 = $14,880.10 (14.5%)
  AAPL  : 54 @ $185.20 = $10,000.80 (9.7%)
  MSFT  : 24 @ $420.50 = $10,092.00 (9.8%)

📈 Current Prices (Top Picks):
  NVDA: $875.30
  AAPL: $185.20
  MSFT: $420.50
  GOOGL: $140.80
  PLTR: $18.75
  TSLA: $242.10
======================================================================
```

### Key Metrics to Watch:
- **Portfolio Value**: Total account value
- **Total Return**: Percentage gain/loss
- **Active Positions**: Current stock holdings
- **Signal Generation**: BUY/SELL signals with confidence
- **Market Status**: OPEN/CLOSED indicator

## 🎛️ Step 7: Trading Strategy Configuration

### Default Parameters
```cpp
// Position sizing
double position_size_pct_ = 0.05;  // 5% of portfolio per trade

// Technical indicators
int sma_fast_ = 5;                 // Fast moving average
int sma_slow_ = 20;                // Slow moving average
double momentum_threshold_ = 0.02; // 2% momentum threshold

// Risk management
double stop_loss_pct_ = 0.05;      // 5% stop loss
double take_profit_pct_ = 0.15;    // 15% take profit
```

### Strategy Logic
1. **Momentum Strategy**: Buy when fast SMA > slow SMA + threshold
2. **Mean Reversion**: Sell when fast SMA < slow SMA - threshold
3. **Position Sizing**: 5% of portfolio per trade
4. **Risk Management**: Stop losses and take profits

### Customization Options
You can modify the strategy by editing `alpaca_simple.cpp`:
- Adjust SMA periods
- Change momentum thresholds
- Modify position sizing
- Add new technical indicators

## 🏆 Step 8: 2024 Investment Recommendations

### Top AI/Technology Picks
- **NVDA** ($875) - AI chip leader with massive growth potential
- **MSFT** ($420) - Cloud computing + AI integration
- **GOOGL** ($140) - Undervalued AI leader with search dominance
- **PLTR** ($18.50) - Government AI contracts, strong fundamentals

### Diversified Holdings
- **AAPL** ($185) - Stable ecosystem with AI hardware potential
- **AMZN** ($155) - E-commerce dominance + AWS growth
- **SPY** ($485) - Broad market exposure for stability
- **QQQ** ($415) - Technology sector concentration

### Growth Opportunities
- **TSLA** ($240) - Electric vehicle + energy storage leader
- **SNOW** ($140) - Cloud data platform with enterprise adoption
- **CRWD** ($285) - Cybersecurity growth in AI era
- **ARKK** ($48) - Disruptive innovation ETF

## 🎮 Step 9: Demo Session

### Quick Start Demo
```bash
# 1. Set credentials
set ALPACA_API_KEY=your_key
set ALPACA_SECRET_KEY=your_secret

# 2. Run 30-minute demo
alpaca_live_trading.exe

# Expected output:
# - Connection test
# - Market status check
# - Signal generation
# - Order execution
# - Portfolio updates
```

### Demo Features
- **Real-time price simulation** for 20 stocks
- **Momentum-based signal generation**
- **Automatic order execution**
- **Portfolio value tracking**
- **Risk management alerts**

## 📈 Step 10: Performance Analysis

### Export Results
The application automatically exports results to:
- `live_trading_results.txt` - Session summary
- `live_trading_results.csv` - Detailed performance data (if using Python setup)

### Key Performance Metrics
- **Total Return**: Overall profit/loss percentage
- **Sharpe Ratio**: Risk-adjusted return measure
- **Maximum Drawdown**: Largest portfolio decline
- **Win Rate**: Percentage of profitable trades

### Performance Targets
- **Monthly Return**: 2-5%
- **Sharpe Ratio**: > 1.0
- **Maximum Drawdown**: < 10%
- **Win Rate**: > 55%

## 🚨 Step 11: Risk Management

### Built-in Risk Controls
- **Position Limits**: Maximum 5% per position
- **Stop Losses**: 5% automatic stop losses
- **Cash Management**: Always maintain cash reserves
- **Correlation Limits**: Avoid concentrated positions

### Manual Risk Controls
- **Daily Monitoring**: Check dashboard regularly
- **Position Sizing**: Never risk more than 2% per trade
- **Diversification**: Spread risk across sectors
- **Market Conditions**: Adjust strategy based on volatility

## 🔧 Step 12: Troubleshooting

### Common Issues

#### API Connection Errors
```
❌ Failed to connect to Alpaca API
```
**Solution**: 
- Check API credentials
- Verify internet connection
- Ensure Alpaca account is active

#### Market Closed
```
📊 Market Status: CLOSED 🔴
```
**Solution**: 
- Bot will simulate trading when market is closed
- Check market hours (9:30 AM - 4:00 PM EST)
- Wait for market open for live trading

#### No Signals Generated
```
Generated 0 trading signals
```
**Solution**: 
- Market may be ranging (no clear trends)
- Adjust momentum threshold
- Check price data quality

### Advanced Troubleshooting
```bash
# Check API status
curl -H "APCA-API-KEY-ID: your_key" \
     -H "APCA-API-SECRET-KEY: your_secret" \
     https://paper-api.alpaca.markets/v2/account

# Verify market hours
curl https://paper-api.alpaca.markets/v2/clock

# Test order submission
# (See full API documentation at https://alpaca.markets/docs/)
```

## 📚 Step 13: Additional Resources

### Alpaca API Documentation
- **Main Docs**: https://alpaca.markets/docs/
- **Python SDK**: https://github.com/alpacahq/alpaca-trade-api-python
- **Market Data**: https://alpaca.markets/docs/market-data/
- **Paper Trading**: https://alpaca.markets/docs/trading/paper-trading/

### Trading Strategy Resources
- **Technical Analysis**: https://www.investopedia.com/technical-analysis/
- **Algorithmic Trading**: https://www.quantstart.com/
- **Risk Management**: https://www.investopedia.com/risk-management/

### Community and Support
- **Alpaca Community**: https://forum.alpaca.markets/
- **Discord**: https://discord.gg/alpaca
- **GitHub Issues**: https://github.com/alpacahq/alpaca-trade-api-python/issues

## 🎉 Step 14: Next Steps

### Phase 1: Extended Testing (1-2 weeks)
1. Run extended paper trading sessions
2. Monitor performance metrics
3. Adjust strategy parameters
4. Test different market conditions

### Phase 2: Strategy Enhancement (2-4 weeks)
1. Add more technical indicators
2. Implement ML-based signals
3. Enhance risk management
4. Add sector rotation logic

### Phase 3: Production Readiness (2-4 weeks)
1. Add proper error handling
2. Implement database logging
3. Create monitoring dashboard
4. Add automated alerts

### Phase 4: Live Trading (When Ready)
1. Switch to live API endpoints
2. Start with small capital
3. Monitor performance closely
4. Scale gradually

## ⚠️ Important Disclaimers

### Risk Warnings
- **Paper Trading**: Uses simulated money, not real funds
- **Live Trading**: Involves real financial risk
- **Market Risk**: All investments can lose value
- **Algorithm Risk**: Automated systems can malfunction

### Compliance Notes
- **Regulatory**: Ensure compliance with local regulations
- **Tax Implications**: Consult tax advisor for trading activities
- **Financial Advice**: This is not investment advice
- **Due Diligence**: Perform your own research

### Best Practices
- **Start Small**: Begin with paper trading
- **Continuous Learning**: Stay updated on market conditions
- **Risk Management**: Never risk more than you can afford to lose
- **Diversification**: Don't put all eggs in one basket

## 📞 Support

If you encounter issues:
1. Check this guide first
2. Review error messages carefully
3. Test with paper trading
4. Consult Alpaca documentation
5. Join community forums

---

**Happy Trading! 🚀📈**

Remember: The goal is to build a systematic, disciplined approach to trading that can generate consistent returns while managing risk effectively. Start with paper trading, learn from the experience, and gradually build confidence before moving to live trading.