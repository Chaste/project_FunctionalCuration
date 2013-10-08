package uk.ac.ox.cs.chaste.fc.beans;

import java.util.Vector;


public class PageHeader
{
	private Vector<PageHeaderLink> links;
	private Vector<PageHeaderMeta> metas;
	private Vector<PageHeaderScript> scripts;
	
	public PageHeader ()
	{
		links = new Vector<PageHeaderLink> ();
		metas = new Vector<PageHeaderMeta> ();
		scripts = new Vector<PageHeaderScript> ();
	}

	
	public Vector<PageHeaderLink> getLinks ()
	{
		return links;
	}

	
	public void setLinks (PageHeaderLink links)
	{
		this.links.add (links);
	}

	
	public Vector<PageHeaderMeta> getMetas ()
	{
		return metas;
	}

	
	public void setMetas (PageHeaderMeta metas)
	{
		this.metas.add (metas);
	}

	
	public Vector<PageHeaderScript> getScripts ()
	{
		return scripts;
	}

	
	public void addScripts (PageHeaderScript scripts)
	{
		this.scripts.add (scripts);
	}
	
	
}
