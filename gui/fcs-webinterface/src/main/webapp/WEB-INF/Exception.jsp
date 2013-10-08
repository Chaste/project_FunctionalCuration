<%@ page language="java" contentType="text/html; charset=UTF-8" pageEncoding="UTF-8"%>
<%@taglib prefix="t" tagdir="/WEB-INF/tags" %>
<%@ taglib prefix="c" uri="http://java.sun.com/jsp/jstl/core" %>

<t:skeleton headerImports="${PageHeader}" notes="${Notifications}" user="${User}" title="${Title}Error - " contextPath="${contextPath}">
    <h1>Error</h1>
    <p>
		<c:choose>
			<c:when test="${exceptionMessage}">
				${exceptionMessage}
			</c:when>
			<c:otherwise>
							Sorry, but we're not able to produce the requested page.
			</c:otherwise>
		</c:choose>
    </p>
</t:skeleton>

