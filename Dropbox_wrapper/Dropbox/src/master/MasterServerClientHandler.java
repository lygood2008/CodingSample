/**
 * File: MasterServerClientHandler.java
 * Author: Yan Li (yan_li@brown.edu)
 * Date: Apr 21 2014
 */

package master;

import java.io.*;
import java.net.*;

import common.*;

/**
 * 
 * Class MasterServerClientHandler
 * Description: Handles the new connected client (Not stoppable, not suspendable)
 */
class MasterServerClientHandler extends ThreadBase{
	
	private Socket _sock;
	private MasterServer _server;
	
	private void _dlog(String str){
		
		if (_server.debugMode())
			System.out.println("[MasterServerClientHandler (DEBUG)]:" + str);
	}
	
	private static void _elog(String str){
		System.err.println("[MasterServerClientHandler (ERROR)]:" + str);
	}
	
	private static void _log(String str){
		System.out.println("[MasterServerClientHandler]:" + str);
	}
	
	/**
	 * Constructor
	 * @param sock: the connected socket
	 * @param server: the server object
	 */
	public MasterServerClientHandler(Socket sock, MasterServer server){
		
		super("MasterServerClientHandler", server.debugMode());
		assert _server != null;
		_sock = sock;
		_server = server;
	}
	
	/**
	 * identify: read from the data stream and identify the connected client 
	 * 			 from its name and password
	 * @return: null if there is no client record, or return the corresponding
	 * 			record
	 */
	public ClientRecord identify(){
		
		try{
			DataInputStream is = new DataInputStream(_sock.getInputStream());
			int pack_head = is.readInt();
			if (pack_head != ProtocolConstants.PACK_INIT_HEAD){
				return null;
			}
			int i = 0;
			/* read name*/
			String name = "";
			while (i < DropboxConstants.MAX_CLIENT_NAME_LEN){
				char c = is.readChar();
				if (c == '\n'){
					break;
				}
				else{
					name = name + c;
					i++;
					if (i == DropboxConstants.MAX_CLIENT_NAME_LEN){
						return null;
					}
				}
			}
			i = 0;
			String password = "";
			while (i < DropboxConstants.MAX_PASSWORD_NAME_LEN){
				char c = is.readChar();
				if (c == '\n'){
					break;
				}
				else{
					password = password + c;
					i++;
					if (i == DropboxConstants.MAX_PASSWORD_NAME_LEN){
						return null;
					}
				}
			}
			
			ClientRecord cr = _server.findClient(name, password);
			_dlog("Account information confirmed from" + _sock.getInetAddress());
			return cr;
			
		}catch (IOException e){
			if (!_server.noException()){
				_elog(e.toString());
			}
			if (_server.debugMode()){
				e.printStackTrace();
			}
			return null;
		}
	}
	
	/**
	 * sendInvalid: send an Invalid package
	 */
	public void sendInvalid() throws IOException{
		
		DataOutputStream os = new DataOutputStream(_sock.getOutputStream());
		os.writeInt(ProtocolConstants.PACK_FAIL_HEAD);
	}
	
	/**
	 * sendFileServerInfo: send the client record to the file server
	 * @param cr: the client record object
	 */
	public void sendFileServerInfo(ClientRecord cr) throws IOException {
		
		assert cr != null;
		DataOutputStream os =  new DataOutputStream(_sock.getOutputStream());
		os.writeInt(ProtocolConstants.PACK_FS_INFO_HEAD);
		os.writeChars(cr.getOwner().getIP());
		os.writeChar('\n');
	}
	
	/**
	 * run: handles the connection here
	 */
	@Override
	public void run(){
		/* No cancel point because no while loop */
		/* Also not suspendable*/ 
		_log("Handling the connection from " + _sock.getInetAddress());
		ClientRecord cr = identify();
		try{
			if (cr == null){
				sendInvalid();
			}
			else{
				sendFileServerInfo(cr);
			}
		}catch (IOException e){
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
	 * clear: close the socket
	 */
	@Override
	protected void clear(){
		_dlog("Do clear...");
		
		try{
			if (_sock != null && !_sock.isClosed())
				_sock.close();
			_sock = null;
		}catch (IOException e){
			if (!_server.noException()){
				_elog(e.toString());
			}
			if (_server.debugMode()){
				e.printStackTrace();
			}
		}
		_dlog("Finished");
	}
	
	/**
	 * Getters
	 */
	public Socket getSocket(){
		return _sock;
	}
}
