import serial
import time
import pymysql
import json

from flask import Flask, render_template, jsonify, redirect, request

app = Flask(__name__)

# Dictionary of pins with name of pin and state ON/OFF
pins = {
    6 : {'name' : 'LIGHT', 'state' : 0 },
    3 : {'name' : 'FAN', 'state' : 0 }
    }
def readData():
    while ser.inWaiting()!=19:
        if ser.inWaiting()<19:
            pass
        if ser.inWaiting()>19:
            ser.readline()
    ser.flush()
    data=ser.readline().decode('utf-8').rstrip()
    print(data)
    try:
        states=data.split(',')
        pins[3]['state']=int(states[0])
        print(states[0])
        pins[6]['state']=int(states[1])
        print(states[1])

        #print(dbConn)
        dbConn=pymysql.connect("localhost","banaynay1109","","room_db") or die('could not connect to database')
        with dbConn:
            cursor=dbConn.cursor()
            cursor.execute("INSERT INTO humidlog (humid) VALUES(%s)",(states[2],))
            dbConn.commit
            cursor.close()
        print(states[2])
        print(states[3])
    except:
        print("An exception occured")
    return states
    

    
# Main function when accessing the website
@app.route("/")
def index():
    # TODO: Read the status of the pins ON/OFF and update dictionary
    readData()
    #This data wii be sent to index.html (pins dictionary)
    templateData = { 'pins' : pins }
    
    # Pass the template data into the template index.html and return it
    return render_template('index.html', **templateData)

# Function with buttons that toggle depending on the status
@app.route("/<changePin>/<toggle>")
def toggle_function(changePin, toggle):
    # Convert the pin from the URL into an interger:
    changePin = int(changePin)
    # Get the device name for the pin being chnaged:
    deviceName = pins[changePin]['name']
    # If the action part of the URL is "on," execute the code intended below:
    if toggle == "on":
        #Set the pin high
        if changePin == 6:
            ser.write(b"1")
            pins[changePin]['state'] = 1
        if changePin == 3:
            ser.write(b"3")
            pins[changePin]['state'] = 1
        #Save the status message to be passed into the template:
        message = "Turned" + deviceName + "on."
    if toggle == "off":
        if changePin == 6:
            ser.write(b"2")
            pins[changePin]['state'] = 0
        if changePin == 3:
            ser.write(b"4")
            pins[changePin]['state'] = 0
        #Set the pin low
        message = "Turned" + deviceName + "off."
    
    #This data wii be sent to index.html (pins dictionary)
    #templateData = { 'pins' : pins }
    
    # Pass the template data into the template index.html and return it
    while ser.inWaiting()>0:
        ser.readline()
    time.sleep(1)
    while ser.inWaiting()>0:
        ser.readline()
    templateData = { 'pins' : pins }
    return render_template('index.html', **templateData)
    #return redirect("/")

#Function to send simple commands
@app.route("/threshold", methods=['GET','POST'])
def changethreshold():
    if request.method == 'POST':
        ser.write(request.form["threshold"].encode('utf-8'))
        
    templateData = { 'pins' : pins }
    return render_template('index.html', **templateData)

@app.route("/<action>")
def action(action):
    if action == 'action1':
        ser.write(b"1")
        pins[6]['state'] = 1
    if action == 'action2':
        ser.write(b"2")
        pins[6]['state'] = 0
    if action == 'action3':
        ser.write(b"3")
        pins[3]['state'] = 1
    if action == 'action4':
        ser.write(b"4")
        pins[3]['state'] = 0
    
    
    while ser.inWaiting()>0:
        ser.readline()
    time.sleep(1)
    while ser.inWaiting()>0:
        ser.readline()
    #This data wii be sent to index.html (pins dictionary)
    templateData = { 'pins' : pins }
    
    # Pass the template data into the template index.html and return it
    return render_template('index.html', **templateData)
    #return redirect("/")

def minHumid():
    dbConn=pymysql.connect("localhost","banaynay1109","","room_db") or die('could not connect to database')
    with dbConn:
        cursor=dbConn.cursor()
        cursor.execute("SELECT humid FROM humidlog ORDER BY humid ASC LIMIT 1")
        minHumid=cursor.fetchone()
        cursor.close()
        return minHumid[0]
        
def maxHumid():
    dbConn=pymysql.connect("localhost","banaynay1109","","room_db") or die('could not connect to database')
    with dbConn:
        cursor=dbConn.cursor()
        cursor.execute("SELECT humid FROM humidlog ORDER BY humid DESC LIMIT 1")
        maxHumid=cursor.fetchone()
        cursor.close()
        return maxHumid[0]
    

#Function to jsonify and send data to the template repeatedly
@app.route("/_datas")
def datas():
    read=readData()
    miniHumid=minHumid()
    maxaHumid=maxHumid()
    combined={"pins":{6: pins[6]["state"], 3: pins[3]["state"]}, "humid": read[2] , "minmax": {"min": miniHumid,"max": maxaHumid}, "threshold": read[3]}
    combined_json=json.dumps(combined)
    templateData = { 'pins' : pins }
    render_template('index.html', **templateData)
    print(combined)
    return combined
    

# Main function, set up serial bus, indicate port for the webserver,
# ans start the service.
if __name__ == '__main__':
    ser = serial.Serial('/dev/ttyS0', 9600, timeout=0)
    ser.flush()
    app.run(host='0.0.0.0', port = 80, debug = True)    
      

    
    
