import streamlit as st
import pandas as pd
import time
import plotly.express as px
import firebase_admin
from firebase_admin import db, credentials

refresh_interval = 3 #seconds between updating display

cred = credentials.Certificate('credentials.json') #admin credentials supersede security rules
if not firebase_admin._apps:
    app = firebase_admin.initialize_app(cred, {
        'databaseURL': 'https://iot-study-8372a-default-rtdb.firebaseio.com'
    })

def fetch_data():
    data = db.reference('/ESP32').get() #get all data as a dict from folder
    df = pd.DataFrame(data).T #convert to transposed dataframe
    return df

st.set_page_config(
    page_title="Firebase Testing",
    page_icon="☁️",
    layout="wide",
)
st.title('Here is my weather app!')
st.header('Live Data')
display = st.empty() 
temp_graph = st.empty()
humidity_graph = st.empty()
uv_graph = st.empty()
while True:
    #Get data, remove data format column, normalize time stamps to start at 0 so data is in seconds from first datapoint
    df = fetch_data()
    df = df.reset_index(drop=True)
    df = df.sort_values(by=['Timestamp'])
    df['Timestamp'] = df['Timestamp'] - df['Timestamp'].min()
    
    #Gets the most recent and second most recent datapoints and finds the difference for the display
    current_temp, current_humidity, current_uv = df.at[df.index[-1], "Temp"], df.at[df.index[-1], "Humidity"], df.at[df.index[-1], "UV"]
    old_temp, old_humidity, old_uv = df.at[df.index[-2], "Temp"], df.at[df.index[-2], "Humidity"], df.at[df.index[-2], "UV"]
    temp_change, humidity_change, uv_change = int(current_temp)-int(old_temp), int(current_humidity)-int(old_humidity), int(current_uv)-int(old_uv)

    #Creates dyamic display based on above values
    with display.container():
        temp_display, humidity_display, uv_display = st.columns(3)
        temp_display.metric(label = "Temperature", value = "{} °F".format(current_temp), delta = "{} °F".format(temp_change))
        humidity_display.metric(label = "Humidity", value = "{}%".format(current_humidity), delta = "{}%".format(humidity_change))
        uv_display.metric(label = "UV", value = "{}".format(current_uv), delta = "{}".format(uv_change))  
    with temp_graph:
        temp_data = df[['Timestamp','Temp']].copy()
        fig = px.line(temp_data,x="Timestamp", y="Temp", title="Temperature over time")
        st.plotly_chart(fig)
    with humidity_graph:
        humidity_data = df[['Timestamp','Humidity']].copy()
        fig = px.line(humidity_data,x="Timestamp", y="Humidity", title="Humidity over time")
        st.plotly_chart(fig)
    with uv_graph:
        uv_data = df[['Timestamp','UV']].copy()
        fig = px.line(uv_data,x="Timestamp", y="UV", title="UV Index over time")
        st.plotly_chart(fig)

    time.sleep(refresh_interval)
