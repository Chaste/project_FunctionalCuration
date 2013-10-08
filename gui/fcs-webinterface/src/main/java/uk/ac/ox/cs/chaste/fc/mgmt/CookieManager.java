/**
 * 
 */
package uk.ac.ox.cs.chaste.fc.mgmt;

import java.util.HashMap;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;


/**
 * @author martin
 *
 */
public class CookieManager
{
	private HashMap<String, Cookie> cookies;
	private HttpServletResponse response;
	
	public CookieManager (HttpServletRequest request, HttpServletResponse response)
	{
		this.cookies = new HashMap<String, Cookie> ();
		Cookie[] cookies = request.getCookies ();
		this.response = response;
		if (cookies != null)
		for (Cookie c : cookies)
			this.cookies.put (c.getName (), c);
	}
	
	public Cookie getCookie (String name)
	{
		return cookies.get (name);
	}
	
	public void setCookie (Cookie c)
	{
		cookies.put (c.getName (), c);
		response.addCookie (c);
	}
	
	
}
