#!/usr/bin/env python3
"""
EVsecure Synthetic Data Generator

This script generates synthetic EV charging session data for testing
the anomaly detection system.

Usage:
    python generate_synthetic_data.py --output sessions.csv --sessions 100
"""

import argparse
import pandas as pd
import numpy as np
import random
from datetime import datetime, timedelta
import os

def generate_normal_session(session_id, start_time, duration_minutes):
    """
    Generate a normal charging session.
    
    Args:
        session_id: Session identifier
        start_time: Start time of the session
        duration_minutes: Duration of the session in minutes
        
    Returns:
        DataFrame: Session data
    """
    # Calculate number of samples (250ms intervals)
    num_samples = int(duration_minutes * 60 * 4)  # 4 samples per second
    
    # Generate timestamps
    timestamps = [start_time + timedelta(milliseconds=i*250) for i in range(num_samples)]
    
    # Normal charging parameters
    base_voltage = 230.0  # V
    base_current = 15.0   # A
    base_power = base_voltage * base_current / 1000  # kW
    
    # Add some realistic variation
    voltage_variation = np.random.normal(0, 5, num_samples)  # ±5V
    current_variation = np.random.normal(0, 2, num_samples)  # ±2A
    
    # Generate power measurements
    voltages = base_voltage + voltage_variation
    currents = base_current + current_variation
    powers = (voltages * currents) / 1000
    
    # Power factor (should be close to 1.0 for normal charging)
    power_factors = np.random.normal(0.95, 0.03, num_samples)
    power_factors = np.clip(power_factors, 0.8, 1.0)
    
    # THD (Total Harmonic Distortion) - normal values
    thd_v = np.random.normal(2.5, 0.5, num_samples)  # 2-3% for voltage
    thd_i = np.random.normal(3.5, 0.8, num_samples)  # 3-5% for current
    
    # Derivatives (rate of change)
    dvdt = np.random.normal(0, 2, num_samples)  # V/s
    didt = np.random.normal(0, 1, num_samples)  # A/s
    
    # OCPP metrics (normal operation)
    ocpp_rate = np.random.normal(5.0, 1.0, num_samples)  # messages per second
    remote_stop_cnt = np.zeros(num_samples)  # No remote stops in normal session
    malformed = np.zeros(num_samples)  # No malformed messages
    out_of_seq = np.zeros(num_samples)  # No out-of-sequence messages
    
    # System status
    fw_ok = np.ones(num_samples)  # Firmware OK
    tamper = np.zeros(num_samples)  # No tampering
    
    # Temperature (normal operating temperature)
    temp_c = np.random.normal(25.0, 5.0, num_samples)  # 20-30°C
    
    # Create DataFrame
    data = {
        'timestamp': timestamps,
        'device_id': [f'evsec-esp32-{session_id:03d}'] * num_samples,
        'session_id': [f'sess_{session_id:03d}_{start_time.strftime("%Y%m%d_%H%M%S")}'] * num_samples,
        'v_rms': voltages,
        'i_rms': currents,
        'p_kw': powers,
        'pf': power_factors,
        'thd_v': thd_v,
        'thd_i': thd_i,
        'dvdt': dvdt,
        'didt': didt,
        'ocpp_rate': ocpp_rate,
        'remote_stop_cnt': remote_stop_cnt,
        'malformed': malformed,
        'out_of_seq': out_of_seq,
        'fw_ok': fw_ok,
        'tamper': tamper,
        'temp_c': temp_c
    }
    
    return pd.DataFrame(data)

