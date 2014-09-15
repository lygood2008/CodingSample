/**
 * File: FileServerNode.java
 * Author: Yan Li (yan_li@brown.edu)
 * Date: Apr 21 2014
 */

package master;

import java.net.*;
import java.io.*;
import java.util.*;
/**
 * 
 * class: FileServerNode
 * Description: Used for storing the connected file servers
 *              CAUTION: this class is not reusable
 */
final class FileServerNode {
	
	private Socket _userSocket;
	private Socket _socket;
	private String _ip;	
	private int _id;

	private int _prio;
	private int _maxClients;
	private boolean _alive;
	private MasterServerFileServerAccept _fsaNet;
	private MasterServerFileServerRequest _fsqNet;
	private LinkedList<ClientRecord> _clients;
	
	public FileServerNode(){
		
		_clients = new LinkedList<ClientRecord>();
		_alive = true;
	}
	
	/**
	 * Setters
	 */
	public synchronized void setRequestThread(MasterServerFileServerRequest fsqNet){ 
		assert _fsqNet == null;
		_fsqNet = fsqNet;
	}
	
	public synchronized void setAcceptThread(MasterServerFileServerAccept fsaNet){ 
		assert _fsaNet == null;
		_fsaNet = fsaNet;	
	}
	
	public synchronized void setSocket(Socket s){
		assert _socket == null;
		_socket = s;
	}
	
	public synchronized void setUserSocket(Socket s){
		assert _userSocket == null;
		_userSocket = s;
	}
	
	public synchronized void setID(int id){
		_id = id;
	}
	
	public synchronized void setIP(String ip){
		_ip = ip;
	}
	
	public synchronized void setPriority(int priority){ 
		_prio = priority;
	}
	
	public synchronized void setMaxClients(int maxClients){
		_maxClients = maxClients;
	}
	/**
	 * Getters
	 */
	public int getMaxClients(){
		return _maxClients;
	}
	
	public int getNumClients(){
		return _clients.size();
	}
	
	public int getID(){
		return _id;
	}
	
	public int getPriority(){
		return _prio;
	}
	
	public String getIP(){
		return _ip;
	}
	
	/**
	 * isAlive: is the file server alive?
	 */
	public boolean isAlive(){
		return _alive;
	}
	
	/**
	 * clearClients: clear the clients' records
	 */
	public synchronized void clearClients(){
		_clients.clear();
	}
	
	/**
	 * addEntry: add one client record into _clients
	 * @param cr: the new clientRecord
	 */
	public synchronized void addEntry(ClientRecord cr){	
		_clients.add(cr);
	}
	
	/**
	 * removeEntry: remove the specified entry
	 * @param cr: the client record that needs to be removed
	 */
	public synchronized void removeEntry(ClientRecord cr){
		_clients.remove(cr);
	}
	
	/**
	 * destroy: destroy myself
	 */
	public synchronized void destroy(){
		clear();
		_alive = false;
	}
	
	/**
	 * clear: clear the data in this node. reset to null
	 */
	public synchronized void clear(){
		
		if (_fsqNet != null &&!_fsqNet.terminated()){
			_fsqNet.stop();
		}
		if (_fsaNet != null &&!_fsaNet.terminated()){
			_fsaNet.stop();
		}
		try
		{
			if (_userSocket != null && !_userSocket.isClosed())
				_userSocket.close();
			if (_socket != null &&!_socket.isClosed())
				_socket.close();
		}catch (IOException e){
			e.printStackTrace();
		}
		_userSocket = null;
		_socket = null;
		_ip = null;
		clearClients();
		_clients = null;
		_fsqNet = null;
		_fsaNet = null;
	}
}
