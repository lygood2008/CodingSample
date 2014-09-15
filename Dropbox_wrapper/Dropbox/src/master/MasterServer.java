/**
 * File: MasterServer.java
 * Author: Yan Li (yan_li@brown.edu)
 * Date: Apr 21 2014
 */

package master;

import common.*;

import java.util.*;


/**
 * 
 * class MasterServer
 * Description: The big server! It's mainly responsible
 * 				for handling dispatching client connection to
 *              a specified file server.
 */
final class MasterServer {

	private int _clientsPort;
	private int _clusterPort;
	private boolean _debug;
	private boolean _useUI;
	private boolean _hideException;
	private MasterServerNet _serverNet;
	private volatile LinkedList<ClientRecord> _clients;
	private volatile LinkedList<FileServerNode> _fsnodes;
	private Timer _timer;
	
	private void _dlog(String str){
		
		if (_debug)
			System.out.println("[MasterServer (DEBUG)]:" + str);
	}
	
	@SuppressWarnings("unused")
	private static void _elog(String str){
		System.err.println("[MasterServer (ERROR)]:" + str);
	}
	
	private static void _log(String str){
		System.out.println("[MasterServer]:" + str);
	}
	
	/**
	 * Constructor
	 * @param clientPort: the port for listening to clients
	 * @param clusterPort: the port for listening to file servers
	 * @param useUI: use UI or not? (not used for now)
	 * @param debug: debug or not?
	 * @param hideException: hide exceptions or not?
	 */
	public MasterServer(int clientPort,
						int clusterPort,
						boolean useUI,
						boolean debug,
						boolean hideException
						){
		
		_clientsPort = clientPort;
		_clusterPort = clusterPort;
		_useUI = useUI;
		_debug = debug;
		_hideException = hideException;
		_clients = new LinkedList<ClientRecord>();
		 
		_fsnodes = new LinkedList<FileServerNode>();
		initNet();
		initTimer();
		printStatus();
	}
	
	/**
	 * initNet: initialize the serverNet
	 */
	private void initNet(){
		_serverNet = new MasterServerNet(this);
	}
	
	/**
	 * initTimer: initialize the timer
	 */
	private void initTimer(){
		
		_timer = new Timer();
		_timer.scheduleAtFixedRate(new Echo(this),
				   DropboxConstants.ECHO_MASTER, DropboxConstants.ECHO_MASTER);
	}
	
	/**
	 * run: run the serverNet. it will start to listen
	 */
	public void run(){
		_serverNet.run();
	}
	
	public void printStatus(){
		
		_log("Master Server configuration:");
		_log("Debug:" + _debug);
		_log("UseUI:" + _useUI);
		_log("HideException:" + _hideException);
		_log("Client port:" + _clientsPort);
		_log("Cluster port:" + _clusterPort);
    	System.out.println();
	}
	
	public void usage(){
		
		_log("Master Server:");
		_log("-d: for debug mode (default false)" );
		_log("-u: to use user interface (default false)");
		_log("-cp: to specify the listen port for clients"
				+ " (default " + DropboxConstants.MASTER_CLIENT_PORT+")");
		_log("-lp: to specify the listen port for cluster"
				+ " (default " + DropboxConstants.MASTER_CLUSTER_PORT+")");
    	_log("-e: to hide non-runtime exceptions' reports");
		System.out.println();
	}
	
	/**
	 * Getters
	 */
	public int clientsPort(){
		return _clientsPort;
	}
	
	public int clusterPort(){
		return _clusterPort;
	}
	
	public boolean debugMode(){
		return _debug;
	}
	
	public boolean noException(){
		return _hideException;
	}
	
	public LinkedList<FileServerNode> getFS(){
		return _fsnodes;
	}
	
	/**
	 * findFileServer:find the file server from the given id
	 * @param id: the id number
	 * @return: the file servernode, or null if not there.
	 */
	public FileServerNode findFileServer(int id){
		
		for(FileServerNode fsn: _fsnodes){
			if (fsn.getID() == id){
				return fsn;
			}
		}
		return null;
	}
	
	/**
	 * insertFileServer: insert file server node into _fsnode list.
	 * @param fs: the file server node that needs to be added
	 */
	public synchronized void insertFileServer(FileServerNode fs){
		_fsnodes.add(fs);
	}
	
	/**
	 * removeFileServer: remove the file server based on its id
	 * @param id: the id of the file server
	 */
	public synchronized void removeFileServer(int id){

		int i = 0;
		for(FileServerNode fsn: _fsnodes){
			if (id == fsn.getID()){
				fsn.clear();
				break;
			}
			i++;
		}
		_fsnodes.remove(i);
	}
	
	public void printFileServers(){
		
		_log("There are " + _fsnodes.size() + " file server connected");
		int i = 0;
		for(FileServerNode fsn: _fsnodes){
			_log("Server " + i);
			_log("ID:" + fsn.getID());
			_log("IP:" + fsn.getIP());
			_log("MAX CLIENTS:" + fsn.getMaxClients());
			_log("PRIO:" + fsn.getPriority());
			_log("CONNECTED CLIENTS:"+fsn.getNumClients());
			System.out.println();
			i++;
		}
	}
	
