select C.* from Course C 
left join Enrollment E 
on E.Course_ID=C.Course_ID 
group by C.Course_ID 
having COUNT(E.Student_ID)>10 AND AVG(E.Grade) = ( 
	select MIN(avgGrade) from (
		select AVG(Grade) as avgGrade from Enrollment 
		group by Course_ID 
		having COUNT(Student_ID)>10
	) as SUB
);