def generate_anomalous_session(session_id, start_time, duration_minutes, anomaly_type):
    """
    Generate an anomalous charging session.
    
    Args:
        session_id: Session identifier
        start_time: Start time of the session
        duration_minutes: Duration of the session in minutes
        anomaly_type: Type of anomaly ('remote_stop', 'malformed', 'thd', 'tamper')
        
    Returns:
        DataFrame: Session data
    """
    # Generate base normal session
    df = generate_normal_session(session_id, start_time, duration_minutes)
    
    num_samples = len(df)
    
    if anomaly_type == 'remote_stop':
        # Simulate burst of remote stop commands
        burst_start = random.randint(num_samples // 4, 3 * num_samples // 4)
        burst_end = min(burst_start + 10, num_samples)  # 10 samples = 2.5 seconds
        
        df.loc[burst_start:burst_end, 'remote_stop_cnt'] = 1
        df.loc[burst_start:burst_end, 'ocpp_rate'] = np.random.normal(15.0, 3.0, burst_end - burst_start + 1)
        
    elif anomaly_type == 'malformed':
        # Simulate malformed OCPP messages
        malformed_indices = random.sample(range(num_samples), min(20, num_samples // 10))
        df.loc[malformed_indices, 'malformed'] = 1
        df.loc[malformed_indices, 'ocpp_rate'] = np.random.normal(8.0, 2.0, len(malformed_indices))
        
    elif anomaly_type == 'thd':
        # Simulate high THD (harmonic distortion)
        anomaly_start = random.randint(num_samples // 4, 3 * num_samples // 4)
        anomaly_end = min(anomaly_start + 50, num_samples)  # 50 samples = 12.5 seconds
        
        # High THD values
        df.loc[anomaly_start:anomaly_end, 'thd_v'] = np.random.normal(8.0, 1.0, anomaly_end - anomaly_start + 1)
        df.loc[anomaly_start:anomaly_end, 'thd_i'] = np.random.normal(12.0, 2.0, anomaly_end - anomaly_start + 1)
        
        # Reduced OCPP rate during anomaly
        df.loc[anomaly_start:anomaly_end, 'ocpp_rate'] = np.random.normal(2.0, 0.5, anomaly_end - anomaly_start + 1)
        
    elif anomaly_type == 'tamper':
        # Simulate tampering detection
        tamper_start = random.randint(num_samples // 3, 2 * num_samples // 3)
        tamper_end = min(tamper_start + 30, num_samples)  # 30 samples = 7.5 seconds
        
        df.loc[tamper_start:tamper_end, 'tamper'] = 1
        
        # Abnormal power readings during tampering
        df.loc[tamper_start:tamper_end, 'v_rms'] = np.random.normal(180.0, 10.0, tamper_end - tamper_start + 1)
        df.loc[tamper_start:tamper_end, 'i_rms'] = np.random.normal(25.0, 5.0, tamper_end - tamper_start + 1)
        
    elif anomaly_type == 'firmware':
        # Simulate firmware integrity issues
        fw_issue_start = random.randint(num_samples // 4, 3 * num_samples // 4)
        fw_issue_end = min(fw_issue_start + 20, num_samples)  # 20 samples = 5 seconds
        
        df.loc[fw_issue_start:fw_issue_end, 'fw_ok'] = 0
        
    elif anomaly_type == 'out_of_seq':
        # Simulate out-of-sequence OCPP messages
        out_of_seq_indices = random.sample(range(num_samples), min(15, num_samples // 15))
        df.loc[out_of_seq_indices, 'out_of_seq'] = 1
        
    return df

def generate_synthetic_data(num_sessions, anomaly_ratio=0.1, output_file='sessions.csv'):
    """
    Generate synthetic EV charging session data.
    
    Args:
        num_sessions: Number of sessions to generate
        anomaly_ratio: Ratio of anomalous sessions (0.0 to 1.0)
        output_file: Output CSV file path
    """
    print(f"Generating {num_sessions} charging sessions...")
    print(f"Anomaly ratio: {anomaly_ratio:.1%}")
    
    # Calculate number of anomalous sessions
    num_anomalous = int(num_sessions * anomaly_ratio)
    num_normal = num_sessions - num_anomalous
    
    all_sessions = []
    
    # Generate normal sessions
    print(f"Generating {num_normal} normal sessions...")
    for i in range(num_normal):
        session_id = i + 1
        start_time = datetime.now() - timedelta(days=random.randint(1, 30))
        duration = random.randint(30, 180)  # 30 minutes to 3 hours
        
        session_df = generate_normal_session(session_id, start_time, duration)
        all_sessions.append(session_df)
        
        if (i + 1) % 10 == 0:
            print(f"  Generated {i + 1} normal sessions")
    
    # Generate anomalous sessions
    anomaly_types = ['remote_stop', 'malformed', 'thd', 'tamper', 'firmware', 'out_of_seq']
    
    print(f"Generating {num_anomalous} anomalous sessions...")
    for i in range(num_anomalous):
        session_id = num_normal + i + 1
        start_time = datetime.now() - timedelta(days=random.randint(1, 30))
        duration = random.randint(30, 180)  # 30 minutes to 3 hours
        anomaly_type = random.choice(anomaly_types)
        
        session_df = generate_anomalous_session(session_id, start_time, duration, anomaly_type)
        all_sessions.append(session_df)
        
        if (i + 1) % 5 == 0:
            print(f"  Generated {i + 1} anomalous sessions")
    
    # Combine all sessions
    print("Combining all sessions...")
    combined_df = pd.concat(all_sessions, ignore_index=True)
    
    # Sort by timestamp
    combined_df = combined_df.sort_values('timestamp').reset_index(drop=True)
    
    # Save to CSV
    print(f"Saving to {output_file}...")
    combined_df.to_csv(output_file, index=False)
    
    # Print statistics
    print(f"\nGenerated {len(combined_df)} total samples")
    print(f"Time range: {combined_df['timestamp'].min()} to {combined_df['timestamp'].max()}")
    print(f"Number of sessions: {combined_df['session_id'].nunique()}")
    print(f"Number of devices: {combined_df['device_id'].nunique()}")
    
    # Feature statistics
    print("\nFeature statistics:")
    numeric_features = ['v_rms', 'i_rms', 'p_kw', 'pf', 'thd_v', 'thd_i', 'dvdt', 'didt', 'ocpp_rate', 'temp_c']
    for feature in numeric_features:
        if feature in combined_df.columns:
            mean_val = combined_df[feature].mean()
            std_val = combined_df[feature].std()
            print(f"  {feature}: mean={mean_val:.3f}, std={std_val:.3f}")
    
    # Anomaly statistics
    print("\nAnomaly statistics:")
    anomaly_features = ['remote_stop_cnt', 'malformed', 'out_of_seq', 'fw_ok', 'tamper']
    for feature in anomaly_features:
        if feature in combined_df.columns:
            total_anomalies = combined_df[feature].sum()
            print(f"  {feature}: {total_anomalies} total occurrences")
    
    return combined_df

def main():
    parser = argparse.ArgumentParser(description='Generate synthetic EV charging session data')
    parser.add_argument('--output', default='sessions.csv', help='Output CSV file')
    parser.add_argument('--sessions', type=int, default=100, help='Number of sessions to generate')
    parser.add_argument('--anomaly-ratio', type=float, default=0.1, help='Ratio of anomalous sessions (0.0-1.0)')
    parser.add_argument('--seed', type=int, default=42, help='Random seed for reproducibility')
    
    args = parser.parse_args()
    
    # Set random seed
    random.seed(args.seed)
    np.random.seed(args.seed)
    
    # Generate data
    df = generate_synthetic_data(
        num_sessions=args.sessions,
        anomaly_ratio=args.anomaly_ratio,
        output_file=args.output
    )
    
    print(f"\nSynthetic data generation completed!")
    print(f"Output file: {args.output}")
    print(f"File size: {os.path.getsize(args.output) / 1024:.1f} KB")

if __name__ == '__main__':
    main()
