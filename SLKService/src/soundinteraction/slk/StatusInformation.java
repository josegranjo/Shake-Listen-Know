package soundinteraction.slk;

import android.app.Activity;
import android.app.Service;
import android.bluetooth.*;
import android.location.LocationManager;
import android.net.*;
import android.os.BatteryManager;
import android.widget.Toast;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

/**
 * Manages the status information of the phone.
 * It can be accessed by its public interface.
 * 
 * @author Helene Oberhumer and Jose Granjo
 *
 */
public class StatusInformation extends Activity {
	
	//private boolean _wifiIsEnabled;
	private boolean _bluetoothIsEnabled;
	private int _batteryLevelPerc;
	private boolean _gpsIsEnabled;
	private boolean _batteryIsCharging;
	
	private BluetoothAdapter _blueto;
	private NetworkInfo _wifi;
	private ConnectivityManager _cM;
	private LocationManager _locMa;
	
	public StatusInformation(Service myService){ 
		this._batteryLevelPerc = 0;
		//this._wifiIsEnabled = false;
		this._bluetoothIsEnabled = false;
		this._gpsIsEnabled = false;
		this._batteryIsCharging = false;
		this._blueto = BluetoothAdapter.getDefaultAdapter();
		this._cM = (ConnectivityManager) myService.getSystemService(CONNECTIVITY_SERVICE);
		this._wifi = _cM.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
		this._locMa = (LocationManager) myService.getSystemService(LOCATION_SERVICE); 
		
		/* Battery Level */	
		/* Event-Listener: called when battery status is changed */
		myService.registerReceiver(this.receiver, new IntentFilter(Intent.ACTION_BATTERY_CHANGED));	
	}

	public BroadcastReceiver receiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			if (intent.getAction().equals(Intent.ACTION_BATTERY_CHANGED)){
				int level = intent.getIntExtra("level", 0);
			    _batteryLevelPerc = level;
			
			    if (intent.getIntExtra("status", -1) == BatteryManager.BATTERY_STATUS_CHARGING) {
			    	_batteryIsCharging = true;
			    }
			    else {
			    	_batteryIsCharging = false;
			    } 
			}
		}
	};
	
	public boolean getWifiIsEnabled()  {
		return _wifi.isConnected();
	}
	
	public boolean getBluetoothIsEnabled() {
		/* BLUETOOTH */
		/* bluetooth not supported from the emulator !! */
		/* four states: changing to ON, ON, changing to OFF , OFF */
		
		if(_blueto == null) {
			_bluetoothIsEnabled = false;
		} 
		else {
			_bluetoothIsEnabled = _blueto.isEnabled();
		}
		
		return _bluetoothIsEnabled;
	}
	
	public int getBatteryLevelPerc() {
		return _batteryLevelPerc;
	}
	
	public int setBatteryLevelPerc(int newBatteryLevel) {
		return _batteryLevelPerc = newBatteryLevel;
	}
	
	public boolean getGPSIsEnabled() {
		
		try {
			this._gpsIsEnabled = _locMa.isProviderEnabled(LocationManager.GPS_PROVIDER);
		}
		catch(Exception e) {
			Toast.makeText(this, e.getMessage(), Toast.LENGTH_LONG);
		}
		
		return _gpsIsEnabled;
	}
	
	public boolean getBatteryIsCharging() {
		return _batteryIsCharging;
	}

}
