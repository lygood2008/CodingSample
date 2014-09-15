/**
 * File: ClientRecord.java
 * Author: Yan Li (yan_li@brown.edu)
 * Date: Apr 21 2014
 */

package master;

/**
 * 
 * Class ClientRecord
 * Description: The container for each client in master side 
 */
final class ClientRecord {

	private String _name;
	private String _password;
	private volatile FileServerNode _serverNode;
	
	/**
	 * Constructor
	 * @param name: the client's name
	 * @param password: the client's password
	 * @param serverNode: the serverNode
	 */
	public ClientRecord(String name, String password, FileServerNode serverNode){
		
		_name = name;
		_password = password;
		_serverNode = serverNode;
	}
	
	/**
	 * Getters
	 */
	public String getName(){
		return _name;
	}
	
	public String getPassword(){
		return _password;
	}
	
	public synchronized FileServerNode getOwner(){
		return _serverNode;
	}
	
	/**
	 * isMatch: if the name and password matches the current record
	 * @param name: the name
	 * @param password: the password
	 * @return: true for match, false for not match
	 */
	public boolean isMatch(String name, String password){
		return _name.equals(name) && _password.equals(password);
	}
	
	/**
	 * changePassword: change the password to the given one
	 * @param password: the given new password
	 */
	public synchronized void changePassword(String password){
		_password = password;
	}
	
	/**
	 * clear: clear everything. reset to null
	 */
	public synchronized void clear(){
		_name = null;
		_password = null;
		_serverNode = null;
	}
}
