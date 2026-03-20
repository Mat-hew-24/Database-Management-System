select S.Name from Enrollment E 
left join Student S 
on S.Student_ID=E.Student_ID 
left join Course C 
on C.Course_ID=E.Course_ID 
left join Faculty F 
on C.Faculty_ID=F.Faculty_ID 
where F.Name="Dr. Albert Einstein" 
group by S.Student_ID 
having COUNT(DISTINCT C.Course_ID) = ( 
	select COUNT(*) from Course C1 
	left join Faculty F1 
	on F1.Faculty_ID=C1.Faculty_ID 
	where F1.Name="Dr. Albert Einstein"
);
