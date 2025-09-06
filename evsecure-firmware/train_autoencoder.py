#!/usr/bin/env python3
"""
EVsecure Autoencoder Training Script

This script trains an autoencoder model on "good" EV charging session data
to detect anomalies in charging behavior.

Usage:
    python train_autoencoder.py --data sessions.csv --output model.tflite
"""

import argparse
import pandas as pd
import numpy as np
import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import layers
from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import train_test_split
import matplotlib.pyplot as plt
import os
import json

def load_and_preprocess_data(csv_file):
    """
    Load and preprocess the CSV session data.
    
    Args:
        csv_file: Path to the CSV file containing session data
        
    Returns:
        X: Preprocessed feature matrix
        scaler: Fitted StandardScaler for later use
    """
    print(f"Loading data from {csv_file}...")
    
    # Load CSV data
    df = pd.read_csv(csv_file)
    
    # Define feature columns (matching the C struct)
    feature_columns = [
        'v_rms', 'i_rms', 'p_kw', 'pf', 'thd_v', 'thd_i',
        'dvdt', 'didt', 'ocpp_rate', 'remote_stop_cnt',
        'malformed', 'out_of_seq', 'fw_ok', 'tamper', 'temp_c'
    ]
    
    # Check if all required columns exist
    missing_cols = [col for col in feature_columns if col not in df.columns]
    if missing_cols:
        print(f"Warning: Missing columns: {missing_cols}")
        print("Available columns:", df.columns.tolist())
        
        # Create missing columns with default values
        for col in missing_cols:
            if col in ['fw_ok', 'tamper']:
                df[col] = 1  # Default to True/1
            elif col in ['remote_stop_cnt', 'malformed', 'out_of_seq']:
                df[col] = 0  # Default to 0
            else:
                df[col] = 0.0  # Default to 0.0
    
    # Extract features
    X = df[feature_columns].values
    
    # Handle missing values
    X = np.nan_to_num(X, nan=0.0)
    
    # Scale features
    scaler = StandardScaler()
    X_scaled = scaler.fit_transform(X)
    
    print(f"Loaded {len(X)} samples with {len(feature_columns)} features")
    print("Feature statistics:")
    for i, col in enumerate(feature_columns):
        print(f"  {col}: mean={scaler.mean_[i]:.3f}, std={scaler.scale_[i]:.3f}")
    
    return X_scaled, scaler

def create_autoencoder(input_dim, encoding_dim=8):
    """
    Create an autoencoder model.
    
    Args:
        input_dim: Number of input features
        encoding_dim: Dimension of the latent space
        
    Returns:
        autoencoder: Compiled autoencoder model
    """
    # Encoder
    input_layer = layers.Input(shape=(input_dim,))
    encoded = layers.Dense(32, activation='relu')(input_layer)
    encoded = layers.Dropout(0.2)(encoded)
    encoded = layers.Dense(16, activation='relu')(encoded)
    encoded = layers.Dropout(0.2)(encoded)
    encoded = layers.Dense(encoding_dim, activation='relu', name='encoded')(encoded)
    
    # Decoder
    decoded = layers.Dense(16, activation='relu')(encoded)
    decoded = layers.Dropout(0.2)(decoded)
    decoded = layers.Dense(32, activation='relu')(decoded)
    decoded = layers.Dropout(0.2)(decoded)
    decoded = layers.Dense(input_dim, activation='linear', name='decoded')(decoded)
    
    # Create autoencoder
    autoencoder = keras.Model(input_layer, decoded)
    
    # Compile model
    autoencoder.compile(
        optimizer='adam',
        loss='mse',
        metrics=['mae']
    )
    
    return autoencoder

def train_model(X_train, X_val, model, epochs=100, batch_size=32):
    """
    Train the autoencoder model.
    
    Args:
        X_train: Training data
        X_val: Validation data
        model: Autoencoder model
        epochs: Number of training epochs
        batch_size: Batch size for training
        
    Returns:
        history: Training history
    """
    print(f"Training autoencoder for {epochs} epochs...")
    
    # Early stopping to prevent overfitting
    early_stopping = keras.callbacks.EarlyStopping(
        monitor='val_loss',
        patience=10,
        restore_best_weights=True
    )
    
    # Train the model
    history = model.fit(
        X_train, X_train,
        validation_data=(X_val, X_val),
        epochs=epochs,
        batch_size=batch_size,
        callbacks=[early_stopping],
        verbose=1
    )
    
    return history

