<%@ page language="java" contentType="text/html; charset=UTF-8" pageEncoding="UTF-8"%>
<%@taglib prefix="t" tagdir="/WEB-INF/tags" %>

<t:skeleton headerImports="${PageHeader}" notes="${Notifications}" user="${User}" title="${Title}DB - " contextPath="${contextPath}">
    <h1>DB</h1>
    
    <button id="modelchooser">models</button>
    <button id="protocolchooser">protocols</button>
    <button id="experimentchooser">experiments</button>
    
    <section id="modeldb">
	    <h2>Explore Models</h2>
	    <input type="text" id="modelfilter" placeholder="search for a model" /> <button>search</button>
    </section>
    
    <section id="protocoldb">
	    <h2>Explore Protocols</h2>
	    <input type="text" id="protocolfilter" placeholder="search for a protocol" /> <button>search</button>
    </section>
    
    <section id="experimentdb">
	    <h2>Explore Experiments</h2>
	    <input type="text" id="experimentfilter" placeholder="search for an experiment" /> <button>search</button>
    </section>
</t:skeleton>

