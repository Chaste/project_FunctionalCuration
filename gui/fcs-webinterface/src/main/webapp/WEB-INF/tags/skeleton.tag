<!--                                                      
                                                      
    ___     / __      ___      ___    __  ___  ___    
  //   ) ) //   ) ) //   ) ) ((   ) )  / /   //___) ) 
 //       //   / / //   / /   \ \     / /   //        
((____   //   / / ((___( ( //   ) )  / /   ((____

     
-->
<%@ tag language="java" pageEncoding="UTF-8"%>
<%@ taglib prefix="c" uri="http://java.sun.com/jsp/jstl/core" %>
<%@ attribute name="contextPath" required="true"%>
<%@ attribute name="title" required="false"%>
<%@ attribute name="user" required="false" type="uk.ac.ox.cs.chaste.fc.beans.User"%>
<%@ attribute name="headerImports" required="true" type="uk.ac.ox.cs.chaste.fc.beans.PageHeader"%>
<%@ attribute name="notes" required="false" type="uk.ac.ox.cs.chaste.fc.beans.Notifications"%>

<!DOCTYPE html>
<html>
	<head>
		<title><c:if test="${not empty title}">${title}</c:if>FunctionalCuration @ Chaste</title>
		
		<%-- static header fields -> used on every page --%>
		<script type='text/javascript'>
			var contextPath = "${contextPath}";
		</script>
		<meta http-equiv='Content-Type' content='text/html;charset=utf-8' />
		<link rel='stylesheet' type='text/css' href='${contextPath}/res/css/style.css' />
		<script type='text/javascript' src='${contextPath}/res/js/main.js' charset='UTF-8'></script>
		<script type='text/javascript' src='${contextPath}/res/js/3rd/jquery-2.0.3.min.js' charset='UTF-8'></script>
		<script type='text/javascript' src='${contextPath}/res/js/3rd/colorbox/jquery.colorbox-min.js' charset='UTF-8'></script>
		<script type='text/javascript' src='${contextPath}/res/js/3rd/xdate.js' charset='UTF-8'></script>
		
		<%-- dynamic header fields -> just import them as needed --%>
		<c:if test="${headerImports != null}">
			<c:forEach items="${headerImports.metas}" var="himp" >
				<meta http-equiv='${himp.httpEquiv}' content='${himp.content}'/>
			</c:forEach>
			<c:forEach items="${headerImports.links}" var="himp" >
				<link rel='${himp.rel}' type='${himp.type}' href='${contextPath}/${himp.href}'/>
			</c:forEach>
			<c:forEach items="${headerImports.scripts}" var="himp" >
				<script type='${himp.type}' src='${contextPath}/${himp.src}' charset='${himp.charset}'>${himp.content}</script>
			</c:forEach>
		</c:if>
	</head>
	<body>
	  <div id="page">
	  	
	  	<%-- top of the page start --%>
		<header>
			<div id="logo">
				<a href="${contextPath}/index.html">
					<img alt="Chaste - FunctionalCuration" src="${contextPath}/res/img/chaste.jpg" />
				</a>
			</div>
			
			<nav>
				<ul class="navbar">
					<li><a href="${contextPath}/index.html">Home</a></li>
					<li><a href="${contextPath}/db.html">Kown Stuff</a></li>
					<li><a href="${contextPath}/contact.html">Contact/Team</a></li>
					<c:choose>
						<c:when test="${user != null && user.authorized}">
							<li><a href="${contextPath}/myfiles.html">${user.nick}</a>
								<ul>
									<li><a href="${contextPath}/myfiles.html">Files</a></li>
									<li><a href="${contextPath}/myaccount.html">Account</a></li>
									<c:if test="${user.role == 'ADMIN'}">
										<li><a href="${contextPath}/admin.html">Admin</a></li>
									</c:if>
									<li><a href="${contextPath}/logout.html">logout</a></li>
								</ul>
							</li>
						</c:when>
						<c:otherwise>
							<li><a href="${contextPath}/login.html">Login</a></li>
							<li><a href="${contextPath}/register.html">Register</a></li>
						</c:otherwise>
					</c:choose>
				</ul>
			</nav>
			
			<div id="breadcrumb">
			</div>
		</header>
	  	<%-- top of the page done --%>
		
		
		
		
		<%-- notification stuff start --%>
		<div id="notifications">
				
				
				
					<div id="error"  <c:if test="${notes == null || !notes.error}">class="invisible"</c:if>     >
						<h3>Following errors occurred:</h3>
						<ul id='errorlist'>
							<c:forEach items="${notes.errors}" var="err" >
								<li>${err}</li>
							</c:forEach>
						</ul>
						<a class="pointer" id="dismisserrors">dismiss</a>
					</div><%-- #error --%>
					
					
					
				
				
				
					<div id="info"  <c:if test="${notes == null || !notes.info}">class="invisible"</c:if>     >
						<h3>Note:</h3>
						<ul id='infolist'>
							<c:forEach items="${notes.infos}" var="info" >
								<li>${info}</li>
							</c:forEach>
						</ul>
						<a class="pointer" id="dismissnotes">dismiss</a>
					</div><%-- #info --%>
		</div><%-- #notifications --%>
		<%-- notification stuff done --%>
		
		
		
		
		<%-- here comes the action --%>
		<div id="body">
			<jsp:doBody/>
		</div>
		<%-- action done --%>
		
		
		
		<%-- finish the site --%>
		<footer>
			some footer &copy; 2013 Team Jonathan
		</footer>
		
		
	  </div><%-- #page --%>
	</body>
</html>