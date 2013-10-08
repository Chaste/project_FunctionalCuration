<%@ page language="java" contentType="text/html; charset=UTF-8" pageEncoding="UTF-8"%>
<%@ taglib prefix="t" tagdir="/WEB-INF/tags" %>
<%@ taglib prefix="c" uri="http://java.sun.com/jsp/jstl/core" %>

<t:skeleton headerImports="${PageHeader}" notes="${Notifications}" user="${User}" title="${Title}CreateModel - " contextPath="${contextPath}">
	<div id='newmodelform'>
	    <h1>Create Model</h1>
	    <p>
	    	<label for="modelname">Name of the model:</label>
	    	<br/>
			<c:choose>
				<c:when test="${not empty newmodelname}">
					<strong>${newmodelname}</strong>
					<input type="hidden" name="modelname" id="modelname" value="${newmodelname}" placeholder="model name">
				</c:when>
				<c:otherwise>
			    	<input type="text" name="modelname" id="modelname" placeholder="model name">
			    	<span id="modelaction"></span>
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
	    	<button id="savebutton">Create model</button>
	    	<span id="saveaction"></span>
	    </p>
    </div>
</t:skeleton>

