<%@ page language="java" contentType="text/html; charset=UTF-8" pageEncoding="UTF-8"%>
<%@taglib prefix="t" tagdir="/WEB-INF/tags" %>

<t:skeleton headerImports="${PageHeader}" notes="${Notifications}" user="${User}" title="${Title}" contextPath="${contextPath}">
    <h1>Welcome</h1>
    <p>
   		This is Chaste. At least the part which cares about <strong>Functional Curation</strong>.
    </p>
    <p>
    	You are encouraged to browse around. You need to sign up in order to upload own models or protocols and to simulate your experiments. But don't be afraid, that just takes a second. Or two.
    </p>
</t:skeleton>

