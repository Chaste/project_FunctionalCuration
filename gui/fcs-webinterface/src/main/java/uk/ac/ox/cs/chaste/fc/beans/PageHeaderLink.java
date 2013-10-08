package uk.ac.ox.cs.chaste.fc.beans;


public class PageHeaderLink
{
	private String href;
	private String type;
	private String rel;
	
	public PageHeaderLink (String href, String type, String rel)
	{
		this.href= href;
		this.rel = rel;
		this.type = type;
	}

	
	public String getHref ()
	{
		return href;
	}

	
	public String getType ()
	{
		return type;
	}

	
	public String getRel ()
	{
		return rel;
	}
}
