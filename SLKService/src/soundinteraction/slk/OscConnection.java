package soundinteraction.slk;

import java.io.IOException;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException;

import com.illposed.osc.OSCMessage;
import com.illposed.osc.OSCPortOut;

/**
 * Manages the connection using JavaOSC library:
 * http://www.illposed.com/software/javaosc.html
 * 
 * @author Helene Oberhumer and Jose Granjo
 *
 */
public class OscConnection {
	private int _port;
	private InetAddress _addr;
	private OSCPortOut _sender;
	
	public OscConnection(String host, int port) throws UnknownHostException, SocketException {
		_port = port;
		_addr = InetAddress.getByName(host);
		_sender = new OSCPortOut(_addr, _port);
	}
	
	public void close() {
		_sender.close();
	}
	
	public void send(String msg, Object args[]) {
	
		OSCMessage oscMsg = new OSCMessage(msg, args);
		
		try {
			_sender.send(oscMsg);
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}
