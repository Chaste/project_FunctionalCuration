<%@ page language="java" contentType="text/html; charset=UTF-8" pageEncoding="UTF-8"%>
<%@taglib prefix="t" tagdir="/WEB-INF/tags" %>

<t:skeleton headerImports="${PageHeader}" notes="${Notifications}" user="${User}" title="${Title}MyAccount - " contextPath="${contextPath}">
    <h1>Your Account</h1>
    <table id="myaccounttable" class="leftright">
    	<tr>
    		<th>Mail Address</th>
    		<td>${User.mail}</td>
    	</tr>
    	<tr>
    		<th>Nick</th>
    		<td>${User.nick}</td>
    	</tr>
    	<tr>
    		<th>Institution</th>
    		<td>${User.institution}</td>
    	</tr>
    	<tr>
    		<th>Role</th>
    		<td>${User.role}</td>
    	</tr>
    	<tr>
    		<th>Registered</th>
    		<td><time>${User.created}</time></td>
    	</tr>
    </table>
    
    <h2>Change Your Password</h2>
    <p>
    	<label for="oldpassword">Old Password:</label><br/>
    	<input type="password" id="oldpassword" placeholder="old password"/>
    </p>
    <p>
    	<label for="newpassword1">New Password:</label><br/>
    	<input type="password" id="newpassword1" placeholder="new password"/>
    </p>
    <p>
    	<label for="newpassword2">Please repeat the new Password:</label><br/>
    	<input type="password" id="newpassword2" placeholder="repeat password"/>
    </p>
    <p>
    	<button id="pwchanger">Change Password</button>
    	 <span id="changeaction"></span>
    </p>
</t:skeleton>

