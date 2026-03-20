select S.Name from Student S 
join Enrollment E 
on E.Student_ID=S.Student_ID 
join Course C 
on E.Course_ID=C.Course_ID
join Department D
on D.Department_ID=C.Department_ID
where D.Department_Name in ('Mathematics','Physics')
AND S.Student_ID NOT IN (
	select E1.Student_ID from Enrollment E1
	join Course C1
	on C1.Course_ID=E1.Course_ID
	join Department D1
	on D1.Department_ID=C1.Department_ID
	where D1.Department_Name="Computer Science"
) 
group by S.Student_ID 
having COUNT(DISTINCT D.Department_ID)=2;