def evaluate_model(model, X_test, scaler):
    """
    Evaluate the trained model and calculate reconstruction errors.
    
    Args:
        model: Trained autoencoder model
        X_test: Test data
        scaler: Fitted StandardScaler
        
    Returns:
        reconstruction_errors: Array of reconstruction errors
        threshold: Suggested anomaly threshold
    """
    print("Evaluating model...")
    
    # Get reconstructions
    X_reconstructed = model.predict(X_test)
    
    # Calculate reconstruction errors
    reconstruction_errors = np.mean(np.square(X_test - X_reconstructed), axis=1)
    
    # Calculate threshold (95th percentile of reconstruction errors)
    threshold = np.percentile(reconstruction_errors, 95)
    
    print(f"Reconstruction error statistics:")
    print(f"  Mean: {np.mean(reconstruction_errors):.6f}")
    print(f"  Std: {np.std(reconstruction_errors):.6f}")
    print(f"  Min: {np.min(reconstruction_errors):.6f}")
    print(f"  Max: {np.max(reconstruction_errors):.6f}")
    print(f"  Suggested threshold (95th percentile): {threshold:.6f}")
    
    return reconstruction_errors, threshold

def convert_to_tflite(model, output_file, quantize=True):
    """
    Convert the trained model to TFLite format.
    
    Args:
        model: Trained Keras model
        output_file: Output TFLite file path
        quantize: Whether to quantize the model
        
    Returns:
        model_size: Size of the TFLite model in bytes
    """
    print(f"Converting model to TFLite: {output_file}")
    
    if quantize:
        # Create a representative dataset for quantization
        def representative_dataset():
            for _ in range(100):
                data = np.random.random((1, 15)).astype(np.float32)
                yield [data]
        
        # Convert to TFLite with quantization
        converter = tf.lite.TFLiteConverter.from_keras_model(model)
        converter.optimizations = [tf.lite.Optimize.DEFAULT]
        converter.representative_dataset = representative_dataset
        converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
        converter.inference_input_type = tf.int8
        converter.inference_output_type = tf.int8
        
        tflite_model = converter.convert()
    else:
        # Convert to TFLite without quantization
        converter = tf.lite.TFLiteConverter.from_keras_model(model)
        tflite_model = converter.convert()
    
    # Save the model
    with open(output_file, 'wb') as f:
        f.write(tflite_model)
    
    model_size = len(tflite_model)
    print(f"Model saved: {output_file}")
    print(f"Model size: {model_size} bytes ({model_size/1024:.1f} KB)")
    
    return model_size

def plot_training_history(history, output_dir):
    """
    Plot training history.
    
    Args:
        history: Training history
        output_dir: Output directory for plots
    """
    os.makedirs(output_dir, exist_ok=True)
    
    # Plot loss
    plt.figure(figsize=(10, 6))
    plt.plot(history.history['loss'], label='Training Loss')
    plt.plot(history.history['val_loss'], label='Validation Loss')
    plt.title('Model Loss')
    plt.xlabel('Epoch')
    plt.ylabel('Loss')
    plt.legend()
    plt.grid(True)
    plt.savefig(os.path.join(output_dir, 'training_loss.png'))
    plt.close()
    
    # Plot MAE
    plt.figure(figsize=(10, 6))
    plt.plot(history.history['mae'], label='Training MAE')
    plt.plot(history.history['val_mae'], label='Validation MAE')
    plt.title('Model MAE')
    plt.xlabel('Epoch')
    plt.ylabel('MAE')
    plt.legend()
    plt.grid(True)
    plt.savefig(os.path.join(output_dir, 'training_mae.png'))
    plt.close()

