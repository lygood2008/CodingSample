/**
 * File: MasterServerFileServerAccept.java
 * Author: Yan Li (yan_li@brown.edu)
 * Date: Apr 21 2014
 */

package master;

import java.net.*;
import java.util.StringTokenizer;
import java.io.*;

import common.*;

/**
 * 
 * Class MasterServerFileServerAccept
 * Description: Handles the new connected file server
 *              Mainly accept the message from file server
 */
class MasterServerFileServerAccept extends ThreadBase{
	
	private Socket _sock;
	private BufferedReader _in;
	private PrintWriter _out;
	private MasterServer _server;
	private FileServerNode _node;
	
	private void _dlog(String str){
		
		if (_server.debugMode())
			System.out.println("[MasterServerFileServerAccept (DEBUG)]:" + str);
	}
	
	private static void _elog(String str){
		System.err.println("[MasterServerFileServerAccept (ERROR)]:" + str);
	}
	
	private static void _log(String str){
		System.out.println("[MasterServerFileServerAccept]:" + str);
	}
	
	/**
	 * Constructor
	 * @param server: the master server object
	 * @param out: the print writer
	 * @param in: the buffered reader
	 * @param sock: the socket
	 * @param node: the file server node
	 */
	public MasterServerFileServerAccept(MasterServer server, PrintWriter out, 
				BufferedReader in, Socket sock, FileServerNode node){
		
		super("MasterServerFileServerAccept", server.debugMode());
		_server = server;
		assert _server != null;
		_sock = sock;
		_in = in;
		_out = out;
		_node = node;
	}
	
	/**
	 * run: continuously read the message from buffered reader
	 */
	@Override
	public void run(){
		
		Thread thisThread = Thread.currentThread();
		try{
			while(_sock != null && !_sock.isClosed() && thisThread == _t){
				synchronized (this){
					while(_suspended){ // Suspension point
						wait();

					}
				}
				String message = NetComm.receive(_in);
				parse(message);
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
	 * sendCurrentStatus: send the current status to the print writer
	 */
	protected void sendCurrentStatus() throws Exception{
		// TODO: No package header, safe or not
		String str = "There are " + (_server.getFS().size()-1) + 
				     " file servers like you are connected";
		NetComm.send(str, _out);
	}
	
	/**
	 * parse: parse the string and send response
	 * @param str: the given string
	 */
	protected void parse(String str) throws Exception{
		
		StringTokenizer st = new StringTokenizer(str);
		String tkn = st.nextToken();
		
		if (!st.hasMoreTokens() && 
			tkn.equals(ProtocolConstants.PACK_STR_CLOSE_HEAD)){
			
			_dlog("I got a close package, now close");
			_node.destroy(); // Will stop myself too
			// Cancel the other thread
		}
		else if (!st.hasMoreTokens() && 
				  tkn.equals(ProtocolConstants.PACK_STR_REQUEST_FS_HEAD)){
			
			_dlog("I got a request package, now send back request");
			// TODO: For now it just send back how many file servers connected
			//       We cannot provide the information to details to file server
			//       Need to think more.
			sendCurrentStatus();
		}
		else if (st.hasMoreTokens() && 
				 tkn.equals(ProtocolConstants.PACK_STR_ADD_CLIENT_HEAD)){
			
			_dlog("I got a add-client package, now add it");
			
			String name = st.nextToken();
			if (!st.hasMoreTokens()){
				_elog("Invalid package");
				return;
			}
			
			String pwd = st.nextToken();
			if (st.hasMoreTokens()){
				_elog("Invalid package");
				return;
			}
			// If the result is false it means we
			// have a conflict when adding client
			boolean result = _server.addClient(name, pwd, _node);
			
			if (result)
				NetComm.send(ProtocolConstants.PACK_STR_CONFIRM_HEAD,_out);
			else
				NetComm.send(ProtocolConstants.PACK_STR_ERRMES_HEAD + " "
						+ "Conflict in adding client,"
						+ " the client name already exists!", _out);
		}
		else if (st.hasMoreTokens() &&
				 tkn.equals(ProtocolConstants.PACK_STR_SET_PRIO_HEAD)){
			
			_dlog("I got a set-priority package, now set it");
			
			String num = st.nextToken();
			
			if (st.hasMoreTokens()){
				_elog("Invalid package");
				return;
			}
			_node.setPriority(Integer.parseInt(num));
			NetComm.send(ProtocolConstants.PACK_STR_CONFIRM_HEAD,_out);
		}
		else if (st.hasMoreTokens() &&
				 tkn.equals(ProtocolConstants.PACK_STR_REMOVE_CLIENT_HEAD)){
			_dlog("I got a remove-client package, now remove it");
			
			String name = st.nextToken();
			if (st.hasMoreTokens()){
				_elog("Invalid package");
				return;
			}
			
			boolean result = _server.removeClient(name);
			if (result)
				NetComm.send(ProtocolConstants.PACK_STR_CONFIRM_HEAD,_out);
			else
				NetComm.send(ProtocolConstants.PACK_STR_ERRMES_HEAD + " "
						+ "Error in removing client,"
						+ " the client name doesn't exist!", _out);
		}
		else if (st.hasMoreTokens() && 
				 tkn.equals(ProtocolConstants.PACK_STR_CHANGE_PWD_HEAD)){
			
			_dlog("I got a change-password package, now change it");
			_dlog(str);
			
			String name = st.nextToken();
			String pwd = st.nextToken();
			
			if (st.hasMoreTokens()){
				_elog("Invalid package");
				return;
			}
			
			if (_server.changePassword(name, pwd))
				NetComm.send(ProtocolConstants.PACK_STR_CONFIRM_HEAD,_out);
			else
				NetComm.send(ProtocolConstants.PACK_STR_BAD_HEAD, _out);
		}
		else{
			_log(str);
			_elog("Invalid package");
		}
	}
	
	/**
	 * Getters
	 */
	public Socket getSocket(){
		return _sock;
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
