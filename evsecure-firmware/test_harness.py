#!/usr/bin/env python3
"""
EVsecure Test Harness

This script simulates the EVsecure device and tests anomaly detection
scenarios by sending features to the dashboard API.

Usage:
    python test_harness.py --api-url https://your-dashboard.com --api-key your_key
"""

import argparse
import requests
import json
import time
import random
import numpy as np
from datetime import datetime, timedelta
import threading
import queue
import signal
import sys

class EVsecureSimulator:
    def __init__(self, api_url, api_key, device_id="evsec-esp32-test"):
        self.api_url = api_url.rstrip('/')
        self.api_key = api_key
        self.device_id = device_id
        self.session_id = None
        self.running = False
        self.feature_queue = queue.Queue()
        self.alert_queue = queue.Queue()
        
        # Session state
        self.safety_state = "IDLE"
        self.anomaly_score = 0.0
        self.feature_count = 0
        
        # Baseline values
        self.baseline_thd_i = 3.5
        self.baseline_ocpp_rate = 5.0
        
        # Attack scenarios
        self.attack_scenarios = {
            'remote_stop_burst': False,
            'malformed_messages': False,
            'thd_spike': False,
            'tamper_detection': False,
            'firmware_corruption': False
        }
        
    def generate_session_id(self):
        """Generate a new session ID."""
        timestamp = int(time.time())
        self.session_id = f"sess_{timestamp}_{random.randint(1000, 9999)}"
        return self.session_id
    
    def generate_normal_features(self):
        """Generate normal charging features."""
        # Normal charging parameters
        v_rms = random.normalvariate(230.0, 5.0)  # 230V ± 5V
        i_rms = random.normalvariate(15.0, 2.0)   # 15A ± 2A
        p_kw = (v_rms * i_rms) / 1000
        pf = random.normalvariate(0.95, 0.03)     # Power factor
        pf = max(0.8, min(1.0, pf))
        
        # THD values
        thd_v = random.normalvariate(2.5, 0.5)    # 2-3%
        thd_i = random.normalvariate(3.5, 0.8)    # 3-5%
        
        # Derivatives
        dvdt = random.normalvariate(0, 2)         # V/s
        didt = random.normalvariate(0, 1)         # A/s
        
        # OCPP metrics
        ocpp_rate = random.normalvariate(5.0, 1.0)
        remote_stop_cnt = 0
        malformed = 0
        out_of_seq = 0
        
        # System status
        fw_ok = True
        tamper = False
        
        # Temperature
        temp_c = random.normalvariate(25.0, 5.0)
        
        return {
            'v_rms': round(v_rms, 3),
            'i_rms': round(i_rms, 3),
            'p_kw': round(p_kw, 3),
            'pf': round(pf, 3),
            'thd_v': round(thd_v, 3),
            'thd_i': round(thd_i, 3),
            'dvdt': round(dvdt, 3),
            'didt': round(didt, 3),
            'ocpp_rate': round(ocpp_rate, 3),
            'remote_stop_cnt': remote_stop_cnt,
            'malformed': malformed,
            'out_of_seq': out_of_seq,
            'fw_ok': fw_ok,
            'tamper': tamper,
            'temp_c': round(temp_c, 1)
        }
    
    def inject_attack_features(self, features):
        """Inject attack-specific features."""
        if self.attack_scenarios['remote_stop_burst']:
            features['remote_stop_cnt'] = random.randint(3, 8)
            features['ocpp_rate'] = random.normalvariate(15.0, 3.0)
            
        if self.attack_scenarios['malformed_messages']:
            features['malformed'] = random.randint(2, 5)
            features['out_of_seq'] = random.randint(1, 3)
            
        if self.attack_scenarios['thd_spike']:
            features['thd_i'] = random.normalvariate(12.0, 2.0)  # High THD
            features['ocpp_rate'] = random.normalvariate(2.0, 0.5)  # Low OCPP rate
            
        if self.attack_scenarios['tamper_detection']:
            features['tamper'] = True
            features['v_rms'] = random.normalvariate(180.0, 10.0)  # Abnormal voltage
            features['i_rms'] = random.normalvariate(25.0, 5.0)    # Abnormal current
            
        if self.attack_scenarios['firmware_corruption']:
            features['fw_ok'] = False
            
        return features
    
    def calculate_anomaly_score(self, features):
        """Calculate anomaly score using rule-based and ML components."""
        rule_score = 0.0
        ml_score = 0.0
        
        # Rule-based scoring
        if features['remote_stop_cnt'] > 3:
            rule_score += 0.6
            
        if features['malformed'] > 2 or features['out_of_seq'] > 2:
            rule_score += 0.4
            
        if (features['thd_i'] > self.baseline_thd_i * 1.5 and 
            features['ocpp_rate'] < self.baseline_ocpp_rate * 0.6):
            rule_score += 0.5
            
        if features['tamper'] or not features['fw_ok']:
            rule_score = 1.0  # Critical
            
        # ML score (simulated autoencoder reconstruction error)
        # In a real implementation, this would come from the TFLite model
        ml_score = random.uniform(0.0, 0.3)  # Normal range
        if rule_score > 0.5:
            ml_score = random.uniform(0.3, 0.8)  # Higher for anomalies
            
        # Combined score
        combined_score = 0.6 * rule_score + 0.4 * ml_score
        return min(1.0, combined_score)
    
    def send_features(self, features):
        """Send features to the dashboard API."""
        payload = {
            'ts': int(time.time()),
            'device_id': self.device_id,
            'session_id': self.session_id,
            'v_rms': features['v_rms'],
            'i_rms': features['i_rms'],
            'p_kw': features['p_kw'],
            'pf': features['pf'],
            'thd_v': features['thd_v'],
            'thd_i': features['thd_i'],
            'dvdt': features['dvdt'],
            'didt': features['didt'],
            'ocpp': {
                'rate': features['ocpp_rate'],
                'remote_stop_cnt': features['remote_stop_cnt'],
                'malformed': features['malformed'],
                'out_of_seq': features['out_of_seq']
            },
            'fw_hash_ok': features['fw_ok'],
            'tamper': features['tamper'],
            'temp_c': features['temp_c']
        }
        
        headers = {
            'Content-Type': 'application/json',
            'Authorization': f'ApiKey {self.api_key}'
        }
        
        try:
            response = requests.post(
                f"{self.api_url}/ingest/features",
                json=payload,
                headers=headers,
                timeout=10
            )
            
            if response.status_code == 200:
                print(f"✓ Features sent successfully (score: {self.anomaly_score:.3f})")
            else:
                print(f"✗ Failed to send features: {response.status_code} - {response.text}")
                
        except requests.exceptions.RequestException as e:
            print(f"✗ Network error sending features: {e}")
    
    def send_alert(self, level, score):
        """Send alert to the dashboard API."""
        payload = {
            'device_id': self.device_id,
            'session_id': self.session_id,
            'timestamp': int(time.time()),
            'level': level,
            'score': score
        }
        
        headers = {
            'Content-Type': 'application/json',
            'Authorization': f'ApiKey {self.api_key}'
        }
        
        try:
            response = requests.post(
                f"{self.api_url}/ingest/alerts",
                json=payload,
                headers=headers,
                timeout=10
            )
            
            if response.status_code == 200:
                print(f"✓ Alert sent successfully (level: {level}, score: {score:.3f})")
            else:
                print(f"✗ Failed to send alert: {response.status_code} - {response.text}")
                
        except requests.exceptions.RequestException as e:
            print(f"✗ Network error sending alert: {e}")
    
    def feature_generator(self):
        """Generate features at regular intervals."""
        while self.running:
            # Generate features
            features = self.generate_normal_features()
            features = self.inject_attack_features(features)
            
            # Calculate anomaly score
            self.anomaly_score = self.calculate_anomaly_score(features)
            
            # Update safety state
            if self.anomaly_score >= 0.8:
                self.safety_state = "LOCKDOWN"
            elif self.anomaly_score >= 0.5:
                self.safety_state = "SUSPICIOUS"
            elif self.feature_count < 10:
                self.safety_state = "HANDSHAKE"
            else:
                self.safety_state = "CHARGING"
            
            # Send features
            self.send_features(features)
            
            # Send alert if threshold exceeded
            if self.anomaly_score >= 0.5:
                level = "CRITICAL" if self.anomaly_score >= 0.8 else "WARNING"
                self.send_alert(level, self.anomaly_score)
            
            self.feature_count += 1
            
            # Wait for next sample
            time.sleep(0.25)  # 250ms intervals
    
    def start_attack_scenario(self, scenario_name, duration_seconds=30):
        """Start a specific attack scenario."""
        if scenario_name in self.attack_scenarios:
            print(f"🚨 Starting attack scenario: {scenario_name}")
            self.attack_scenarios[scenario_name] = True
            
            # Stop the attack after duration
            def stop_attack():
                time.sleep(duration_seconds)
                self.attack_scenarios[scenario_name] = False
                print(f"✅ Attack scenario ended: {scenario_name}")
            
            threading.Thread(target=stop_attack, daemon=True).start()
        else:
            print(f"❌ Unknown attack scenario: {scenario_name}")
    
    def start(self):
        """Start the simulator."""
        print(f"🚀 Starting EVsecure simulator...")
        print(f"   Device ID: {self.device_id}")
        print(f"   API URL: {self.api_url}")
        print(f"   Session ID: {self.generate_session_id()}")
        
        self.running = True
        
        # Start feature generation
        feature_thread = threading.Thread(target=self.feature_generator, daemon=True)
        feature_thread.start()
        
        print("📊 Feature generation started (250ms intervals)")
        print("💡 Available commands:")
        print("   remote_stop - Simulate remote stop burst attack")
        print("   malformed - Simulate malformed message attack")
        print("   thd - Simulate THD spike attack")
        print("   tamper - Simulate tampering detection")
        print("   firmware - Simulate firmware corruption")
        print("   quit - Stop the simulator")
        
        # Command loop
        try:
            while self.running:
                command = input("> ").strip().lower()
                
                if command == "quit":
                    break
                elif command == "remote_stop":
                    self.start_attack_scenario("remote_stop_burst")
                elif command == "malformed":
                    self.start_attack_scenario("malformed_messages")
                elif command == "thd":
                    self.start_attack_scenario("thd_spike")
                elif command == "tamper":
                    self.start_attack_scenario("tamper_detection")
                elif command == "firmware":
                    self.start_attack_scenario("firmware_corruption")
                elif command == "status":
                    print(f"Status: {self.safety_state}, Score: {self.anomaly_score:.3f}")
                elif command == "help":
                    print("Available commands: remote_stop, malformed, thd, tamper, firmware, status, quit")
                else:
                    print("Unknown command. Type 'help' for available commands.")
                    
        except KeyboardInterrupt:
            print("\n⚠️ Interrupted by user")
        
        self.stop()
    
    def stop(self):
        """Stop the simulator."""
        print("🛑 Stopping EVsecure simulator...")
        self.running = False

def main():
    parser = argparse.ArgumentParser(description='EVsecure device simulator and test harness')
    parser.add_argument('--api-url', required=True, help='Dashboard API URL')
    parser.add_argument('--api-key', required=True, help='API key for authentication')
    parser.add_argument('--device-id', default='evsec-esp32-test', help='Device ID')
    parser.add_argument('--auto-attack', help='Automatically start attack scenario')
    parser.add_argument('--attack-duration', type=int, default=30, help='Attack duration in seconds')
    
    args = parser.parse_args()
    
    # Create simulator
    simulator = EVsecureSimulator(args.api_url, args.api_key, args.device_id)
    
    # Handle auto-attack
    if args.auto_attack:
        def delayed_attack():
            time.sleep(10)  # Wait 10 seconds before starting attack
            simulator.start_attack_scenario(args.auto_attack, args.attack_duration)
        
        threading.Thread(target=delayed_attack, daemon=True).start()
    
    # Start simulator
    try:
        simulator.start()
    except KeyboardInterrupt:
        print("\n⚠️ Interrupted by user")
    finally:
        simulator.stop()

if __name__ == '__main__':
    main()
