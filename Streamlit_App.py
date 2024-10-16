import streamlit as st
import pandas as pd
import plotly.express as px
from pycaret.regression import load_model, predict_model
import datetime

# Load the saved model
@st.cache_resource
def load_saved_model():
    return load_model('food_wastage_model')

# Function to predict wastage
def predict_wastage(model, stall, day):
    day_mapping = {
        'Monday': 0, 'Tuesday': 1, 'Wednesday': 2, 'Thursday': 3,
        'Friday': 4, 'Saturday': 5, 'Sunday': 6
    }
    predict_df = pd.DataFrame({'stall': [stall], 'day_num': [day_mapping[day]]})
    prediction = predict_model(model, data=predict_df)
    return prediction['prediction_label'].iloc[0]

# Streamlit app
def main():
    st.title("Food Waste Prediction Dashboard")

    # Sidebar for input
    st.sidebar.header("Input Parameters")
    stall = st.sidebar.number_input("Stall Number", min_value=1, max_value=10, value=1)
    selected_date = st.sidebar.date_input("Select Date", datetime.date.today())
    day = selected_date.strftime("%A")

    # Load model
    model = load_saved_model()

    # Make prediction
    predicted_wastage = predict_wastage(model, stall, day)

    # Display prediction
    st.header(f"Predicted Food Waste for Stall {stall} on {day}")
    st.subheader(f"{predicted_wastage:.2f} units")

    # Create data for the week
    week_data = []
    for i in range(7):
        date = selected_date + datetime.timedelta(days=i)
        day = date.strftime("%A")
        wastage = predict_wastage(model, stall, day)
        week_data.append({"Date": date, "Day": day, "Predicted Wastage": wastage})

    week_df = pd.DataFrame(week_data)

    # Line chart for the week
    st.subheader("Weekly Food Waste Prediction")
    fig_line = px.line(week_df, x="Date", y="Predicted Wastage", title="Predicted Food Waste for the Week")
    st.plotly_chart(fig_line)

    # Bar chart for the week
    fig_bar = px.bar(week_df, x="Day", y="Predicted Wastage", title="Daily Food Waste Prediction")
    st.plotly_chart(fig_bar)

    # Pie chart for waste composition (assuming equal parts of rice, meat, and veggies)
    waste_composition = pd.DataFrame({
        "Component": ["Rice", "Meat", "Vegetables"],
        "Percentage": [33.33, 33.33, 33.34]
    })
    fig_pie = px.pie(waste_composition, values="Percentage", names="Component", title="Estimated Waste Composition")
    st.plotly_chart(fig_pie)

    # Display the raw data
    st.subheader("Raw Prediction Data")
    st.dataframe(week_df)

if __name__ == "__main__":
    main()
