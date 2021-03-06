/**
 * File: DummyFile.java
 * Author: Yan Li (yan_li@brown.edu)
 * Date: Apr 21 2014
 */

package common;

import java.io.*;
/**
 * 
 * Class: DummyFile
 * Description: We encapsulate the File class here because we may
 * 				use a file that does not exist. We need to store manually
 * 			    whether the file is directory here 
 */
public class DummyFile {
	
	private boolean _isDir;
	private File _file;
	private long _lastModifiedTime;
	
	/**
	 * Default constructor
	 */
	public DummyFile(){
		
		_isDir = false;
		_file = null;
		_lastModifiedTime = 0;
	}
	
	/**
	 * Constructor
	 * @param isDir: is directory or not
	 * @param file: is file or not
	 */
	public DummyFile(boolean isDir, File file){
		
		_isDir = isDir;
		_file = file;
		_lastModifiedTime = 0;
	}
	
	/**
	 *  Setters
	 */
	public void setLastModifiedTime(long lastModifiedTime){
		_lastModifiedTime = lastModifiedTime;
	}
	
	/**
	 *  Getters
	 */
	public long getLastModifiedTime(){
		return _lastModifiedTime;
	}
	
	public File getFile(){
		return _file;
	}
	
	public boolean isDir(){
		return _isDir;
	}
	
	public boolean equals(Object o){
		
		DummyFile d = (DummyFile)o;
		if (d._isDir == _isDir &&
				d._lastModifiedTime == _lastModifiedTime &&
				d._file.getName().equals(_file.getName())){
			return true;
		}
		else
			return false;
	}
}
