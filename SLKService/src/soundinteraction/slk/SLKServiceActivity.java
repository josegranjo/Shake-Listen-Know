package soundinteraction.slk;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;

/**
 * Starts the Service that runs in the background
 * 
 * @author Helene Oberhumer and Jose Granjo
 *
 */
public class SLKServiceActivity extends Activity implements OnClickListener {
	/** Called when the activity is first created. */

	private Button _buttonStart, _buttonStop;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);

		_buttonStart = (Button) findViewById(R.id.buttonStart);
		_buttonStop = (Button) findViewById(R.id.buttonStop);       
		_buttonStart.setText("Start Service");
		_buttonStop.setText("Stop Service"); 

		TextView tv = (TextView) findViewById(R.id.textViewBanner);
		tv.setText("Shake - Listen - Know");
		tv = (TextView) findViewById(R.id.textViewIP);
		tv.setText("IP-address:");

		_buttonStart.setOnClickListener(this);
		_buttonStop.setOnClickListener(this);
	}

	@Override
	public void onClick(View v) {
		Intent intent = new Intent(this, SLKServiceClass.class);

		switch (v.getId()) {
		case R.id.buttonStart:
			{
				// Starts the Service giving the IP for the connection
				TextView te = (TextView) findViewById(R.id.editTextIPAddress);
				Bundle b = new Bundle();
				b.putString("ipaddress", te.getText().toString());
				intent.putExtras(b);
				startService(intent);
			}
			break;
		case R.id.buttonStop:
			stopService(intent);
			break;
		}
	}

	protected void onResume() {
		super.onResume();
	}

	protected void onPause() {
		super.onPause();
	}

	protected void onDestroy() {
		super.onDestroy();
	}
}