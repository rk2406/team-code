import threading
from threading import Lock
from flask import Flask, render_template,session, request
from flask_socketio import SocketIO, emit, disconnect
from random import random
import serial
import Queue

# Set this variable to "threading", "eventlet" or "gevent" to test the
# different async modes, or leave it set to None for the application to choose
# the best option based on installed packages.
# Personally, I install and use eventlet
async_mode = None

app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app, async_mode=async_mode)
thread = None
thread_lock = Lock()

# The websocket is maintained in the background, and this
# function outputs a random number every 5 seconds
def background_thread():
    count=0	
    while True:
        socketio.sleep(1)
        count += 1
        number = str(round(random()*10, 3))
        socketio.emit('my_response',{'data': number, 'count': count},namespace='/test')

# Route for serving up the index page
@app.route('/')
def index():
    return render_template('index2.html', async_mode=socketio.async_mode)

# This function is called when a web browser connects
@socketio.on('connect', namespace='/test')
def test_connect():
    global thread
    with thread_lock:
        if thread is None:
            thread = socketio.start_background_task(target=background_thread)
    emit('my_response', {'data': 'Connected', 'count': 0})


# Notification that a client has disconnected
@socketio.on('disconnect', namespace='/test')
def test_disconnect():
    print('Client disconnected', request.sid)

# Run the web app
if __name__ == '__main__':
        socketio.run(app, debug=True, host='0.0.0.0')

