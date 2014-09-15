/**
 * File: MasterServerFileServerRequest.java
 * Author: Yan Li (yan_li@brown.edu)
 * Date: Apr 21 2014
 */

package master;

import java.io.*;
import java.net.*;

import common.*;
/**
 * 
 * class MasterServerFileServerRequest
 * Description: Actually just responsible for sending heartbeat message to
 *              get the status of file servers
 */
public class MasterServerFileServerRequest extends ThreadBase{

	private MasterServer _server;
	private PrintWriter _out;
	private BufferedReader _in;
	private Socket _sock;
	private FileServerNode _node;
	
	private void _dlog(String str){
		
		if (_server.debugMode())
			System.out.println("[MasterServerFileServerRequest (DEBUG)]:" + str);
	}
	
	private static void _elog(String str){
		System.err.println("[MasterServerFileServerRequest (ERROR)]:" + str);
	}
	
	private static void _log(String str){
		System.out.println("[MasterServerFileServerRequest]:" + str);
	}
	
	/**
	 * Constructor
	 * @param serve: the master server object
	 * @param out: the print writer
	 * @param in: the buffered reader
	 * @param sock: the connected socket
	 * @param node: the file server node
	 */
	public MasterServerFileServerRequest(MasterServer server, PrintWriter out,
					BufferedReader in, Socket sock, FileServerNode node){
		
		super("MasterServerFileServerRequest",server.debugMode());
		_server = server;
		assert _server != null;
		_sock = sock;
		_in = in;
		_out = out;
		_node = node;
	}
	
	protected void parse(String str){
		
		if (str.equals(ProtocolConstants.PACK_STR_HEARTBEAT_HEAD)){
			_dlog("HearBeat confirmed");
		}
		else{
			_elog("Invalid message");
		}
	}
	
	/**
	 * sendHeartBeat: send the heartbeat message
	 */
	protected void sendHeartBeat() throws Exception{
		// TODO: add more story here
		String reply = NetComm.sendAndRecv(
							ProtocolConstants.PACK_STR_HEARTBEAT_HEAD,
							_out,
							_in);
		parse(reply);
	}
	
	/**
	 * run: continuously send the heartbeat message
	 */
	public void run(){
		
		Thread thisThread = Thread.currentThread();
		try{
			while(_sock != null && !_sock.isClosed() && thisThread == _t){
				synchronized (this){
					while(_suspended){ // Suspension point
						wait();
					}
				}
				Thread.sleep(DropboxConstants.HEART_BEAT_HZ);
				sendHeartBeat();
			}
		}catch (InterruptedException e){
			if (!_server.noException()){
				_elog(e.toString());
			}
			if (_server.debugMode()){
				e.printStackTrace();
			}
		}catch (Exception e){
			if (!_server.noException()){
				_elog(e.toString());
			}
			if (_server.debugMode()){
				e.printStackTrace();
			}
		}
		clear();
		_log(_threadName + " is stopped");
	}
	
	/**
	 * clear: close the socket and stream
	 */
	protected void clear(){
		
		_dlog("Do clear..");
		try{
			if (!_sock.isClosed())
				_sock.close();
			/* Close stream */
			_in.close();
			_out.close();

			/* Set to null */
			_sock = null;
			_in = null;
			_out = null;

		}catch (IOException e){
			if (!_server.noException()){
				_elog(e.toString());
			}
			if (_server.debugMode())
				e.printStackTrace();
		}
		if (_node != null && _node.isAlive())
			_node.destroy();
		if (_node != null)
			_server.removeDead(_node);
		_node = null;
		_dlog("Finished");
	}
}
