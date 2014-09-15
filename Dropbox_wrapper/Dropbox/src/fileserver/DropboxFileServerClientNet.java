/**
 * File: DropboxFileServerClientNet.java
 * Author: Yan Li (yan_li@brown.edu)
 * Date: Apr 21 2014
 */
package fileserver;

import java.io.*;
import java.net.*;

import common.*;

/**
 * 
 * Class: DropboxFileServerClientNet
 * Description: Listen to new client connected
 */
class DropboxFileServerClientNet extends GeneralServer{

	private DropboxFileServer _server;
	
	@SuppressWarnings("unused")
	private void _dlog(String str){
		
		if (_server.debugMode())
			System.out.println("[DropboxFileServerClientNet (DEBUG)]:" + str);
	}
	
	private static void _elog(String str){
		System.err.println("[DropboxFileServerClientNet (ERROR)]:" + str);
	}
	
	private static void _log(String str){
		System.out.println("[DropboxFileServerClientNet]:" + str);
	}

	/**
	 * DropboxFileServerClientNet
	 * @param server: the server object
	 */
	public DropboxFileServerClientNet(DropboxFileServer server){
		
		super("DropboxFileServerClientNet", server.noException(),
			  server.debugMode());
		_server = server;
		assert _server != null;
	}
	
	/**
	 * identify: based on the stream got from the socket, return a client node
	 * 			 if identify successfully
	 * @param s: the socket
	 * @return: null if cannot find a corresponding client node, or return
	 *          the right node
	 */
	protected ClientNode identify(Socket s){

		try{
			DataInputStream is = new DataInputStream(s.getInputStream());
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
			ClientNode cn = _server.findMatch(name, password);
			return cn;
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
	 * run: start to listen to new client and spawn a new syncer for new
	 *      connected client
	 */
	public void run(){
		
		_log("Listening to new client...");
		Thread thisThread = Thread.currentThread();
    	Socket client = null;
    	try{
    		_serverSocket = new ServerSocket(_server.listenPort());

    		while (true)
    		{
    			if (_serverSocket == null ||
    				_serverSocket.isClosed() ||
    				_t != thisThread)// Cancel point
    				break;
    			try{
    				synchronized(this) {
    					while (_suspended) { // Suspension point
    						wait();
    					}
    				}
    			}catch (InterruptedException e){ // Cancel point
    				if (!_server.noException()){
    					_elog(e.toString());
    				}
    				if (_server.debugMode())
    					e.printStackTrace();
    				break;
    			}
    			client = _serverSocket.accept();
    			_log("Get connection from " +
    			     client.getInetAddress().getHostAddress());
    			
    			ClientNode cn = identify(client);
    			DataOutputStream os = 
    							new DataOutputStream(client.getOutputStream());
    			if (cn == null){
    				_elog("Identification check not passed");
    				os.writeInt(ProtocolConstants.PACK_FAIL_HEAD);
    			}
    			else{
    				// TODO: remove this when do part 2
    				if (cn.getNumSyncer() == 1){
        				os.writeInt(ProtocolConstants.PACK_FULL_HEAD);
        				// close the connection then
        				client.close();
        				os.close();
    				}else{
    					os.writeInt(ProtocolConstants.PACK_CONFIRM_HEAD);
    					DropboxFileServerSyncer newSyncer =
    							new DropboxFileServerSyncer(client,
    														_server,
    														cn.getDir());
    					cn.addSyncer(newSyncer);
    					_elog("Start syncing");
    					newSyncer.start();
    				}
    			}
    		}
    	}catch (InterruptedIOException e){
    		if (!_server.noException()){
				_elog(e.toString());
			}
    		if (_server.debugMode()){
    			e.printStackTrace();
    		}
    		
    	}catch (IOException e){
    		if (!_server.noException()){
				_elog(e.toString());
			}
    		if (_server.debugMode()){
    			e.printStackTrace();
    		}
    	}finally{
    		stop();
    	}
		clear();
		_log(_threadName + " is stopped");
	}
}
