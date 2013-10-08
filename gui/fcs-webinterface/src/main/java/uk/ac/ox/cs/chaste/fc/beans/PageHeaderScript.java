package uk.ac.ox.cs.chaste.fc.beans;


public class PageHeaderScript
{
	private String src;
	private String type;
	private String charset;
	private String content;
	
	public PageHeaderScript (String src, String type, String charset, String content)
	{
		this.src = src;
		this.type = type;
		this.charset = charset;
		this.content = content;
	}

	
	public String getSrc ()
	{
		return src;
	}

	
	public String getType ()
	{
		return type;
	}

	
	public String getCharset ()
	{
		return charset;
	}

	
	public String getContent ()
	{
		return content;
	}
}
