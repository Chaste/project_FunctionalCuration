<%@ page language="java" contentType="text/html; charset=UTF-8" pageEncoding="UTF-8"%>
<%@ taglib prefix="t" tagdir="/WEB-INF/tags" %>
<%@ taglib prefix="c" uri="http://java.sun.com/jsp/jstl/core" %>

<t:skeleton headerImports="${PageHeader}" notes="${Notifications}" user="${User}" title="${Title}MyFiles - " contextPath="${contextPath}">
    <h1>Your Files</h1>
    
    <button id="modelchooser">models</button>
    <button id="protocolchooser">protocols</button>
    <button id="experimentchooser">experiments</button>
    
    <section id="modellist">
	    <h2>Your Models</h2>
	    <small><a href="${contextPath}/model/createnew" id="addmodellink" class="pointer">create a new model</a></small>
	    
    	<c:forEach items="${models}" var="model" >
    		<h3><a href="${contextPath}/model/${model.url}/${model.id}/">${model.name}</a></h3>
    		<ul>
	    		<c:forEach items="${model.versions}" var="version" >
		    		<li>
		    			<strong><a href="${contextPath}/model/${model.url}/${model.id}/${version.value.url}/${version.value.id}/">${version.value.version}</a></strong> by <em>${version.value.author}</em><br/>
		    			<span class="suppl"><small>created </small> <time>${version.value.created}</time> <small>containing</small> ${version.value.numFiles} File<c:if test="${version.value.numFiles!=1}">s</c:if>.</span>
		    		</li>
	    		</c:forEach>
	    		<li>
	    			<small><a href="${contextPath}/model/createnew/?newmodelname=${model.id}">add new version</a></small>
	    		</li>
    		</ul>
    	</c:forEach>
    </section>
    
    <section id="protocollist">
	    <h2>Your Protocols</h2>
	    <small><a href="${contextPath}/protocol/createnew" id="addprotocol" class="pointer">create a new protocol</a></small>
	    
    	<c:forEach items="${protocols}" var="protocol" >
    		<h3><a href="${contextPath}/protocol/${protocol.url}/${protocol.id}/">${protocol.name}</a></h3>
    		<ul>
	    		<c:forEach items="${protocol.versions}" var="version" >
		    		<li>
		    			<strong><a href="${contextPath}/protocol/${protocol.url}/${protocol.id}/${version.value.url}/${version.value.id}/">${version.value.version}</a></strong> by <em>${version.value.author}</em><br/>
		    			<span class="suppl"><small>created </small> <time>${version.value.created}</time> <small>containing</small> ${version.value.numFiles} File<c:if test="${version.value.numFiles!=1}">s</c:if>.</span>
		    		</li>
	    		</c:forEach>
	    		<li>
	    			<small><a href="${contextPath}/protocol/createnew/?newprotocolname=${protocol.id}">add new version</a></small>
	    		</li>
    		</ul>
    	</c:forEach>
    </section>
    
    <section id="experimentlist">
	    <h2>Your Experiments</h2>
	    <small><a id="addexperiment" class="pointer">create a new experiment</a></small>
    </section>
</t:skeleton>

