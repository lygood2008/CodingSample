/**
 * File: MasterServerClientNet.java
 * Author: Yan Li (yan_li@brown.edu)
 * Date: Apr 21 2014
 */

package master;

import java.io.*;
import java.net.*;
import common.*;

/**
 * 
 * class: MasterServerClientNet
 * Description: Listen to clients connected and spawn a new thread to handle it
 */
class MasterServerClientNet extends GeneralServer{
	
	private MasterServer _server;
	
	private void _dlog(String str){
		
		if (_server.debugMode())
			System.out.println("[MasterServerClientsNet (DEBUG)]:" + str);
	}
	
	private static void _elog(String str){
		System.err.println("[MasterServerClientsNet (ERROR)]:" + str);
	}
	
	private static void _log(String str){
		System.out.println("[MasterServerClientsNet]:" + str);
	}
	
	/**
	 * Constructor:
	 * @param server: the server object
	 */
	public MasterServerClientNet(MasterServer server){
		
		super("MasterServerClientNet", server.noException(), server.debugMode());
		_server = server;
		assert _server != null;
	}
	
	/**
	 * run: the net basically listens to new connected 
	 * 	    client and spawns a new handler for that client
	 */
	@Override
	public void run(){
		
		_log("listening to new client...");
    	Socket client = null;
    	
    	try{
    		_serverSocket = new ServerSocket(_server.clientsPort());
    		if (_server.debugMode()){
    			_dlog("Server timeout after 100 seconds");
    			_serverSocket.setSoTimeout(1000*100);
    		}
    		
    		while(true)
    		{
    			if (_serverSocket == null || _serverSocket.isClosed())
    				break;
    			
    			synchronized(this){
    				while(_suspended){
    					wait();
    				}
    			}
    			
    			client = _serverSocket.accept();
    			_dlog("Get connection from " +
    				  client.getInetAddress().getHostAddress());
    			//TODO: here, it should control how many threads
    			//      we could accept at most
    			//TEST
    			//TODO: add the thread pool
    			// We spawn a thread to handle this connection
    			MasterServerClientHandler ms =
    							 new MasterServerClientHandler(client,_server);
    			ms.start();
    		}
    	}catch (InterruptedException e){
    		if (!_server.noException()){
				_elog(e.toString());
			}
			if (_server.debugMode()){
				e.printStackTrace();
			}
    	}
    	catch (InterruptedIOException e){
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
    	_server = null;
    	_log(_threadName + " is stopped");
	}
}
