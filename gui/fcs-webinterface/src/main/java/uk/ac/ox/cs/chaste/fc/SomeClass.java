package uk.ac.ox.cs.chaste.fc;


public class SomeClass
{
	private String msg;
	
	public SomeClass ()
	{
		msg = "my message";
	}
	public SomeClass (String t)
	{
		msg = "my message: " + t;
	}
	
	public String getMsg ()
	{
		return msg;
	}
}
