#!/usr/bin/env python3
"""
Advanced Paper Trading Setup Script
===================================

This script helps set up the advanced paper trading environment with Alpaca API
and provides utilities for monitoring and analysis.
"""

import os
import json
import requests
import pandas as pd
from datetime import datetime, timedelta
import yfinance as yf
import numpy as np
from typing import Dict, List, Tuple
import matplotlib.pyplot as plt
import seaborn as sns

class PaperTradingSetup:
    def __init__(self):
        self.config = {}
        self.universe = [
            # Large Cap Growth
            "AAPL", "MSFT", "GOOGL", "AMZN", "TSLA", "NVDA", "META",
            # Diversified Sectors
            "JPM", "JNJ", "PG", "HD", "BA", "XOM",
            # Growth & Innovation
            "PLTR", "SNOW", "CRWD", "ZS", "DDOG", "MDB",
            # ETFs
            "SPY", "QQQ", "IWM", "VTI", "ARKK", "ICLN",
            # Defensive
            "BRK.B", "WMT", "KO", "PFE", "VZ", "T"
        ]
        
    def setup_alpaca_credentials(self):
        """Setup Alpaca API credentials"""
        print("Setting up Alpaca API credentials...")
        print("Please visit: https://alpaca.markets/")
        print("1. Create an account")
        print("2. Go to 'Paper Trading' section")
        print("3. Generate API keys")
        
        api_key = input("Enter your Alpaca API Key: ")
        secret_key = input("Enter your Alpaca Secret Key: ")
        
        self.config['alpaca'] = {
            'api_key': api_key,
            'secret_key': secret_key,
            'base_url': 'https://paper-api.alpaca.markets',
            'data_url': 'https://data.alpaca.markets/v2'
        }
        
        # Test connection
        if self.test_alpaca_connection():
            print("✅ Alpaca API connection successful!")
            return True
        else:
            print("❌ Alpaca API connection failed!")
            return False
    
    def test_alpaca_connection(self):
        """Test Alpaca API connection"""
        try:
            import alpaca_trade_api as tradeapi
            api = tradeapi.REST(
                self.config['alpaca']['api_key'],
                self.config['alpaca']['secret_key'],
                self.config['alpaca']['base_url'],
                api_version='v2'
            )
            account = api.get_account()
            print(f"Account Status: {account.status}")
            print(f"Buying Power: ${float(account.buying_power):,.2f}")
            return True
        except Exception as e:
            print(f"Connection error: {e}")
            return False
    
    def download_market_data(self, days_back: int = 365):
        """Download historical market data for backtesting"""
        print(f"Downloading {days_back} days of market data...")
        
        end_date = datetime.now()
        start_date = end_date - timedelta(days=days_back)
        
        data = {}
        for symbol in self.universe:
            try:
                ticker = yf.Ticker(symbol)
                hist = ticker.history(start=start_date, end=end_date)
                data[symbol] = hist
                print(f"✅ Downloaded {symbol}: {len(hist)} days")
            except Exception as e:
                print(f"❌ Failed to download {symbol}: {e}")
        
        # Save to CSV
        os.makedirs('data', exist_ok=True)
        for symbol, df in data.items():
            df.to_csv(f'data/{symbol}_history.csv')
        
        print(f"Market data saved to 'data' directory")
        return data
    
    def calculate_technical_indicators(self, data: pd.DataFrame) -> pd.DataFrame:
        """Calculate technical indicators for ML features"""
        df = data.copy()
        
        # Price-based indicators
        df['SMA_20'] = df['Close'].rolling(window=20).mean()
        df['SMA_50'] = df['Close'].rolling(window=50).mean()
        df['EMA_12'] = df['Close'].ewm(span=12).mean()
        df['EMA_26'] = df['Close'].ewm(span=26).mean()
        
        # RSI
        delta = df['Close'].diff()
        gain = (delta.where(delta > 0, 0)).rolling(window=14).mean()
        loss = (-delta.where(delta < 0, 0)).rolling(window=14).mean()
        rs = gain / loss
        df['RSI'] = 100 - (100 / (1 + rs))
        
        # MACD
        df['MACD'] = df['EMA_12'] - df['EMA_26']
        df['MACD_Signal'] = df['MACD'].ewm(span=9).mean()
        df['MACD_Histogram'] = df['MACD'] - df['MACD_Signal']
        
        # Bollinger Bands
        df['BB_Middle'] = df['Close'].rolling(window=20).mean()
        bb_std = df['Close'].rolling(window=20).std()
        df['BB_Upper'] = df['BB_Middle'] + (bb_std * 2)
        df['BB_Lower'] = df['BB_Middle'] - (bb_std * 2)
        df['BB_Width'] = df['BB_Upper'] - df['BB_Lower']
        df['BB_Position'] = (df['Close'] - df['BB_Lower']) / (df['BB_Upper'] - df['BB_Lower'])
        
        # Volume indicators
        df['Volume_SMA'] = df['Volume'].rolling(window=20).mean()
        df['Volume_Ratio'] = df['Volume'] / df['Volume_SMA']
        
        # Momentum
        df['Momentum_10'] = df['Close'] / df['Close'].shift(10) - 1
        df['Momentum_20'] = df['Close'] / df['Close'].shift(20) - 1
        df['Momentum_50'] = df['Close'] / df['Close'].shift(50) - 1
        
        # Volatility
        df['Volatility_20'] = df['Close'].pct_change().rolling(window=20).std()
        
        # Average True Range
        df['TR'] = np.maximum(df['High'] - df['Low'], 
                             np.maximum(abs(df['High'] - df['Close'].shift(1)),
                                       abs(df['Low'] - df['Close'].shift(1))))
        df['ATR'] = df['TR'].rolling(window=14).mean()
        
        return df
    
    def analyze_correlations(self, data: Dict[str, pd.DataFrame]) -> pd.DataFrame:
        """Analyze correlations between assets"""
        returns = pd.DataFrame()
        
        for symbol, df in data.items():
            if len(df) > 1:
                returns[symbol] = df['Close'].pct_change()
        
        correlation_matrix = returns.corr()
        
        # Plot heatmap
        plt.figure(figsize=(12, 10))
        sns.heatmap(correlation_matrix, annot=True, cmap='coolwarm', center=0)
        plt.title('Asset Correlation Matrix')
        plt.tight_layout()
        plt.savefig('correlation_heatmap.png')
        plt.close()
        
        return correlation_matrix
    
    def generate_risk_metrics(self, data: Dict[str, pd.DataFrame]) -> Dict:
        """Generate risk metrics for each asset"""
        risk_metrics = {}
        
        for symbol, df in data.items():
            if len(df) > 252:  # Need at least 1 year of data
                returns = df['Close'].pct_change().dropna()
                
                risk_metrics[symbol] = {
                    'annual_return': returns.mean() * 252,
                    'annual_volatility': returns.std() * np.sqrt(252),
                    'sharpe_ratio': (returns.mean() * 252) / (returns.std() * np.sqrt(252)),
                    'max_drawdown': self.calculate_max_drawdown(df['Close']),
                    'var_95': returns.quantile(0.05),
                    'skewness': returns.skew(),
                    'kurtosis': returns.kurtosis()
                }
        
        return risk_metrics
    
    def calculate_max_drawdown(self, prices: pd.Series) -> float:
        """Calculate maximum drawdown"""
        cumulative = (1 + prices.pct_change()).cumprod()
        running_max = cumulative.expanding().max()
        drawdown = (cumulative - running_max) / running_max
        return drawdown.min()
    
    def create_portfolio_suggestions(self, risk_metrics: Dict) -> Dict:
        """Create portfolio allocation suggestions"""
        # Risk-adjusted scoring
        scores = {}
        for symbol, metrics in risk_metrics.items():
            if metrics['annual_volatility'] > 0:
                score = (metrics['annual_return'] / metrics['annual_volatility']) * (1 + metrics['sharpe_ratio'])
                scores[symbol] = score
        
        # Sort by score
        sorted_scores = sorted(scores.items(), key=lambda x: x[1], reverse=True)
        
        # Create allocation suggestions
        portfolios = {
            'aggressive': self.create_aggressive_portfolio(sorted_scores),
            'moderate': self.create_moderate_portfolio(sorted_scores),
            'conservative': self.create_conservative_portfolio(sorted_scores)
        }
        
        return portfolios
    
    def create_aggressive_portfolio(self, sorted_scores: List[Tuple[str, float]]) -> Dict:
        """Create aggressive portfolio allocation"""
        allocation = {}
        total_weight = 0
        
        # Top growth stocks (70%)
        growth_stocks = ['NVDA', 'TSLA', 'PLTR', 'SNOW', 'CRWD', 'DDOG', 'MDB']
        for symbol in growth_stocks:
            if any(symbol == s[0] for s in sorted_scores[:15]):
                allocation[symbol] = 0.10
                total_weight += 0.10
        
        # Technology ETFs (20%)
        allocation['QQQ'] = 0.15
        allocation['ARKK'] = 0.05
        total_weight += 0.20
        
        # Cash (10%)
        allocation['CASH'] = 0.10
        total_weight += 0.10
        
        return allocation
    
    def create_moderate_portfolio(self, sorted_scores: List[Tuple[str, float]]) -> Dict:
        """Create moderate portfolio allocation"""
        allocation = {}
        
        # Large cap core (50%)
        core_stocks = ['AAPL', 'MSFT', 'GOOGL', 'AMZN']
        for symbol in core_stocks:
            allocation[symbol] = 0.125
        
        # Sector diversification (30%)
        sector_stocks = ['JPM', 'JNJ', 'PG', 'HD', 'BA', 'XOM']
        for symbol in sector_stocks:
            allocation[symbol] = 0.05
        
        # ETFs (15%)
        allocation['SPY'] = 0.10
        allocation['VTI'] = 0.05
        
        # Cash (5%)
        allocation['CASH'] = 0.05
        
        return allocation
    
    def create_conservative_portfolio(self, sorted_scores: List[Tuple[str, float]]) -> Dict:
        """Create conservative portfolio allocation"""
        allocation = {}
        
        # Defensive stocks (40%)
        defensive_stocks = ['BRK.B', 'WMT', 'KO', 'PFE', 'VZ', 'T', 'JNJ', 'PG']
        for symbol in defensive_stocks:
            allocation[symbol] = 0.05
        
        # Broad market ETFs (40%)
        allocation['SPY'] = 0.25
        allocation['VTI'] = 0.15
        
        # Cash (20%)
        allocation['CASH'] = 0.20
        
        return allocation
    
    def generate_trading_calendar(self, start_date: datetime, end_date: datetime) -> List[datetime]:
        """Generate trading calendar (weekdays, excluding holidays)"""
        trading_days = []
        current_date = start_date
        
        while current_date <= end_date:
            # Skip weekends
            if current_date.weekday() < 5:  # Monday = 0, Friday = 4
                trading_days.append(current_date)
            current_date += timedelta(days=1)
        
        return trading_days
    
    def save_configuration(self):
        """Save configuration to files"""
        # Save main config
        with open('config.json', 'w') as f:
            json.dump(self.config, f, indent=2)
        
        # Save universe
        with open('universe.json', 'w') as f:
            json.dump(self.universe, f, indent=2)
        
        print("Configuration saved to config.json and universe.json")
    
    def run_setup(self):
        """Run the complete setup process"""
        print("🚀 Advanced Paper Trading Setup")
        print("=" * 50)
        
        # 1. Setup Alpaca credentials
        if not self.setup_alpaca_credentials():
            return False
        
        # 2. Download market data
        market_data = self.download_market_data(days_back=500)
        
        # 3. Calculate technical indicators
        print("Calculating technical indicators...")
        enhanced_data = {}
        for symbol, df in market_data.items():
            enhanced_data[symbol] = self.calculate_technical_indicators(df)
        
        # 4. Analyze correlations
        print("Analyzing asset correlations...")
        correlation_matrix = self.analyze_correlations(market_data)
        
        # 5. Generate risk metrics
        print("Generating risk metrics...")
        risk_metrics = self.generate_risk_metrics(market_data)
        
        # 6. Create portfolio suggestions
        print("Creating portfolio suggestions...")
        portfolios = self.create_portfolio_suggestions(risk_metrics)
        
        # 7. Save everything
        self.save_configuration()
        
        # Save analysis results
        with open('risk_metrics.json', 'w') as f:
            json.dump(risk_metrics, f, indent=2, default=str)
        
        with open('portfolio_suggestions.json', 'w') as f:
            json.dump(portfolios, f, indent=2)
        
        correlation_matrix.to_csv('correlation_matrix.csv')
        
        print("\n✅ Setup completed successfully!")
        print("\nNext steps:")
        print("1. Review portfolio_suggestions.json for allocation ideas")
        print("2. Check risk_metrics.json for individual asset analysis")
        print("3. Examine correlation_heatmap.png for diversification insights")
        print("4. Compile and run the advanced_paper_trading.cpp")
        print("5. Monitor performance and adjust strategies as needed")
        
        return True

def main():
    setup = PaperTradingSetup()
    setup.run_setup()

if __name__ == "__main__":
    main()