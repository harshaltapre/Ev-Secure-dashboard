#!/usr/bin/env python3
"""
Test script for EV-Secure Dashboard API endpoints
This script tests all the API endpoints to ensure they're working correctly
"""

import requests
import json
import time
from datetime import datetime

# Configuration
BASE_URL = "http://localhost:3000"  # Change this to your actual URL
API_KEY = "vsr_st001_abc123def456"   # Use the generated API key from settings

# Test data
test_sensor_data = {
    "device_id": "EV_SORA_001",
    "sensor_data": {
        "current": 15.5,
        "voltage": 220.0,
        "power": 3410.0,
        "frequency": 50.0,
        "temperature": 25.5,
        "timestamp": int(time.time() * 1000)
    },
    "ml_prediction": {
        "prediction": "normal",
        "confidence": 0.95,
        "timestamp": int(time.time() * 1000)
    }
}

test_alert_data = {
    "device_id": "EV_SORA_001",
    "alert_type": "current_spike",
    "details": "Current exceeded threshold: 15.5A > 15.0A",
    "severity": "medium",
    "timestamp": int(time.time() * 1000)
}

def test_api_endpoint(endpoint, method="GET", data=None, description=""):
    """Test a single API endpoint"""
    url = f"{BASE_URL}{endpoint}"
    headers = {
        "Authorization": f"Bearer {API_KEY}",
        "Content-Type": "application/json"
    }
    
    print(f"\n{'='*60}")
    print(f"Testing: {description}")
    print(f"URL: {url}")
    print(f"Method: {method}")
    
    try:
        if method == "GET":
            response = requests.get(url, headers=headers, timeout=10)
        elif method == "POST":
            response = requests.post(url, headers=headers, json=data, timeout=10)
        elif method == "DELETE":
            response = requests.delete(url, headers=headers, json=data, timeout=10)
        
        print(f"Status Code: {response.status_code}")
        
        if response.status_code < 400:
            print("✅ SUCCESS")
            try:
                response_data = response.json()
                print(f"Response: {json.dumps(response_data, indent=2)}")
            except:
                print(f"Response: {response.text}")
        else:
            print("❌ FAILED")
            print(f"Error: {response.text}")
            
    except requests.exceptions.RequestException as e:
        print(f"❌ CONNECTION ERROR: {e}")
    except Exception as e:
        print(f"❌ ERROR: {e}")

def main():
    print("EV-Secure Dashboard API Test Suite")
    print(f"Testing against: {BASE_URL}")
    print(f"Using API Key: {API_KEY[:10]}...")
    print(f"Test started at: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    
    # Test 1: Status endpoint
    test_api_endpoint("/api/status", "GET", description="Status Check")
    
    # Test 2: Send sensor data
    test_api_endpoint("/api/data", "POST", test_sensor_data, description="Send Sensor Data")
    
    # Test 3: Get sensor data
    test_api_endpoint("/api/data", "GET", description="Get Sensor Data")
    
    # Test 4: Send alert
    test_api_endpoint("/api/alerts", "POST", test_alert_data, description="Send Alert")
    
    # Test 5: Get alerts
    test_api_endpoint("/api/alerts", "GET", description="Get Alerts")
    
    # Test 6: Get commands (should return empty)
    test_api_endpoint("/api/commands", "GET", description="Get Commands")
    
    # Test 7: Test invalid API key
    print(f"\n{'='*60}")
    print("Testing: Invalid API Key")
    url = f"{BASE_URL}/api/status"
    headers = {
        "Authorization": "Bearer invalid_key_123",
        "Content-Type": "application/json"
    }
    try:
        response = requests.get(url, headers=headers, timeout=10)
        print(f"Status Code: {response.status_code}")
        if response.status_code == 401:
            print("✅ SUCCESS - Correctly rejected invalid key")
        else:
            print("❌ FAILED - Should have rejected invalid key")
    except Exception as e:
        print(f"❌ ERROR: {e}")
    
    # Test 8: Test without API key
    print(f"\n{'='*60}")
    print("Testing: No API Key")
    url = f"{BASE_URL}/api/status"
    headers = {"Content-Type": "application/json"}
    try:
        response = requests.get(url, headers=headers, timeout=10)
        print(f"Status Code: {response.status_code}")
        if response.status_code == 401:
            print("✅ SUCCESS - Correctly rejected request without key")
        else:
            print("❌ FAILED - Should have rejected request without key")
    except Exception as e:
        print(f"❌ ERROR: {e}")
    
    print(f"\n{'='*60}")
    print("Test suite completed!")
    print(f"Completed at: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")

if __name__ == "__main__":
    main()
