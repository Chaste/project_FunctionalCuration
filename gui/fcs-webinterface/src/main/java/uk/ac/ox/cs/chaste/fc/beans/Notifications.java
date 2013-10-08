package uk.ac.ox.cs.chaste.fc.beans;

import java.util.Vector;


public class Notifications
{
	private Vector<String> errors;
	private Vector<String> infos;
	
	public Notifications ()
	{
		errors = new Vector<String> ();
		infos = new Vector<String> ();
	}
	
	public boolean isNotifications ()
	{
		return errors.size () > 0 || infos.size () > 0;
	}
	
	public void addError (String err)
	{
		errors.add (err);
	}
	
	public void addInfo (String info)
	{
		infos.add (info);
	}

	
	public Vector<String> getErrors ()
	{
		return errors;
	}

	
	public Vector<String> getInfos ()
	{
		return infos;
	}
	
	public boolean isError ()
	{
		return errors.size () > 0;
	}
	
	public boolean isInfo ()
	{
		return infos.size () > 0;
	}
}
