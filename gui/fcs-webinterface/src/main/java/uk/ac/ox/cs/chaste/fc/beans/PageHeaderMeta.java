package uk.ac.ox.cs.chaste.fc.beans;


public class PageHeaderMeta
{
	private String httpEquiv;
	private String content;
	
	public PageHeaderMeta (String httpEquiv, String content)
	{
		this.httpEquiv = httpEquiv;
		this.content = content;
	}

	
	public String getHttpEquiv ()
	{
		return httpEquiv;
	}

	
	public String getContent ()
	{
		return content;
	}
}
