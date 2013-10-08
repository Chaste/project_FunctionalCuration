<%@ page language="java" contentType="text/html; charset=UTF-8" pageEncoding="UTF-8"%>
<%@ taglib prefix="t" tagdir="/WEB-INF/tags" %>
<%@ taglib prefix="c" uri="http://java.sun.com/jsp/jstl/core" %>

<t:skeleton headerImports="${PageHeader}" notes="${Notifications}" user="${User}" title="${Title}MyAccount - " contextPath="${contextPath}">
    <h1>Users</h1>
    <table id="accounttable" class="leftright">
    	<tr>
    		<th>Mail Address</th>
    		<th>Acronym</th>
    		<th>Institution</th>
    		<th>Role</th>
    		<th>Registered</th>
    	</tr>
		<c:forEach items="${Users}" var="cur" >
			<tr>
	    		<td>${cur.mail}</td>
	    		<td>${cur.nick}</td>
	    		<td>${cur.institution}</td>
	    		<td>
	    		<c:choose>
						<c:when test="${User.id == cur.id}">
							its you
						</c:when>
						<c:otherwise>
							<select class='role-chooser' id='user-role-${cur.id}'>
								<option <c:if test="${cur.role == 'GUEST'}">selected="selected"</c:if> value="GUEST">guest</option>
								<option <c:if test="${cur.role == 'MODELER'}">selected="selected"</c:if> value="MODELER">modeler</option>
								<option <c:if test="${cur.role == 'ADMIN'}">selected="selected"</c:if> value="ADMIN">admin</option>
							</select>
						</c:otherwise>
				</c:choose>
	    		</td>
	    		<td><small><time>${cur.created}</time></small></td>
    		</tr>
		</c:forEach>
    </table>
</t:skeleton>

