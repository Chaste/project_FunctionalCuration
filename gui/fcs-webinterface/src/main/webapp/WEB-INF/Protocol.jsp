<%@ page language="java" contentType="text/html; charset=UTF-8" pageEncoding="UTF-8"%>
<%@ taglib prefix="t" tagdir="/WEB-INF/tags" %>
<%@ taglib prefix="c" uri="http://java.sun.com/jsp/jstl/core" %>

<t:skeleton headerImports="${PageHeader}" notes="${Notifications}" user="${User}" title="${Title}ProtocolView - " contextPath="${contextPath}">
    <h1 id="protocolname">${chasteProtocol.name}</h1>
	<div class="suppl"><small>Protocol created</small> <time>${chasteProtocol.created}</time> <small>by</small> <em>${chasteProtocol.author}</em></div>
	
    
    <div id="protocoldetails">
    	<h2>Versions</h2>
   		<c:forEach items="${chasteProtocol.versions}" var="version" >
    		<p>
    			<strong><a class="protocolversionlink" href="${contextPath}/protocol/${chasteProtocol.url}/${chasteProtocol.id}/${version.value.url}/${version.value.id}/">${version.value.version}</a></strong> by <em>${version.value.author}</em><br/>
    			<span class="suppl"><small>created </small> <time>${version.value.created}</time> <small>containing</small> ${version.value.numFiles} File<c:if test="${version.value.numFiles!=1}">s</c:if>.</span>
    		</p>
   		</c:forEach>
    </div>
    
    <div id="protocolversion">
    	<div class="closebtn"><small><a id="protocolversionclose">&otimes; close</a></small></div>
    	<h2 id="protocolversionname"></h2>
	    <div class="suppl"><small>Version created</small> <time id="protocolversiontime"></time> <small>by</small> <em id="protocolversionauthor"></em>.</div>
	    
	    <div id="protocolversiondetails">
		    <h3>Files attached to this protocol</h3>
		    <table id="protocolversionfilestable">
			 </table>
		</div>
			 
		<div id="protocolversionfiledetails">
    		<div class="closebtn"><small><a id="protocolversionfileclose">&otimes; close</a></small></div>
	    	<h3 id="protocolversionfilename"></h3>
		    <div class="suppl"><small>File created</small> <time id="protocolversionfiletime"></time> <small>by</small> <em id="protocolversionfileauthor"></em>.</div>
		</div>
	</div>
</t:skeleton>

