select C.*,AVG(E.Grade) from Enrollment E 
join Course C 
on C.Course_ID=E.Course_ID 
group by C.Course_ID 
having AVG(E.Grade)=(
 select MAX(avgGrade) 
 from (
 	select AVG(Grade) as avgGrade from Enrollment 
 	group by Course_ID
 ) as sub
);
