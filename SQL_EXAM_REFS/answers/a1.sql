select C.* from Course C
join Department D 
on D.Department_ID=C.Department_ID 
where D.Department_Name="Computer Science";
