select C.Course_Name as Course,S.Name as Student_Name 
from Enrollment E 
join Student S 
on S.Student_ID=E.Student_ID 
join Course C 
on C.Course_ID=E.Course_ID 
where E.Grade = (
	select MAX(Grade) from Enrollment 
	where Course_ID = E.Course_ID
);
