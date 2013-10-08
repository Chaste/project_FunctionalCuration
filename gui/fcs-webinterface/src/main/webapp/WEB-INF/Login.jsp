<%@ page language="java" contentType="text/html; charset=UTF-8" pageEncoding="UTF-8"%>
<%@taglib prefix="t" tagdir="/WEB-INF/tags" %>

<t:skeleton headerImports="${PageHeader}" notes="${Notifications}" user="${User}" title="${Title}Login - " contextPath="${contextPath}">
    <h1>Authentication</h1>
      <section id="loginform">
    	<p>
	    	<input type="email" name="mail" id="mail" placeholder="your email address" /> <code>martin@shadow.informatik.uni-rostock.de</code>
	    	<br/>
	    	<input type="password" name="password" id="password" placeholder="your password" /> <code>2t5944ja8f</code>
    	</p>
    	
    	<p>
    		<input type="checkbox" name="remember" id="remember"/>
    		<label for="remember">Remember me on this computer. <small>(you need to accept cookies)</small></label>
    	</p>
    	
    	<p>
    		<button id="loginsubmit">Check in</button> <span id="submitaction"></span>
    	</p>
      </section>
</t:skeleton>

