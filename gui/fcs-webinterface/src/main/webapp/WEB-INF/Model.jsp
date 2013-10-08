<%@ page language="java" contentType="text/html; charset=UTF-8" pageEncoding="UTF-8"%>
<%@ taglib prefix="t" tagdir="/WEB-INF/tags" %>
<%@ taglib prefix="c" uri="http://java.sun.com/jsp/jstl/core" %>

<t:skeleton headerImports="${PageHeader}" notes="${Notifications}" user="${User}" title="${Title}ModelView - " contextPath="${contextPath}">
    <h1 id="modelname">${compModel.name}</h1>
	<div class="suppl"><small>Model created</small> <time>${compModel.created}</time> <small>by</small> <em>${compModel.author}</em></div>
	
    
    <div id="modeldetails">
    	<h2>Versions</h2>
   		<c:forEach items="${compModel.versions}" var="version" >
    		<p>
    			<strong><a class="modelversionlink" href="${contextPath}/model/${compModel.url}/${compModel.id}/${version.value.url}/${version.value.id}/">${version.value.version}</a></strong> by <em>${version.value.author}</em><br/>
    			<span class="suppl"><small>created </small> <time>${version.value.created}</time> <small>containing</small> ${version.value.numFiles} File<c:if test="${version.value.numFiles!=1}">s</c:if>.</span>
    		</p>
   		</c:forEach>
    </div>
    
    <div id="modelversion">
    	<div class="closebtn"><small><a id="modelversionclose">&otimes; close</a></small></div>
    	<h2 id="modelversionname"></h2>
	    <div class="suppl"><small>Version created</small> <time id="modelversiontime"></time> <small>by</small> <em id="modelversionauthor"></em>.</div>
	    
	    <div id="modelversiondetails">
		    <h3>Files attached to this model</h3>
		    <table id="modelversionfilestable">
			 </table>
		</div>
			 
		<div id="modelversionfiledetails">
    		<div class="closebtn"><small><a id="modelversionfileclose">&otimes; close</a></small></div>
	    	<h3 id="modelversionfilename"></h3>
		    <div class="suppl"><small>File created</small> <time id="modelversionfiletime"></time> <small>by</small> <em id="modelversionfileauthor"></em>.</div>
		</div>
	</div>
</t:skeleton>

