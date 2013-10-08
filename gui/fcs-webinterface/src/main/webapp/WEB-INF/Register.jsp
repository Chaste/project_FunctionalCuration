<%@ page language="java" contentType="text/html; charset=UTF-8" pageEncoding="UTF-8"%>
<%@taglib prefix="t" tagdir="/WEB-INF/tags" %>

<t:skeleton headerImports="${PageHeader}" notes="${Notifications}" user="${User}" title="${Title}Register - " contextPath="${contextPath}">
    <h1>Register for Chaste</h1>
      <section id="registerform">
    	<p>
	    	<label for="mail">Mail Address <small>(we'll send you a password)</small></label>
	    	<br/>
	    	<input type="email" name="mail" id="mail" placeholder="your email address" /> <span id="mailaction"></span>
    	</p>
    	
    	<p>
	    	<label for="institution">Nick <small>(will be associated to your content)</small></label>
	    	<br/>
	    	<input type="text" name="nick" id="nick" placeholder="your alias goes here" /> <span id="nickaction"></span>
    	</p>
    	
    	<p>
	    	<label for="institution">Institution <small>(just for our records)</small></label>
	    	<br/>
	    	<input type="text" name="institution" id="institution" placeholder="your institution" />
    	</p>
    	
    	<p>
    		<button id="registersubmit">Join Chaste</button> <span id="submitaction"></span>
    	</p>
      </section>
</t:skeleton>


