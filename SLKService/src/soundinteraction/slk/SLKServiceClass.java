package soundinteraction.slk;

import java.net.SocketException;
import java.net.UnknownHostException;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.widget.Toast;

import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.hardware.Sensor;
import android.hardware.SensorEvent;

/**
 * Three main operations:
 * (1) Establish an UDP connection to send OSC messages
 * (2) Get phone's status information and send it when the phone is shacked
 * Supported status info: Battery Level, Battery charging status, bluetooth and GPS.
 * (3) Detection of a shaking gesture. 
 * Algorithm was based on: http://www.clingmarks.com/?p=25
 * 
 * @author Helene Oberhumer and Jose Granjo
 * 
 */
public class SLKServiceClass extends Service implements SensorEventListener {

	private SensorManager _sensMa;
	private Sensor _sensAcc;
	private OscConnection _oscCon;
	private StatusInformation _statInf;
	private int _notShakingCounter;
	private static final int PORT = 23002;

	// Shaking gesture detection
	SensorManager _sensorMgr;
	private long _lastUpdate;
	private float _x, _y, _z;
	private float _last_x, _last_y, _last_z;
	private static final int SHAKE_THRESHOLD = 800;
	
	// Helps to get a more accurate shaking gesture detection
	private boolean shouldSendNotShaking() {
		if (_notShakingCounter == 1) {
			resetNotShakingCounter();
			return true;
		}
		else {
			_notShakingCounter--;
			return false;
		}
	}

	private void resetNotShakingCounter() {
		_notShakingCounter = 5;
	}

	@Override
	public void onCreate() {
		Toast.makeText(this, "SLK created", Toast.LENGTH_LONG).show();
		_sensMa = (SensorManager) getSystemService(SENSOR_SERVICE);
		_sensAcc = _sensMa.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
		_sensMa.registerListener(this, _sensAcc, SensorManager.SENSOR_DELAY_NORMAL);
		_statInf = new StatusInformation(this);
		resetNotShakingCounter();

		// Shaking gesture detection
		_sensorMgr = (SensorManager)getSystemService(SENSOR_SERVICE);
		_sensorMgr.registerListener(this, _sensAcc, SensorManager.SENSOR_DELAY_GAME);
		_lastUpdate = -1;
	}

	@Override
	public void onDestroy() {
		Toast.makeText(this, "SLK stopped", Toast.LENGTH_LONG).show();
		_sensMa.unregisterListener(this);
		this.unregisterReceiver(_statInf.receiver);
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		try {
			_oscCon = new OscConnection(intent.getStringExtra("ipaddress"), PORT);
		} 
		catch (UnknownHostException e) {
			e.printStackTrace();
		} 
		catch (SocketException e) {
			e.printStackTrace();
		}

		return startId;
	}

	@Override
	public void onAccuracyChanged(Sensor sensor, int accuracy) {
		// TODO Auto-generated method stub
	}

	@Override
	public void onSensorChanged(SensorEvent event) {
		long curTime = System.currentTimeMillis();
		
		if(event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
			// Shaking gestute detection
			if ((curTime - _lastUpdate) > 100) {
				long diffTime = (curTime - _lastUpdate);
				_lastUpdate = curTime;

				_x = event.values[0];
				_y = event.values[1];
				_z = event.values[2];

				float speed = Math.abs(_x+_y+_z - _last_x -_last_y - _last_z) / diffTime * 10000;

				Object args[] = new Object[1];

				if (speed > SHAKE_THRESHOLD) {
					args[0] = new Integer((_statInf.getGPSIsEnabled())?1:0);
					_oscCon.send("/shakelistenknow/gps", args);
					args[0] = new Integer((_statInf.getBluetoothIsEnabled())?1:0);
					_oscCon.send("/shakelistenknow/bluetooth", args);
					args[0] = new Integer(_statInf.getBatteryLevelPerc());
					_oscCon.send("/shakelistenknow/battery/level", args);
					args[0] = new Integer(_statInf.getBatteryIsCharging()?1:0);	// If it is charging
					_oscCon.send("/shakelistenknow/battery/charging", args);
					args[0] = new Integer(1);
					_oscCon.send("/shakelistenknow/shaking", args);			
				}
				else {
					if (shouldSendNotShaking()) {
						args[0] = new Integer(0);
						_oscCon.send("/shakelistenknow/shaking", args);	
						_oscCon.send("/shakelistenknow/bluetooth", args);
						_oscCon.send("/shakelistenknow/gps", args);
					}				
				}
			}
			
			_last_x = _x;
			_last_y = _y;
			_last_z = _z;
		}
	}

	@Override
	public IBinder onBind(Intent arg0) {
		// TODO Auto-generated method stub
		return null;
	}
}
