import os
from dotenv import load_dotenv
from supabase import create_client, Client
import pandas as pd
from pycaret.regression import *

# Load environment variables
load_dotenv()

# Supabase setup
SUPABASE_URL = os.getenv("SUPABASE_URL")
SUPABASE_API_KEY = os.getenv("SUPABASE_API_KEY")
supabase: Client = create_client(SUPABASE_URL, SUPABASE_API_KEY)

def fetch_data_from_supabase():
    # Fetch data from Supabase
    response = supabase.table('food_wastage').select('*').execute()
    data = response.data
    return pd.DataFrame(data)

def preprocess_data(df):
    # Convert 'day' to numerical values
    day_mapping = {
        'Monday': 0, 'Tuesday': 1, 'Wednesday': 2, 'Thursday': 3,
        'Friday': 4, 'Saturday': 5, 'Sunday': 6
    }
    df['day_num'] = df['day'].map(day_mapping)
    
    # Calculate total food wastage
    df['total_wastage'] = df['rice'] + df['meat'] + df['veggies']
    
    return df

def train_model(df):
    # Initialize PyCaret setup
    reg_setup = setup(data=df, target='total_wastage', session_id=123,
                      numeric_features=['stall', 'day_num'],
                      ignore_features=['id', 'day', 'created_at'])
    
    # Compare models and select the best one
    best_model = compare_models()
    
    # Finalize the model
    final_model = finalize_model(best_model)
    
    return final_model

def save_model(model):
    # Save the model
    save_model(model, 'food_wastage_model')

def load_saved_model():
    # Load the saved model
    return load_model('food_wastage_model')

def predict_wastage(model, stall, day):
    # Create a dataframe for prediction
    day_mapping = {
        'Monday': 0, 'Tuesday': 1, 'Wednesday': 2, 'Thursday': 3,
        'Friday': 4, 'Saturday': 5, 'Sunday': 6
    }
    predict_df = pd.DataFrame({'stall': [stall], 'day_num': [day_mapping[day]]})
    
    # Make prediction
    prediction = predict_model(model, data=predict_df)
    return prediction['prediction_label'].iloc[0]

def main():
    # Fetch and preprocess data
    df = fetch_data_from_supabase()
    df = preprocess_data(df)
    
    # Train the model
    model = train_model(df)
    
    # Save the model
    save_model(model)
    
    # Example prediction
    stall = 1
    day = 'Monday'
    loaded_model = load_saved_model()
    predicted_wastage = predict_wastage(loaded_model, stall, day)
    print(f"Predicted total food wastage for Stall {stall} on {day}: {predicted_wastage:.2f}")

if __name__ == "__main__":
    main()