	/**
	 * clientExist: check if the client exists
	 * @param name: the client name
	 * @return: true for exist, false for not there.
	 */
	public boolean clientExist(String name){
		
		for(ClientRecord cr: _clients){
			if (cr.getName().equals(name)){
				return true;
			}
		}
		return false;
	}
	
	/**
	 * findClient: find if the client is already there
	 * @param name: the client's name
	 * @param password: the client's password
	 * @return: the corresponding client record object
	 */
	public ClientRecord findClient(String name, String password){
		
		for(ClientRecord cr: _clients){
			if (cr.getName().equals(name) && cr.getPassword().equals(password)){
				return cr;
			}
		}
		return null;
	}
	
	/**
	 * addClient: add the client into _client list.
	 * 			  it will create a new client record from name, password and 
	 *            file server node
	 * @param name: the name of the client
	 * @param password: the password of the client
	 * @param fs: the file server node
	 * @return: true for success, false for failure (already there)
	 */
	public synchronized boolean addClient(String name, String password,
										  FileServerNode fs){
		
		boolean exist = clientExist(name);
		if (exist){
			return false;
		}
		else{
			ClientRecord cr = new ClientRecord(name, password, fs);
			_clients.add(cr);
			return true;
		}
	}
	
	/**
	 * removeClient: remove the client from the _client list based on name
	 * @param name: the client's name
	 * @return: true for success, false for failure
	 */
	public synchronized boolean removeClient(String name){
		
		boolean exist = clientExist(name);
		if (!exist){
			return false;
		}
		Iterator it = _clients.iterator();
		while (it.hasNext()){
			ClientRecord cr = (ClientRecord)it.next();
			if (cr.getName().equals(name)){
				it.remove();
				break;
			}
		}
		return true;
	}
	
	/**
	 * removeClient: remove the client from the _client list
	 * @param cr: the given client record
	 * @return: true for success, false for failure
	 */
	public synchronized boolean removeClient(ClientRecord cr){
		
		if (!_clients.contains(cr))
			return false;
		_clients.remove(cr);
		return true;
	}
	
	/**
	 * changePassword: search through all of the clients and change the password
	 *                 of that client based on name and pwd
	 * @param name: the name of the client
	 * @param pwd: the password of the client
	 * @return: true for success, false for failure
	 */
	public synchronized boolean changePassword(String name, String pwd){
		
		for(ClientRecord cr: _clients){
			if (cr.getName().equals(name)){
				cr.changePassword(pwd);
				return true;
			}
		}
		return false;
	}
	
	public synchronized void printClients(){
		
		_log("There are " + _clients.size() + " clients");
		int i = 0;
		for(ClientRecord cr: _clients){
			_log("client " + i + ": " + cr.getName() + " owner: file server " +
					cr.getOwner().getID() + " password:"+cr.getPassword());
		}
	}
	
	/**
	 * removeDead: remove the dead file server node
	 * @param fs: the file server node
	 */
	public synchronized void removeDead(FileServerNode fs){
		
		assert fs.isAlive() == false;
		
		_log("Remove fileserver " + fs.getID());
		/* Also remove associate clients */
		Iterator it = _clients.iterator();
		while (it.hasNext()){
			ClientRecord cr = (ClientRecord)it.next();
			if (cr.getOwner() == fs){
				it.remove();
			}
		}
		_fsnodes.remove(fs);
		printFileServers();
	}
	
	/**
	 * garbageCollection: periodically collects those dead nodes
	 */
	public synchronized void garbageCollection(){
		
		_dlog("Run garbage collection");
		/* Need to use iterator to loop */
		Iterator<FileServerNode> it = _fsnodes.iterator();
		while (it.hasNext()){
			FileServerNode node = it.next();
			if (!node.isAlive()){
				_log("Remove fileserver " + node.getID());
				it.remove();
			}
		}
	}
	
	/**
	 * 
	 * class Echo
	 * Description: Walk through the linked list and do garbage collection.
	 *              Also print out the status of the server
	 */
	private class Echo extends TimerTask{
		
		private MasterServer _server; // For future use
		
		private void _log(String str){
			System.out.println("[Echo]:"+str);
		}
		
		/**
		 * Constructor: set the server for future use
		 * @param server: the server object
		 */
		public Echo(MasterServer server){
			_server = server;
		}
		
		/**
		 * run: run the garbage collection here, and also print the status for
		 * 		debugging
		 */
		@Override
		public void run(){
			
			garbageCollection();
			printFileServers();
			printClients();
		}
	}
	
	public static void main(String[] args) {
		
		int clientPort = DropboxConstants.MASTER_CLIENT_PORT;
		int clusterPort = DropboxConstants.MASTER_CLUSTER_PORT;
		boolean useUI = false; // Not used now
		boolean debug = false;
		boolean hideException = false;
		
		for( int i = 0; i < args.length; i++ ){
    		if (args[i].equals("-d"))
				debug = true;
			else if (args[i].equals("-u"))
				useUI = true;
			else if (args[i].equals("-cp")){
				i++;
				clientPort = Integer.parseInt(args[i]);
			}
			else if (args[i].equals("-lp")){
				i++;
				clusterPort = Integer.parseInt(args[i]);
			}
			else if (args[i].equals("-e")){
				hideException = true;
			}
    	}
		
		MasterServer server = new MasterServer(clientPort,
											   clusterPort,
											   useUI,
											   debug,
											   hideException);
		server.run();
	}
}
