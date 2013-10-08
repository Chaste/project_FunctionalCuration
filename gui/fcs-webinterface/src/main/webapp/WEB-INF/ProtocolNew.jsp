<%@ page language="java" contentType="text/html; charset=UTF-8" pageEncoding="UTF-8"%>
<%@ taglib prefix="t" tagdir="/WEB-INF/tags" %>
<%@ taglib prefix="c" uri="http://java.sun.com/jsp/jstl/core" %>

<t:skeleton headerImports="${PageHeader}" notes="${Notifications}" user="${User}" title="${Title}CreateModel - " contextPath="${contextPath}">
	<div id='newprotocolform'>
	    <h1>Create Protocol</h1>
	    <p>
	    	<label for="protocolname">Name of the Protocol:</label>
	    	<br/>
			<c:choose>
				<c:when test="${not empty newprotocolname}">
					<strong>${newprotocolname}</strong>
					<input type="hidden" name="protocolname" id="protocolname" value="${newprotocolname}" placeholder="protocol name">
				</c:when>
				<c:otherwise>
			    	<input type="text" name="protocolname" id="protocolname" placeholder="protocol name">
			    	<span id="protocolaction"></span>
				</c:otherwise>
			</c:choose>
	    </p>
	    <p>
	    	<label for="versionname">Version:</label>
	    	<br/>
	    	<input type="text" name="versionname" id="versionname" placeholder="version identifier">
	    	<a class="pointer" id="dateinserter"><small>use current date</small></a>
	    	<span id="versionaction"></span>
	    </p>
	    <t:upload/>
	    <p>
	    	<button id="savebutton">Create protocol</button>
	    	<span id="saveaction"></span>
	    </p>
    </div>
</t:skeleton>