def generate_model_header(tflite_file, output_file):
    """
    Generate a C header file with the model data.
    
    Args:
        tflite_file: Path to the TFLite model file
        output_file: Output header file path
    """
    print(f"Generating model header: {output_file}")
    
    with open(tflite_file, 'rb') as f:
        model_data = f.read()
    
    with open(output_file, 'w') as f:
        f.write("#ifndef MODEL_DATA_H\n")
        f.write("#define MODEL_DATA_H\n\n")
        f.write("#include <stddef.h>\n\n")
        f.write("// Autoencoder model data\n")
        f.write(f"extern const unsigned char model_data[{len(model_data)}];\n")
        f.write(f"extern const size_t model_data_size = {len(model_data)};\n\n")
        f.write("// Model metadata\n")
        f.write("#define MODEL_INPUT_SIZE 15\n")
        f.write("#define MODEL_OUTPUT_SIZE 15\n")
        f.write("#define MODEL_ARENA_SIZE 32768\n\n")
        f.write("#endif // MODEL_DATA_H\n")
    
    # Generate the C source file
    c_source_file = output_file.replace('.h', '.c')
    with open(c_source_file, 'w') as f:
        f.write('#include "model_data.h"\n\n')
        f.write("// Autoencoder model data\n")
        f.write("const unsigned char model_data[] = {\n")
        
        # Write model data as hex bytes
        for i, byte in enumerate(model_data):
            if i % 16 == 0:
                f.write("    ")
            f.write(f"0x{byte:02x}")
            if i < len(model_data) - 1:
                f.write(", ")
            if i % 16 == 15:
                f.write("\n")
        
        if len(model_data) % 16 != 0:
            f.write("\n")
        f.write("};\n")
    
    print(f"Model header generated: {output_file}")
    print(f"Model source generated: {c_source_file}")

def main():
    parser = argparse.ArgumentParser(description='Train autoencoder for EV charging anomaly detection')
    parser.add_argument('--data', required=True, help='Input CSV file with session data')
    parser.add_argument('--output', default='model.tflite', help='Output TFLite model file')
    parser.add_argument('--epochs', type=int, default=100, help='Number of training epochs')
    parser.add_argument('--batch-size', type=int, default=32, help='Batch size for training')
    parser.add_argument('--encoding-dim', type=int, default=8, help='Latent space dimension')
    parser.add_argument('--no-quantize', action='store_true', help='Disable quantization')
    parser.add_argument('--output-dir', default='./output', help='Output directory for plots and files')
    
    args = parser.parse_args()
    
    # Create output directory
    os.makedirs(args.output_dir, exist_ok=True)
    
    # Load and preprocess data
    X, scaler = load_and_preprocess_data(args.data)
    
    # Split data
    X_train, X_temp = train_test_split(X, test_size=0.3, random_state=42)
    X_val, X_test = train_test_split(X_temp, test_size=0.5, random_state=42)
    
    print(f"Data split: {len(X_train)} train, {len(X_val)} validation, {len(X_test)} test")
    
    # Create and train model
    model = create_autoencoder(X.shape[1], args.encoding_dim)
    model.summary()
    
    history = train_model(X_train, X_val, model, args.epochs, args.batch_size)
    
    # Evaluate model
    reconstruction_errors, threshold = evaluate_model(model, X_test, scaler)
    
    # Convert to TFLite
    output_path = os.path.join(args.output_dir, args.output)
    model_size = convert_to_tflite(model, output_path, not args.no_quantize)
    
    # Generate model header
    header_path = os.path.join(args.output_dir, 'model_data.h')
    generate_model_header(output_path, header_path)
    
    # Plot training history
    plot_training_history(history, args.output_dir)
    
    # Save model metadata
    metadata = {
        'model_size_bytes': model_size,
        'model_size_kb': model_size / 1024,
        'input_features': X.shape[1],
        'encoding_dim': args.encoding_dim,
        'anomaly_threshold': float(threshold),
        'quantized': not args.no_quantize,
        'training_samples': len(X_train),
        'validation_samples': len(X_val),
        'test_samples': len(X_test)
    }
    
    metadata_path = os.path.join(args.output_dir, 'model_metadata.json')
    with open(metadata_path, 'w') as f:
        json.dump(metadata, f, indent=2)
    
    print(f"\nTraining completed successfully!")
    print(f"Model saved: {output_path}")
    print(f"Model header: {header_path}")
    print(f"Model metadata: {metadata_path}")
    print(f"Model size: {model_size} bytes ({model_size/1024:.1f} KB)")
    print(f"Suggested anomaly threshold: {threshold:.6f}")

if __name__ == '__main__':
    main()
